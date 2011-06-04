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
#include <CmdAppend.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdAppend::CmdAppend ()
{
  _keyword     = "append";
  _usage       = "task append <IDs> [tags] [attrs] desc...";
  _description = "Append more description to an existing task.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAppend::execute (std::string& output)
{
  if (!context.task.has ("description"))
    throw std::string ("Additional text must be provided.");

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

  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    std::vector <Task>::iterator other;
    for (other = all.begin (); other != all.end (); ++other)
    {
      if (other->id             == task->id               || // Self
          (task->has ("parent") &&
           task->get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")   == task->get ("parent"))     // Parent
      {
        Task before (*other);

        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        changes += deltaAppend (*other);
        changes += deltaTags (*other);
        changes += deltaAttributes (*other);
        changes += deltaSubstitutions (*other);

        if (taskDiff (before, *other))
        {
          // Only allow valid tasks.
          other->validate ();

          if (changes && permission.confirmed (before, taskDifferences (before, *other) + "Proceed with change?"))
          {
            context.tdb.update (*other);

            if (context.config.getBoolean ("echo.command"))
              out << "Appended '"
                  << context.task.get ("description")
                  << "' to task "
                  << other->id
                  << ".\n";

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
    out << "Appended " << count << " task" << (count == 1 ? ".\n" : "s.\n");

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
