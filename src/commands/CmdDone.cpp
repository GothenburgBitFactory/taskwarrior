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
#include <CmdDone.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDone::CmdDone ()
{
  _keyword     = "done";
  _usage       = "task done ID [tags] [attrs] [desc...]";
  _description = "Marks the specified task as completed.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDone::execute (const std::string&, std::string& output)
{
  int rc = 0;
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    context.footnote ("No tasks specified.");
    return 1;
  }

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  bool nagged = false;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    if (task->getStatus () == Task::pending ||
        task->getStatus () == Task::waiting)
    {
      Task before (*task);

      // Apply other deltas.
      if (deltaDescription (*task))
        permission.bigChange ();

      deltaTags (*task);
      deltaAttributes (*task);
      deltaSubstitutions (*task);

      // Add an end date.
      char entryTime[16];
      sprintf (entryTime, "%u", (unsigned int) time (NULL));
      task->set ("end", entryTime);

      // Change status.
      task->setStatus (Task::completed);

      // Stop the task, if started.
      if (task->has ("start") &&
          context.config.getBoolean ("journal.time"))
        task->addAnnotation (context.config.get ("journal.time.stop.annotation"));

      // Only allow valid tasks.
      task->validate ();

      if (taskDiff (before, *task))
      {
        if (permission.confirmed (before, taskDifferences (before, *task) + "Proceed with change?"))
        {
          context.tdb.update (*task);

          if (context.config.getBoolean ("echo.command"))
            out << "Completed "
                << task->id
                << " '"
                << task->get ("description")
                << "'.\n";

          dependencyChainOnComplete (*task);
          context.footnote (onProjectChange (*task, false));

          ++count;
        }
      }

      updateRecurrenceMask (all, *task);
      if (!nagged)
        nagged = nag (*task);
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is neither pending nor waiting.\n";
      rc = 1;
    }
  }

  if (count)
    context.tdb.commit ();

  context.tdb.unlock ();

  if (context.config.getBoolean ("echo.command"))
    out << "Marked "
        << count
        << " task"
        << (count == 1 ? "" : "s")
        << " as done.\n";

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
