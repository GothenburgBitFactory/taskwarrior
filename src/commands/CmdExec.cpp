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

#include <stdlib.h>
#include <CmdExec.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdExec::CmdExec ()
: _external_command ("")
{
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
bool CmdExec::implements (const std::string& command_line)
{
  if (context.args.size () > 1 &&
      (context.args[0] == "exec" ||
       context.args[0] == "exe"  ||
       context.args[0] == "ex"))
  {
    for (int i = 1; i < context.args.size (); ++i)
    {
      if (i > 1)
        _external_command += " ";

      _external_command += context.args[i];
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdExec::execute (const std::string&, std::string&)
{
  system (_external_command.c_str ());
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
