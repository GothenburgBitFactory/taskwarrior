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

#define L10N                                           // Localization complete.

#include <sstream>
#include <Context.h>
#include <Permission.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <CmdStop.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStop::CmdStop ()
{
  _keyword     = "stop";
  _usage       = "task <filter> stop [<modifications>]";
  _description = STRING_CMD_STOP_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStop::execute (std::string& output)
{
  int rc = 0;
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  context.tdb.loadPending (tasks);

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Apply the command line modifications to the stopped task.
  A3 modifications = context.a3.extract_modifications ();

  Permission permission;
  if (filtered.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    if (task->has ("start"))
    {
      Task before (*task);

      modify_task_annotate (*task, modifications);
      apply_defaults (*task);

      // Remove the start time.
      task->remove ("start");

      if (context.config.getBoolean ("journal.time"))
        task->addAnnotation (context.config.get ("journal.time.stop.annotation"));

      // Only allow valid tasks.
      task->validate ();

      if (taskDiff (before, *task))
      {
        if (permission.confirmed (before, taskDifferences (before, *task) + STRING_CMD_DONE_PROCEED))
        {
          context.tdb.update (*task);
          ++count;

          if (context.config.getBoolean ("echo.command"))
            out << format (STRING_CMD_STOP_DONE,
                           task->id,
                           task->get ("description"))
                << "\n";
        }
      }
    }
    else
    {
      out << format (STRING_CMD_STOP_NOT,
                     task->id,
                     task->get ("description"))
          << "\n";
      rc = 1;
    }
  }

  if (count)
    context.tdb.commit ();

  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
