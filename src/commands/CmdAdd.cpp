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
  _usage       = "task add <modifications>";
  _description = STRING_CMD_ADD_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAdd::execute (std::string& output)
{
  int rc = 0;

  // Apply the command line modifications to the new task.
  Task task;
  modify_task_description_replace (task, context.a3.extract_modifications ());
  context.tdb2.add (task);

  // TODO This should be a call in to feedback.cpp.
  if (context.verbose ("new-id"))
    output = format (STRING_CMD_ADD_FEEDBACK, context.tdb2.next_id ()) + "\n";

  context.footnote (onProjectChange (task));

  context.tdb2.commit ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
