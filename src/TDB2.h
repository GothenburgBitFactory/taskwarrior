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
#include <ViewText.h>
#include <File.h>
#include <Task.h>

// TF2 Class represents a single file in the task database.
class TF2
{
public:
  TF2 ();
  ~TF2 ();

  void target (const std::string&);

  const std::vector <Task>&        get_tasks ();
  const std::vector <std::string>& get_lines ();
  const std::string&               get_contents ();

  void add_task (const Task&);
  void modify_task (const Task&);
  void add_line (const std::string&);
  void clear_lines ();
  void commit ();

  void load_tasks ();
  void load_lines ();
  void load_contents ();

  // ID <--> UUID mapping.
  std::string uuid (int);
  int id (const std::string&);

  const std::string dump ();

public:
  bool _read_only;
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

private:
  std::map <int, std::string> _I2U; // ID -> UUID map
  std::map <std::string, int> _U2I; // UUID -> ID map
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
  void synch ();
  int  gc ();
  int  next_id ();

  // Generalized task accessors.
  bool get (int, Task&);
  bool get (const std::string&, Task&);
  const std::vector <Task> siblings (Task&);

  void dump ();

private:
  bool verifyUniqueUUID (const std::string&);

public:
  TF2 pending;
  TF2 completed;
  TF2 undo;
  TF2 backlog;
  TF2 synch_key;

private:
  std::string _location;
  int _id;
};

#endif
////////////////////////////////////////////////////////////////////////////////
