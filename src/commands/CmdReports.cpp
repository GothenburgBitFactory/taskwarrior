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

#include <sstream>
#include <algorithm>
#include <Context.h>
#include <ViewText.h>
#include <text.h>
#include <CmdReports.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdReports::CmdReports ()
{
  _keyword     = "reports";
  _usage       = "task reports";
  _description = "Lists all supported reports.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdReports::execute (std::string& output)
{
  std::vector <std::string> reports;

  // Add custom reports.
  std::vector <std::string> vars;
  context.config.all (vars);

  std::vector <std::string>::iterator i;
  for (i = vars.begin (); i != vars.end (); ++i)
  {
    if (i->substr (0, 7) == "report.")
    {
      std::string report = i->substr (7);
      std::string::size_type columns = report.find (".columns");
      if (columns != std::string::npos)
        reports.push_back (report.substr (0, columns));
    }
  }

  // Add known reports.
  reports.push_back ("burndown.daily");
  reports.push_back ("burndown.monthly");
  reports.push_back ("burndown.weekly");
  reports.push_back ("ghistory.annual");
  reports.push_back ("ghistory.monthly");
  reports.push_back ("history.annual");
  reports.push_back ("history.monthly");
  reports.push_back ("information");
  reports.push_back ("projects");
  reports.push_back ("summary");
  reports.push_back ("tags");

  std::sort (reports.begin (), reports.end ());

  // Compose the output.
  std::stringstream out;
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string", "Report"));
  view.add (Column::factory ("string", "Description"));

  // If an alternating row color is specified, notify the table.
  if (context.color ())
  {
    Color alternate (context.config.get ("color.alternate"));
    view.colorOdd (alternate);
    view.intraColorOdd (alternate);
  }

  std::vector <std::string>::iterator report;
  for (report = reports.begin (); report != reports.end (); ++report)
  {
    int row = view.addRow ();
    view.set (row, 0, *report);
    view.set (row, 1, context.commands[*report]->description ());
  }

  out << optionalBlankLine ()
      << view.render ()
      << optionalBlankLine ()
      << reports.size ()
      << " reports"
      << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
