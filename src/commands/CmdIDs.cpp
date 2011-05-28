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

#include <sstream>
#include <Context.h>
#include <main.h>
#include <util.h>
#include <CmdIds.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdIds::CmdIds ()
{
  _keyword     = "ids";
  _usage       = "task ids [<filter>]";
  _description = "Shows only the IDs of matching tasks, in the form of a range.";
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdIds::execute (const std::string& command_line, std::string& output)
{
  // Scan the pending tasks, applying any filter.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Find number of matching tasks.
  std::vector <int> ids;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    if (task->id)
      ids.push_back (task->id);

  std::sort (ids.begin (), ids.end ());
  output = compressIds (ids) + "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionIds::CmdCompletionIds ()
{
  _keyword     = "_ids";
  _usage       = "task _ids [<filter>]";
  _description = "Shows only the IDs of matching tasks, in the form of a list.";
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionIds::execute (const std::string& command_line, std::string& output)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  std::vector <int> ids;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    if (task->getStatus () != Task::deleted &&
        task->getStatus () != Task::completed)
      ids.push_back (task->id);

  std::sort (ids.begin (), ids.end ());

  std::stringstream out;
  std::vector <int>::iterator id;
  for (id = ids.begin (); id != ids.end (); ++id)
    out << *id << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCompletionIds::CmdZshCompletionIds ()
{
  _keyword     = "_zshids";
  _usage       = "task _zshids [<filter>]";
  _description = "Shows the IDs and descriptions of matching tasks.";
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCompletionIds::execute (const std::string& command_line, std::string& output)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    if (task->getStatus () != Task::deleted &&
        task->getStatus () != Task::completed)
      out << task->id
          << ":"
          << task->get ("description")
          << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
