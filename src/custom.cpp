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
#include <iomanip>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Context.h"
#include "Date.h"
#include "Table.h"
#include "text.h"
#include "util.h"
#include "main.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// This report will eventually become the one report that many others morph into
// via the .taskrc file.
std::string handleCustomReport (const std::string& report)
{
  // Load report configuration.
  std::string columnList = context.config.get ("report." + report + ".columns");
  std::vector <std::string> columns;
  split (columns, columnList, ',');
  validReportColumns (columns);

  std::string labelList = context.config.get ("report." + report + ".labels");
  std::vector <std::string> labels;
  split (labels, labelList, ',');

  if (columns.size () != labels.size () && labels.size () != 0)
    throw std::string ("There are a different number of columns than labels ") +
          "for report '" + report + "'.";

  std::map <std::string, std::string> columnLabels;
  if (labels.size ())
    for (unsigned int i = 0; i < columns.size (); ++i)
      columnLabels[columns[i]] = labels[i];

  std::string sortList   = context.config.get ("report." + report + ".sort");
  std::vector <std::string> sortOrder;
  split (sortOrder, sortList, ',');
  validSortColumns (columns, sortOrder);

  std::string filterList = context.config.get ("report." + report + ".filter");
  std::vector <std::string> filterArgs;
  split (filterArgs, filterList, ' ');

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  // TODO Include filter from custom report.
  context.tdb.load (tasks, context.filter);
  context.tdb.unlock ();

/*
  // Load all pending tasks.
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);

  // Apply filters.
  {
    std::string ignore;
    T filterTask;
    parse (filterArgs, ignore, filterTask);

    filter (tasks, filterTask);  // Filter from custom report
    filter (tasks, task);        // Filter from command line
  }
*/

  // Initialize colorization for subsequent auto colorization.
  initializeColorRules ();

  Table table;
  table.setTableWidth (context.getWidth ());
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

  for (unsigned int i = 0; i < tasks.size (); ++i)
    table.addRow ();

  int columnCount = 0;
  int dueColumn = -1;
  foreach (col, columns)
  {
    // Add each column individually.
    if (*col == "id")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "ID");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].id);
    }

    else if (*col == "uuid")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "UUID");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].get ("uuid"));
    }

    else if (*col == "project")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Project");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].get ("project"));
    }

    else if (*col == "priority")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Pri");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].get ("priority"));
    }

    else if (*col == "entry")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Added");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string entered;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        entered = tasks[row].get ("entry");
        if (entered.length ())
        {
          Date dt (::atoi (entered.c_str ()));
          entered = dt.toString (context.config.get ("dateformat", "m/d/Y"));
          table.addCell (row, columnCount, entered);
        }
      }
    }

    else if (*col == "start")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Started");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string started;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        started = tasks[row].get ("start");
        if (started.length ())
        {
          Date dt (::atoi (started.c_str ()));
          started = dt.toString (context.config.get ("dateformat", "m/d/Y"));
          table.addCell (row, columnCount, started);
        }
      }
    }

    else if (*col == "end")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Completed");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string started;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        started = tasks[row].get ("end");
        if (started.length ())
        {
          Date dt (::atoi (started.c_str ()));
          started = dt.toString (context.config.get ("dateformat", "m/d/Y"));
          table.addCell (row, columnCount, started);
        }
      }
    }

    else if (*col == "due")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Due");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string due;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        due = tasks[row].get ("due");
        if (due.length ())
        {
          Date dt (::atoi (due.c_str ()));
          due = dt.toString (context.config.get ("dateformat", "m/d/Y"));
          table.addCell (row, columnCount, due);
        }
      }

      dueColumn = columnCount;
    }

    else if (*col == "age")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Age");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string created;
      std::string age;
      Date now;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        created = tasks[row].get ("entry");
        if (created.length ())
        {
          Date dt (::atoi (created.c_str ()));
          age = formatSeconds ((time_t) (now - dt));
          table.addCell (row, columnCount, age);
        }
      }
    }

    else if (*col == "age_compact")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Age");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string created;
      std::string age;
      Date now;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        created = tasks[row].get ("entry");
        if (created.length ())
        {
          Date dt (::atoi (created.c_str ()));
          age = formatSecondsCompact ((time_t) (now - dt));
          table.addCell (row, columnCount, age);
        }
      }
    }

    else if (*col == "active")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Active");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        if (tasks[row].get ("start") != "")
          table.addCell (row, columnCount, "*");
    }

    else if (*col == "tags")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Tags");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      std::vector <std::string> all;
      std::string tags;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        tasks[row].getTags (all);
        join (tags, " ", all);
        table.addCell (row, columnCount, tags);
      }
    }

    else if (*col == "description_only")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Description");
      table.setColumnWidth (columnCount, Table::flexible);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].get ("description"));
    }

    else if (*col == "description")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Description");
      table.setColumnWidth (columnCount, Table::flexible);
      table.setColumnJustification (columnCount, Table::left);

      std::string description;
      std::string when;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        description = tasks[row].get ("description");

        std::vector <Att> annotations;
        tasks[row].getAnnotations (annotations);
        foreach (anno, annotations)
        {
          Date dt (::atoi (anno->name ().substr (11, std::string::npos).c_str ()));
          when = dt.toString (context.config.get ("dateformat", "m/d/Y"));
          description += "\n" + when + " " + anno->value ();
        }

        table.addCell (row, columnCount, description);
      }
    }

    else if (*col == "recur")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Recur");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      for (unsigned int row = 0; row < tasks.size (); ++row)
        table.addCell (row, columnCount, tasks[row].get ("recur"));
    }

    else if (*col == "recurrence_indicator")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "R");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      for (unsigned int row = 0; row < tasks.size (); ++row)
        table.addCell (row, columnCount,
                       tasks[row].get ("recur") != "" ? "R" : "");
    }

    else if (*col == "tag_indicator")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "T");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      for (unsigned int row = 0; row < tasks.size (); ++row)
        table.addCell (row, columnCount,
                       tasks[row].getTagCount () ? "+" : "");
    }

    // Common to all columns.
    // Add underline.
    if ((context.config.get (std::string ("color"), true) || context.config.get (std::string ("_forcecolor"), false)) &&
        context.config.get (std::string ("fontunderline"), "true"))
      table.setColumnUnderline (columnCount);
    else
      table.setTableDashedUnderline ();

    ++columnCount;
  }

  // Dynamically add sort criteria.
  // Build a map of column names -> index.
  std::map <std::string, unsigned int> columnIndex;
  for (unsigned int c = 0; c < columns.size (); ++c)
    columnIndex[columns[c]] = c;

  foreach (sortColumn, sortOrder)
  {
    // Separate column and direction.
    std::string column = sortColumn->substr (0, sortColumn->length () - 1);
    char direction = (*sortColumn)[sortColumn->length () - 1];

    if (column == "id")
      table.sortOn (columnIndex[column],
                    (direction == '+' ?
                      Table::ascendingNumeric :
                      Table::descendingNumeric));

    else if (column == "priority")
      table.sortOn (columnIndex[column],
                    (direction == '+' ?
                      Table::ascendingPriority :
                      Table::descendingPriority));

    else if (column == "entry" || column == "start" || column == "due")
      table.sortOn (columnIndex[column],
                    (direction == '+' ?
                      Table::ascendingDate :
                      Table::descendingDate));

    else if (column == "recur")
      table.sortOn (columnIndex[column],
                    (direction == '+' ?
                      Table::ascendingPeriod :
                      Table::descendingPeriod));

    else
      table.sortOn (columnIndex[column],
                    (direction == '+' ?
                      Table::ascendingCharacter :
                      Table::descendingCharacter));
  }

  // Now auto colorize all rows.
  std::string due;
  bool imminent;
  bool overdue;
  for (unsigned int row = 0; row < tasks.size (); ++row)
  {
    imminent = false;
    overdue  = false;
    due = tasks[row].get ("due");
    if (due.length ())
    {
      switch (getDueState (due))
      {
      case 2: overdue  = true; break;
      case 1: imminent = true; break;
      case 0:
      default:                 break;
      }
    }

    if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
    {
      Text::color fg = Text::colorCode (tasks[row].get ("fg"));
      Text::color bg = Text::colorCode (tasks[row].get ("bg"));
      autoColorize (tasks[row], fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (dueColumn != -1)
        {
          if (overdue)
            table.setCellFg (row, columnCount, Text::colorCode (context.config.get ("color.overdue", "red")));
          else if (imminent)
            table.setCellFg (row, columnCount, Text::colorCode (context.config.get ("color.due", "yellow")));
        }
      }
    }
  }

  // Limit the number of rows according to the report definition.
  int maximum = context.config.get (std::string ("report.") + report + ".limit", (int)0);

  // If the custom report has a defined limit, then allow an override, which
  // will show up as a single ID sequence.

  // If the custom report has a defined limit, then allow a numeric override.
  // This is an integer specified on the command line (task oldest 4), which is
  // parsed as an ID.
  if (context.config.get (std::string ("report.") + report + ".limit", (int)0) != 0)
    if (context.sequence.size () == 1)
      maximum = context.sequence[0];

  std::stringstream out;
  if (table.rowCount ())
    out << optionalBlankLine ()
        << table.render (maximum)
        << optionalBlankLine ()
        << table.rowCount ()
        << (table.rowCount () == 1 ? " task" : " tasks")
        << std::endl;
  else
    out << "No matches."
        << std::endl;

  return out.str ();
}

///////////////////////////////////////////////////////////////////////////////
