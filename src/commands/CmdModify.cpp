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
#include <CmdModify.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdModify::CmdModify ()
{
  // TODO Mention substitutions.
  _keyword     = "modify";
  _usage       = "task modify ID [tags] [attrs] [desc...]\n"
                 "task        ID [tags] [attrs] [desc...]";
  _description = "Modifies the existing task with provided arguments.\n"
                 "The 'modify' keyword is optional.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::execute (const std::string&, std::string& output)
{
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

  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    // Perform some logical consistency checks.
    if (context.task.has ("recur") &&
        !context.task.has ("due")  &&
        !task->has ("due"))
      throw std::string ("You cannot specify a recurring task without a due date.");

    if (context.task.has ("until")  &&
        !context.task.has ("recur") &&
        !task->has ("recur"))
      throw std::string ("You cannot specify an until date for a non-recurring task.");

    if (task->has ("recur")     &&
        task->has ("due")        &&
        context.task.has ("due") &&
        context.task.get ("due") == "")
      throw std::string ("You cannot remove the due date from a recurring task.");

    if (task->has ("recur")        &&
        context.task.has ("recur") &&
        context.task.get ("recur") == "")
      throw std::string ("You cannot remove the recurrence from a recurring task.");

    // Make all changes.
    bool warned = false;
    std::vector <Task>::iterator other;
    for (other = all.begin (); other != all.end (); ++other)
    {
      // Skip wait: modification to a parent task, and other child tasks.  Too
      // difficult to achieve properly without losing 'waiting' as a status.
      // Soon...
      if (other->id             == task->id               || // Self
          (! context.task.has ("wait") &&                    // skip waits
           task->has ("parent") &&                           // is recurring
           task->get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")   == task->get ("parent"))     // Parent
      {
        if (task->has ("parent") && !warned)
        {
          warned = true;
          std::cout << "Task "
                    << task->id
                    << " is a recurring task, and all other instances of this"
                    << " task will be modified.\n";
        }

        Task before (*other);

        // A non-zero value forces a file write.
        int changes = 0;

        // If a task is being made recurring, there are other cascading
        // changes.
        if (!task->has ("recur") &&
            context.task.has ("recur"))
        {
          other->setStatus (Task::recurring);
          other->set ("mask", "");
          ++changes;

          std::cout << "Task "
                    << other->id
                    << " is now a recurring task.\n";
        }

        // Apply other deltas.
        if (deltaDescription (*other))
        {
          permission.bigChange ();
          ++changes;
        }

        changes += deltaTags (*other);
        changes += deltaAttributes (*other);
        changes += deltaSubstitutions (*other);

        if (taskDiff (before, *other))
        {
          // Only allow valid tasks.
          other->validate ();

          if (changes && permission.confirmed (before, taskDifferences (before, *other) + "Proceed with change?"))
          {
            // TODO Are dependencies being explicitly removed?
            //      Either we scan context.task for negative IDs "depends:-n"
            //      or we ask deltaAttributes (above) to record dependency
            //      removal.
            dependencyChainOnModify (before, *other);

            context.tdb.update (*other);

            if (before.get ("project") != other->get ("project"))
              context.footnote (onProjectChange (before, *other));

            ++count;
          }
        }
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  if (context.config.getBoolean ("echo.command"))
    out << "Modified " << count << " task" << (count == 1 ? ".\n" : "s.\n");

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
