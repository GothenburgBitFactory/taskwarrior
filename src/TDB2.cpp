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

#include <cmake.h>
#include <TDB2.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <list>
#include <unordered_set>
#include <stdlib.h>
#include <signal.h>
#include <Context.h>
#include <Color.h>
#include <Datetime.h>
#include <Table.h>
#include <shared.h>
#include <format.h>
#include <main.h>
#include <util.h>
#include "tc/Server.h"
#include "tc/util.h"

bool TDB2::debug_mode = false;
static void dependency_scan (std::vector<Task> &);

////////////////////////////////////////////////////////////////////////////////
TDB2::TDB2 ()
: replica {tc::Replica()} // in-memory Replica
, _working_set {}
{
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::open_replica (const std::string& location, bool create_if_missing)
{
  File pending_data = File (location + "/pending.data");
  if (pending_data.exists()) {
    Color warning = Color (Context::getContext ().config.get ("color.warning"));
    std::cerr << warning.colorize (
      format ("Found existing '.data' files in {1}", location)) << "\n";
    std::cerr << "  Taskwarrior's storage format changed in 3.0, requiring a manual migration.\n";
    std::cerr << "  See https://github.com/GothenburgBitFactory/taskwarrior/releases.\n";
  }
  replica = tc::Replica(location, create_if_missing);
}

////////////////////////////////////////////////////////////////////////////////
// Add the new task to the replica.
void TDB2::add (Task& task)
{
  // Ensure the task is consistent, and provide defaults if necessary.
  // bool argument to validate() is "applyDefault", to apply default values for
  // properties not otherwise given.
  task.validate (true);

  std::string uuid = task.get ("uuid");
  changes[uuid] = task;

  auto innertask = replica.import_task_with_uuid (uuid);

  {
    auto guard = replica.mutate_task(innertask);

    // add the task attributes
    for (auto& attr : task.all ()) {
      // TaskChampion does not store uuid or id in the taskmap
      if (attr == "uuid" || attr == "id") {
        continue;
      }

      // Use `set_status` for the task status, to get expected behavior
      // with respect to the working set.
      else if (attr == "status") {
        innertask.set_status (Task::status2tc (Task::textToStatus (task.get (attr))));
      }

      // use `set_modified` to set the modified timestamp, avoiding automatic
      // updates to this field by TaskChampion.
      else if (attr == "modified") {
        auto mod = (time_t) std::stoi (task.get (attr));
        innertask.set_modified (mod);
      }

      // otherwise, just set the k/v map value
      else {
        innertask.set_value (attr, std::make_optional (task.get (attr)));
      }
    }
  }

  auto ws = replica.working_set ();

  // get the ID that was assigned to this task
  auto id = ws.by_uuid (uuid);

  // update the cached working set with the new information
  _working_set = std::make_optional (std::move (ws));

  // run hooks for this new task
  Context::getContext ().hooks.onAdd (task);

  if (id.has_value ()) {
      task.id = id.value();
  }

}

////////////////////////////////////////////////////////////////////////////////
// Modify the task in storage to match the given task.
//
// Note that there are a few race conditions to consider here.  Taskwarrior
// loads the enitre task into memory and this method then essentially writes
// the entire task back to the database. So, if the task in the database
// changes between loading the task and this method being called, this method
// will "revert" those changes. In practice this would only occur when multiple
// `task` invocatoins run at the same time and try to modify the same task.
//
// There is also the possibility that another task process has deleted the task
// from the database between the time this process loaded the tsak and called
// this method. In this case, this method throws an error that will make sense
// to the user. This is especially unlikely since tasks are only deleted when
// they have been unmodified for a long time.
void TDB2::modify (Task& task)
{
  // All locally modified tasks are timestamped, implicitly overwriting any
  // changes the user or hooks tried to apply to the "modified" attribute.
  task.setAsNow ("modified");
  task.validate (false);
  auto uuid = task.get ("uuid");

  changes[uuid] = task;

	// invoke the hook and allow it to modify the task before updating
  Task original;
  get (uuid, original);
	Context::getContext ().hooks.onModify (original, task);

  auto maybe_tctask = replica.get_task (uuid);
  if (!maybe_tctask.has_value ()) {
    throw std::string ("task no longer exists");
  }
  auto tctask = std::move (maybe_tctask.value ());
  auto guard = replica.mutate_task(tctask);
  auto tctask_map = tctask.get_taskmap ();

  std::unordered_set<std::string> seen;
  for (auto k : task.all ()) {
    // ignore task keys that aren't stored
    if (k == "uuid") {
      continue;
    }
    seen.insert(k);
    bool update = false;
    auto v_new = task.get(k);
    try {
      auto v_tctask = tctask_map.at(k);
      update = v_tctask != v_new;
    } catch (const std::out_of_range& oor) {
      // tctask_map does not contain k, so update it
      update = true;
    }
    if (update) {
      // An empty string indicates the value should be removed.
      if (v_new == "") {
        tctask.set_value(k, {});
      } else {
        tctask.set_value(k, make_optional (v_new));
      }
    }
  }

  // we've now added and updated properties; but must find any deleted properties
  for (auto kv : tctask_map) {
    if (seen.find (kv.first) == seen.end ()) {
      tctask.set_value (kv.first, {});
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const tc::WorkingSet &TDB2::working_set ()
{
  if (!_working_set.has_value ()) {
    _working_set = std::make_optional (replica.working_set ());
  }
  return _working_set.value ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::get_changes (std::vector <Task>& changes)
{
  std::map<std::string, Task>& changes_map = this->changes;
  changes.clear();
  std::transform(changes_map.begin(), changes_map.end(), std::back_inserter(changes),
                 [](const auto& kv) { return kv.second; });
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert ()
{
  auto undo_ops = replica.get_undo_ops();
  if (undo_ops.len == 0) {
    std::cout << "No operations to undo.";
    return;
  }
  if (confirm_revert(undo_ops)) {
    // Has the side-effect of freeing undo_ops.
    replica.commit_undo_ops(undo_ops, NULL);
  } else {
    replica.free_replica_ops(undo_ops);
  }
  replica.rebuild_working_set (false);
}

////////////////////////////////////////////////////////////////////////////////
bool TDB2::confirm_revert (struct tc::ffi::TCReplicaOpList undo_ops)
{
  // TODO Use show_diff rather than this basic listing of operations, though
  // this might be a worthy undo.style itself.
  std::cout << "The following " << undo_ops.len << " operations would be reverted:\n";
  for (size_t i = 0; i < undo_ops.len; i++) {
    std::cout << "- ";
    tc::ffi::TCReplicaOp op = undo_ops.items[i];
    switch(op.operation_type) {
      case tc::ffi::TCReplicaOpType::Create:
        std::cout << "Create " << replica.get_op_uuid(op);
        break;
      case tc::ffi::TCReplicaOpType::Delete:
        std::cout << "Delete " << replica.get_op_old_task_description(op);
        break;
      case tc::ffi::TCReplicaOpType::Update:
        std::cout << "Update " << replica.get_op_uuid(op) << "\n";
        std::cout << "    " << replica.get_op_property(op) << ": " << option_string(replica.get_op_old_value(op)) << " -> " << option_string(replica.get_op_value(op));
        break;
      case tc::ffi::TCReplicaOpType::UndoPoint:
        throw std::string ("Can't undo UndoPoint.");
        break;
      default:
        throw std::string ("Can't undo non-operation.");
        break;
    }
    std::cout << "\n";
  }
  return ! Context::getContext ().config.getBoolean ("confirmation") ||
        confirm ("The undo command is not reversible.  Are you sure you want to revert to the previous state?");
}

////////////////////////////////////////////////////////////////////////////////
std::string TDB2::option_string(std::string input) {
  return input == "" ? "<empty>" : input;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::show_diff (
  const std::string& current,
  const std::string& prior,
  const std::string& when)
{
  Datetime lastChange (strtoll (when.c_str (), nullptr, 10));

  // Set the colors.
  Color color_red   (Context::getContext ().color () ? Context::getContext ().config.get ("color.undo.before") : "");
  Color color_green (Context::getContext ().color () ? Context::getContext ().config.get ("color.undo.after") : "");

  auto before = prior == "" ? Task() : Task(prior);
  auto after = Task(current);

  if (Context::getContext ().config.get ("undo.style") == "side")
  {
    Table view = before.diffForUndoSide(after);

    std::cout << '\n'
              << format ("The last modification was made {1}", lastChange.toString ())
              << '\n'
              << '\n'
              << view.render ()
              << '\n';
  }

  else if (Context::getContext ().config.get ("undo.style") == "diff")
  {
    Table view = before.diffForUndoPatch(after, lastChange);
    std::cout << '\n'
              << view.render ()
              << '\n';
  }
}

void TDB2::gc ()
{
  Timer timer;

  // Allowed as an override, but not recommended.
  if (Context::getContext ().config.getBoolean ("gc"))
  {
    replica.rebuild_working_set (true);
  }

  Context::getContext ().time_gc_us += timer.total_us ();
}

////////////////////////////////////////////////////////////////////////////////
// Latest ID is that of the last pending task.
int TDB2::latest_id ()
{
  const tc::WorkingSet &ws = working_set ();
  return (int)ws.largest_index ();
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::all_tasks ()
{
  auto all_tctasks = replica.all_tasks();
  std::vector <Task> all;
  for (auto& tctask : all_tctasks)
    all.push_back (Task (std::move (tctask)));

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::pending_tasks ()
{
  const tc::WorkingSet &ws = working_set ();
  auto largest_index = ws.largest_index ();

  std::vector <Task> result;
  for (size_t i = 0; i <= largest_index; i++) {
    auto maybe_uuid = ws.by_index (i);
    if (maybe_uuid.has_value ()) {
      auto maybe_task = replica.get_task (maybe_uuid.value ());
      if (maybe_task.has_value ()) {
        result.push_back (Task (std::move (maybe_task.value ())));
      }
    }
  }

  dependency_scan(result);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::completed_tasks ()
{
  auto all_tctasks = replica.all_tasks();
  const tc::WorkingSet &ws = working_set ();

  std::vector <Task> result;
  for (auto& tctask : all_tctasks) {
    // if this task is _not_ in the working set, return it.
    if (!ws.by_uuid (tctask.get_uuid ())) {
      result.push_back (Task (std::move (tctask)));
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by ID, wherever it is.
bool TDB2::get (int id, Task& task)
{
  const tc::WorkingSet &ws = working_set ();
  const auto maybe_uuid = ws.by_index (id);
  if (maybe_uuid) {
    auto maybe_task = replica.get_task(*maybe_uuid);
    if (maybe_task) {
      task = Task{std::move(*maybe_task)};
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by UUID, including by partial ID, wherever it is.
bool TDB2::get (const std::string& uuid, Task& task)
{
  // try by raw uuid, if the length is right
  if (uuid.size () == 36) {
    try {
      auto maybe_task = replica.get_task (uuid);
      if (maybe_task) {
        task = Task{std::move (*maybe_task)};
        return true;
      }
    } catch (const std::string &err) {
      return false;
    }
  }

  // Nothing to do but iterate over all tasks and check whether it's closeEnough
  for (auto& tctask : replica.all_tasks ()) {
    if (closeEnough (tctask.get_uuid (), uuid, uuid.length ())) {
      task = Task{std::move (tctask)};
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by UUID, wherever it is.
bool TDB2::has (const std::string& uuid)
{
  Task task;
  return get(uuid, task);
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::siblings (Task& task)
{
  std::vector <Task> results;
  if (task.has ("parent"))
  {
    std::string parent = task.get ("parent");

    for (auto& i : this->pending_tasks())
    {
      // Do not include self in results.
      if (i.id != task.id)
      {
        // Do not include completed or deleted tasks.
        if (i.getStatus () != Task::completed &&
            i.getStatus () != Task::deleted)
        {
          // If task has the same parent, it is a sibling.
          if (i.has ("parent") &&
              i.get ("parent") == parent)
          {
            results.push_back (i);
          }
        }
      }
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::children (Task& parent)
{
  // scan _pending_ tasks for those with `parent` equal to this task
	std::vector <Task> results;
  std::string this_uuid = parent.get ("uuid");

  const tc::WorkingSet &ws = working_set ();
  size_t end_idx = ws.largest_index ();

  for (size_t i = 0; i <= end_idx; i++) {
    auto uuid_opt = ws.by_index (i);
    if (!uuid_opt) {
      continue;
    }
    auto uuid = uuid_opt.value ();

    // skip self-references
    if (uuid == this_uuid) {
      continue;
    }

    auto task_opt = replica.get_task (uuid_opt.value ());
    if (!task_opt) {
      continue;
    }
    auto task = std::move (task_opt.value ());

    auto parent_uuid_opt = task.get_value ("parent");
    if (!parent_uuid_opt) {
      continue;
    }
    auto parent_uuid = parent_uuid_opt.value ();

    if (parent_uuid == this_uuid) {
      results.push_back (Task (std::move (task)));
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
std::string TDB2::uuid (int id)
{
  const tc::WorkingSet &ws = working_set ();
  return ws.by_index ((size_t)id).value_or ("");
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::id (const std::string& uuid)
{
  const tc::WorkingSet &ws = working_set ();
  return (int)ws.by_uuid (uuid).value_or (0);
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::num_local_changes ()
{
  return (int)replica.num_local_operations ();
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::num_reverts_possible ()
{
  return (int)replica.num_undo_points ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::sync (tc::Server server, bool avoid_snapshots)
{
  replica.sync(std::move(server), avoid_snapshots);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::dump ()
{
  // TODO
}

////////////////////////////////////////////////////////////////////////////////
// For any task that has depenencies, follow the chain of dependencies until the
// end.  Along the way, update the Task::is_blocked and Task::is_blocking data
// cache.
static void dependency_scan (std::vector<Task> &tasks)
{
  for (auto& left : tasks)
  {
    for (auto& dep : left.getDependencyUUIDs ())
    {
      for (auto& right : tasks)
      {
        if (right.get ("uuid") == dep)
        {
          // GC hasn't run yet, check both tasks for their current status
          Task::status lstatus = left.getStatus ();
          Task::status rstatus = right.getStatus ();
          if (lstatus != Task::completed &&
              lstatus != Task::deleted &&
              rstatus != Task::completed &&
              rstatus != Task::deleted)
          {
            left.is_blocked = true;
            right.is_blocking = true;
          }

          // Only want to break out of the "right" loop.
          break;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 et sw=2
