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

#include <algorithm>
#include <CmdHelp.h>
#include <ViewText.h>
#include <Context.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdHelp::CmdHelp ()
{
  _keyword     = "help";
  _usage       = "task help";
  _description = "Displays this usage help text";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHelp::execute (const std::string&, std::string& output)
{
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string.left", ""));
  view.add (Column::factory ("string.left", ""));
  view.add (Column::factory ("string.left", ""));

  // Static first row.
  int row = view.addRow ();
  view.set (row, 0, "Usage:");
  view.set (row, 1, "task");

  // Obsolete method of getting a list of all commands.
  std::vector <std::string> all;
  std::map <std::string, Command*>::iterator i;
  for (i = context.commands.begin (); i != context.commands.end (); ++i)
    all.push_back (i->first);

  // Sort alphabetically by usage.
  std::sort (all.begin (), all.end ());

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

  for (name = all.begin (); name != all.end (); ++name)
  {
    if ((*name)[0] == '_')
    {
      row = view.addRow ();
      view.set (row, 1, context.commands[*name]->usage ());
      view.set (row, 2, context.commands[*name]->description ());
    }
  }

/*
  row = view.addRow ();
  view.set (row, 1, "task ID [tags] [attrs] [desc...]");
  view.set (row, 2, "Modifies the existing task with provided arguments.");

  row = view.addRow ();
  view.set (row, 1, "task ID /from/to/g");
  view.set (row, 2, "Performs substitution on the task description and "
                         "annotations.  The 'g' is optional, and causes "
                         "substitutions for all matching text, not just the "
                         "first occurrence.");

  row = view.addRow ();
  view.set (row, 1, "task ID");
  view.set (row, 2, "Specifying an ID without a command invokes the 'info' command.");

  row = view.addRow ();
  view.set (row, 1, "task merge URL");
  view.set (row, 2, "Merges the specified undo.data file with the local data files.");

  row = view.addRow ();
  view.set (row, 1, "task push URL");
  view.set (row, 2, "Pushes the local *.data files to the URL.");

  row = view.addRow ();
  view.set (row, 1, "task pull URL");
  view.set (row, 2, "Overwrites the local *.data files with those found at the URL.");
*/

  output = "\n"
         + view.render ()
         + "\n"
         + "Documentation for taskwarrior can be found using 'man task', "
           "'man taskrc', 'man task-tutorial', 'man task-color', 'man task-faq' "
           "or at http://taskwarrior.org"
         + "\n"
         + "\n"
         + "ID is the numeric identifier displayed by the 'task list' command. "
           "You can specify multiple IDs for task commands, and multiple tasks "
           "will be affected.  To specify multiple IDs make sure you use one "
           "of these forms:\n"
           "  task delete 1,2,3\n"
           "  task info 1-3\n"
           "  task pri:H 1,2-5,19\n"
           "\n"
           "Tags are arbitrary words, any quantity:\n"
           "  +tag               The + means add the tag\n"
           "  -tag               The - means remove the tag\n"
           "\n"
           "Attributes are:\n"
           "  project:           Project name\n"
           "  priority:          Priority\n"
           "  due:               Due date\n"
           "  recur:             Recurrence frequency\n"
           "  until:             Recurrence end date\n"
           "  fg:                Foreground color\n"
           "  bg:                Background color\n"
           "  limit:             Desired number of rows in report, or 'page'\n"
           "  wait:              Date until task becomes pending\n"
           "\n"
           "Attribute modifiers improve filters.  Supported modifiers are:\n"
           "  before     (synonyms under, below)\n"
           "  after      (synonyms over, above)\n"
           "  none\n"
           "  any\n"
           "  is         (synonym equals)\n"
           "  isnt       (synonym not)\n"
           "  has        (synonym contains)\n"
           "  hasnt\n"
           "  startswith (synonym left)\n"
           "  endswith   (synonym right)\n"
           "  word\n"
           "  noword\n"
           "\n"
           "  For example:\n"
           "    task list due.before:eom priority.not:L\n"
           "\n"
           "  Modifiers can be inverted with the ~ character:\n"
           "    project.~is  is equivalent to project.isnt\n"
           "\n"
           "The default .taskrc file can be overridden with:\n"
           "  task rc:<alternate file> ...\n"
           "\n"
           "The values in .taskrc (or alternate) can be overridden with:\n"
           "  task ... rc.<name>:<value>\n"
           "\n"
           "Any command or attribute name may be abbreviated if still unique:\n"
           "  task list project:Home\n"
           "  task li       pro:Home\n"
           "\n"
           "Some task descriptions need to be escaped because of the shell:\n"
           "  task add \"quoted ' quote\"\n"
           "  task add escaped \\' quote\n"
           "\n"
           "The argument -- tells taskwarrior to treat all other args as description.\n"
           "  task add -- project:Home needs scheduling\n"
           "\n"
           "Many characters have special meaning to the shell, including:\n"
           "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~\n"
           "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
