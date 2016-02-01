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
#include <CmdHelp.h>
#include <algorithm>
#include <ViewText.h>
#include <Context.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <iostream> // TODO Remove

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdHelp::CmdHelp ()
{
  _keyword               = "help";
  _usage                 = "task          help ['usage']";
  _description           = STRING_CMD_HELP_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHelp::execute (std::string& output)
{
  auto words = context.cli2.getWords ();
  if (words.size () == 1 && closeEnough ("usage", words[0]))
    output = "\n"
           + composeUsage ()
           + "\n";
  else
    output = "\n"
           + composeUsage ()
           + "\n"
           + STRING_CMD_HELP_TEXT;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdHelp::composeUsage () const
{
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string.left", ""));
  view.add (Column::factory ("string.left", ""));
  view.add (Column::factory ("string.left", ""));

  // Static first row.
  int row = view.addRow ();
  view.set (row, 0, STRING_CMD_HELP_USAGE_LABEL);
  view.set (row, 1, "task");
  view.set (row, 2, STRING_CMD_HELP_USAGE_DESC);

  // Obsolete method of getting a list of all commands.
  std::vector <std::string> all;
  for (auto& cmd : context.commands)
    all.push_back (cmd.first);

  // Sort alphabetically by usage.
  std::sort (all.begin (), all.end ());

  // Add the regular commands.
  for (auto& name : all)
  {
    if (name[0] != '_')
    {
      row = view.addRow ();
      view.set (row, 1, context.commands[name]->usage ());
      view.set (row, 2, context.commands[name]->description ());
    }
  }

  // Add the helper commands.
  for (auto& name : all)
  {
    if (name[0] == '_')
    {
      row = view.addRow ();
      view.set (row, 1, context.commands[name]->usage ());
      view.set (row, 2, context.commands[name]->description ());
    }
  }

  // Add the aliases commands.
  row = view.addRow ();
  view.set (row, 1, " ");

  for (auto& alias : context.config)
  {
    if (alias.first.substr (0, 6) == "alias.")
    {
      row = view.addRow ();
      view.set (row, 1, alias.first.substr (6));
      view.set (row, 2, format (STRING_CMD_HELP_ALIASED, alias.second));
    }
  }

  return view.render ();
}

////////////////////////////////////////////////////////////////////////////////
