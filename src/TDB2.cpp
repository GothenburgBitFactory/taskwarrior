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

#include <iostream> // TODO Remove.
#include <Context.h>
#include <Color.h>
#include <Date.h>
#include <text.h>
#include <TDB2.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
TF2::TF2 ()
: _read_only (false)
, _dirty (false)
, _loaded_tasks (false)
, _loaded_lines (false)
, _loaded_contents (false)
, _contents ("")
{
}

////////////////////////////////////////////////////////////////////////////////
TF2::~TF2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
void TF2::target (const std::string& f)
{
  _file = File (f);
  _read_only = ! _file.writable ();

//  std::cout << "# TF2::target " << f << "\n";
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <Task>& TF2::get_tasks ()
{
//  std::cout << "# TF2::get_tasks " << _file._data << "\n";

  if (! _loaded_tasks)
  {
    load_tasks ();

    // Apply previously added tasks.
    std::vector <Task>::iterator i;
    for (i = _added_tasks.begin (); i != _added_tasks.end (); ++i)
      _tasks.push_back (*i);

//    std::cout << "# TF2::get_tasks added " << _added_tasks.size () << " tasks\n";
  }

  return _tasks;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string>& TF2::get_lines ()
{
//  std::cout << "# TF2::get_lines " << _file._data << "\n";

  if (! _loaded_lines)
  {
    load_lines ();

    // Apply previously added lines.
    std::vector <std::string>::iterator i;
    for (i = _added_lines.begin (); i != _added_lines.end (); ++i)
      _lines.push_back (*i);

//    std::cout << "# TF2::get_lines added " << _added_lines.size () << " lines\n";
  }

  return _lines;
}

////////////////////////////////////////////////////////////////////////////////
const std::string& TF2::get_contents ()
{
//  std::cout << "# TF2::get_contents " << _file._data << "\n";

  if (! _loaded_contents)
    load_contents ();

  return _contents;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_task (const Task& task)
{
//  std::cout << "# TF2::add_task " << _file._data << "\n";

  _tasks.push_back (task);           // For subsequent queries
  _added_tasks.push_back (task);     // For commit/synch

/*
  int id = context.tdb2.next_id ();
  _I2U[id] = task.get ("uuid");
  _U2I[task.get ("uuid")] = id;
*/

  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::modify_task (const Task& task)
{
//  std::cout << "# TF2::modify_task " << _file._data << "\n";

  // Modify in-place.
  std::vector <Task>::iterator i;
  for (i = _tasks.begin (); i != _tasks.end (); ++i)
  {
    if (i->get ("uuid") == task.get ("uuid"))
    {
      *i = task;
      break;
    }
  }

  _modified_tasks.push_back (task);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_line (const std::string& line)
{
//  std::cout << "# TF2::add_line " << _file._data << "\n";

  _lines.push_back (line);
  _added_lines.push_back (line);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// This is so that synch.key can just overwrite and not grow.
void TF2::clear_lines ()
{
//  std::cout << "# TF2::clear_lines " << _file._data << "\n";
  _lines.clear ();
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Top-down recomposition.
void TF2::commit ()
{
//  std::cout << "# TF2::commit " << _file._data << "\n";

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
        std::vector <Task>::iterator task;
        for (task = _added_tasks.begin ();
             task != _added_tasks.end ();
             ++task)
        {
          _file.append (task->composeF4 ());
        }

        _added_tasks.clear ();

        // Write out all the added lines.
        std::vector <std::string>::iterator line;
        for (line = _added_lines.begin ();
             line != _added_lines.end ();
             ++line)
        {
          _file.append (*line);
        }

        _added_lines.clear ();
        _file.close ();
        _dirty = false;
      }
    }
    else
    {
      if (_file.open ())
      {
        // Truncate the file and rewrite.
        _file.truncate ();

        // only write out _tasks, because any deltas have already been applied.
        std::vector <Task>::iterator task;
        for (task = _tasks.begin ();
             task != _tasks.end ();
             ++task)
        {
          _file.append (task->composeF4 ());
        }

        // Write out all the added lines.
        std::vector <std::string>::iterator line;
        for (line = _added_lines.begin ();
             line != _added_lines.end ();
             ++line)
        {
          _file.append (*line);
        }

        _added_lines.clear ();
        _file.close ();
        _dirty = false;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_tasks ()
{
//  std::cout << "# TF2::load_tasks " << _file._data << "\n";
  context.timer_load.start ();

  if (! _loaded_lines)
  {
    load_lines ();

    // Apply previously added lines.
    std::vector <std::string>::iterator i;
    for (i = _added_lines.begin (); i != _added_lines.end (); ++i)
      _lines.push_back (*i);

//    std::cout << "# TF2::load_tasks added " << _added_lines.size () << " lines\n";
  }

  int line_number = 0;
  try
  {
    std::vector <std::string>::iterator i;
    for (i = _lines.begin (); i != _lines.end (); ++i)
    {
      ++line_number;
      Task task (*i);

      // Every task gets an ID.
      task.id = context.tdb2.next_id ();
      _tasks.push_back (task);

      // Maintain mapping for ease of link/dependency resolution.
      // Note that this mapping is not restricted by the filter, and is
      // therefore a complete set.
      if (task.id)
      {
        _I2U[task.id] = task.get ("uuid");
        _U2I[task.get ("uuid")] = task.id;
      }
    }

    _loaded_tasks = true;
  }

  catch (std::string& e)
  {
    throw e + format (" in {1} at line {2}", _file._data, line_number);
  }

  context.timer_load.stop ();
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_lines ()
{
//  std::cout << "# TF2::load_lines " << _file._data << "\n";

  if (! _loaded_contents)
    load_contents ();

  split_minimal (_lines, _contents, '\n');
  _loaded_lines = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_contents ()
{
//  std::cout << "# TF2::load_contents " << _file._data << "\n";

  _contents = "";

  if (_file.open ())
  {
    if (context.config.getBoolean ("locking"))
      _file.lock ();

    _file.read (_contents);
    _loaded_contents = true;
  }
  // TODO Error handling?
}

////////////////////////////////////////////////////////////////////////////////
std::string TF2::uuid (int id)
{
  if (! _loaded_tasks)
  {
    load_tasks ();

    // Apply previously added tasks.
    std::vector <Task>::iterator i;
    for (i = _added_tasks.begin (); i != _added_tasks.end (); ++i)
      _tasks.push_back (*i);

//    std::cout << "# TF2::uuid added " << _added_tasks.size () << " tasks\n";
  }

  std::map <int, std::string>::const_iterator i;
  if ((i = _I2U.find (id)) != _I2U.end ())
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
    std::vector <Task>::iterator i;
    for (i = _added_tasks.begin (); i != _added_tasks.end (); ++i)
      _tasks.push_back (*i);

//    std::cout << "# TF2::id added " << _added_tasks.size () << " tasks\n";
  }

  std::map <std::string, int>::const_iterator i;
  if ((i = _U2I.find (uuid)) != _U2I.end ())
    return i->second;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::clear ()
{
  _read_only       = false;
  _dirty           = false;
  _loaded_tasks    = false;
  _loaded_lines    = false;
  _loaded_contents = false;

  _contents        = "";
  _file._data      = "";

  _tasks.clear ();
  _added_tasks.clear ();
  _modified_tasks.clear ();
  _lines.clear ();
  _added_lines.clear ();
  _I2U.clear ();
  _U2I.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// <label> <rw><dirty> <tasks> <lines> <contents>
//
// label:    <label %+14s>
// rw:       <rw>
// dirty:    <!|->
// tasks:    T<tasks %04d>+<added %03d>~<changed %03d>
// lines:    L<lines %04d>+<added %03d>
// contents: C<bytes %06d>
const std::string TF2::dump ()
{
  Color red    ("rgb500 on rgb100");
  Color yellow ("rgb550 on rgb220");
  Color green  ("rgb050 on rgb010");

  // File label.
  std::string label;
  std::string::size_type slash = _file._data.rfind ('/');
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
  std::string contents       = green.colorize  (rightJustifyZero ((int) _contents.size (),       6));

  char buffer[256];  // Composed string is actually 246 bytes.  Yikes.
  snprintf (buffer, 256, "%14s %s %s T%s+%s~%s L%s+%s C%s",
            label.c_str (),
            mode.c_str (),
            hygiene.c_str (),
            tasks.c_str (),
            tasks_added.c_str (),
            tasks_modified.c_str (),
            lines.c_str (),
            lines_added.c_str (),
            contents.c_str ());

  return std::string (buffer);
}

////////////////////////////////////////////////////////////////////////////////









////////////////////////////////////////////////////////////////////////////////
TDB2::TDB2 ()
: _location ("")
, _id (1)
{
}

////////////////////////////////////////////////////////////////////////////////
// Deliberately no file writes on destruct.  TDB2::commit should have been
// already called, if data is to be preserved.
TDB2::~TDB2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Once a location is known, the files can be set up.  Note that they are not
// read.
void TDB2::set_location (const std::string& location)
{
//  std::cout << "# TDB2::set_location " << location << "\n";
  _location = location;

  pending.target   (location + "/pending.data");
  completed.target (location + "/completed.data");
  undo.target      (location + "/undo.data");
  backlog.target   (location + "/backlog.data");
  synch_key.target (location + "/synch.key");
}

////////////////////////////////////////////////////////////////////////////////
// Add the new task to the appropriate file.
void TDB2::add (Task& task)
{
//  std::cout << "# TDB2::add\n";

  // Ensure the task is consistent, and provide defaults if necessary.
  task.validate ();

  // If the tasks are loaded, then verify that this uuid is not already in
  // the file.
  if (!verifyUniqueUUID (task.get ("uuid")))
    throw format ("Cannot add task because the uuid '{1}' is not unique.", task.get ("uuid"));

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
  undo.add_line ("time " + Date ().toEpochString () + "\n");
  undo.add_line ("new " + task.composeF4 ());
  undo.add_line ("---\n");

  // Add task to backlog.
  backlog.add_task (task);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::modify (Task& task)
{
//  std::cout << "# TDB2::modify\n";

  // Ensure the task is consistent, and provide defaults if necessary.
  task.validate ();

  // Update task in either completed or deleted.
  // TODO Find task, overwrite it.
  std::string status = task.get ("status");
  if (status == "completed" ||
      status == "deleted")
    completed.modify_task (task);
  else
    pending.modify_task (task);

  // time <time>
  // old <task>
  // new <task>
  // ---
  Task original;
  get (task.get ("uuid"), original);

  undo.add_line ("time " + Date ().toEpochString () + "\n");
  undo.add_line ("old " + original.composeF4 ());
  undo.add_line ("new " + task.composeF4 ());
  undo.add_line ("---\n");

  // Add modified task to backlog.
  backlog.add_task (task);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::commit ()
{
  dump ();
  context.timer_gc.start ();

  pending.commit ();
  completed.commit ();
  undo.commit ();
  backlog.commit ();
  synch_key.commit ();

  context.timer_gc.stop ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::synch ()
{
  context.timer_synch.start ();

  // TODO Need stub here.

  context.timer_synch.stop ();
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::revert ()
{
/*
  Directory location (context.config.get ("data.location"));

  std::string undoFile      = location._data + "/undo.data";
  std::string pendingFile   = location._data + "/pending.data";
  std::string completedFile = location._data + "/completed.data";

  // load undo.data
  std::vector <std::string> u;
  File::read (undoFile, u);

  if (u.size () < 3)
    throw std::string ("There are no recorded transactions to undo.");

  // pop last tx
  u.pop_back (); // separator.

  std::string current = u.back ().substr (4);
  u.pop_back ();

  std::string prior;
  std::string when;
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

  Date lastChange (atoi (when.c_str ()));

  // Set the colors.
  Color color_red   (context.color () ? context.config.get ("color.undo.before") : "");
  Color color_green (context.color () ? context.config.get ("color.undo.after") : "");

  if (context.config.get ("undo.style") == "side")
  {
    std::cout << "\n"
              << "The last modification was made "
              << lastChange.toString ()
              << "\n";

    // Attributes are all there is, so figure the different attribute names
    // between before and after.
    ViewText view;
    view.width (context.getWidth ());
    view.intraPadding (2);
    view.add (Column::factory ("string", ""));
    view.add (Column::factory ("string", "Prior Values"));
    view.add (Column::factory ("string", "Current Values"));

    Task after (current);

    if (prior != "")
    {
      Task before (prior);

      std::vector <std::string> beforeAtts;
      foreach (att, before)
        beforeAtts.push_back (att->first);

      std::vector <std::string> afterAtts;
      foreach (att, after)
        afterAtts.push_back (att->first);

      std::vector <std::string> beforeOnly;
      std::vector <std::string> afterOnly;
      listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

      int row;
      foreach (name, beforeOnly)
      {
        row = view.addRow ();
        view.set (row, 0, *name);
        view.set (row, 1, renderAttribute (*name, before.get (*name)), color_red);
      }

      foreach (name, before)
      {
        std::string priorValue   = before.get (name->first);
        std::string currentValue = after.get  (name->first);

        if (currentValue != "")
        {
          row = view.addRow ();
          view.set (row, 0, name->first);
          view.set (row, 1, renderAttribute (name->first, priorValue),
                    (priorValue != currentValue ? color_red : Color ()));
          view.set (row, 2, renderAttribute (name->first, currentValue),
                    (priorValue != currentValue ? color_green : Color ()));
        }
      }

      foreach (name, afterOnly)
      {
        row = view.addRow ();
        view.set (row, 0, *name);
        view.set (row, 2, renderAttribute (*name, after.get (*name)), color_green);
      }
    }
    else
    {
      int row;
      foreach (name, after)
      {
        row = view.addRow ();
        view.set (row, 0, name->first);
        view.set (row, 2, renderAttribute (name->first, after.get (name->first)), color_green);
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
    view.set (row, 0, "--- previous state", color_red);
    view.set (row, 1, "Undo will restore this state", color_red);

    row = view.addRow ();
    view.set (row, 0, "+++ current state ", color_green);  // Note trailing space.
    view.set (row, 1, "Change made " + lastChange.toString (context.config.get ("dateformat")), color_green);

    view.addRow ();

    // Add rows to table showing diffs.
    std::vector <std::string> all = context.getColumns ();

    // Now factor in the annotation attributes.
    Task::iterator it;
    for (it = before.begin (); it != before.end (); ++it)
      if (it->first.substr (0, 11) == "annotation_")
        all.push_back (it->first);

    for (it = after.begin (); it != after.end (); ++it)
      if (it->first.substr (0, 11) == "annotation_")
        all.push_back (it->first);

    // Now render all the attributes.
    std::sort (all.begin (), all.end ());

    std::string before_att;
    std::string after_att;
    std::string last_att;
    foreach (a, all)
    {
      if (*a != last_att)  // Skip duplicates.
      {
        last_att = *a;

        before_att = before.get (*a);
        after_att  = after.get (*a);

        // Don't report different uuid.
        // Show nothing if values are the unchanged.
        if (*a == "uuid" ||
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
          view.set (row, 0, "-" + *a + ":", color_red);
          view.set (row, 1, before_att, color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + *a + ":", color_green);
        }

        // Attribute added.
        else if (before_att == "" && after_att != "")
        {
          row = view.addRow ();
          view.set (row, 0, "-" + *a + ":", color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + *a + ":", color_green);
          view.set (row, 1, after_att, color_green);
        }

        // Attribute changed.
        else
        {
          row = view.addRow ();
          view.set (row, 0, "-" + *a + ":", color_red);
          view.set (row, 1, before_att, color_red);

          row = view.addRow ();
          view.set (row, 0, "+" + *a + ":", color_green);
          view.set (row, 1, after_att, color_green);
        }
      }
    }

    std::cout << "\n"
              << view.render ()
              << "\n";
  }

  // Output displayed, now confirm.
  if (context.config.getBoolean ("confirmation") &&
      !confirm ("The undo command is not reversible.  Are you sure you want to revert to the previous state?"))
  {
    std::cout << "No changes made.\n";
    return;
  }

  // Extract identifying uuid.
  std::string uuid;
  std::string::size_type uuidAtt = current.find ("uuid:\"");
  if (uuidAtt != std::string::npos)
    uuid = current.substr (uuidAtt, 43); // 43 = uuid:"..."
  else
    throw std::string ("Cannot locate UUID in task to undo.");

  // load pending.data
  std::vector <std::string> p;
  File::read (pendingFile, p);

  // is 'current' in pending?
  foreach (task, p)
  {
    if (task->find (uuid) != std::string::npos)
    {
      context.debug ("TDB::undo - task found in pending.data");

      // Either revert if there was a prior state, or remove the task.
      if (prior != "")
      {
        *task = prior;
        std::cout << "Modified task reverted.\n";
      }
      else
      {
        p.erase (task);
        std::cout << "Task removed.\n";
      }

      // Rewrite files.
      File::write (pendingFile, p);
      File::write (undoFile, u);
      return;
    }
  }

  // load completed.data
  std::vector <std::string> c;
  File::read (completedFile, c);

  // is 'current' in completed?
  foreach (task, c)
  {
    if (task->find (uuid) != std::string::npos)
    {
      context.debug ("TDB::undo - task found in completed.data");

      // If task now belongs back in pending.data
      if (prior.find ("status:\"pending\"")   != std::string::npos ||
          prior.find ("status:\"waiting\"")   != std::string::npos ||
          prior.find ("status:\"recurring\"") != std::string::npos)
      {
        c.erase (task);
        p.push_back (prior);
        File::write (completedFile, c);
        File::write (pendingFile, p);
        File::write (undoFile, u);
        std::cout << "Modified task reverted.\n";
        context.debug ("TDB::undo - task belongs in pending.data");
      }
      else
      {
        *task = prior;
        File::write (completedFile, c);
        File::write (undoFile, u);
        std::cout << "Modified task reverted.\n";
        context.debug ("TDB::undo - task belongs in completed.data");
      }

      std::cout << "Undo complete.\n";
      return;
    }
  }

  // Perhaps user hand-edited the data files?
  // Perhaps the task was in completed.data, which was still in file format 3?
  std::cout << "Task with UUID "
            << uuid.substr (6, 36)
            << " not found in data.\n"
            << "No undo possible.\n";
*/
}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
// Now reverts expired waiting tasks to pending.
// Now cleans up dangling dependencies.
int TDB2::gc ()
{
  context.timer_gc.start ();

  // Allowed as a temporary override.
  if (context.config.getBoolean ("gc"))
  {
/*
    pending.load_tasks
    completed.load_tasks

    for each pending
      if status == completed || status == deleted
        pending.remove
        completed.add
      if status == waiting && wait < now
        status = pending
        wait.clear

    for each completed
      if status == pending || status == waiting
        completed.remove
        pending.add
*/

    // TODO Remove dangling dependencies
    // TODO Wake up expired waiting tasks
  }

  context.timer_gc.stop ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Next ID is that of the last pending task plus one.
int TDB2::next_id ()
{
  return _id++;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by ID.
bool TDB2::get (int id, Task& task)
{
  // First load and scan pending.
  if (! pending._loaded_tasks)
    pending.load_tasks ();

  std::vector <Task>::iterator i;
  for (i = pending._tasks.begin (); i != pending._tasks.end (); ++i)
  {
    if (i->id == id)
    {
      task = *i;
      return true;
    }
  }

  // Next load and scan completed.
  // Note that this is harmless, because it is only performed if the above
  // load and search fails.
  if (! completed._loaded_tasks)
    completed.load_tasks ();

  for (i = completed._tasks.begin (); i != completed._tasks.end (); ++i)
  {
    if (i->id == id)
    {
      task = *i;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Locate task by UUID.
bool TDB2::get (const std::string& uuid, Task& task)
{
  // First load and scan pending.
  if (! pending._loaded_tasks)
    pending.load_tasks ();

  std::vector <Task>::iterator i;
  for (i = pending._tasks.begin (); i != pending._tasks.end (); ++i)
  {
    if (i->get ("uuid") == uuid)
    {
      task = *i;
      return true;
    }
  }

  // Next load and scan completed.
  // Note that this is harmless, because it is only performed if the above
  // load and search fails.
  if (! completed._loaded_tasks)
    completed.load_tasks ();

  for (i = completed._tasks.begin (); i != completed._tasks.end (); ++i)
  {
    if (i->get ("uuid") == uuid)
    {
      task = *i;
      return true;
    }
  }

  return false;
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

    std::vector <Task>::iterator i;
    for (i = pending._tasks.begin (); i != pending._tasks.end (); ++i)
    {
      // Do not include self in results.
      if (i->id != task.id)
      {
        // Do not include completed or deleted tasks.
        if (i->getStatus () != Task::completed &&
            i->getStatus () != Task::deleted)
        {
          // If task has the same parent, it is a sibling.
          if (i->has ("parent") &&
              i->get ("parent") == parent)
          {
            results.push_back (*i);
          }
        }
      }
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
bool TDB2::verifyUniqueUUID (const std::string& uuid)
{
  if (pending.id (uuid)   != 0 ||
      completed.id (uuid) != 0)
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::clear ()
{
  pending.clear ();
  completed.clear ();
  undo.clear ();
  backlog.clear ();
  synch_key.clear ();

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
    context.debug (synch_key.dump ());
    context.debug ("");
  }
}

////////////////////////////////////////////////////////////////////////////////
