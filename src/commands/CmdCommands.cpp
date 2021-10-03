////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <CmdCommands.h>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <Context.h>
#include <Table.h>
#include <Command.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdCommands::CmdCommands ()
{
  _keyword               = "commands";
  _usage                 = "task          commands";
  _description           = "Generates a list of all commands, with behavior details";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCommands::execute (std::string& output)
{
  Table view;
  view.width (Context::getContext ().getWidth ());
  view.add ("Command");
  view.add ("Category");
  view.add ("R/W",     false);
  view.add ("ID",      false);
  view.add ("GC",      false);
  view.add ("Context", false);
  view.add ("Filter",  false);
  view.add ("Mods",    false);
  view.add ("Misc",    false);
  view.add ("Description");
  view.leftMargin (Context::getContext ().config.getInteger ("indent.report"));
  view.extraPadding (Context::getContext ().config.getInteger ("row.padding"));
  view.intraPadding (Context::getContext ().config.getInteger ("column.padding"));
  setHeaderUnderline (view);

  for (auto& command : Context::getContext ().commands)
  {
    auto row = view.addRow ();
    view.set (row, 0, command.first);
    view.set (row, 1, Command::categoryNames.at (command.second->category ()));

    if (command.second->read_only ())
      view.set (row, 2, "RO");
    else
      view.set (row, 2, "RW");

    if (command.second->displays_id ())
      view.set (row, 3, "ID");

    if (command.second->needs_gc ())
      view.set (row, 4, "GC");

    if (command.second->uses_context ())
      view.set (row, 5, "Ctxt");

    if (command.second->accepts_filter ())
      view.set (row, 6, "Filt");

    if (command.second->accepts_modifications ())
      view.set (row, 7, "Mods");

    if (command.second->accepts_miscellaneous ())
      view.set (row, 8, "Misc");

    view.set (row, 9, command.second->description ());
  }

  output = optionalBlankLine ()
         + view.render ()
         + optionalBlankLine ()
         + '\n';

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionCommands::CmdCompletionCommands ()
{
  _keyword               = "_commands";
  _usage                 = "task          _commands";
  _description           = "Generates a list of all commands, for autocompletion purposes";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionCommands::execute (std::string& output)
{
  // Get a list of all commands.
  std::vector <std::string> commands;
  for (const auto& command : Context::getContext ().commands)
    commands.push_back (command.first);

  // Sort alphabetically.
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  for (const auto& c : commands)
    out << c << '\n';

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCommands::CmdZshCommands ()
{
  _keyword               = "_zshcommands";
  _usage                 = "task          _zshcommands";
  _description           = "Generates a list of all commands, for zsh autocompletion purposes";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
struct ZshCommand
{
  bool operator< (const struct ZshCommand&) const;

  Command::Category _category;
  std::string _command;
  std::string _description;
};

////////////////////////////////////////////////////////////////////////////////
bool ZshCommand::operator< (const struct ZshCommand& other) const
{
  // Lexicographical comparison.
  if (_category != other._category)
    return (_category < other._category);

  if (_command != other._command)
    return (_command < other._command);

  if (_description != other._description)
    return (_description < other._description);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCommands::execute (std::string& output)
{
  // Get a list of all command descriptions, sorted by category and then
  // alphabetically by command name.

  // Since not all supported compilers support tuples, we use a least common
  // denominator alternative: a custom struct type.

  std::vector <ZshCommand> commands;
  for (auto& command : Context::getContext ().commands)
  {
    ZshCommand zshCommand {command.second->category (),
                           command.first,
                           command.second->description ()};
    commands.push_back (zshCommand);
  }

  std::sort (commands.begin (), commands.end ());

  // Emit the commands in order.
  std::stringstream out;
  for (const auto& zc : commands)
    out << zc._command << ':'
        << Command::categoryNames.at (zc._category) << ':'
        << zc._description << '\n';

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
