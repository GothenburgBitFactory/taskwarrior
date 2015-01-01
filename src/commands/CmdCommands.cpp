////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <Context.h>
#include <Command.h>
#include <CmdCommands.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCompletionCommands::CmdCompletionCommands ()
{
  _keyword     = "_commands";
  _usage       = "task          _commands";
  _description = STRING_CMD_HCOMMANDS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionCommands::execute (std::string& output)
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
  _usage       = "task          _zshcommands";
  _description = STRING_CMD_ZSHCOMMANDS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCommands::execute (std::string& output)
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
