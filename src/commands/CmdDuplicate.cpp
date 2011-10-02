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
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>
#include <CmdDuplicate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDuplicate::CmdDuplicate ()
{
  _keyword     = "duplicate";
  _usage       = "task <filter> duplicate <mods>";
  _description = STRING_CMD_DUPLICATE_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDuplicate::execute (std::string& output)
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

  // Apply the command line modifications to the completed task.
  A3 modifications = context.a3.extract_modifications ();

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task dup (*task);
    dup.set ("uuid", uuid ());     // Needs a new UUID.
    dup.setStatus (Task::pending); // Does not inherit status.
    dup.remove ("start");          // Does not inherit start date.
    dup.remove ("end");            // Does not inherit end date.
    dup.remove ("entry");          // Does not inherit entry date.

    // Recurring tasks are duplicated and downgraded to regular tasks.
    if (task->getStatus () == Task::recurring)
    {
      dup.remove ("parent");
      dup.remove ("recur");
      dup.remove ("until");
      dup.remove ("imak");
      dup.remove ("imask");

      out << format (STRING_CMD_DUPLICATE_NON_REC, task->id)
          << "\n";
    }

    modify_task_annotate (dup, modifications);
    context.tdb2.add (dup);
    ++count;

    if (context.verbose ("affected") ||
        context.config.getBoolean ("echo.command")) // Deprecated 2.0
      out << format (STRING_CMD_DUPLICATE_DONE,
                     task->id,
                     task->get ("description"))
          << "\n";

    // TODO This should be a call in to feedback.cpp.
    if (context.verbose ("new-id"))
      out << format (STRING_CMD_ADD_FEEDBACK, context.tdb2.next_id ()) + "\n";

    context.footnote (onProjectChange (dup));
  }

  // TODO Add count summary, like the 'done' command.

  context.tdb2.commit ();
  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
