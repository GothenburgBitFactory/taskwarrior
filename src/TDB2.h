////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_TDB2
#define INCLUDED_TDB2

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdio.h>
#include <ViewText.h>
#include <FS.h>
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

  bool get (int, Task&);
  bool get (const std::string&, Task&);
  bool has (const std::string&);

  void add_task (Task&);
  bool modify_task (const Task&);
  void add_line (const std::string&);
  void clear_tasks ();
  void clear_lines ();
  void commit ();

  Task load_task (const std::string&);
  void load_gc (Task&);
  void load_tasks (bool from_gc = false);
  void load_lines ();

  // ID <--> UUID mapping.
  std::string uuid (int);
  int id (const std::string&);

  void has_ids ();
  void auto_dep_scan ();
  void clear ();
  const std::string dump ();

  void dependency_scan ();

  bool _read_only;
  bool _dirty;
  bool _loaded_tasks;
  bool _loaded_lines;
  bool _has_ids;
  bool _auto_dep_scan;
  std::vector <Task> _tasks;

  // _tasks_map was introduced mainly for speeding up "task import".
  // Iterating over all _tasks for each imported task is slow, making use of
  // appropriate data structures is fast.
  std::unordered_map <std::string, Task> _tasks_map;

  std::vector <Task> _added_tasks;
  std::vector <Task> _modified_tasks;
  std::vector <std::string> _lines;
  std::vector <std::string> _added_lines;
  File _file;

private:
  std::unordered_map <int, std::string> _I2U; // ID -> UUID map
  std::unordered_map <std::string, int> _U2I; // UUID -> ID map
};

// TDB2 Class represents all the files in the task database.
class TDB2
{
public:
  static bool debug_mode;

  TDB2 ();

  void set_location (const std::string&);
  void add (Task&, bool add_to_backlog = true);
  void modify (Task&, bool add_to_backlog = true);
  void commit ();
  void get_changes (std::vector <Task>&);
  void revert ();
  void gc ();
  int  next_id ();
  int  latest_id ();

  // Generalized task accessors.
  const std::vector <Task> all_tasks ();
  bool get (int, Task&);
  bool get (const std::string&, Task&);
  bool has (const std::string&);
  const std::vector <Task> siblings (Task&);
  const std::vector <Task> children (Task&);

  // ID <--> UUID mapping.
  std::string uuid (int);
  int id (const std::string&);

  // Read-only mode.
  bool read_only ();

  void clear ();
  void dump ();

private:
  void gather_changes ();
  void update (Task&, const bool, const bool addition = false);
  bool verifyUniqueUUID (const std::string&);
  void show_diff (const std::string&, const std::string&, const std::string&);
  void revert_undo (std::vector <std::string>&, std::string&, std::string&, std::string&, std::string&);
  void revert_pending (std::vector <std::string>&, const std::string&, const std::string&);
  void revert_completed (std::vector <std::string>&, std::vector <std::string>&, const std::string&, const std::string&);
  void revert_backlog (std::vector <std::string>&, const std::string&, const std::string&, const std::string&);

public:
  TF2 pending;
  TF2 completed;
  TF2 undo;
  TF2 backlog;

private:
  std::string        _location;
  int                _id;
  std::vector <Task> _changes;
};

#endif
////////////////////////////////////////////////////////////////////////////////
