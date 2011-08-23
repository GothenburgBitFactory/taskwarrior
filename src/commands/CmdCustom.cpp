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
#include <map>
#include <vector>
#include <stdlib.h>
#include <Context.h>
#include <ViewTask.h>
#include <text.h>
#include <main.h>
#include <CmdCustom.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCustom::CmdCustom (
  const std::string& k,
  const std::string& u,
  const std::string& d)
{
  _keyword     = k;
  _usage       = u;
  _description = d;
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCustom::execute (std::string& output)
{
  int rc = 0;

  // Load report configuration.
  std::string reportColumns = context.config.get ("report." + _keyword + ".columns");
  std::string reportLabels  = context.config.get ("report." + _keyword + ".labels");
  std::string reportSort    = context.config.get ("report." + _keyword + ".sort");
  std::string reportFilter  = context.config.get ("report." + _keyword + ".filter");

  std::vector <std::string> columns;
  split (columns, reportColumns, ',');
  validateReportColumns (columns);

  std::vector <std::string> labels;
  split (labels, reportLabels, ',');

  if (columns.size () != labels.size () && labels.size () != 0)
    throw std::string ("There are different numbers of columns and labels ") +
          "for report '" + _keyword + "'.";

  std::map <std::string, std::string> columnLabels;
  if (labels.size ())
    for (unsigned int i = 0; i < columns.size (); ++i)
      columnLabels[columns[i]] = labels[i];

  std::vector <std::string> sortOrder;
  split (sortOrder, reportSort, ',');
  validateSortColumns (sortOrder);

  // Prepend the argument list with those from the report filter.
  std::vector <std::string> filterArgs;
  split (filterArgs, reportFilter, ' ');
  std::vector <std::string>::iterator arg;
  for (arg = filterArgs.begin (); arg != filterArgs.end (); ++ arg)
    context.a3.capture_first (*arg);

  context.a3.categorize ();
  context.a3.dump ("CmdCustom::execute");

  // Load the data.
  handleRecurrence ();
  std::vector <Task> filtered;
  filter (filtered);

  // Sort the tasks.
  std::vector <int> sequence;
  for (unsigned int i = 0; i < filtered.size (); ++i)
    sequence.push_back (i);

  sort_tasks (filtered, sequence, reportSort);

  // Configure the view.
  ViewTask view;
  view.width (context.getWidth ());
  view.leftMargin (context.config.getInteger ("indent.report"));
  view.extraPadding (context.config.getInteger ("row.padding"));
  view.intraPadding (context.config.getInteger ("column.padding"));

  Color label (context.config.get ("color.label"));
  view.colorHeader (label);

  Color alternate (context.config.get ("color.alternate"));
  view.colorOdd (alternate);
  view.intraColorOdd (alternate);

  // Add the columns and labels.
  for (unsigned int i = 0; i < columns.size (); ++i)
  {
    Column* c = Column::factory (columns[i], _keyword);
    if (i < labels.size ())
      c->setLabel (labels[i]);
    view.add (c);
  }

  // How many lines taken up by table header?
  // TODO Consider rc.verbose
  int table_header;
  if (context.color () && context.config.getBoolean ("fontunderline"))
    table_header = 1;  // Underlining doesn't use extra line.
  else
    table_header = 2;  // Dashes use an extra line.

  // Report output can be limited by rows or lines.
  int maxrows = 0;
  int maxlines = 0;
  getLimits (_keyword, maxrows, maxlines);

  // Adjust for fluff in the output.
  // TODO Consider rc.verbose
  if (maxlines)
    maxlines -= (context.verbose ("blank") ? 1 : 0)
              + table_header
              + context.footnotes.size ()
              + 1;  // "X tasks shown ..."

  // Render.
  // TODO Consider rc.verbose
  std::stringstream out;
  if (filtered.size ())
  {
    view.truncateRows (maxrows);
    view.truncateLines (maxlines);

    out << optionalBlankLine ()
        << view.render (filtered, sequence)
        << optionalBlankLine ()
        << filtered.size ()
        << (filtered.size () == 1 ? " task" : " tasks");

    if (maxrows && maxrows < (int)filtered.size ())
      out << ", " << maxrows << " shown";

    if (maxlines && maxlines < (int)filtered.size ())
      out << ", truncated to " << maxlines - table_header << " lines";

    out << "\n";
  }
  else
  {
    out << "No matches."
        << std::endl;
    rc = 1;
  }

  context.tdb2.commit ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void CmdCustom::validateReportColumns (std::vector <std::string>& columns)
{
  // One-time initialization, on demand.
  static std::map <std::string, std::string> legacyMap;
  if (! legacyMap.size ())
  {
    legacyMap["priority_long"]        = "priority.long";        // Deprecated.
    legacyMap["entry_time"]           = "entry";                // Deprecated.
    legacyMap["start_time"]           = "start";                // Deprecated.
    legacyMap["end_time"]             = "end";                  // Deprecated.
    legacyMap["countdown"]            = "due.countdown";        // Deprecated.
    legacyMap["countdown_compact"]    = "due.countdown";        // Deprecated.
    legacyMap["age"]                  = "entry.age";            // Deprecated.
    legacyMap["age_compact"]          = "entry.age";            // Deprecated.
    legacyMap["active"]               = "start.active";         // Deprecated.
    legacyMap["recurrence_indicator"] = "recur.indicator";      // Deprecated.
    legacyMap["tag_indicator"]        = "tags.indicator";       // Deprecated.
    legacyMap["description_only"]     = "description.desc";     // Deprecated.
  }

  std::vector <std::string>::iterator i;
  for (i = columns.begin (); i != columns.end (); ++i)
  {
    // If a legacy column was used, complain about it, but modify it anyway.
    std::map <std::string, std::string>::iterator found = legacyMap.find (*i);
    if (found != legacyMap.end ())
    {
      context.footnote (std::string ("Deprecated report field '")
                                   + *i
                                   + "' used.  Please modify this to '"
                                   + found->second
                                   + "'.");
      *i = found->second;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CmdCustom::validateSortColumns (std::vector <std::string>& columns)
{
  // One-time initialization, on demand.
  static std::map <std::string, std::string> legacyMap;
  if (! legacyMap.size ())
  {
    legacyMap["priority_long"]        = "priority";             // Deprecated
    legacyMap["entry_time"]           = "entry";                // Deprecated
    legacyMap["start_time"]           = "start";                // Deprecated
    legacyMap["end_time"]             = "end";                  // Deprecated
    legacyMap["countdown"]            = "due";                  // Deprecated
    legacyMap["countdown_compact"]    = "due";                  // Deprecated
    legacyMap["age"]                  = "entry";                // Deprecated
    legacyMap["age_compact"]          = "entry";                // Deprecated
    legacyMap["active"]               = "start";                // Deprecated
    legacyMap["recurrence_indicator"] = "recur";                // Deprecated
    legacyMap["tag_indicator"]        = "tags";                 // Deprecated
    legacyMap["description_only"]     = "description";          // Deprecated
  }

  std::vector <std::string>::iterator i;
  for (i = columns.begin (); i != columns.end (); ++i)
  {
    // If a legacy column was used, complain about it, but modify it anyway.
    std::map <std::string, std::string>::iterator found = legacyMap.find (*i);
    if (found != legacyMap.end ())
    {
      context.footnote (std::string ("Deprecated sort field '")
                                   + *i
                                   + "' used.  Please modify this to '"
                                   + found->second
                                   + "'.");
      *i = found->second;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// A value of zero mean unlimited.
// A value of 'page' means however many screen lines there are.
// A value of a positive integer is a row/task limit.
void CmdCustom::getLimits (const std::string& report, int& rows, int& lines)
{
  rows = 0;
  lines = 0;

  int screenheight = 0;

  // If a report has a stated limit, use it.
  if (report != "")
  {
    std::string name = "report." + report + ".limit";
    if (context.config.get (name) == "page")
      lines = screenheight = context.getHeight ();
    else
      rows = context.config.getInteger (name);
  }

  // If the custom report has a defined limit, then allow a numeric override.
  // This is an integer specified as a filter (limit:10).
  std::string limit = context.a3.find_limit ();
  if (limit != "")
  {
    if (limit == "page")
    {
      if (screenheight == 0)
        screenheight = context.getHeight ();

      rows = 0;
      lines = screenheight;
    }
    else
    {
      rows = (int) strtol (limit.c_str (), NULL, 10);
      lines = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
