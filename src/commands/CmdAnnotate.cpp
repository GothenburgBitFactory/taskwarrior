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
#include <Permission.h>
#include <main.h>
#include <CmdAnnotate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdAnnotate::CmdAnnotate ()
{
  _keyword     = "annotate";
  _usage       = "task annotate ID desc...";
  _description = "Adds an annotation to an existing task.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAnnotate::execute (std::string& output)
{
  int rc = 0;

/*
  if (!context.task.has ("description"))
    throw std::string ("Cannot apply a blank annotation.");

  if (context.sequence.size () == 0)
    throw std::string ("ID needed to apply an annotation.");

  std::stringstream out;

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

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    Task before (*task);
    task->addAnnotation (context.task.get ("description"));

    if (taskDiff (before, *task))
    {
      // Only allow valid tasks.
      task->validate ();

      if (permission.confirmed (before, taskDifferences (before, *task) + "Proceed with change?"))
      {
        context.tdb.update (*task);

        if (context.config.getBoolean ("echo.command"))
          out << "Annotated "
              << task->id
              << " with '"
              << context.task.get ("description")
              << "'.\n";
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  output = out.str ();
*/
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
