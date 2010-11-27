////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_TDB
#define INCLUDED_TDB

#include <map>
#include <vector>
#include <string>
#include "Location.h"
#include "Filter.h"
#include "Task.h"

// Length of longest line.
#define T_LINE_MAX 32768

class TDB
{
public:
  TDB ();  // Default constructor
  ~TDB (); // Destructor

  TDB (const TDB&);
  TDB& operator= (const TDB&);

  void  clear ();
  void  location (const std::string&);

  void  lock (bool lockFile = true);
  void  unlock ();

  int   load (std::vector <Task>&, Filter&);
  int   loadPending (std::vector <Task>&, Filter&);
  int   loadCompleted (std::vector <Task>&, Filter&);

  const std::vector <Task>& getAllPending ();
  const std::vector <Task>& getAllNew ();
  const std::vector <Task>& getAllCompleted ();
  const std::vector <Task>& getAllModified ();

  void  add (const Task&);    // Single task add to pending
  void  update (const Task&); // Single task update to pending
  int   commit ();            // Write out all tasks
  int   gc ();                // Clean up pending
  int   nextId ();
  void  undo ();
  void  merge (const std::string&);

  std::string uuid (int) const;
  int id (const std::string&) const;

private:
  FILE* openAndLock (const std::string&);
  void writeUndo (const Task&, FILE*);
  void writeUndo (const Task&, const Task&, FILE*);
  bool uuidAlreadyUsed (const std::string&);
  bool uuidAlreadyUsed (const std::string&, const std::vector <Task>&);

private:
  std::vector <Location> mLocations;
  bool mLock;
  bool mAllOpenAndLocked;
  int mId;

  std::vector <Task> mPending;   // Contents of pending.data
  std::vector <Task> mCompleted; // Contents of pending.data
  std::vector <Task> mNew;       // Uncommitted new tasks
  std::vector <Task> mModified;  // Uncommitted modified tasks

  std::map <int, std::string> mI2U; // ID -> UUID map
  std::map <std::string, int> mU2I; // UUID -> ID map
};

#endif
////////////////////////////////////////////////////////////////////////////////
