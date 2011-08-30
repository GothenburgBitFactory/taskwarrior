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
#include <CmdStart.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStart::CmdStart ()
{
  _keyword     = "start";
  _usage       = "task <filter> start [<modifications>]";
  _description = STRING_CMD_START_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStart::execute (std::string& output)
{
  int rc = 0;
  int count = 0;
  std::stringstream out;

  // Apply filter.
  std::vector <Task> filtered;
  filter (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Apply the command line modifications to the started task.
  A3 modifications = context.a3.extract_modifications ();

  Permission permission;
  if (filtered.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  bool nagged = false;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    if (! task->has ("start"))
    {
      Task before (*task);

      modify_task_annotate (*task, modifications);
      apply_defaults (*task);

      // Add a start time.
      task->setStart ();

      if (context.config.getBoolean ("journal.time"))
        task->addAnnotation (context.config.get ("journal.time.start.annotation"));

      // Only allow valid tasks.
      task->validate ();

      if (taskDiff (before, *task))
      {
        if (permission.confirmed (before, taskDifferences (before, *task) + STRING_CMD_DONE_PROCEED))
        {
          context.tdb2.modify (*task);
          ++count;

          if (context.config.getBoolean ("echo.command"))
            out << format (STRING_CMD_START_DONE,
                           task->id,
                           task->get ("description"))
                << "\n";

          dependencyChainOnStart (*task);
        }
      }

      if (!nagged)
        nagged = nag (*task);
    }
    else
    {
      out << format (STRING_CMD_START_ALREADY,
                     task->id,
                     task->get ("description"))
          << "\n";
      rc = 1;
    }
  }

  context.tdb2.commit ();
  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
