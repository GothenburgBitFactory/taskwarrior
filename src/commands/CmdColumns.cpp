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
#include <Context.h>
#include <ViewText.h>
#include <Color.h>
#include <text.h>
#include <i18n.h>
#include <main.h>
#include <CmdColumns.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdColumns::CmdColumns ()
{
  _keyword     = "columns";
  _usage       = "task columns";
  _description = STRING_CMD_COLUMNS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdColumns::execute (std::string& output)
{
  // Include all columns in the table.
  std::vector <std::string> names;
  std::map <std::string, Column*>::const_iterator col;
  for (col = context.columns.begin (); col != context.columns.end (); ++col)
    names.push_back (col->first);

  std::sort (names.begin (), names.end ());

  // Render a list of column names, formats and examples.
  ViewText formats;
  formats.width (context.getWidth ());
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_COLUMN));
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_STYLES));
  formats.add (Column::factory ("string", STRING_COLUMN_LABEL_EXAMPLES));

  Color alternate (context.config.get ("color.alternate"));
  formats.colorOdd (alternate);
  formats.intraColorOdd (alternate);

  std::vector <std::string>::iterator name;
  for (name = names.begin (); name != names.end (); ++name)
  {
    const std::vector <std::string> styles   = context.columns[*name]->styles ();
    const std::vector <std::string> examples = context.columns[*name]->examples ();

    for (unsigned int i = 0; i < styles.size (); ++i)
    {
      int row = formats.addRow ();
      formats.set (row, 0, i == 0 ? *name : "");
      formats.set (row, 1, styles[i] + (i == 0 ? "*" : ""));
      formats.set (row, 2, i < examples.size () ? examples[i] : "");
    }
  }

  output = optionalBlankLine ()
         + formats.render ()
         + "\n"
         + STRING_CMD_COLUMNS_NOTE
         + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionColumns::CmdCompletionColumns ()
{
  _keyword     = "_columns";
  _usage       = "task _columns";
  _description = STRING_CMD_COLUMNS_USAGE2;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionColumns::execute (std::string& output)
{
  // Include all columns.
  std::vector <std::string> names;
  std::map <std::string, Column*>::const_iterator col;
  for (col = context.columns.begin (); col != context.columns.end (); ++col)
    names.push_back (col->first);

  std::sort (names.begin (), names.end ());

  // Render only the column names.
  std::vector <std::string>::iterator name;
  for (name = names.begin (); name != names.end (); ++name)
    output += *name + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
