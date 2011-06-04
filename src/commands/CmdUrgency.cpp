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
#include <stdlib.h>
#include <Context.h>
#include <main.h>
#include <CmdUrgency.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdUrgency::CmdUrgency ()
{
  _keyword     = "_urgency";
  _usage       = "task _urgency <IDs>";
  _description = "Displays the urgency measure of a task.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUrgency::execute (std::string& output)
{
/*
  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    context.footnote ("No tasks specified.");
    return 1;
  }

  // Find the task(s).
  std::stringstream out;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    out << "task "
        << task->id
        << " urgency "
        << task->urgency ()
        << "\n";

  output = out.str ();
*/
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
