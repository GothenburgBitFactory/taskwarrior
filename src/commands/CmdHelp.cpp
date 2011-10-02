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

#include <algorithm>
#include <CmdHelp.h>
#include <ViewText.h>
#include <Context.h>
#include <i18n.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdHelp::CmdHelp ()
{
  _keyword     = "help";
  _usage       = "task          help";
  _description = STRING_CMD_HELP_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHelp::execute (std::string& output)
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
  std::map <std::string, Command*>::iterator i;
  for (i = context.commands.begin (); i != context.commands.end (); ++i)
    all.push_back (i->first);

  // Sort alphabetically by usage.
  std::sort (all.begin (), all.end ());

  // Add the regular commands.
  std::vector <std::string>::iterator name;
  for (name = all.begin (); name != all.end (); ++name)
  {
    if ((*name)[0] != '_')
    {
      row = view.addRow ();
      view.set (row, 1, context.commands[*name]->usage ());
      view.set (row, 2, context.commands[*name]->description ());
    }
  }

  // Add the helper commands.
  for (name = all.begin (); name != all.end (); ++name)
  {
    if ((*name)[0] == '_')
    {
      row = view.addRow ();
      view.set (row, 1, context.commands[*name]->usage ());
      view.set (row, 2, context.commands[*name]->description ());
    }
  }

  // Add the aliases commands.
  row = view.addRow ();
  view.set (row, 1, " ");

  std::map <std::string, std::string>::iterator alias;
  for (alias =  context.aliases.begin ();
       alias != context.aliases.end ();
       ++alias)
  {
    row = view.addRow ();
    view.set (row, 1, alias->first);
    view.set (row, 2, format (STRING_CMD_HELP_ALIASED, alias->second));
  }

  output = "\n"
         + view.render ()
         + "\n"
         + STRING_CMD_HELP_TEXT;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
