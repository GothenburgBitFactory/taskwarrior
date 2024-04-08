////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_TDB2
#define INCLUDED_TDB2

#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdio.h>
#include <FS.h>
#include <Task.h>
#include <tc/WorkingSet.h>
#include <tc/Replica.h>

namespace tc {
class Server;
}

// TDB2 Class represents all the files in the task database.
class TDB2
{
public:
  static bool debug_mode;

  TDB2 ();

  void open_replica (const std::string&, bool create_if_missing);
  void add (Task&);
  void modify (Task&);
  void get_changes (std::vector <Task>&);
  void revert ();
  void gc ();
  int  latest_id ();

  // Generalized task accessors.
  const std::vector <Task> all_tasks ();
  const std::vector <Task> pending_tasks ();
  const std::vector <Task> completed_tasks ();
  bool get (int, Task&);
  bool get (const std::string&, Task&);
  bool has (const std::string&);
  const std::vector <Task> siblings (Task&);
  const std::vector <Task> children (Task&);

  // ID <--> UUID mapping.
  std::string uuid (int);
  int id (const std::string&);

  int num_local_changes ();
  int num_reverts_possible ();

  void dump ();

  void sync (tc::Server server, bool avoid_snapshots);
  bool confirm_revert(struct tc::ffi::TCReplicaOpList);

private:
  tc::Replica replica;
  std::optional<tc::WorkingSet> _working_set;

  // UUID -> Task containing all tasks modified in this invocation.
  std::map<std::string, Task> changes;

  const tc::WorkingSet &working_set ();
  static std::string option_string (std::string input);
  static void show_diff (const std::string&, const std::string&, const std::string&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
