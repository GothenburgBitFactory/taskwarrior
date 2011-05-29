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
#include <algorithm>
#include <stdlib.h>
#include <Context.h>
#include <Command.h>
#include <CmdCommands.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCompletionCommands::CmdCompletionCommands ()
{
  _keyword     = "_commands";
  _usage       = "task _commands";
  _description = "Generates a list of all commands, for autocompletion purposes";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionCommands::execute (const std::string&, std::string& output)
{
  // Get a list of all commands.
  std::vector <std::string> commands;

  std::map <std::string, Command*>::iterator command;
  for (command = context.commands.begin ();
       command != context.commands.end ();
       ++command)
  {
    commands.push_back (command->first);
  }

  // Sort alphabetically.
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  std::vector <std::string>::iterator c;
  for (c = commands.begin (); c != commands.end (); ++c)
    out << *c << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCommands::CmdZshCommands ()
{
  _keyword     = "_zshcommands";
  _usage       = "task _zshcommands";
  _description = "Generates a list of all commands, for zsh autocompletion purposes";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCommands::execute (const std::string&, std::string& output)
{
  // Get a list of all commands.
  std::vector <std::string> commands;

  std::map <std::string, Command*>::iterator command;
  for (command = context.commands.begin ();
       command != context.commands.end ();
       ++command)
  {
    commands.push_back (command->first);
  }

  // Sort alphabetically.
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  std::vector <std::string>::iterator c;
  for (c = commands.begin (); c != commands.end (); ++c)
    out << *c << ":" << context.commands[*c]->description () << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
