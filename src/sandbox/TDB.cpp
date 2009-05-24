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

#include <string>
#include <sys/file.h>
#include "text.h"
#include "util.h"
#include "TDB.h"
#include "task.h"

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
TDB::TDB ()
: mLock (true)
, mAllOpenAndLocked (false)
{
}

////////////////////////////////////////////////////////////////////////////////
TDB::TDB (const TDB& other)
{
  throw std::string ("unimplemented TDB::TDB");
  mLocations        = other.mLocations;
  mLock             = other.mLock;
  mAllOpenAndLocked = false;  // Deliberately so.

  // Set all to NULL.
  foreach (location, mLocations)
    mLocations[location->first] = NULL;
}

////////////////////////////////////////////////////////////////////////////////
TDB& TDB::operator= (const TDB& other)
{
  throw std::string ("unimplemented TDB::operator=");
  if (this != &other)
  {
    mLocations        = other.mLocations;
    mLock             = other.mLock;
    mAllOpenAndLocked = false;  // Deliberately so.

    // Set all to NULL.
    foreach (location, mLocations)
      mLocations[location->first] = NULL;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
TDB::~TDB ()
{
  if (mAllOpenAndLocked)
    unlock ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB::location (const std::string& path)
{
  if (access (expandPath (path).c_str (), F_OK))
    throw std::string ("Data location '") +
          path +
          "' does not exist, or is not readable and writable.";

  mLocations[path] = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::lock (bool lockFile /* = true */)
{
  mLock = lockFile;

  foreach (location, mLocations)
    mLocations[location->first] = openAndLock (location->first);

  mAllOpenAndLocked = true;
}

////////////////////////////////////////////////////////////////////////////////
void TDB::unlock ()
{
  foreach (location, mLocations)
  {
    if (mLocations[location->first] != NULL)
    {
      fclose (mLocations[location->first]);
      mLocations[location->first] = NULL;
    }
  }

  mAllOpenAndLocked = false;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Returns number of filtered tasks.
int TDB::load (std::vector <T>& tasks, Filter& filter)
{
  char line[T_LINE_MAX];
  foreach (location, mLocations)
  {
    while (fgets (line, T_LINE_MAX, location->second))
    {
      int length = ::strlen (line);
      if (length > 1)
      {
        line[length - 1] = '\0'; // Kill \n

        T task (line);

        if (filter.pass (task))
        {
          // TODO Add hidden attribute indicating source.
          tasks.push_back (task);
        }
      }
    }
  }

  return tasks.size ();
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
void TDB::add (T& after)
{
  throw std::string ("unimplemented TDB::add");

  // TODO Seek to end of pending.
  // TODO write after.composeFF4 ().
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
void TDB::update (T& before, T& after)
{
  throw std::string ("unimplemented TDB::update");
}

////////////////////////////////////////////////////////////////////////////////
// TODO writes all, including comments
int TDB::commit ()
{
  throw std::string ("unimplemented TDB::commit");
}

////////////////////////////////////////////////////////////////////////////////
// TODO -> FF4
void TDB::upgrade ()
{
  throw std::string ("unimplemented TDB::upgrade");
}

////////////////////////////////////////////////////////////////////////////////
void TDB::getPendingFiles (std::vector <std::string> files)
{
  files.clear ();

  foreach (location, mLocations)
    files.push_back (location->first + "/pending.data");
}

////////////////////////////////////////////////////////////////////////////////
void TDB::getCompletedFiles (std::vector <std::string> files)
{
  files.clear ();

  foreach (location, mLocations)
    files.push_back (location->first + "/completed.data");
}

////////////////////////////////////////////////////////////////////////////////
void TDB::getContactFiles (std::vector <std::string> files)
{
  files.clear ();

  foreach (location, mLocations)
    files.push_back (location->first + "/contact.data");
}

////////////////////////////////////////////////////////////////////////////////
void TDB::getUndoStack (std::string& file)
{
  throw std::string ("unimplemented TDB::getUndoStack");
}

////////////////////////////////////////////////////////////////////////////////
FILE* TDB::openAndLock (const std::string& file)
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
