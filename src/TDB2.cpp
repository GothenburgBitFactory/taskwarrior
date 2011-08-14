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

//#include <iostream> // TODO Remove.
#include <Context.h>
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
//  std::cout << "# TF2::get_tasks\n";

  if (! _loaded_tasks)
    load_tasks ();

  return _tasks;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string>& TF2::get_lines ()
{
//  std::cout << "# TF2::get_lines\n";

  if (! _loaded_lines)
    load_lines ();

  return _lines;
}

////////////////////////////////////////////////////////////////////////////////
const std::string& TF2::get_contents ()
{
//  std::cout << "# TF2::get_contents\n";

  if (! _loaded_contents)
    load_contents ();

  return _contents;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::add_task (const Task& task)
{
//  std::cout << "# TF2::add_task\n";

  _tasks.push_back (task);           // For subsequent queries
  _added_tasks.push_back (task);     // For commit/synch
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::modify_task (const Task& task)
{
//  std::cout << "# TF2::modify_task\n";

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
//  std::cout << "# TF2::add_line\n";

  _added_lines.push_back (line);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// This is so that synch.key can just overwrite and not grow.
void TF2::clear_lines ()
{
//  std::cout << "# TF2::clear_lines\n";
  _lines.clear ();
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Top-down recomposition.
void TF2::commit ()
{
//  std::cout << "# TF2::commit " << _file.data << "\n";

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
      }
    }
    else
    {
      // TODO _file.truncate ();
      // TODO only write out _tasks, because any deltas have already been applied.
      // TODO append _added_lines.
    }

    _dirty = false;
  }


  // --------------------------- old implementation -------------------------
/*
  // Load the lowest form, to allow
  if (_dirty)
  {
    load_contents ();

    if (_modified_tasks.size ())
    {
      std::map <std::string, Task> modified;
      std::vector <Task>::iterator it;
      for (it = _modified_tasks.begin (); it != _modified_tasks.end (); ++it)
        modified[it->get ("uuid")] = *it;

//    for (it = _

     _modified_tasks.clear ();
    }

    if (_added_tasks.size ())
    {
      std::vector <Task>::iterator it;
      for (it = _added_tasks.begin (); it != _added_tasks.end (); ++it)
        _lines.push_back (it->composeF4 ());

      _added_tasks.clear ();
    }

    if (_added_lines.size ())
    {
      //_lines += _added_lines;
      _added_lines.clear ();
    }

// TODO This clobbers minimal case.

    _contents = "";  // TODO Verify no resize.
    join (_contents, "\n", _lines);
    _file.write (_contents);

    _dirty = false;
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_tasks ()
{
//  std::cout << "# TF2::load_tasks\n";

  if (! _loaded_lines)
    load_lines ();

  int line_number = 0;
  try
  {
    std::vector <std::string>::iterator i;
    for (i = _lines.begin (); i != _lines.end (); ++i)
    {
      ++line_number;
      Task task (*i);
// TODO Find a way to number pending tasks, but not others.
//      task.id = _id++;
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
/*
    std::stringstream s;
    s << " in " << _file.data << " at line " << line_number;
    throw e + s.str ();
*/
    throw e + format (" in {1} at line {2}", _file.data, line_number);
  }
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_lines ()
{
//  std::cout << "# TF2::load_lines\n";

  if (! _loaded_contents)
    load_contents ();

  split_minimal (_lines, _contents, '\n');
  _loaded_lines = true;
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_contents ()
{
//  std::cout << "# TF2::load_contents\n";

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
    load_tasks ();

  std::map <int, std::string>::const_iterator i;
  if ((i = _I2U.find (id)) != _I2U.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int TF2::id (const std::string& uuid)
{
  if (! _loaded_tasks)
    load_tasks ();

  std::map <std::string, int>::const_iterator i;
  if ((i = _U2I.find (uuid)) != _U2I.end ())
    return i->second;

  return 0;
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
void TDB2::add (const Task& task)
{
//  std::cout << "# TDB2::add\n";

  std::string status = task.get ("status");
  if (status == "completed" ||
      status == "deleted")
    completed.add_task (task);
  else
    pending.add_task (task);

  backlog.add_task (task);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::modify (const Task& task)
{
//  std::cout << "# TDB2::modify\n";

  std::string status = task.get ("status");
  if (status == "completed" ||
      status == "deleted")
    completed.modify_task (task);
  else
    pending.modify_task (task);

  backlog.modify_task (task);
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::commit ()
{
  dump ();
//  std::cout << "# TDB2::commit\n";
  pending.commit ();
  completed.commit ();
  undo.commit ();
  backlog.commit ();
  synch_key.commit ();
  dump ();
}

////////////////////////////////////////////////////////////////////////////////
// Scans the pending tasks for any that are completed or deleted, and if so,
// moves them to the completed.data file.  Returns a count of tasks moved.
// Now reverts expired waiting tasks to pending.
// Now cleans up dangling dependencies.
int TDB2::gc ()
{
//  std::cout << "# TDB2::gc\n";
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

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int TDB2::next_id ()
{
  return _id++;
}

////////////////////////////////////////////////////////////////////////////////
//
//  File           RW State Tasks + - ~ lines + - Bytes
//  -------------- -- ----- ----- - - - ----- - - -----
//  pending.data   rw clean   123t  +2t  -1t ~1t
//  completed.data rw clean   123t  +2t      ~1t
//  undo.data      rw clean   123t  +2t      ~1t
//  backlog.data   rw clean   123t  +2t      ~1t
//  synch-key.data rw clean                        123b
//
void TDB2::dump ()
{
  if (context.config.getBoolean ("debug"))
  {
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string",       "File"));
    view.add (Column::factory ("string.right", "RW"));
    view.add (Column::factory ("string.right", "State"));
    view.add (Column::factory ("string.right", "Tasks"));
    view.add (Column::factory ("string.right", "+"));
    view.add (Column::factory ("string.right", "~"));
    view.add (Column::factory ("string.right", "Lines"));
    view.add (Column::factory ("string.right", "+"));
    view.add (Column::factory ("string.right", "Bytes"));

    dump_file (view, "pending.data", pending);
    dump_file (view, "completed.data", completed);
    dump_file (view, "undo.data", undo);
    dump_file (view, "backlog.data", backlog);
    dump_file (view, "synch_key.data", synch_key);
    context.debug (view.render ());
  }
}

////////////////////////////////////////////////////////////////////////////////
void TDB2::dump_file (ViewText& view, const std::string& label, TF2& tf)
{
  int row = view.addRow ();
  view.set (row, 0, label);
  view.set (row, 1, std::string (tf._file.readable () ? "r" : "-") +
                    std::string (tf._file.writable () ? "w" : "-"));
  view.set (row, 2, tf._dirty ? "dirty" : "clean");
  view.set (row, 3, tf._loaded_tasks ? (format ((int)tf._tasks.size ())) : "-");
  view.set (row, 4, (int)tf._added_tasks.size ());
  view.set (row, 5, (int)tf._modified_tasks.size ());
  view.set (row, 6, tf._loaded_lines ? (format ((int)tf._lines.size ())) : "-");
  view.set (row, 7, (int)tf._added_lines.size ());
  view.set (row, 8, tf._loaded_contents ? (format ((int)tf._contents.size ())) : "-");
}

////////////////////////////////////////////////////////////////////////////////
