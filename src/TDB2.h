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
#ifndef INCLUDED_TDB2
#define INCLUDED_TDB2
#define L10N                                           // Localization complete.

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <File.h>
#include <Task.h>

// TF2 Class represents a single file in the task database.
class TF2
{
public:
  TF2 ();
  ~TF2 ();

  void target (const std::string&);

  std::vector <Task>& get_tasks ();
  std::vector <std::string>& get_lines ();
  std::string& get_contents ();

  void add_task (const Task&);
  void modify_task (const Task&);
  void add_line (const std::string&);
  void clear_lines ();
  void commit ();

private:
  void load_tasks ();
  void load_lines ();
  void load_contents ();

private:
  bool _dirty;
  bool _loaded_tasks;
  bool _loaded_lines;
  bool _loaded_contents;
  std::vector <Task> _tasks;
  std::vector <Task> _added_tasks;
  std::vector <Task> _modified_tasks;
  std::vector <std::string> _lines;
  std::vector <std::string> _added_lines;
  std::string _contents;
  File _file;
};


// TDB2 Class represents all the files in the task database.
class TDB2
{
public:
  TDB2 ();
  ~TDB2 ();

  void set_location (const std::string&);
  void add (const Task&);
  void modify (const Task&);
  void commit ();

public:
  TF2 pending;
  TF2 completed;
  TF2 undo;
  TF2 backlog;
  TF2 synch_key;

private:
  std::string _location;
};





/*
#include <Location.h>
#include <Filter.h>
#include <Task.h>

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
*/

#endif
////////////////////////////////////////////////////////////////////////////////
