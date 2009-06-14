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
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include "text.h"
#include "util.h"
#include "TDB2.h"
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
//  |  |  +- TDB::update (T, T')
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
TDB2::TDB2 ()
: mLock (true)
, mAllOpenAndLocked (false)
, mId (1)
{
}

////////////////////////////////////////////////////////////////////////////////
TDB2::~TDB2 ()
{
  if (mAllOpenAndLocked)
    unlock ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::clear ()
{
  mLocations.clear ();
  mLock = true;

  if (mAllOpenAndLocked)
    unlock ();

  mAllOpenAndLocked = false;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::location (const std::string& path)
{
  if (access (expandPath (path).c_str (), F_OK))
    throw std::string ("Data location '") +
          path +
          "' does not exist, or is not readable and writable.";

  mLocations.push_back (Location (path));
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::lock (bool lockFile /* = true */)
{
  mLock = lockFile;

  mPending.clear ();
//  mCompleted.clear ();
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
void TDB2::unlock ()
{
  if (mAllOpenAndLocked)
  {
    mPending.clear ();
//    mCompleted.clear ();
    mNew.clear ();
    mModified.clear ();

    foreach (location, mLocations)
    {
      fclose (location->pending);
      location->pending = NULL;
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
int TDB2::load (std::vector <Task>& tasks, Filter& filter)
{
  loadPending   (tasks, filter);
  loadCompleted (tasks, filter);

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB2::loadPending (std::vector <Task>& tasks, Filter& filter)
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
          line[length - 1] = '\0'; // Kill \n
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
    s << "  int " << file << " at line " << line_number;
    throw e + s.str ();
  }

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// Returns number of filtered tasks.
// Note: tasks.clear () is deliberately not called, to allow the combination of
//       multiple files.
int TDB2::loadCompleted (std::vector <Task>& tasks, Filter& filter)
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
        if (length > 1)
        {
          // TODO Add hidden attribute indicating source?
          line[length - 1] = '\0'; // Kill \n
          Task task (line);
          task.id = mId++;

//          mCompleted.push_back (task);
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
    s << "  int " << file << " at line " << line_number;
    throw e + s.str ();
  }

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
// Note: mLocations[0] is where all tasks are written.
void TDB2::add (Task& task)
{
  mNew.push_back (task);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
void TDB2::update (Task& before, Task& after)
{
  mModified.push_back (after);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Writes all, including comments
// Interestingly, only the pending file gets written to.  The completed file is
// only modified by TDB2::gc.
int TDB2::commit ()
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
    fseek (mLocations[0].pending, 0, SEEK_SET);
    // TODO Do I need to truncate the file?  Does file I/O even work that way 
    //      any more?  I forget.
    foreach (task, mPending)
      fputs (task->composeF4 ().c_str (), mLocations[0].pending);
  }

  return quantity;
}

////////////////////////////////////////////////////////////////////////////////
// TODO -> FF4
void TDB2::upgrade ()
{
  // TODO Read all pending
  // TODO Write out all pending

  // TODO Read all completed
  // TODO Write out all completed

  throw std::string ("unimplemented TDB2::upgrade");
}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
int TDB2::gc ()
{
  int count = 0;
/*
  // Read everything from the pending file.
  std::vector <T> all;
  allPendingT (all);

  // A list of the truly pending tasks.
  std::vector <T> pending;

  std::vector<T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    // Some tasks stay in the pending file.
    if (it->getStatus () == T::pending ||
        it->getStatus () == T::recurring)
    {
      pending.push_back (*it);
    }

    // Others are transferred to the completed file.
    else
    {
      writeCompleted (*it);
      ++count;
    }
  }

  // Dump all clean tasks into pending.  But don't bother unless at least one
  // task was transferred.
  if (count)
    overwritePending (pending);
*/
  return count;
}

////////////////////////////////////////////////////////////////////////////////
FILE* TDB2::openAndLock (const std::string& file)
{
  // Check for access.
  if (access (file.c_str (), F_OK | R_OK | W_OK))
    throw std::string ("Task does not have the correct permissions for '") +
          file + "'.";

  // Open the file.
  FILE* in = fopen (file.c_str (), "r+");
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
