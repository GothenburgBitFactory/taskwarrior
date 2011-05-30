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
#include <CmdStop.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStop::CmdStop ()
{
  _keyword     = "stop";
  _usage       = "task stop ID";
  _description = "Removes the 'start' time from a task.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStop::execute (const std::string&, std::string& output)
{
  int rc = 0;
  std::stringstream out;

  context.disallowModification ();

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    context.footnote ("No tasks specified.");
    return 1;
  }

  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    if (task->has ("start"))
    {
      task->remove ("start");

      if (context.config.getBoolean ("journal.time"))
        task->addAnnotation (context.config.get ("journal.time.stop.annotation"));

      context.tdb.update (*task);

      if (context.config.getBoolean ("echo.command"))
        out << "Stopped "
            << task->id
            << " '"
            << task->get ("description")
            << "'.\n";
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' not started.\n";
      rc = 1;
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
