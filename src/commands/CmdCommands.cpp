////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <CmdCommands.h>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <Command.h>
#include <ColString.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCommands::CmdCommands ()
{
  _keyword               = "commands";
  _usage                 = "task          commands";
  _description           = STRING_CMD_COMMANDS_USAGE;
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
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string",       STRING_COLUMN_LABEL_COMMAND));
  view.add (Column::factory ("string",       STRING_COLUMN_LABEL_CATEGORY));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_RO));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_SHOWS_ID));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_GC));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_CONTEXT));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_FILTER));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_MODS));
  view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_MISC));
  view.add (Column::factory ("string.left",  STRING_COLUMN_LABEL_DESC));

  if (context.color ())
  {
    Color label (context.config.get ("color.label"));
    view.colorHeader (label);

    Color alternate (context.config.get ("color.alternate"));
    view.colorOdd (alternate);
    view.intraColorOdd (alternate);
  }

  view.leftMargin (context.config.getInteger ("indent.report"));
  view.extraPadding (context.config.getInteger ("row.padding"));
  view.intraPadding (context.config.getInteger ("column.padding"));

  for (auto& command : context.commands)
  {
    int row = view.addRow ();
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
         + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionCommands::CmdCompletionCommands ()
{
  _keyword               = "_commands";
  _usage                 = "task          _commands";
  _description           = STRING_CMD_HCOMMANDS_USAGE;
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

  for (auto& command : context.commands)
    commands.push_back (command.first);

  // Sort alphabetically.
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  for (auto& c : commands)
    out << c << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCommands::CmdZshCommands ()
{
  _keyword               = "_zshcommands";
  _usage                 = "task          _zshcommands";
  _description           = STRING_CMD_ZSHCOMMANDS_USAGE;
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
  for (auto& command : context.commands)
  {
    ZshCommand zshCommand {command.second->category (),
                           command.first,
                           command.second->description ()};
    commands.push_back (zshCommand);
  }

  std::sort (commands.begin (), commands.end ());

  // Emit the commands in order.
  std::stringstream out;
  for (auto& zc : commands)
    out << zc._command << ":"
        << Command::categoryNames.at (zc._category) << ":"
        << zc._description << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
