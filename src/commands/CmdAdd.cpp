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
#include <CmdAdd.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdAdd::CmdAdd ()
{
  _keyword     = "add";
  _usage       = "task add [tags] [attrs] desc...";
  _description = "Adds a new task.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAdd::execute (const std::string&, std::string& output)
{
  int rc = 0;
  std::stringstream out;

  context.task.set ("uuid", uuid ());
  context.task.setEntry ();

  // Recurring tasks get a special status.
  if (context.task.has ("due") &&
      context.task.has ("recur"))
  {
    context.task.setStatus (Task::recurring);
    context.task.set ("mask", "");
  }

  // Tasks with a wait: date get a special status.
  else if (context.task.has ("wait"))
    context.task.setStatus (Task::waiting);

  // By default, tasks are pending.
  else
    context.task.setStatus (Task::pending);

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
  std::vector <std::string>::iterator tag;
  for (tag = context.tagAdditions.begin ();
       tag != context.tagAdditions.end ();
       ++tag)
    context.task.addTag (*tag);

  // Must load pending to resolve dependencies, and to provide a new ID.
  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <Task> all;
  Filter none;
  context.tdb.loadPending (all, none);

  // Resolve dependencies.
  if (context.task.has ("depends"))
  {
    // Convert ID to UUID.
    std::vector <std::string> deps;
    split (deps, context.task.get ("depends"), ',');

    // Eliminate the ID-based set.
    context.task.set ("depends", "");

    std::vector <std::string>::iterator i;
    for (i = deps.begin (); i != deps.end (); i++)
    {
      int id = atoi (i->c_str ());
      if (id < 0)
        context.task.removeDependency (-id);
      else
        context.task.addDependency (id);
    }
  }

  // Only valid tasks can be added.
  context.task.validate ();

  context.tdb.add (context.task);

#ifdef FEATURE_NEW_ID
  out << "Created task " << context.tdb.nextId () << ".\n";
#endif

  context.footnote (onProjectChange (context.task));

  context.tdb.commit ();
  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
