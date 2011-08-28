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

#include <stdlib.h>
#include <Context.h>
#include <i18n.h>
#include <CmdExec.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdExec::CmdExec ()
{
  _keyword     = "execute";
  _usage       = "task execute <external command>";
  _description = STRING_CMD_EXEC_USAGE;
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdExec::execute (std::string& output)
{
  std::string command_line;
  std::vector <Arg>::iterator arg;
  for (arg = context.a3.begin (); arg != context.a3.end (); ++arg)
  {
    if (arg != context.a3.begin () &&
        arg->_raw != "execute")
    {
      if (command_line.length ())
        command_line += " ";

      command_line += arg->_raw;
    }
  }

  return system (command_line.c_str ());
}

////////////////////////////////////////////////////////////////////////////////
