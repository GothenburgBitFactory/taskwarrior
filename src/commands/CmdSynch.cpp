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

#include <iostream>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <CmdSynch.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdSynch::CmdSynch ()
{
  _keyword     = "synchronize";
  _usage       = "task          synchronize";
  _description = STRING_CMD_SYNCH_USAGE;
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdSynch::execute (std::string& output)
{
  // TODO Tempporary.
  std::cout << "\n"
            << "Task Server Synchronization is not implemented in 2.0.0beta3.\n"
            << "\n";

  // If no server is set up, quit.
  std::string connection = context.config.get ("taskd.server");
  if (connection == "" ||
      connection.find (':') == std::string::npos)
    throw std::string (STRING_CMD_SYNCH_NO_SERVER);

  // Obtain credentials.
  std::string credentials = context.config.get ("taskd.credentials");

  // TODO Obtain synch key.

  // TODO Compose backlog into ticket.
  // TODO Request synch.

  // TODO Receive synch data.
  // TODO Extract remote mods.
  // TODO Extract new synch key.
  // TODO Apply remote mods.
  // TODO Store new synch key.
  // TODO Truncate backlog.

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
