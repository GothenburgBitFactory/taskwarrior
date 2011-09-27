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
#include <algorithm>
#include <Color.h>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <CmdShell.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdShell::CmdShell ()
{
  _keyword     = "shell";
  _usage       = "task shell";
  _description = STRING_CMD_SHELL_USAGE;
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdShell::execute (std::string&)
{
  // Display some kind of welcome message.
  Color bold (Color::nocolor, Color::nocolor, false, true, false);
  std::cout << (context.color () ? bold.colorize (PACKAGE_STRING) : PACKAGE_STRING)
            << " shell\n\n"
            << STRING_CMD_SHELL_HELP1 << "\n"
            << STRING_CMD_SHELL_HELP2 << "\n"
            << STRING_CMD_SHELL_HELP3 << "\n\n";

  // Make a copy because context.clear will delete them.
  std::string permanent_overrides;
  std::vector <Arg>::iterator i;
  for (i = context.a3.begin (); i != context.a3.end (); ++i)
  {
    if (i->_category == Arg::cat_rc ||
        i->_category == Arg::cat_override)
    {
      if (i != context.a3.begin ())
        permanent_overrides += " ";

      permanent_overrides += i->_raw;
    }
  }

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
    std::string decoratedCommand = "task " + trim (command + permanent_overrides);

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
          context.a3.capture (*arg);

        context.initialize (0, NULL);
        context.run ();
      }

      catch (std::string& error)
      {
        std::cout << error << "\n";
      }

      catch (...)
      {
        std::cerr << STRING_UNKNOWN_ERROR << "\n";
      }
    }
  }
  while (keepGoing && !std::cin.eof ());

  // No need to repeat any overrides after the shell quits.
  context.clearMessages ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
