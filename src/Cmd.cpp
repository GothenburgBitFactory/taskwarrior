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
#include <Cmd.h>
#include <Context.h>
#include <util.h>
#include <text.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Cmd::Cmd ()
: command ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Cmd::Cmd (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Cmd::~Cmd ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Determines whether the string represents a unique command name or custom
// report name.
//
// To be a valid command:
//   1. 'input' should autocomplete to one of 'commands'.
bool Cmd::valid (const std::string& input)
{
  load ();

  std::vector <std::string> matches;
  autoComplete (lowerCase (input), commands, matches);
  if (matches.size () == 1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Determines whether the string represents a valid custom report name.
//
// To be a valid custom command:
//   1. 'input' should autocomplete to one of 'commands'.
//   2. the result, canonicalized, should autocomplete to one of
//      'customreports'.
bool Cmd::validCustom (const std::string& input)
{
  load ();

  std::vector <std::string> matches;
  autoComplete (lowerCase (input), commands, matches);
  if (matches.size () == 1)
  {
/*
    std::string canonical = context.canonicalize (matches[0]);
    matches.clear ();
    autoComplete (canonical, customReports, matches);
    if (matches.size () == 1)
*/
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// To be a valid custom command:
//   1. 'input' should autocomplete to one of 'commands'.
//   2. the result may then canonicalize to another command.
void Cmd::parse (const std::string& input)
{
  load ();

  std::vector <std::string> matches;
  autoComplete (input, commands, matches);
  if (1 == matches.size ())
    /*command = context.canonicalize (matches[0])*/;

  else if (0 == matches.size ())
    command = "";

  else
  {
    std::string error = "Ambiguous command '" + input + "' - could be either of "; // TODO i18n

    std::sort (matches.begin (), matches.end ());
    std::string combined;
    join (combined, ", ", matches);
    throw error + combined;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Cmd::load ()
{
  if (commands.size () == 0)
  {
    commands.push_back ("_config");
    commands.push_back ("_query");
    commands.push_back ("export.csv");
    commands.push_back ("export.ical");
    commands.push_back ("export.yaml");
    commands.push_back ("history.monthly");
    commands.push_back ("history.annual");
    commands.push_back ("ghistory.monthly");
    commands.push_back ("ghistory.annual");
    commands.push_back ("burndown.daily");
    commands.push_back ("burndown.weekly");
    commands.push_back ("burndown.monthly");
    commands.push_back ("add");
    commands.push_back ("annotate");
    commands.push_back ("denotate");
    commands.push_back ("calendar");
    commands.push_back ("colors");
    commands.push_back ("config");
    commands.push_back ("delete");
    commands.push_back ("done");
    commands.push_back ("duplicate");
    commands.push_back ("import");
    commands.push_back ("log");
    commands.push_back ("start");
    commands.push_back ("stop");
    commands.push_back ("summary");
    commands.push_back ("timesheet");
    commands.push_back ("undo");
    commands.push_back ("merge");
    commands.push_back ("push");
    commands.push_back ("pull");

    // Now load the custom reports.
    std::vector <std::string> all;
    context.config.all (all);

    std::vector <std::string>::iterator i;
    for (i = all.begin (); i != all.end (); ++i)
    {
      if (i->substr (0, 7) == "report.")
      {
        std::string report = i->substr (7);
        std::string::size_type columns = report.find (".columns");
        if (columns != std::string::npos)
        {
          report = report.substr (0, columns);

          // Make sure a custom report does not clash with a built-in
          // command.
          if (std::find (commands.begin (), commands.end (), report) != commands.end ())
            throw std::string ("Custom report '") + report +
                  "' conflicts with built-in task command.";

          // A custom report is also a command.
          customReports.push_back (report);
          commands.push_back (report);
        }
      }
    }

    // Now load the aliases.
    std::map <std::string, std::string>::iterator it;
    for (it = context.aliases.begin (); it != context.aliases.end (); ++it)
      commands.push_back (it->first);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Cmd::allCustomReports (std::vector <std::string>& all) const
{
  all = customReports;
}

////////////////////////////////////////////////////////////////////////////////
void Cmd::allCommands (std::vector <std::string>& all) const
{
  all.clear ();
  std::vector <std::string>::const_iterator c;
  for (c = commands.begin (); c != commands.end (); ++c)
    if (c->substr (0, 1) != "_")
      all.push_back (*c);
}

////////////////////////////////////////////////////////////////////////////////
// Commands that do not directly modify the data files.
bool Cmd::isReadOnlyCommand ()
{
  if (command == "_config"                                                   ||
      command == "_query"                                                    ||
      command == "export.csv"                                                ||
      command == "export.ical"                                               ||
      command == "export.yaml"                                               ||
      command == "history.monthly"                                           ||
      command == "history.annual"                                            ||
      command == "ghistory.monthly"                                          ||
      command == "ghistory.annual"                                           ||
      command == "burndown.daily"                                            ||
      command == "burndown.weekly"                                           ||
      command == "burndown.monthly"                                          ||
      command == "calendar"                                                  ||
      command == "colors"                                                    ||
      command == "config"                                                    ||
			command == "push"                                                      ||
      command == "summary"                                                   ||
      command == "timesheet"                                                 ||
      validCustom (command))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Commands that directly modify the data files.
bool Cmd::isWriteCommand ()
{
  if (command == "merge"     ||
      command == "add"       ||
      command == "annotate"  ||
      command == "denotate"  ||
      command == "delete"    ||
      command == "done"      ||
      command == "duplicate" ||
      command == "import"    ||
      command == "log"       ||
      command == "pull"      ||
      command == "start"     ||
      command == "stop"      ||
      command == "undo")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
