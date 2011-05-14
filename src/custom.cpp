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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include <Context.h>
#include <Date.h>
#include <Duration.h>
#include <ViewTask.h>
#include <text.h>
#include <util.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
void validateReportColumns (std::vector <std::string>& columns)
{
  // One-time initialization, on demand.
  static std::map <std::string, std::string> legacyMap;
  if (! legacyMap.size ())
  {
    legacyMap["priority_long"]        = "priority.long";
    legacyMap["entry_time"]           = "entry";
    legacyMap["start_time"]           = "start";
    legacyMap["end_time"]             = "end";
    legacyMap["countdown"]            = "due.countdown";
    legacyMap["countdown_compact"]    = "due.countdown";
    legacyMap["age"]                  = "entry.age";
    legacyMap["age_compact"]          = "entry.age";
    legacyMap["active"]               = "start.active";
    legacyMap["recurrence_indicator"] = "recur.indicator";
    legacyMap["tag_indicator"]        = "tags.indicator";
    legacyMap["description_only"]     = "description.desc";
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
void validateSortColumns (std::vector <std::string>& columns)
{
  // One-time initialization, on demand.
  static std::map <std::string, std::string> legacyMap;
  if (! legacyMap.size ())
  {
    legacyMap["priority_long"]        = "priority";
    legacyMap["entry_time"]           = "entry";
    legacyMap["start_time"]           = "start";
    legacyMap["end_time"]             = "end";
    legacyMap["countdown"]            = "due";
    legacyMap["countdown_compact"]    = "due";
    legacyMap["age"]                  = "entry";
    legacyMap["age_compact"]          = "entry";
    legacyMap["active"]               = "start";
    legacyMap["recurrence_indicator"] = "recur";
    legacyMap["tag_indicator"]        = "tags";
    legacyMap["description_only"]     = "description";
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
void getLimits (const std::string& report, int& rows, int& lines)
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
  if (context.task.has ("limit"))
  {
    if (context.task.get ("limit") == "page")
    {
      if (screenheight == 0)
        screenheight = context.getHeight ();

      rows = 0;
      lines = screenheight;
    }
    else
    {
      rows = atoi (context.task.get ("limit").c_str ());
      lines = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
int handleCustomReport (const std::string& report, std::string& outs)
{
  int rc = 0;

  // Load report configuration.
  std::string reportColumns = context.config.get ("report." + report + ".columns");
  std::string reportLabels  = context.config.get ("report." + report + ".labels");
  std::string reportSort    = context.config.get ("report." + report + ".sort");
  std::string reportFilter  = context.config.get ("report." + report + ".filter");

  std::vector <std::string> columns;
  split (columns, reportColumns, ',');
  validateReportColumns (columns);

  std::vector <std::string> labels;
  split (labels, reportLabels, ',');

  if (columns.size () != labels.size () && labels.size () != 0)
    throw std::string ("There are a different number of columns and labels ") +
          "for report '" + report + "'.";

  std::map <std::string, std::string> columnLabels;
  if (labels.size ())
    for (unsigned int i = 0; i < columns.size (); ++i)
      columnLabels[columns[i]] = labels[i];

  std::vector <std::string> sortOrder;
  split (sortOrder, reportSort, ',');
  validateSortColumns (sortOrder);

  // Apply rc overrides.
  std::vector <std::string> filterArgs;
  std::vector <std::string> filteredArgs;
  split (filterArgs, reportFilter, ' ');
  context.applyOverrides (filterArgs, filteredArgs);

  {
    Cmd cmd (report);
    Task task;
    Sequence sequence;
    Subst subst;
    Filter filter;
    context.parse (filteredArgs, cmd, task, sequence, subst, filter);

    context.sequence.combine (sequence);

    // Special case: Allow limit to be overridden by the command line.
    if (!context.task.has ("limit") && task.has ("limit"))
      context.task.set ("limit", task.get ("limit"));

    foreach (att, filter)
      context.filter.push_back (*att);
  }

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Filter sequence.
  if (context.sequence.size ())
    context.filter.applySequence (tasks, context.sequence);

  // Sort the tasks.
  std::vector <int> sequence;
  for (int i = 0; i < tasks.size (); ++i)
    sequence.push_back (i);

  sort_tasks (tasks, sequence, reportSort);

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

  // Add the columns.
  std::vector <std::string>::iterator it;
  for (it = columns.begin (); it != columns.end (); ++it)
    view.add (Column::factory (*it, report));

  // How many lines taken up by table header?
  int table_header;
  if (context.color () && context.config.getBoolean ("fontunderline"))
    table_header = 1;  // Underlining doesn't use extra line.
  else
    table_header = 2;  // Dashes use an extra line.

  // Report output can be limited by rows or lines.
  int maxrows = 0;
  int maxlines = 0;
  getLimits (report, maxrows, maxlines);

  // Adjust for fluff in the output.
  if (maxlines)
    maxlines -= (context.verbose ("blank") ? 1 : 0)
              + table_header
              + context.headers.size ()
              + context.footnotes.size ();

  std::stringstream out;
  if (tasks.size ())
  {
    view.truncateRows (maxrows);
    view.truncateLines (maxlines);

    out << optionalBlankLine ()
        << view.render (tasks, sequence)
        << optionalBlankLine ()
        << tasks.size ()
        << (tasks.size () == 1 ? " task" : " tasks");

    if (maxrows && maxrows < tasks.size ())
      out << ", " << maxrows << " shown";

    if (maxlines && maxlines < tasks.size ())
      out << ", truncated to " << maxlines - table_header << " lines";

    out << "\n";
  }
  else
  {
    out << "No matches."
      << std::endl;
    rc = 1;
  }

  outs = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
