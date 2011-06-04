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
#include <text.h>
#include <util.h>
#include <main.h>
#include <CmdDuplicate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDuplicate::CmdDuplicate ()
{
  _keyword     = "dulicate";
  _usage       = "task duplicate ID [tags] [attrs] [desc...]";
  _description = "Duplicates the specified task, and allows modifications.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDuplicate::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;
  int count = 0;

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
    Task dup (*task);
    dup.set ("uuid", uuid ());  // Needs a new UUID.
    dup.setStatus (Task::pending);
    dup.remove ("start");   // Does not inherit start date.
    dup.remove ("end");     // Does not inherit end date.

    // Recurring tasks are duplicated and downgraded to regular tasks.
    if (task->getStatus () == Task::recurring)
    {
      dup.remove ("parent");
      dup.remove ("recur");
      dup.remove ("until");
      dup.remove ("imak");
      dup.remove ("imask");

      out << "Note: task "
          << task->id
          << " was a recurring task.  The new task is not.\n";
    }

    // Apply deltas.
    deltaDescription (dup);
    deltaTags (dup);
    deltaAttributes (dup);
    deltaSubstitutions (dup);

    // A New task needs a new entry time.
    char entryTime[16];
    sprintf (entryTime, "%u", (unsigned int) time (NULL));
    dup.set ("entry", entryTime);

    // Only allow valid tasks.
    dup.validate ();

    context.tdb.add (dup);

    if (context.config.getBoolean ("echo.command"))
      out << "Duplicated "
          << task->id
          << " '"
          << task->get ("description")
          << "'.\n";

    context.footnote (onProjectChange (dup));

    ++count;
  }

  if (tasks.size () == 0)
  {
    out << "No matches.\n";
    rc = 1;
  }
  else if (context.config.getBoolean ("echo.command"))
  {
#ifdef FEATURE_NEW_ID
    // All this, just for an id number.
    std::vector <Task> all;
    Filter none;
    context.tdb.loadPending (all, none);
    out << "Created task " << context.tdb.nextId () << ".\n";
#endif
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
