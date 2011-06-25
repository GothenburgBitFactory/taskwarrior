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
#include <i18n.h>
#include <CmdUndo.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdUndo::CmdUndo ()
{
  _keyword     = "undo";
  _usage       = "task undo";
  _description = STRING_CMD_UNDO_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUndo::execute (std::string& output)
{
  context.disallowModification ();

  context.tdb.lock (context.config.getBoolean ("locking"));
  context.tdb.undo ();
  context.tdb.unlock ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
