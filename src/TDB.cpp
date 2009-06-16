////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include "text.h"
#include "util.h"
#include "TDB.h"
#include "main.h"

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
//  |  |  +- TDB::load (Filter)
//  |  |  |    read all
//  |  |  |    apply filter
//  |  |  |    return subset
//  |  |  |
//  |  |  +- TDB::add (T)
//  |  |  |
//  |  |  +- TDB::update (T)
//  |  |  |
//  |  |  +- TDB::commit
//  |  |      write all
//  |  |
//  |  +- TDB::unlock
//  |       [unlock]
//  |       close
//  |
//  +- TDB::~TDB
//       [TDB::unlock]
//
TDB::TDB ()
: mLock (true)
, mAllOpenAndLocked (false)
, mId (1)
{
}

////////////////////////////////////////////////////////////////////////////////
TDB::~TDB ()
{
  if (mAllOpenAndLocked)
    unlock ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB::clear ()
{
  mLocations.clear ();
  mLock = true;

  if (mAllOpenAndLocked)
    unlock ();

  mAllOpenAndLocked = false;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::location (const std::string& path)
{
  if (access (expandPath (path).c_str (), F_OK))
    throw std::string ("Data location '") +
          path +
          "' does not exist, or is not readable and writable.";

  mLocations.push_back (Location (path));
}

////////////////////////////////////////////////////////////////////////////////
void TDB::lock (bool lockFile /* = true */)
{
  mLock = lockFile;

  mPending.clear ();
  mNew.clear ();
  mPending.clear ();

  foreach (location, mLocations)
  {
    location->pending   = openAndLock (location->path + "/pending.data");
    location->completed = openAndLock (location->path + "/completed.data");
  }

  mAllOpenAndLocked = true;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::unlock ()
{
  if (mAllOpenAndLocked)
  {
    mPending.clear ();
    mNew.clear ();
    mModified.clear ();

    foreach (location, mLocations)
    {
      fflush (location->pending);
      fclose (location->pending);
      location->pending = NULL;

      fflush (location->completed);
      fclose (location->completed);
      location->completed = NULL;
    }

    mAllOpenAndLocked = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB::load (std::vector <Task>& tasks, Filter& filter)
{
#ifdef FEATURE_TDB_OPT
  // Special optimization: if the filter contains Att ('status', '', 'pending'),
  // and no other 'status' filters, then loadCompleted can be skipped.
  int numberStatusClauses = 0;
  int numberSimpleStatusClauses = 0;
  foreach (att, filter)
  {
    if (att->name () == "status")
    {
      ++numberStatusClauses;

      if (att->mod () == "" && att->value () == "pending")
        ++numberSimpleStatusClauses;
    }
  }
#endif

  loadPending (tasks, filter);

#ifdef FEATURE_TDB_OPT
  if (numberStatusClauses == 0 ||
      numberStatusClauses != numberSimpleStatusClauses)
    loadCompleted (tasks, filter);
  else
    std::cout << "[1;31m# TDB::load optimization short circuit[0m" << std::endl;
#else
  loadCompleted (tasks, filter);
#endif

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB::loadPending (std::vector <Task>& tasks, Filter& filter)
{
  std::string file;
  int line_number;

  try
  {
    char line[T_LINE_MAX];
    foreach (location, mLocations)
    {
      line_number = 1;
      file = location->path + "/pending.data";

      fseek (location->pending, 0, SEEK_SET);
      while (fgets (line, T_LINE_MAX, location->pending))
      {
        int length = ::strlen (line);
        if (length > 1)
        {
          // TODO Add hidden attribute indicating source?
          Task task (line);
          task.id = mId++;

          mPending.push_back (task);
          if (filter.pass (task))
            tasks.push_back (task);
        }

        ++line_number;
      }
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
int TDB::loadCompleted (std::vector <Task>& tasks, Filter& filter)
{
  std::string file;
  int line_number;

  try
  {
    char line[T_LINE_MAX];
    foreach (location, mLocations)
    {
      // TODO If the filter contains Status:x where x is not deleted or
      //      completed, then this can be skipped.

      line_number = 1;
      file = location->path + "/completed.data";

      fseek (location->completed, 0, SEEK_SET);
      while (fgets (line, T_LINE_MAX, location->completed))
      {
        int length = ::strlen (line);
        if (length > 2)
        {
          // TODO Add hidden attribute indicating source?

          if (line[length - 1] == '\n')
            line[length - 1] = '\0';

          Task task (line);
          task.id = mId++;

          if (filter.pass (task))
            tasks.push_back (task);
        }

        ++line_number;
      }
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
// TODO Write to transaction log.
// Note: mLocations[0] is where all tasks are written.
void TDB::add (const Task& task)
{
  mNew.push_back (task);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
void TDB::update (const Task& task)
{
  mModified.push_back (task);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Writes all, including comments
// Interestingly, only the pending file gets written to.  The completed file is
// only modified by TDB::gc.
int TDB::commit ()
{
  int quantity = mNew.size () + mModified.size ();

  // This is an optimization.  If there are only new tasks, and none were
  // modified, simply seek to the end of pending and write.
  if (mNew.size () && ! mModified.size ())
  {
    fseek (mLocations[0].pending, 0, SEEK_END);
    foreach (task, mNew)
    {
      mPending.push_back (*task);
      fputs (task->composeF4 ().c_str (), mLocations[0].pending);
    }

    mNew.clear ();
    return quantity;
  }

  // The alternative is to potentially rewrite both files.
  else if (mNew.size () || mModified.size ())
  {
    foreach (task, mPending)
      foreach (mtask, mModified)
        if (task->id == mtask->id)
          *task = *mtask;

    mModified.clear ();

    foreach (task, mNew)
      mPending.push_back (*task);

    mNew.clear ();

    // Write out all pending.
    if (fseek (mLocations[0].pending, 0, SEEK_SET) == 0)
    {
      ftruncate (fileno (mLocations[0].pending), 0);
      foreach (task, mPending)
        fputs (task->composeF4 ().c_str (), mLocations[0].pending);
    }
  }

  return quantity;
}

////////////////////////////////////////////////////////////////////////////////
// TODO -> FF4
void TDB::upgrade ()
{
  // TODO Read all pending
  // TODO Write out all pending

  // TODO Read all completed
  // TODO Write out all completed

  throw std::string ("unimplemented TDB::upgrade");
}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
int TDB::gc ()
{
  int count = 0;

  // Set up a second TDB.
  Filter filter;
  TDB tdb;
  tdb.location (mLocations[0].path);
  tdb.lock ();

  std::vector <Task> pending;
  tdb.loadPending (pending, filter);

  std::vector <Task> completed;
  tdb.loadCompleted (completed, filter);

  // Now move completed and deleted tasks from the pending list to the
  // completed list.  Isn't garbage collection easy?
  std::vector <Task> still_pending;
  foreach (task, pending)
  {
    std::string st = task->get ("status");
    Task::status s = task->getStatus ();
    if (s == Task::completed ||
        s == Task::deleted)
    {
      completed.push_back (*task);
      ++count;
    }
    else
      still_pending.push_back (*task);
  }

  pending = still_pending;

  // No commit - all updates performed manually.
  if (count > 0)
  {
    if (fseek (tdb.mLocations[0].pending, 0, SEEK_SET) == 0)
    {
      ftruncate (fileno (tdb.mLocations[0].pending), 0);
      foreach (task, pending)
        fputs (task->composeF4 ().c_str (), tdb.mLocations[0].pending);
    }

    if (fseek (tdb.mLocations[0].completed, 0, SEEK_SET) == 0)
    {
      ftruncate (fileno (tdb.mLocations[0].completed), 0);
      foreach (task, completed)
        fputs (task->composeF4 ().c_str (), tdb.mLocations[0].completed);
    }
  }

  // Close files.
  tdb.unlock ();

  return count;
}

////////////////////////////////////////////////////////////////////////////////
FILE* TDB::openAndLock (const std::string& file)
{
  // TODO Need provision here for read-only locations.

  // Check for access.
  bool exists = access (file.c_str (), F_OK) ? false : true;
  if (exists)
    if (access (file.c_str (), R_OK | W_OK))
      throw std::string ("Task does not have the correct permissions for '") +
            file + "'.";

  // Open the file.
  FILE* in = fopen (file.c_str (), (exists ? "r+" : "w+"));
  if (!in)
    throw std::string ("Could not open '") + file + "'.";

  // Lock if desired.  Try three times before failing.
  int retry = 0;
  if (mLock)
    while (flock (fileno (in), LOCK_EX) && ++retry <= 3)
      delay (0.1);

  if (retry > 3)
    throw std::string ("Could not lock '") + file + "'.";

  return in;
}

////////////////////////////////////////////////////////////////////////////////
