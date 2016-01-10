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
#include <ISO8601.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <main.h>

extern Context context;

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
    std::cout << format (STRING_TDB2_DIRTY_EXIT, std::string (_file))
              << "\n";
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
  if (context.cli2.getCommand () == "import")
    _tasks_map.insert (std::pair<std::string, Task> (task.get("uuid"), task));

  Task::status status = task.getStatus ();
  if (task.id == 0 &&
      (status == Task::pending   ||
       status == Task::recurring ||
       status == Task::waiting))
  {
    task.id = context.tdb2.next_id ();
  }

  _I2U[task.id] = task.get ("uuid");
  _U2I[task.get ("uuid")] = task.id;

  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
bool TF2::modify_task (const Task& task)
{
  std::string uuid = task.get ("uuid");

  if (context.cli2.getCommand () == "import")
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
    if (!_modified_tasks.size () &&
        (_added_tasks.size () || _added_lines.size ()))
    {
      if (_file.open ())
      {
        if (context.config.getBoolean ("locking"))
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
        if (context.config.getBoolean ("locking"))
          _file.lock ();

        // Truncate the file and rewrite.
        _file.truncate ();

        // Only write out _tasks, because any deltas have already been applied.
        _file.append (std::string(""));  // Seek to end of file
        for (auto& task : _tasks)
          _file.write_raw (task.composeF4 () + "\n");

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
    if (! context.run_gc ||
        (status != Task::completed && status != Task::deleted))
      task.id = context.tdb2.next_id ();
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
  ISO8601d now;

  std::string status = task.get ("status");
  if (status == "pending" ||
      status == "recurring")
  {
    context.tdb2.pending._tasks.push_back (task);
  }
  else if (status == "waiting")
  {
    ISO8601d wait (task.get_date ("wait"));
    if (wait < now)
    {
      task.set ("status", "pending");
      task.remove ("wait");
      // Unwaiting pending tasks is the only case not caught by the size()
      // checks in TDB2::gc(), so we need to signal it here.
      context.tdb2.pending._dirty = true;

      if (context.verbose ("unwait"))
        context.footnote (format (STRING_TDB2_UNWAIT, task.get ("description")));
    }

    context.tdb2.pending._tasks.push_back (task);
  }
  else
  {
    context.tdb2.completed._tasks.push_back (task);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_tasks (bool from_gc /* = false */)
{
  context.timer_load.start ();

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

      if (context.cli2.getCommand () == "import")  // For faster lookup only
        _tasks_map.insert (std::pair<std::string, Task> (task.get("uuid"), task));
    }

    // TDB2::gc() calls this after loading both pending and completed
    if (_auto_dep_scan && !from_gc)
      dependency_scan ();

    _loaded_tasks = true;
  }

  catch (const std::string& e)
  {
    throw e + format (STRING_TDB2_PARSE_ERROR, _file._data, line_number);
  }

  context.timer_load.stop ();
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_lines ()
{
  if (_file.open ())
  {
    if (context.config.getBoolean ("locking"))
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
    if (left.has ("depends"))
    {
      std::vector <std::string> deps;
      left.getDependencies (deps);

      for (auto& dep : deps)
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
  std::string mode = std::string (_file.readable () ? "r" : "-") +
                     std::string (_file.writable () ? "w" : "-");
       if (mode == "r-") mode = red.colorize (mode);
  else if (mode == "rw") mode = green.colorize (mode);
  else                   mode = yellow.colorize (mode);

  // Hygiene.
  std::string hygiene = _dirty ? red.colorize ("O") : green.colorize ("-");

  std::string tasks          = green.colorize  (rightJustifyZero ((int) _tasks.size (),          4));
  std::string tasks_added    = red.colorize    (rightJustifyZero ((int) _added_tasks.size (),    3));
  std::string tasks_modified = yellow.colorize (rightJustifyZero ((int) _modified_tasks.size (), 3));
  std::string lines          = green.colorize  (rightJustifyZero ((int) _lines.size (),          4));
  std::string lines_added    = red.colorize    (rightJustifyZero ((int) _added_lines.size (),    3));

  char buffer[256];  // Composed string is actually 246 bytes.  Yikes.
  snprintf (buffer, 256, "%14s %s %s T%s+%s~%s L%s+%s",
            label.c_str (),
            mode.c_str (),
            hygiene.c_str (),
            tasks.c_str (),
            tasks_added.c_str (),
            tasks_modified.c_str (),
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
void TDB2::add (Task& task, bool add_to_backlog /* = true */)
{
  // Ensure the task is consistent, and provide defaults if necessary.
  // bool argument to validate() is "applyDefault". Pass add_to_backlog through
  // in order to not apply defaults to synchronized tasks.
  task.validate (add_to_backlog);
  std::string uuid = task.get ("uuid");

  // If the tasks are loaded, then verify that this uuid is not already in
  // the file.
  if (!verifyUniqueUUID (uuid))
    throw format (STRING_TDB2_UUID_NOT_UNIQUE, uuid);

  // Only locally-added tasks trigger hooks.  This means that tasks introduced
  // via 'sync' do not trigger hooks.
  if (add_to_backlog)
    context.hooks.onAdd (task);

  update (task, add_to_backlog, true);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::modify (Task& task, bool add_to_backlog /* = true */)
{
  // Ensure the task is consistent.
  task.validate (false);
  std::string uuid = task.get ("uuid");

  // Get the unmodified task as reference, so the hook can compare.
  if (add_to_backlog)
  {
    Task original;
    get (uuid, original);
    context.hooks.onModify (original, task);
  }

  update (task, add_to_backlog);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::update (
  Task& task,
  const bool add_to_backlog,
  const bool addition /* = false */)
{
  // Validate to add metadata.
  task.validate (false);

  // If the task already exists, it is a modification, else addition.
  Task original;
  if (not addition && get (task.get ("uuid"), original))
  {
    // Update only if the tasks differ
    if (task == original)
      return;

    if (add_to_backlog)
    {
      // All locally modified tasks are timestamped, implicitly overwriting any
      // changes the user or hooks tried to apply to the "modified" attribute.
      task.setAsNow ("modified");
    }

    // Update the task, wherever it is.
    if (!pending.modify_task (task))
      completed.modify_task (task);

    // time <time>
    // old <task>
    // new <task>
    // ---
    undo.add_line ("time " + ISO8601d ().toEpochString () + "\n");
    undo.add_line ("old " + original.composeF4 () + "\n");
    undo.add_line ("new " + task.composeF4 () + "\n");
    undo.add_line ("---\n");
  }
  else
  {
    // Add new task to either pending or completed.
    std::string status = task.get ("status");
    if (status == "completed" ||
        status == "deleted")
      completed.add_task (task);
    else
      pending.add_task (task);

    // Add undo data lines:
    //   time <time>
    //   new <task>
    //   ---
    undo.add_line ("time " + ISO8601d ().toEpochString () + "\n");
    undo.add_line ("new " + task.composeF4 () + "\n");
    undo.add_line ("---\n");
  }

  // Add task to backlog.
  if (add_to_backlog)
    backlog.add_line (task.composeJSON () + "\n");
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::commit ()
{
  // Ignore harmful signals.
  signal (SIGHUP,    SIG_IGN);
  signal (SIGINT,    SIG_IGN);
  signal (SIGPIPE,   SIG_IGN);
  signal (SIGTERM,   SIG_IGN);
  signal (SIGUSR1,   SIG_IGN);
  signal (SIGUSR2,   SIG_IGN);

  dump ();
  context.timer_commit.start ();

  gather_changes ();

  pending.commit ();
  completed.commit ();
  undo.commit ();
  backlog.commit ();

  // Restore signal handling.
  signal (SIGHUP,    SIG_DFL);
  signal (SIGINT,    SIG_DFL);
  signal (SIGPIPE,   SIG_DFL);
  signal (SIGTERM,   SIG_DFL);
  signal (SIGUSR1,   SIG_DFL);
  signal (SIGUSR2,   SIG_DFL);

  context.timer_commit.stop ();
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
  changes = _changes;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert ()
{
  // Extract the details of the last txn, and roll it back.
  std::vector <std::string> u = undo.get_lines ();
  std::string uuid;
  std::string when;
  std::string current;
  std::string prior;
  revert_undo (u, uuid, when, current, prior);

  // Display diff and confirm.
  show_diff (current, prior, when);
  if (! context.config.getBoolean ("confirmation") ||
      confirm (STRING_TDB2_UNDO_CONFIRM))
  {
    // There are six kinds of change possible.  Determine which one, and act
    // accordingly.
    //
    // Revert: task add
    // [1] 0 --> p
    //   - erase from pending
    //   - if in backlog, erase, else cannot undo
    //
    // Revert: task modify
    // [2] p --> p'
    //   - write prior over current in pending
    //   - add prior to backlog
    //
    // Revert: task done/delete
    // [3] p --> c
    //   - add prior to pending
    //   - erase from completed
    //   - add prior to backlog
    //
    // Revert: task modify
    // [4] c --> p
    //   - add prior to completed
    //   - erase from pending
    //   - add prior to backlog
    //
    // Revert: task modify
    // [5] c --> c'
    //   - write prior over current in completed
    //   - add prior to backlog
    //
    // Revert: task log
    // [6] 0 --> c
    //   - erase from completed
    //   - if in backlog, erase, else cannot undo

    // Modify other data files accordingly.
    std::vector <std::string> p = pending.get_lines ();
    revert_pending (p, uuid, prior);

    std::vector <std::string> c = completed.get_lines ();
    revert_completed (p, c, uuid, prior);

    std::vector <std::string> b = backlog.get_lines ();
    revert_backlog (b, uuid, current, prior);

    // Commit.  If processing makes it this far with no exceptions, then we're
    // done.
    File::write (undo._file._data, u);
    File::write (pending._file._data, p);
    File::write (completed._file._data, c);
    File::write (backlog._file._data, b);
  }
  else
    std::cout << STRING_CMD_CONFIG_NO_CHANGE << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_undo (
  std::vector <std::string>& u,
  std::string& uuid,
  std::string& when,
  std::string& current,
  std::string& prior)
{
  if (u.size () < 3)
    throw std::string (STRING_TDB2_NO_UNDO);

  // pop last tx
  u.pop_back (); // separator.

  current = u.back ().substr (4);
  u.pop_back ();

  if (u.back ().substr (0, 5) == "time ")
  {
    when = u.back ().substr (5);
    u.pop_back ();
    prior = "";
  }
  else
  {
    prior = u.back ().substr (4);
    u.pop_back ();
    when = u.back ().substr (5);
    u.pop_back ();
  }

  // Extract identifying uuid.
  auto uuidAtt = current.find ("uuid:\"");
  if (uuidAtt != std::string::npos)
    uuid = current.substr (uuidAtt + 6, 36); // "uuid:"<uuid>" --> <uuid>
  else
    throw std::string (STRING_TDB2_MISSING_UUID);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_pending (
  std::vector <std::string>& p,
  const std::string& uuid,
  const std::string& prior)
{
  std::string uuid_att = "uuid:\"" + uuid + "\"";

  // is 'current' in pending?
  for (auto task = p.begin (); task != p.end (); ++task)
  {
    if (task->find (uuid_att) != std::string::npos)
    {
      context.debug ("TDB::revert - task found in pending.data");

      // Either revert if there was a prior state, or remove the task.
      if (prior != "")
      {
        *task = prior;
        std::cout << STRING_TDB2_REVERTED << "\n";
      }
      else
      {
        p.erase (task);
        std::cout << STRING_TDB2_REMOVED << "\n";
      }

      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_completed (
  std::vector <std::string>& p,
  std::vector <std::string>& c,
  const std::string& uuid,
  const std::string& prior)
{
  std::string uuid_att = "uuid:\"" + uuid + "\"";

  // is 'current' in completed?
  for (auto task = c.begin (); task != c.end (); ++task)
  {
    if (task->find (uuid_att) != std::string::npos)
    {
      context.debug ("TDB::revert_completed - task found in completed.data");

      // Either revert if there was a prior state, or remove the task.
      if (prior != "")
      {
        *task = prior;
        if (task->find ("status:\"pending\"")   != std::string::npos ||
            task->find ("status:\"waiting\"")   != std::string::npos ||
            task->find ("status:\"recurring\"") != std::string::npos)
        {
          c.erase (task);
          p.push_back (prior);
          std::cout << STRING_TDB2_REVERTED << "\n";
          context.debug ("TDB::revert_completed - task belongs in pending.data");
        }
        else
        {
          std::cout << STRING_TDB2_REVERTED << "\n";
          context.debug ("TDB::revert_completed - task belongs in completed.data");
        }
      }
      else
      {
        c.erase (task);

        std::cout << STRING_TDB2_REVERTED << "\n";
        context.debug ("TDB::revert_completed - task removed");
      }

      std::cout << STRING_TDB2_UNDO_COMPLETE << "\n";
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert_backlog (
  std::vector <std::string>& b,
  const std::string& uuid,
  const std::string& current,
  const std::string& prior)
{
  std::string uuid_att = "\"uuid\":\"" + uuid + "\"";

  bool found = false;
  for (auto task = b.rbegin (); task != b.rend (); ++task)
  {
    if (task->find (uuid_att) != std::string::npos)
    {
      context.debug ("TDB::revert_backlog - task found in backlog.data");
      found = true;

      // If this is a new task (no prior), then just remove it from the backlog.
      if (current != "" && prior == "")
      {
        // Yes, this is what is needed, when you want to erase using a reverse
        // iterator.
        b.erase ((++task).base ());
      }

      // If this is a modification of some kind, add the prior to the backlog.
      else
      {
        Task t (prior);
        b.push_back (t.composeJSON ());
      }

      break;
    }
  }

  if (!found)
    throw std::string (STRING_TDB2_UNDO_SYNCED);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::show_diff (
  const std::string& current,
  const std::string& prior,
  const std::string& when)
{
  ISO8601d lastChange (strtol (when.c_str (), NULL, 10));

  // Set the colors.
  Color color_red   (context.color () ? context.config.get ("color.undo.before") : "");
  Color color_green (context.color () ? context.config.get ("color.undo.after") : "");

  if (context.config.get ("undo.style") == "side")
  {
    std::cout << "\n"
              << format (STRING_TDB2_LAST_MOD, lastChange.toString ())
              << "\n";

    // Attributes are all there is, so figure the different attribute names
    // between before and after.
    ViewText view;
    view.width (context.getWidth ());
    view.intraPadding (2);
    view.add (Column::factory ("string", ""));
    view.add (Column::factory ("string", STRING_TDB2_UNDO_PRIOR));
    view.add (Column::factory ("string", STRING_TDB2_UNDO_CURRENT));

    Color label (context.config.get ("color.label"));
    view.colorHeader (label);

    Task after (current);

    if (prior != "")
    {
      Task before (prior);

      std::vector <std::string> beforeAtts;
      for (auto& att : before.data)
        beforeAtts.push_back (att.first);

      std::vector <std::string> afterAtts;
      for (auto& att : after.data)
        afterAtts.push_back (att.first);

      std::vector <std::string> beforeOnly;
      std::vector <std::string> afterOnly;
      listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

      int row;
      for (auto& name : beforeOnly)
      {
        row = view.addRow ();
        view.set (row, 0, name);
        view.set (row, 1, renderAttribute (name, before.get (name)), color_red);
      }

      for (auto& att : before.data)
      {
        std::string priorValue   = before.get (att.first);
        std::string currentValue = after.get  (att.first);

        if (currentValue != "")
        {
          row = view.addRow ();
          view.set (row, 0, att.first);
          view.set (row, 1, renderAttribute (att.first, priorValue),
                    (priorValue != currentValue ? color_red : Color ()));
          view.set (row, 2, renderAttribute (att.first, currentValue),
                    (priorValue != currentValue ? color_green : Color ()));
        }
      }

      for (auto& name : afterOnly)
      {
        row = view.addRow ();
        view.set (row, 0, name);
        view.set (row, 2, renderAttribute (name, after.get (name)), color_green);
      }
    }
    else
    {
      int row;
      for (auto& att : after.data)
      {
        row = view.addRow ();
        view.set (row, 0, att.first);
        view.set (row, 2, renderAttribute (att.first, after.get (att.first)), color_green);
      }
    }

    std::cout << "\n"
              << view.render ()
              << "\n";
  }

  // This style looks like this:
  //  --- before    2009-07-04 00:00:25.000000000 +0200
  //  +++ after    2009-07-04 00:00:45.000000000 +0200
  //
  // - name: old           // att deleted
  // + name:
  //
  // - name: old           // att changed
  // + name: new
  //
  // - name:
  // + name: new           // att added
  //
  else if (context.config.get ("undo.style") == "diff")
  {
    // Create reference tasks.
    Task before;
    if (prior != "")
      before.parse (prior);

    Task after (current);

    // Generate table header.
    ViewText view;
    view.width (context.getWidth ());
    view.intraPadding (2);
    view.add (Column::factory ("string", ""));
    view.add (Column::factory ("string", ""));

    int row = view.addRow ();
    view.set (row, 0, STRING_TDB2_DIFF_PREV, color_red);
    view.set (row, 1, STRING_TDB2_DIFF_PREV_DESC, color_red);

    row = view.addRow ();
    view.set (row, 0, STRING_TDB2_DIFF_CURR, color_green);  // Note trailing space.
    view.set (row, 1, format (STRING_TDB2_DIFF_CURR_DESC,
                              lastChange.toString (context.config.get ("dateformat"))),
                      color_green);

    view.addRow ();

    // Add rows to table showing diffs.
    std::vector <std::string> all = context.getColumns ();

    // Now factor in the annotation attributes.
    for (auto& it : before.data)
      if (it.first.substr (0, 11) == "annotation_")
        all.push_back (it.first);

    for (auto& it : after.data)
      if (it.first.substr (0, 11) == "annotation_")
        all.push_back (it.first);

    // Now render all the attributes.
    std::sort (all.begin (), all.end ());

    std::string before_att;
    std::string after_att;
    std::string last_att;
    for (auto& a : all)
    {
      if (a != last_att)  // Skip duplicates.
      {
        last_att = a;

        before_att = before.get (a);
        after_att  = after.get (a);

        // Don't report different uuid.
        // Show nothing if values are the unchanged.
        if (a == "uuid" ||
            before_att == after_att)
        {
          // Show nothing - no point displaying that which did not change.

          // row = view.addRow ();
          // view.set (row, 0, *a + ":");
          // view.set (row, 1, before_att);
        }

        // Attribute deleted.
        else if (before_att != "" && after_att == "")
        {
          row = view.addRow ();
          view.set (row, 0, "-" + a + ":", color_red);
          view.set (row, 1, before_att, color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + a + ":", color_green);
        }

        // Attribute added.
        else if (before_att == "" && after_att != "")
        {
          row = view.addRow ();
          view.set (row, 0, "-" + a + ":", color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + a + ":", color_green);
          view.set (row, 1, after_att, color_green);
        }

        // Attribute changed.
        else
        {
          row = view.addRow ();
          view.set (row, 0, "-" + a + ":", color_red);
          view.set (row, 1, before_att, color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + a + ":", color_green);
          view.set (row, 1, after_att, color_green);
        }
      }
    }

    std::cout << "\n"
              << view.render ()
              << "\n";
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
// - waiting task in pending that needs to be un-waited
void TDB2::gc ()
{
  context.timer_gc.start ();
  unsigned long load_start = context.timer_load.total ();

  // Allowed as an override, but not recommended.
  if (context.config.getBoolean ("gc"))
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

  // Stop and remove accumulated load time from the GC time, because they
  // overlap.
  context.timer_gc.stop ();
  context.timer_gc.subtract (context.timer_load.total () - load_start);
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
  std::vector <Task> results;
  if (task.has ("parent"))
  {
    std::string parent = task.get ("parent");

    // First load and scan pending.
    if (! pending._loaded_tasks)
      pending.load_tasks ();

    for (auto& i : pending._tasks)
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
const std::vector <Task> TDB2::children (Task& task)
{
  std::vector <Task> results;
  std::string parent = task.get ("uuid");

  // First load and scan pending.
  if (! pending._loaded_tasks)
    pending.load_tasks ();

  for (auto& i : pending._tasks)
  {
    // Do not include self in results.
    if (i.id != task.id)
    {
      // Do not include completed or deleted tasks.
      if (i.getStatus () != Task::completed &&
          i.getStatus () != Task::deleted)
      {
        // If task has the same parent, it is a sibling.
        if (i.get ("parent") == parent)
          results.push_back (i);
      }
    }
  }

  return results;
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
bool TDB2::read_only ()
{
  return pending._read_only   ||
         completed._read_only ||
         undo._read_only      ||
         backlog._read_only
         ;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::clear ()
{
  pending.clear ();
  completed.clear ();
  undo.clear ();
  backlog.clear ();

  _location = "";
  _id = 1;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::dump ()
{
  if (context.config.getBoolean ("debug"))
  {
    context.debug (pending.dump ());
    context.debug (completed.dump ());
    context.debug (undo.dump ());
    context.debug (backlog.dump ());
    context.debug (" ");
  }
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 et sw=2
