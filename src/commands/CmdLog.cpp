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
#include <CmdLog.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdLog::CmdLog ()
{
  _keyword     = "log";
  _usage       = "task log [tags] [attrs] desc...";
  _description = "Adds a new task that is already completed.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdLog::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  context.task.setStatus (Task::completed);
  context.task.set ("uuid", uuid ());
  context.task.setEntry ();

  // Add an end date.
  char entryTime[16];
  sprintf (entryTime, "%u", (unsigned int) time (NULL));
  context.task.set ("end", entryTime);

  // Recurring tasks get a special status.
  if (context.task.has ("recur"))
    throw std::string ("You cannot log recurring tasks.");

  if (context.task.has ("wait"))
    throw std::string ("You cannot log waiting tasks.");

  // It makes no sense to add dependencies to an already-completed task.
  if (context.task.get ("depends") != "")
    throw std::string ("You cannot specify dependencies on a completed task.");

  // Override with default.project, if not specified.
  if (context.task.get ("project") == "")
    context.task.set ("project", context.config.get ("default.project"));

  // Override with default.priority, if not specified.
  if (context.task.get ("priority") == "")
  {
    std::string defaultPriority = context.config.get ("default.priority");
    if (Att::validNameValue ("priority", "", defaultPriority))
      context.task.set ("priority", defaultPriority);
  }

  // Override with default.due, if not specified.
  if (context.task.get ("due") == "")
  {
    std::string defaultDue = context.config.get ("default.due");
    if (defaultDue != "" &&
        Att::validNameValue ("due", "", defaultDue))
      context.task.set ("due", defaultDue);
  }

  // Include tags.
  foreach (tag, context.tagAdditions)
    context.task.addTag (*tag);

  // Only valid tasks can be added.
  context.task.validate ();

  context.tdb.lock (context.config.getBoolean ("locking"));
  context.tdb.add (context.task);
  context.tdb.commit ();

  if (context.config.getBoolean ("echo.command"))
    out << "Logged task.\n";

  context.footnote (onProjectChange (context.task));
  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
