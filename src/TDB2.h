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
#ifndef INCLUDED_TDB2
#define INCLUDED_TDB2

#include <map>
#include <vector>
#include <string>
#include <Location.h>
#include <Filter.h>
#include <Task.h>

// Length of longest line.
#define T_LINE_MAX 32768

class TDB2
{
public:
  TDB2 ();  // Default constructor
  ~TDB2 (); // Destructor

  void  clear ();
  void  location (const std::string&);

  void  lock (bool lockFile = true);
  void  unlock ();

  int   load (std::vector <Task>&, Filter&);
  int   loadPending (std::vector <Task>&, Filter&);
  int   loadCompleted (std::vector <Task>&, Filter&);

  void  add (Task&);             // Single task add to pending
  void  update (Task&, Task&);   // Single task update to pending
  int   commit ();               // Write out all tasks
  void  upgrade ();              // Convert both files to FF4
  int   gc ();                   // Clean up pending

private:
  FILE* openAndLock (const std::string&);

private:
  std::vector <Location> mLocations;
  bool mLock;
  bool mAllOpenAndLocked;
  int mId;

  std::vector <Task> mPending;   // Contents of pending.data
//  std::vector <Task> mCompleted; // Contents of completed.data

  std::vector <Task> mNew;       // Uncommitted new tasks
  std::vector <Task> mModified;  // Uncommitted modified tasks

  // TODO Need cache of raw file contents to preserve comments.
};

#endif
////////////////////////////////////////////////////////////////////////////////
