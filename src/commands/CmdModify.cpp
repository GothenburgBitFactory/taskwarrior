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

#include <iostream>
#include <sstream>
#include <Context.h>
#include <Permission.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <CmdModify.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdModify::CmdModify ()
{
  _keyword     = "modify";
  _usage       = "task <filter>   modify <modifications>\n"
                 "task <sequence>        <modifications>";
  _description = "Modifies the existing task with provided arguments.\n"
                 "The 'modify' keyword is optional.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::execute (std::string& output)
{
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

  // Apply the command line modifications to the new task.
  Arguments modifications = context.args.extract_modifications ();

  Permission permission;
  if (filtered.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);
    modify_task_annotate (*task, modifications);
    apply_defaults (*task);

    // Perform some logical consistency checks.
    if (task->has ("recur") &&
        !task->has ("due")  &&
        !before.has ("due"))
      throw std::string ("You cannot specify a recurring task without a due date.");

    if (task->has ("until")  &&
        !task->has ("recur") &&
        !before.has ("recur"))
      throw std::string ("You cannot specify an until date for a non-recurring task.");

    if (before.has ("recur")     &&
        before.has ("due")        &&
        task->has ("due") &&
        task->get ("due") == "")
      throw std::string ("You cannot remove the due date from a recurring task.");

    if (before.has ("recur")        &&
        task->has ("recur") &&
        task->get ("recur") == "")
      throw std::string ("You cannot remove the recurrence from a recurring task.");

    // Make all changes.
    bool warned = false;
    std::vector <Task>::iterator other;
    for (other = tasks.begin (); other != tasks.end (); ++other)
    {
      // Skip wait: modification to a parent task, and other child tasks.  Too
      // difficult to achieve properly without losing 'waiting' as a status.
      // Soon...
      if (other->id              == task->id               || // Self
          (! task->has ("wait")  &&                           // skip waits
           before.has ("parent") &&                           // is recurring
           before.get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")    == before.get ("parent"))    // Parent
      {
        if (before.has ("parent") && !warned)
        {
          warned = true;
          std::cout << "Task "
                    << before.id
                    << " is a recurring task, and all other instances of this"
                    << " task will be modified.\n";
        }

        Task alternate (*other);

        // If a task is being made recurring, there are other cascading
        // changes.
        if (!before.has ("recur") &&
            task->has ("recur"))
        {
          other->setStatus (Task::recurring);
          other->set ("mask", "");

          std::cout << "Task "
                    << other->id
                    << " is now a recurring task.\n";
        }

        // Apply other deltas.
        modify_task_description_replace (*other, modifications);
        apply_defaults (*other);

        if (taskDiff (alternate, *other))
        {
          // Only allow valid tasks.
          other->validate ();

          if (permission.confirmed (alternate, taskDifferences (alternate, *other) + "Proceed with change?"))
          {
            // TODO Are dependencies being explicitly removed?
            //      Either we scan context.task for negative IDs "depends:-n"
            //      or we ask deltaAttributes (above) to record dependency
            //      removal.
            dependencyChainOnModify (alternate, *other);

            context.tdb.update (*other);
            ++count;

            if (alternate.get ("project") != other->get ("project"))
              context.footnote (onProjectChange (alternate, *other));

          }
        }
      }
    }
  }

  if (count)
    context.tdb.commit ();

  context.tdb.unlock ();

  if (context.config.getBoolean ("echo.command"))
    out << "Modified " << count << " task" << (count == 1 ? ".\n" : "s.\n");

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
