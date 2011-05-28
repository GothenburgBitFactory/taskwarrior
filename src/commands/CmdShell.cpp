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

#include <iostream>
#include <algorithm>
#include <Color.h>
#include <Context.h>
#include <text.h>
#include <CmdShell.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdShell::CmdShell ()
{
  _keyword     = "shell";
  _usage       = "task shell";
  _description = "Launches an interactive shell";
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdShell::execute (const std::string& command_line, std::string& output)
{
  // Display some kind of welcome message.
  Color bold (Color::nocolor, Color::nocolor, false, true, false);
  std::cout << (context.color () ? bold.colorize (PACKAGE_STRING) : PACKAGE_STRING)
            << " shell\n\n"
            << "Enter any task command (such as 'list'), or hit 'Enter'.\n"
            << "There is no need to include the 'task' command itself.\n"
            << "Enter 'quit' (or 'bye', 'exit') to end the session.\n\n";

  // Make a copy because context.clear will delete them.
  std::string permanentOverrides = " " + context.file_override
                                 + " " + context.var_overrides;

  std::vector <std::string> quit_commands;
  quit_commands.push_back ("quit");
  quit_commands.push_back ("exit");
  quit_commands.push_back ("bye");

  std::string command;
  bool keepGoing = true;

  do
  {
    std::cout << context.config.get ("shell.prompt") << " ";

    command = "";
    std::getline (std::cin, command);
    std::string decoratedCommand = trim (command + permanentOverrides);

    // When looking for the 'quit' command, use 'command', not
    // 'decoratedCommand'.
    if (std::find (quit_commands.begin (), quit_commands.end (), lowerCase (command)) != quit_commands.end ())
    {
      keepGoing = false;
    }
    else
    {
      try
      {
        context.clear ();
        std::vector <std::string> args;
        split (args, decoratedCommand, ' ');
        std::vector <std::string>::iterator arg;
        for (arg = args.begin (); arg != args.end (); ++arg)
          context.args.push_back (*arg);

        context.initialize (0, NULL);
        context.run ();
      }

      catch (std::string& error)
      {
        std::cout << error << "\n";
      }

      catch (...)
      {
        std::cerr << "Unknown error." << "\n";
      }
    }
  }
  while (keepGoing && !std::cin.eof ());

  // No need to repeat any overrides after the shell quits.
  context.clearMessages ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
