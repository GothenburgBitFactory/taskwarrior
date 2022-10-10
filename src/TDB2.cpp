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

#define STRING_TDB2_REVERTED         "Modified task reverted."

bool TDB2::debug_mode = false;

////////////////////////////////////////////////////////////////////////////////
TF2::TF2 ()
: _read_only (false)
, _dirty (false)
, _loaded_tasks (false)
, _loaded_lines (false)
, _has_ids (false)
, _auto_dep_scan (false)
{
}

////////////////////////////////////////////////////////////////////////////////
TF2::~TF2 ()
{
  if (_dirty && TDB2::debug_mode)
    std::cout << format ("Exiting with unwritten changes to {1}\n", std::string (_file));
}

////////////////////////////////////////////////////////////////////////////////
void TF2::target (const std::string& f)
{
  _file = File (f);

  // A missing file is not considered unwritable.
  _read_only = false;
  if (_file.exists () && ! _file.writable ())
    _read_only = true;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TF2::get_tasks ()
{
  if (! _loaded_tasks)
    load_tasks ();

  return _tasks;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string>& TF2::get_lines ()
{
  if (! _loaded_lines)
    load_lines ();

  return _lines;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by id.
bool TF2::get (int id, Task& task)
{
  if (! _loaded_tasks)
    load_tasks ();

  // This is an optimization.  Since the 'id' is based on the line number of
  // pending.data file, the task in question cannot appear earlier than line
  // (id - 1) in the file.  It can, however, appear significantly later because
  // it is not known how recent a GC operation was run.
  for (unsigned int i = id - 1; i < _tasks.size (); ++i)
  {
    if (_tasks[i].id == id)
    {
      task = _tasks[i];
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by uuid, which may be a partial UUID.
bool TF2::get (const std::string& uuid, Task& task)
{
  if (! _loaded_tasks)
    load_tasks ();

  if (_tasks_map.size () > 0 && uuid.size () == 36)
  {
    // Fast lookup, same result as below. Only used during "task import".
    auto i = _tasks_map.find (uuid);
    if (i != _tasks_map.end ())
    {
      task = i->second;
      return true;
    }
  }
  else
  {
    // Slow lookup, same result as above.
    for (auto& i : _tasks)
    {
      if (closeEnough (i.get ("uuid"), uuid, uuid.length ()))
      {
        task = i;
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TF2::has (const std::string& uuid)
{
  if (! _loaded_tasks)
    load_tasks ();

  for (auto& i : _tasks)
    if (i.get ("uuid") == uuid)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_task (Task& task)
{
  _tasks.push_back (task);           // For subsequent queries
  _added_tasks.push_back (task);     // For commit/synch

  // For faster lookup
  if (Context::getContext ().cli2.getCommand () == "import")
    _tasks_map.emplace (task.get("uuid"), task);

  Task::status status = task.getStatus ();
  if (task.id == 0 &&
      (status == Task::pending   ||
       status == Task::recurring ||
       status == Task::waiting))
  {
    task.id = Context::getContext ().tdb2.next_id ();
  }

  _I2U[task.id] = task.get ("uuid");
  _U2I[task.get ("uuid")] = task.id;

  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
bool TF2::modify_task (const Task& task)
{
  std::string uuid = task.get ("uuid");

  if (Context::getContext ().cli2.getCommand () == "import")
  {
    // Update map used for faster lookup
    auto i = _tasks_map.find (uuid);
    if (i != _tasks_map.end ())
    {
      i->second = task;
    }
  }

  for (auto& i : _tasks)
  {
    if (i.get ("uuid") == uuid)
    {
      // Modify in-place.
      i = task;
      _modified_tasks.push_back (task);
      _dirty = true;

      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool TF2::purge_task (const Task& task)
{
  // Bail out if task is not found in this file
  std::string uuid = task.get ("uuid");
  if (!has (uuid))
    return false;

  // Mark the task to be purged
  _purged_tasks.insert (uuid);
  _dirty = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_line (const std::string& line)
{
  _lines.push_back (line);
  _added_lines.push_back (line);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::clear_tasks ()
{
  _tasks.clear ();
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::clear_lines ()
{
  _lines.clear ();
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Top-down recomposition.
void TF2::commit ()
{
  // The _dirty flag indicates that the file needs to be written.
  if (_dirty)
  {
    // Special case: added but no modified means just append to the file.
    if (!_modified_tasks.size () && !_purged_tasks.size () &&
        (_added_tasks.size () || _added_lines.size ()))
    {
      if (_file.open ())
      {
        if (Context::getContext ().config.getBoolean ("locking"))
          _file.lock ();

        // Write out all the added tasks.
        _file.append (std::string(""));  // Seek to end of file
        for (auto& task : _added_tasks)
          _file.write_raw (task.composeF4 () + "\n");

        _added_tasks.clear ();

        // Write out all the added lines.
        _file.append (_added_lines);

        _added_lines.clear ();
        _file.close ();
        _dirty = false;
      }
    }
    else
    {
      if (_file.open ())
      {
        if (Context::getContext ().config.getBoolean ("locking"))
          _file.lock ();

        // Truncate the file and rewrite.
        _file.truncate ();

        // Only write out _tasks, because any deltas have already been applied.
        _file.append (std::string(""));  // Seek to end of file
        for (auto& task : _tasks)
          // Skip over the tasks that are marked to be purged
          if (_purged_tasks.find (task.get ("uuid")) == _purged_tasks.end ())
            _file.write_raw (task.composeF4 () + '\n');

        // Write out all the added lines.
        _file.append (_added_lines);

        _added_lines.clear ();
        _file.close ();
        _dirty = false;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Load a single Task object, handle necessary plumbing work
Task TF2::load_task (const std::string& line)
{
  Task task (line);

  // Some tasks get an ID.
  if (_has_ids)
  {
    Task::status status = task.getStatus ();
    // Completed / deleted tasks in pending.data get an ID if GC is off.
    if (! Context::getContext ().run_gc ||
        (status != Task::completed && status != Task::deleted))
      task.id = Context::getContext ().tdb2.next_id ();
  }

  // Maintain mapping for ease of link/dependency resolution.
  // Note that this mapping is not restricted by the filter, and is
  // therefore a complete set.
  if (task.id)
  {
    _I2U[task.id] = task.get ("uuid");
    _U2I[task.get ("uuid")] = task.id;
  }

  return task;
}

////////////////////////////////////////////////////////////////////////////////
// Check whether task needs to be relocated to pending/completed,
// or needs to be 'woken'.
void TF2::load_gc (Task& task)
{
  Datetime now;

  std::string status = task.get ("status");
  if (status == "pending" ||
      status == "recurring")
  {
    Context::getContext ().tdb2.pending._tasks.push_back (task);
  }
  // 2.6.0: Waiting status is deprecated. Convert to pending to upgrade status
  // field value in the data files.
  else if (status == "waiting")
  {
    task.set ("status", "pending");
    Context::getContext ().tdb2.pending._tasks.push_back (task);
    Context::getContext ().tdb2.pending._dirty = true;
  }
  else
  {
    Context::getContext ().tdb2.completed._tasks.push_back (task);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_tasks (bool from_gc /* = false */)
{
  Timer timer;

  if (! _loaded_lines)
  {
    load_lines ();

    // Apply previously added lines.
    for (auto& line : _added_lines)
      _lines.push_back (line);
  }

  // Reduce unnecessary allocations/copies.
  // Calling it on _tasks is the right thing to do even when from_gc is set.
  _tasks.reserve (_lines.size ());

  int line_number = 0;  // Used for error message in catch block.
  try
  {
    for (auto& line : _lines)
    {
      ++line_number;
      auto task = load_task (line);

      if (from_gc)
        load_gc (task);
      else
        _tasks.push_back (task);

      if (Context::getContext ().cli2.getCommand () == "import")  // For faster lookup only
        _tasks_map.emplace (task.get("uuid"), task);
    }

    // TDB2::gc() calls this after loading both pending and completed
    if (_auto_dep_scan && !from_gc)
      dependency_scan ();

    _loaded_tasks = true;
  }

  catch (const std::string& e)
  {
    throw e + format (" in {1} at line {2}", _file._data, line_number);
  }

  Context::getContext ().time_load_us += timer.total_us ();
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_lines ()
{
  if (_file.open ())
  {
    if (Context::getContext ().config.getBoolean ("locking"))
      _file.lock ();

    _file.read (_lines);
    _file.close ();
    _loaded_lines = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string TF2::uuid (int id)
{
  if (! _loaded_tasks)
  {
    load_tasks ();

    // Apply previously added tasks.
    for (auto& task : _added_tasks)
      _tasks.push_back (task);
  }

  auto i = _I2U.find (id);
  if (i != _I2U.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int TF2::id (const std::string& uuid)
{
  if (! _loaded_tasks)
  {
    load_tasks ();

    // Apply previously added tasks.
    for (auto& task : _added_tasks)
      _tasks.push_back (task);
  }

  auto i = _U2I.find (uuid);
  if (i != _U2I.end ())
    return i->second;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::has_ids ()
{
  _has_ids = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::auto_dep_scan ()
{
  _auto_dep_scan = true;
}

////////////////////////////////////////////////////////////////////////////////
// Completely wipe it all clean.
void TF2::clear ()
{
  _read_only       = false;
  _dirty           = false;
  _loaded_tasks    = false;
  _loaded_lines    = false;

  // Note that these are deliberately not cleared.
  //_file._data      = "";
  //_has_ids         = false;
  //_auto_dep_scan   = false;

  _tasks.clear ();
  _added_tasks.clear ();
  _modified_tasks.clear ();
  _purged_tasks.clear ();
  _lines.clear ();
  _added_lines.clear ();
  _I2U.clear ();
  _U2I.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// For any task that has depenencies, follow the chain of dependencies until the
// end.  Along the way, update the Task::is_blocked and Task::is_blocking data
// cache.
void TF2::dependency_scan ()
{
  // Iterate and modify TDB2 in-place.  Don't do this at home.
  for (auto& left : _tasks)
  {
    for (auto& dep : left.getDependencyUUIDs ())
    {
      for (auto& right : _tasks)
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
const std::string TF2::dump ()
{
  Color red    ("rgb500 on rgb100");
  Color yellow ("rgb550 on rgb220");
  Color green  ("rgb050 on rgb010");

  // File label.
  std::string label;
  auto slash = _file._data.rfind ('/');
  if (slash != std::string::npos)
    label = rightJustify (_file._data.substr (slash + 1), 14);

  // File mode.
  std::string mode = std::string (_file.exists () && _file.readable () ? "r" : "-") +
                     std::string (_file.exists () && _file.writable () ? "w" : "-");
       if (mode == "r-") mode = red.colorize (mode);
  else if (mode == "rw") mode = green.colorize (mode);
  else                   mode = yellow.colorize (mode);

  // Hygiene.
  std::string hygiene = _dirty ? red.colorize ("O") : green.colorize ("-");

  std::string tasks          = green.colorize  (rightJustifyZero ((int) _tasks.size (),          4));
  std::string tasks_added    = red.colorize    (rightJustifyZero ((int) _added_tasks.size (),    3));
  std::string tasks_modified = yellow.colorize (rightJustifyZero ((int) _modified_tasks.size (), 3));
  std::string tasks_purged   = red.colorize    (rightJustifyZero ((int) _purged_tasks.size (),   3));
  std::string lines          = green.colorize  (rightJustifyZero ((int) _lines.size (),          4));
  std::string lines_added    = red.colorize    (rightJustifyZero ((int) _added_lines.size (),    3));

  char buffer[256];  // Composed string is actually 246 bytes.  Yikes.
  snprintf (buffer, 256, "%14s %s %s T%s+%s~%s-%s L%s+%s",
            label.c_str (),
            mode.c_str (),
            hygiene.c_str (),
            tasks.c_str (),
            tasks_added.c_str (),
            tasks_modified.c_str (),
            tasks_purged.c_str (),
            lines.c_str (),
            lines_added.c_str ());

  return std::string (buffer);
}

////////////////////////////////////////////////////////////////////////////////
TDB2::TDB2 ()
: _location ("")
, _id (1)
{
  // Mark the pending file as the only one that has ID numbers.
  pending.has_ids ();

  // Indicate that dependencies should be automatically scanned on startup,
  // setting Task::is_blocked and Task::is_blocking accordingly.
  pending.auto_dep_scan ();
}

////////////////////////////////////////////////////////////////////////////////
// Once a location is known, the files can be set up.  Note that they are not
// read.
void TDB2::set_location (const std::string& location)
{
  _location = location;

  pending.target   (location + "/pending.data");
  completed.target (location + "/completed.data");
  undo.target      (location + "/undo.data");
  backlog.target   (location + "/backlog.data");
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
void TDB2::gather_changes ()
{
  _changes.clear ();
  for (auto& task : pending._added_tasks)
    _changes.push_back (task);

  for (auto& task : pending._modified_tasks)
    _changes.push_back (task);

  for (auto& task : completed._added_tasks)
    _changes.push_back (task);

  for (auto& task : completed._modified_tasks)
    _changes.push_back (task);
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

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
// Reverts expired waiting tasks to pending.
// Cleans up dangling dependencies.
//
// Possible scenarios:
// - task in pending that needs to be in completed
// - task in completed that needs to be in pending
void TDB2::gc ()
{
  Timer timer;

  // Allowed as an override, but not recommended.
  if (Context::getContext ().config.getBoolean ("gc"))
  {
    // Load pending, check whether completed changes size
    auto size_before = completed._tasks.size ();
    pending.load_tasks (/*from_gc =*/ true);
    if (size_before != completed._tasks.size ())
    {
      // GC moved tasks from pending to completed
      pending._dirty = true;
      completed._dirty = true;
    }
    else if (pending._dirty)
    {
      // A waiting task in pending was woken up
      pending._dirty = true;
    }

    // Load completed, check whether pending changes size
    size_before = pending._tasks.size ();
    completed.load_tasks (/*from_gc =*/ true);
    if (size_before != pending._tasks.size ())
    {
      // GC moved tasks from completed to pending
      pending._dirty = true;
      completed._dirty = true;
    }

    // Update blocked/blocking status after GC is finished
    if (pending._auto_dep_scan)
      pending.dependency_scan ();
    if (completed._auto_dep_scan)
      completed.dependency_scan ();
  }

  Context::getContext ().time_gc_us += timer.total_us ();
}

////////////////////////////////////////////////////////////////////////////////
// Next ID is that of the last pending task plus one.
int TDB2::next_id ()
{
  return _id++;
}

////////////////////////////////////////////////////////////////////////////////
// Latest ID is that of the last pending task.
int TDB2::latest_id ()
{
  return _id - 1;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::all_tasks ()
{
  std::vector <Task> all (pending._tasks.size () +
                          pending._added_tasks.size () +
                          completed._tasks.size () +
                          completed._added_tasks.size ());
  all = pending.get_tasks ();

  std::vector <Task> extra (completed._tasks.size () +
                            completed._added_tasks.size ());
  extra = completed.get_tasks ();

  for (auto& task : extra)
    all.push_back (task);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::pending_tasks ()
{
  return pending.get_tasks ();
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::completed_tasks ()
{
  return completed.get_tasks ();
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by ID, wherever it is.
bool TDB2::get (int id, Task& task)
{
  return pending.get   (id, task) ||
         completed.get (id, task);
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by UUID, wherever it is.
bool TDB2::get (const std::string& uuid, Task& task)
{
  return pending.get   (uuid, task) ||
         completed.get (uuid, task);
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by UUID, wherever it is.
bool TDB2::has (const std::string& uuid)
{
  return pending.has (uuid) ||
         completed.has (uuid);
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::siblings (Task& task)
{
  // siblings is only used in modification commands, which are not supported
  throw std::string("siblings not implemented");
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task> TDB2::children (Task& task)
{
  // children is only used in modification commands, which are not supported
  throw std::string("children not implemented");
}

////////////////////////////////////////////////////////////////////////////////
std::string TDB2::uuid (int id)
{
  std::string result = pending.uuid (id);
  if (result == "")
    result = completed.uuid (id);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::id (const std::string& uuid)
{
  int result = pending.id (uuid);
  if (result == 0)
    result = completed.id (uuid);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Make sure the specified UUID does not already exist in the data.
bool TDB2::verifyUniqueUUID (const std::string& uuid)
{
  pending.get_tasks ();
  return pending.id (uuid) != 0 ? false : true;
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::num_local_changes ()
{
  std::vector <std::string> lines = backlog.get_lines ();
  return std::count_if(lines.begin(), lines.end(), [](const auto& line){ return line.front() == '{'; });
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::num_reverts_possible ()
{
  std::vector <std::string> lines = undo.get_lines ();
  return std::count(lines.begin(), lines.end(), "---");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::dump ()
{
  if (Context::getContext ().config.getBoolean ("debug"))
  {
    Context::getContext ().debug (pending.dump ());
    Context::getContext ().debug (completed.dump ());
    Context::getContext ().debug (undo.dump ());
    Context::getContext ().debug (backlog.dump ());
    Context::getContext ().debug (" ");
  }
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 et sw=2
