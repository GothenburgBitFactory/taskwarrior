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
#include <stdlib.h>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <util.h>
#include <main.h>
#include <CmdAdd.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdAdd::CmdAdd ()
{
  _keyword     = "add";
  _usage       = "task add [tags] [attrs] desc...";
  _description = STRING_CMD_ADD_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAdd::execute (std::string& output)
{
  int rc = 0;

  // Must load pending to resolve dependencies, and to provide a new ID.
  context.tdb.lock (context.config.getBoolean ("locking"));

  std::vector <Task> all;
  context.tdb.loadPending (all);

  // Every task needs a UUID.
  Task task;
  task.set ("uuid", uuid ());

  // Apply the command line modifications to the new task.
  Arguments modifications = context.args.extract_modifications ();
  modify_task (task, modifications);
  apply_defaults (task);

  // Only valid tasks can be added.
  task.validate ();

  context.tdb.add (task);

  std::stringstream out;
  // TODO This should be a call in to feedback.cpp.
#ifdef FEATURE_NEW_ID
  out << "Created task " << context.tdb.nextId () << ".\n";
#endif

  context.footnote (onProjectChange (task));

  context.tdb.commit ();
  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
