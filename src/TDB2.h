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

#include <Task.h>
#include <taskchampion-cpp/lib.h>

#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Adjacency maps for the dependency graph.
// Index refers to the pending_tasks() vector.
struct DependencyGraph {
  std::unordered_map<std::string, std::vector<size_t>> dependents;
  std::unordered_map<std::string, std::vector<size_t>> dependencies;
};
// We adopt a caching strategy which lazily builds _pending_tasks,
// _completed_tasks, _working_set, _dependency_graph from the Rust
// replica whwnever its state may have changed. Callers receive const
// refs to avoid silent copies of the whole vectors.

// TDB2 Class represents all the files in the task database.
class TDB2 {
 public:
  static bool debug_mode;

  TDB2() = default;

  void open_replica(const std::string&, bool create_if_missing, bool read_write);
  void add(Task&);
  void modify(Task&);
  void purge(Task&);
  void get_changes(std::vector<Task>&);
  void gc();
  void expire_tasks();
  int latest_id();

  // Generalized task accessors.
  // We don't cache the all_tasks vector because no command accesses it more than once.
  // Caching it would cause higher memory use and would require additional rewrites to make tests
  // pass.
  const std::vector<Task> all_tasks();
  // pending and completed tasks are cached after first use.
  const std::vector<Task>& pending_tasks();
  const std::vector<Task>& completed_tasks();
  // dependency_graph is built on first use from pending_tasks() and reused.
  // functions that use it are Task::getDependencyTasks(), getBlockedTasks()
  // and urgency_inherit().
  const DependencyGraph& dependency_graph();

  bool get(int, Task&);
  bool get(const std::string&, Task&);
  bool has(const std::string&);
  const std::vector<Task> siblings(Task&);
  const std::vector<Task> children(Task&);

  // ID <--> UUID mapping.
  std::string uuid(int);
  int id(const std::string&);

  // Index of pending tasks by UUID. The ptr is stable until the
  // cache is invalidated.
  // Used by get() and modify().
  Task* find_pending(const std::string& uuid);

  int num_local_changes();
  int num_reverts_possible();

  rust::Box<tc::Replica>& replica();

 private:
  std::optional<rust::Box<tc::Replica>> _replica;

  // Cached information from the replica
  // All caches are invalidated when state may have changed
  // in the replica.
  std::optional<rust::Box<tc::WorkingSet>> _working_set;
  std::optional<std::vector<Task>> _pending_tasks;
  std::optional<std::vector<Task>> _completed_tasks;
  // Dependency cache.
  std::optional<DependencyGraph> _dependency_graph;
  // Lazily cache UUIDs within the pending set.
  // Avoids scans of the vectors with get/modify..
  std::optional<std::unordered_map<std::string, size_t>> _pending_index;
  void invalidate_cached_info();

  // Return the full pending UUID map.
  const std::unordered_map<std::string, size_t>& pending_index();

  // UUID -> Task containing all tasks modified in this invocation.
  std::map<std::string, Task> changes;

  const rust::Box<tc::WorkingSet>& working_set();
  void maybe_add_undo_point(rust::Vec<tc::Operation>&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
