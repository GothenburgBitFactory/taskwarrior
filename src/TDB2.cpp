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
#include <list>
#include <set>
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

bool TDB2::debug_mode = false;

////////////////////////////////////////////////////////////////////////////////
TDB2::TDB2 ()
: replica {tc::Replica()} // in-memory Replica
, _working_set {}
{
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::open_replica (const std::string& location, bool create_if_missing)
{
  replica = tc::Replica(location, create_if_missing);
}

////////////////////////////////////////////////////////////////////////////////
// Add the new task to the appropriate file.
void TDB2::add (Task&, bool/* = true */)
{
  throw std::string("add not implemented");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::modify (Task&, bool/* = true */)
{
  throw std::string("modify not implemented");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::update (
  Task&,
  const bool,
  const bool/* = false */)
{
  throw std::string("update not implemented");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::commit ()
{
  // does nothing; changes are committed as they are made
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
  // Modifications are not supported, therefore there are no changes
  changes.clear();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert ()
{
  throw std::string("revert not implemented");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::show_diff (
  const std::string& current,
  const std::string& prior,
  const std::string& when)
{
  Datetime lastChange (strtol (when.c_str (), nullptr, 10));

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
    replica.rebuild_working_set ();
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
// Locate task by UUID, wherever it is.
bool TDB2::get (const std::string& uuid, Task& task)
{
  auto maybe_task = replica.get_task(uuid);
  if (maybe_task) {
    task = Task{std::move(*maybe_task)};
    return true;
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
const std::vector <Task> TDB2::siblings (Task&)
{
  // siblings is only used in modification commands, which are not supported
  throw std::string("siblings not implemented");
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::children (Task&)
{
  // children is only used in modification commands, which are not supported
  throw std::string("children not implemented");
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
void TDB2::dump ()
{
  // TODO
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 et sw=2
