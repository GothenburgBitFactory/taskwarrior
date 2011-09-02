////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>
#include <list>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include <stdlib.h>
#include <text.h>
#include <util.h>
#include <TDB.h>
#include <Directory.h>
#include <File.h>
#include <ViewText.h>
#include <Color.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
//  The ctor/dtor do nothing.
//  The lock/unlock methods hold the file open.
//  There should be only one commit.
//
//  +- TDB::TDB
//  |
//  |  +- TDB::lock
//  |  |    open
//  |  |    [lock]
//  |  |
//  |  |  +- TDB::load
//  |  |  |    read all
//  |  |  |    return subset
//  |  |  |
//  |  |  +- TDB::add (T)
//  |  |  |
//  |  |  +- TDB::update (T)
//  |  |  |
//  |  |  +- TDB::commit
//  |  |  |   write all
//  |  |  |
//  |  |  +- TDB::undo
//  |  |
//  |  +- TDB::unlock
//  |       [unlock]
//  |       close
//  |
//  +- TDB::~TDB
//       [TDB::unlock]
//
TDB::TDB ()
: _lock (true)
, _all_opened_and_locked (false)
, _id (1)
{
}

////////////////////////////////////////////////////////////////////////////////
TDB::~TDB ()
{
  if (_all_opened_and_locked)
    unlock ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB::clear ()
{
  _locations.clear ();
  _lock = true;

  if (_all_opened_and_locked)
    unlock ();

  _all_opened_and_locked = false;
  _id = 1;
  _pending.clear ();
  _new.clear ();
  _completed.clear ();
  _modified.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB::location (const std::string& path)
{
  Directory d (path);
  if (!d.exists ())
    throw std::string ("Data location '") +
          path +
          "' does not exist, or is not readable and writable.";

  _locations.push_back (Location (d));
}

////////////////////////////////////////////////////////////////////////////////
void TDB::lock (bool lockFile /* = true */)
{
  _lock = lockFile;

  _pending.clear ();
  _new.clear ();
  _completed.clear ();
  _modified.clear ();

  foreach (location, _locations)
  {
    location->_pending   = openAndLock (location->_path + "/pending.data");
    location->_completed = openAndLock (location->_path + "/completed.data");
    location->_undo      = openAndLock (location->_path + "/undo.data");
  }

  _all_opened_and_locked = true;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::unlock ()
{
  // Do not clear out these items, as they may be used in a read-only fashion.
  // _pending.clear ();
  // _new.clear ();
  // _modified.clear ();

  foreach (location, _locations)
  {
    fflush (location->_pending);
    fclose (location->_pending);
    location->_pending = NULL;

    fflush (location->_completed);
    fclose (location->_completed);
    location->_completed = NULL;

    fflush (location->_undo);
    fclose (location->_undo);
    location->_completed = NULL;
  }

  _all_opened_and_locked = false;
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB::load (std::vector <Task>& tasks)
{
  // Special optimization: if the filter contains Att ('status', '', 'pending'),
  // and no other 'status' filters, then loadCompleted can be skipped.
  int numberStatusClauses = 0;
  int numberSimpleStatusClauses = 0;

  loadPending (tasks);

  if (numberStatusClauses == 0 ||
      numberStatusClauses != numberSimpleStatusClauses)
    loadCompleted (tasks);
  else
    context.debug ("load optimization short circuit");

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB::loadPending (std::vector <Task>& tasks)
{
  std::string file;
  int line_number = 1;

  try
  {
    // Only load if not already loaded.
    if (_pending.size () == 0)
    {
      _id = 1;
      char line[T_LINE_MAX];
      foreach (location, _locations)
      {
        line_number = 1;
        file = location->_path + "/pending.data";

        fseek (location->_pending, 0, SEEK_SET);
        while (fgets (line, T_LINE_MAX, location->_pending))
        {
          int length = strlen (line);
          if (length > 3) // []\n
          {
            // TODO Add hidden attribute indicating source?
            Task task (line);
            task.id = _id++;

            _pending.push_back (task);

            // Maintain mapping for ease of link/dependency resolution.
            // Note that this mapping is not restricted by the filter, and is
            // therefore a complete set.
            _I2U[task.id] = task.get ("uuid");
            _U2I[task.get ("uuid")] = task.id;
          }

          ++line_number;
        }
      }
    }

    // Now filter and return.
    foreach (task, _pending)
      tasks.push_back (*task);

    // Hand back any accumulated additions, if TDB::loadPending is being called
    // repeatedly.
    int fakeId = _id;
    foreach (task, _new)
    {
      task->id = fakeId++;
      tasks.push_back (*task);
    }
  }

  catch (std::string& e)
  {
    std::stringstream s;
    s << " in " << file << " at line " << line_number;
    throw e + s.str ();
  }

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB::loadCompleted (std::vector <Task>& tasks)
{
  std::string file;
  int line_number = 1;

  try
  {
    if (_completed.size () == 0)
    {
      char line[T_LINE_MAX];
      foreach (location, _locations)
      {
        line_number = 1;
        file = location->_path + "/completed.data";

        fseek (location->_completed, 0, SEEK_SET);
        while (fgets (line, T_LINE_MAX, location->_completed))
        {
          int length = strlen (line);
          if (length > 3) // []\n
          {
            // TODO Add hidden attribute indicating source?

            Task task (line);
            task.id = 0;  // Need a value, just not a valid value.

            _completed.push_back (task);
          }

          ++line_number;
        }
      }
    }

    // Now filter and return.
    foreach (task, _completed)
      tasks.push_back (*task);
  }

  catch (std::string& e)
  {
    std::stringstream s;
    s << " in " << file << " at line " << line_number;
    throw e + s.str ();
  }

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TDB::getAllPending ()
{
  return _pending;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TDB::getAllNew ()
{
  return _new;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TDB::getAllCompleted ()
{
  return _completed;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TDB::getAllModified ()
{
  return _modified;
}

////////////////////////////////////////////////////////////////////////////////
// Note: _locations[0] is where all tasks are written.
void TDB::add (const Task& task)
{
  std::string unique;
  Task t (task);
  if (task.get ("uuid") == "")
    unique = ::uuid ();
  else
    unique = task.get ("uuid");

  t.set ("uuid", unique);

  // If the tasks are loaded, then verify that this uuid is not already in
  // the file.
  if (uuidAlreadyUsed (unique, _new)      ||
      uuidAlreadyUsed (unique, _modified) ||
      uuidAlreadyUsed (unique, _pending)  ||
      uuidAlreadyUsed (unique, _completed))
    throw std::string ("Cannot add task because the uuid '") + unique + "' is not unique.";

  _new.push_back (t);
  _I2U[task.id] = unique;
  _U2I[task.get ("uuid")] = t.id;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::update (const Task& task)
{
  _modified.push_back (task);
}

////////////////////////////////////////////////////////////////////////////////
// Interestingly, only the pending file gets written to.  The completed file is
// only modified by TDB::gc.
int TDB::commit ()
{
  context.timer_gc.start ();

  int quantity = _new.size () + _modified.size ();

  // This is an optimization.  If there are only new tasks, and none were
  // modified, simply seek to the end of pending and write.
  if (_new.size () && ! _modified.size ())
  {
    fseek (_locations[0]._pending, 0, SEEK_END);
    foreach (task, _new)
    {
      _pending.push_back (*task);
      fputs (task->composeF4 ().c_str (), _locations[0]._pending);
    }

    fseek (_locations[0]._undo, 0, SEEK_END);
    foreach (task, _new)
      writeUndo (*task, _locations[0]._undo);

    _new.clear ();
    return quantity;
  }

  // The alternative is to potentially rewrite both files.
  else if (_new.size () || _modified.size ())
  {
    // allPending is a copy of _pending, with all modifications included, and
    // new tasks appended.
    std::vector <Task> allPending;
    allPending = _pending;
    foreach (mtask, _modified)
    {
      foreach (task, allPending)
      {
        if (task->id == mtask->id)
        {
          *task = *mtask;
          goto next_mod;
        }
      }

    next_mod:
      ;
    }

    foreach (task, _new)
      allPending.push_back (*task);

    // Write out all pending.
    if (fseek (_locations[0]._pending, 0, SEEK_SET) == 0)
    {
      if (ftruncate (fileno (_locations[0]._pending), 0))
        throw std::string ("Failed to truncate pending.data file ");

      foreach (task, allPending)
        fputs (task->composeF4 ().c_str (), _locations[0]._pending);
    }

    // Update the undo log.
    if (fseek (_locations[0]._undo, 0, SEEK_END) == 0)
    {
      foreach (mtask, _modified)
      {
        foreach (task, _pending)
        {
          if (task->id == mtask->id)
          {
            writeUndo (*task, *mtask, _locations[0]._undo);
            goto next_mod2;
          }
        }

      next_mod2:
        ;
      }

      foreach (task, _new)
        writeUndo (*task, _locations[0]._undo);
    }

    _pending = allPending;

    _modified.clear ();
    _new.clear ();
  }

  context.timer_gc.stop ();
  return quantity;
}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
// Now reverts expired waiting tasks to pending.
// Now cleans up dangling dependencies.
int TDB::gc ()
{
  context.timer_gc.start ();

  // Allowed as a temporary override.
  if (!context.config.getBoolean ("gc"))
  {
    context.timer_gc.stop ();
    return 0;
  }

  int count_pending_changes = 0;
  int count_completed_changes = 0;
  Date now;

  if (_new.size ())
    throw std::string ("Unexpected new tasks found during gc.");

  if (_modified.size ())
    throw std::string ("Unexpected modified tasks found during gc.");

  lock ();

  std::vector <Task> ignore;
  loadPending (ignore);

  // Search for dangling dependencies.  These are dependencies whose uuid cannot
  // be converted to an id by TDB.
  std::vector <std::string> deps;
  foreach (task, _pending)
  {
    if (task->has ("depends"))
    {
      deps.clear ();
      task->getDependencies (deps);
      foreach (dep, deps)
      {
        if (id (*dep) == 0)
        {
          task->removeDependency (*dep);
          ++count_pending_changes;
          context.debug ("GC: Removed dangling dependency "
                          + task->get ("uuid")
                          + " -> "
                          + *dep);
        }
      }
    }
  }

  // Now move completed and deleted tasks from the pending list to the
  // completed list.  Isn't garbage collection easy?
  std::vector <Task> still_pending;
  std::vector <Task> newly_completed;
  foreach (task, _pending)
  {
    Task::status s = task->getStatus ();
    if (s == Task::completed ||
        s == Task::deleted)
    {
      newly_completed.push_back (*task);
      ++count_pending_changes;    // removal
      ++count_completed_changes;  // addition
    }
    else if (s == Task::waiting)
    {
      // Wake up tasks that need to be woken.
      Date wait_date (task->get_date ("wait"));
      if (now > wait_date)
      {
        task->setStatus (Task::pending);
        task->remove ("wait");
        ++count_pending_changes;  // modification

        context.debug (std::string ("TDB::gc waiting -> pending for ") +
                       task->get ("uuid"));
      }

      still_pending.push_back (*task);
    }
    else
      still_pending.push_back (*task);
  }

  // No commit - all updates performed manually.
  if (count_pending_changes > 0)
  {
    if (fseek (_locations[0]._pending, 0, SEEK_SET) == 0)
    {
      if (ftruncate (fileno (_locations[0]._pending), 0))
        throw std::string ("Failed to truncate pending.data file ");

      foreach (task, still_pending)
        fputs (task->composeF4 ().c_str (), _locations[0]._pending);

      // Update cached copy.
      _pending = still_pending;
    }
  }

  // Append the new_completed tasks to completed.data.  No need to write out the
  // whole list.
  if (count_completed_changes > 0)
  {
    fseek (_locations[0]._completed, 0, SEEK_END);
    foreach (task, newly_completed)
      fputs (task->composeF4 ().c_str (), _locations[0]._completed);
  }

  // Close files.
  unlock ();

  std::stringstream s;
  s << "gc " << (count_pending_changes + count_completed_changes) << " tasks";
  context.debug (s.str ());

  context.timer_gc.stop ();
  return count_pending_changes + count_completed_changes;
}

////////////////////////////////////////////////////////////////////////////////
int TDB::nextId ()
{
  return _id++;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::undo ()
{
}

////////////////////////////////////////////////////////////////////////////////
void TDB::merge (const std::string& mergeFile)
{
}

////////////////////////////////////////////////////////////////////////////////
std::string TDB::uuid (int id) const
{
  std::map <int, std::string>::const_iterator i;
  if ((i = _I2U.find (id)) != _I2U.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int TDB::id (const std::string& uuid) const
{
  std::map <std::string, int>::const_iterator i;
  if ((i = _U2I.find (uuid)) != _U2I.end ())
    return i->second;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
FILE* TDB::openAndLock (const std::string& file)
{
  // TODO Need provision here for read-only locations.

  // Check for access.
  File f (file);
  bool exists = f.exists ();
  if (exists)
    if (!f.readable () || !f.writable ())
      throw std::string ("Task does not have the correct permissions for '") +
            file + "'.";

  // Open the file.
  FILE* in = fopen (file.c_str (), (exists ? "r+" : "w+"));
  if (!in)
    throw std::string ("Could not open '") + file + "'.";

  // Lock if desired.  Try three times before failing.
  if (_lock)
  {
    int retry = 0;
    while (flock (fileno (in), LOCK_NB | LOCK_EX) && ++retry <= 3)
    {
      std::cout << "Waiting for file lock...\n";
      while (flock (fileno (in), LOCK_NB | LOCK_EX) && ++retry <= 3)
        delay (0.2);
    }

    if (retry > 3)
      throw std::string ("Could not lock '") + file + "'.";
  }

  return in;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::writeUndo (const Task& after, FILE* file)
{
  fprintf (file,
           "time %u\nnew %s---\n",
           (unsigned int) time (NULL),
           after.composeF4 ().c_str ());
}

////////////////////////////////////////////////////////////////////////////////
void TDB::writeUndo (const Task& before, const Task& after, FILE* file)
{
  fprintf (file,
           "time %u\nold %snew %s---\n",
           (unsigned int) time (NULL),
           before.composeF4 ().c_str (),
           after.composeF4 ().c_str ());
}

////////////////////////////////////////////////////////////////////////////////
bool TDB::uuidAlreadyUsed (
  const std::string& uuid,
  const std::vector <Task>& all)
{
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    if (it->get ("uuid") == uuid)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
