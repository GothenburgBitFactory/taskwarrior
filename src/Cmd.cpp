////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include "Cmd.h"
#include "Context.h"
#include "util.h"
#include "text.h"
#include "i18n.h"

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
bool Cmd::valid (const std::string& input)
{
  loadCommands ();
  loadCustomReports ();

  std::string candidate = lowerCase (input);

  std::vector <std::string> matches;
  autoComplete (candidate, commands, matches);
  if (0 == matches.size ())
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Determines whether the string represents a valid custom report name.
bool Cmd::validCustom (const std::string& input)
{
  loadCustomReports ();

  std::vector <std::string> matches;
  autoComplete (lowerCase (input), customReports, matches);
  return matches.size () == 1 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void Cmd::parse (const std::string& input)
{
  loadCommands ();
  loadCustomReports ();

  std::string candidate = lowerCase (input);

  std::vector <std::string> matches;
  autoComplete (candidate, commands, matches);
  if (1 == matches.size ())
    command = matches[0];

  else if (0 == matches.size ())
    command = "";

  else
  {
    std::string error = "Ambiguous command '" + candidate + "' - could be either of "; // TODO i18n

    std::string combined;
    join (combined, ", ", matches);
    error += combined;

    throw error + combined;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Cmd::loadCommands ()
{
  if (commands.size () == 0)
  {
    commands.push_back (context.stringtable.get (CMD_ACTIVE,    "active"));
    commands.push_back (context.stringtable.get (CMD_ADD,       "add"));
    commands.push_back (context.stringtable.get (CMD_APPEND,    "append"));
    commands.push_back (context.stringtable.get (CMD_ANNOTATE,  "annotate"));
    commands.push_back (context.stringtable.get (CMD_CALENDAR,  "calendar"));
    commands.push_back (context.stringtable.get (CMD_COLORS,    "colors"));
    commands.push_back (context.stringtable.get (CMD_COMPLETED, "completed"));
    commands.push_back (context.stringtable.get (CMD_DELETE,    "delete"));
    commands.push_back (context.stringtable.get (CMD_DONE,      "done"));
    commands.push_back (context.stringtable.get (CMD_DUPLICATE, "duplicate"));
    commands.push_back (context.stringtable.get (CMD_EDIT,      "edit"));
    commands.push_back (context.stringtable.get (CMD_EXPORT,    "export"));
    commands.push_back (context.stringtable.get (CMD_HELP,      "help"));
    commands.push_back (context.stringtable.get (CMD_HISTORY,   "history"));
    commands.push_back (context.stringtable.get (CMD_GHISTORY,  "ghistory"));
    commands.push_back (context.stringtable.get (CMD_IMPORT,    "import"));
    commands.push_back (context.stringtable.get (CMD_INFO,      "info"));
    commands.push_back (context.stringtable.get (CMD_NEXT,      "next"));
    commands.push_back (context.stringtable.get (CMD_OVERDUE,   "overdue"));
    commands.push_back (context.stringtable.get (CMD_PROJECTS,  "projects"));
    commands.push_back (context.stringtable.get (CMD_START,     "start"));
    commands.push_back (context.stringtable.get (CMD_STATS,     "statistics"));
    commands.push_back (context.stringtable.get (CMD_STOP,      "stop"));
    commands.push_back (context.stringtable.get (CMD_SUMMARY,   "summary"));
    commands.push_back (context.stringtable.get (CMD_TAGS,      "tags"));
    commands.push_back (context.stringtable.get (CMD_TIMESHEET, "timesheet"));
    commands.push_back (context.stringtable.get (CMD_UNDELETE,  "undelete"));
    commands.push_back (context.stringtable.get (CMD_UNDO,      "undo"));
    commands.push_back (context.stringtable.get (CMD_VERSION,   "version"));
  }
}

////////////////////////////////////////////////////////////////////////////////
void Cmd::loadCustomReports ()
{
  if (customReports.size () == 0)
  {
    std::vector <std::string> all;
    context.config.all (all);

    foreach (i, all)
    {
      if (i->substr (0, 7) == "report.")
      {
        std::string report = i->substr (7, std::string::npos);
        std::string::size_type columns = report.find (".columns");
        if (columns != std::string::npos)
        {
          report = report.substr (0, columns);

          // A custom report is also a command.
          customReports.push_back (report);
          commands.push_back (report);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
