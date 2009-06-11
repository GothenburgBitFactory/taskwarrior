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
{
}

////////////////////////////////////////////////////////////////////////////////
TDB2::TDB2 (const TDB2& other)
{
  throw std::string ("unimplemented TDB2::TDB2");
//  mLocations        = other.mLocations;
//  mFiles            = other.mFiles;
//  mLock             = other.mLock;
//  mAllOpenAndLocked = false;  // Deliberately so.
//
//  // Set all to NULL, otherwise we are duplicating open file handles.
//  foreach (file, mFiles)
//    mFiles[file->first] = NULL;
}

////////////////////////////////////////////////////////////////////////////////
TDB2& TDB2::operator= (const TDB2& other)
{
  throw std::string ("unimplemented TDB2::operator=");
//  if (this != &other)
//  {
//    mLocations        = other.mLocations;
//    mFiles            = other.mFiles;
//    mLock             = other.mLock;
//    mAllOpenAndLocked = false;  // Deliberately so.
//
//    // Set all to NULL, otherwise we are duplicating open file handles.
//    foreach (file, mFiles)
//      mFiles[file->first] = NULL;
//  }
//
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
TDB2::~TDB2 ()
{
  if (mAllOpenAndLocked)
    unlock ();
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
      std::cout << "# location.path: " << location->path << std::endl;

      line_number = 1;
      file = location->path + "/pending.data";
      while (fgets (line, T_LINE_MAX, location->pending))
      {
        int length = ::strlen (line);
        if (length > 1)
        {
          // TODO Add hidden attribute indicating source?
          line[length - 1] = '\0'; // Kill \n
          Task task (line);
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
      std::cout << "# location.path: " << location->path << std::endl;

      // TODO If the filter contains Status:x where x is not deleted or
      //      completed, then this can be skipped.

      line_number = 1;
      file = location->path + "/completed.data";
      while (fgets (line, T_LINE_MAX, location->completed))
      {
        int length = ::strlen (line);
        if (length > 1)
        {
          // TODO Add hidden attribute indicating source?
          line[length - 1] = '\0'; // Kill \n
          Task task (line);
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
void TDB2::add (Task& after)
{
  throw std::string ("unimplemented TDB2::add");

  // TODO Seek to end of pending.
  // TODO write after.composeFF4 ().
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
void TDB2::update (Task& before, Task& after)
{
  throw std::string ("unimplemented TDB2::update");
}

////////////////////////////////////////////////////////////////////////////////
// TODO writes all, including comments
int TDB2::commit ()
{
  // TODO Two passes: first the pending file.
  //                  then the compelted file.

  throw std::string ("unimplemented TDB2::commit");
}

////////////////////////////////////////////////////////////////////////////////
// TODO -> FF4
void TDB2::upgrade ()
{
  throw std::string ("unimplemented TDB2::upgrade");
}

////////////////////////////////////////////////////////////////////////////////
FILE* TDB2::openAndLock (const std::string& file)
{
  // Check for access.
  if (access (file.c_str (), F_OK | R_OK | W_OK))
    throw std::string ("Task does not have the correct permissions for '") +
          file + "'.";

  // Open the file.
  FILE* in = fopen (file.c_str (), "rw");
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
