////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include "Duration.h"
#include "Table.h"
#include "text.h"
#include "util.h"
#include "main.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

extern Context context;
static std::vector <std::string> customReports;

////////////////////////////////////////////////////////////////////////////////
// This report will eventually become the one report that many others morph into
// via the .taskrc file.
int handleCustomReport (const std::string& report, std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-custom-report-command") &&
      context.hooks.trigger (std::string ("pre-") + report + "-command"))
  {
    // Load report configuration.
    std::string reportColumns = context.config.get ("report." + report + ".columns");
    std::string reportLabels  = context.config.get ("report." + report + ".labels");
    std::string reportSort    = context.config.get ("report." + report + ".sort");
    std::string reportFilter  = context.config.get ("report." + report + ".filter");

    std::vector <std::string> columns;
    split (columns, reportColumns, ',');
    validReportColumns (columns);

    std::vector <std::string> labels;
    split (labels, reportLabels, ',');

    if (columns.size () != labels.size () && labels.size () != 0)
      throw std::string ("There are a different number of columns than labels ") +
            "for report '" + report + "'.";

    std::map <std::string, std::string> columnLabels;
    if (labels.size ())
      for (unsigned int i = 0; i < columns.size (); ++i)
        columnLabels[columns[i]] = labels[i];

    std::vector <std::string> sortOrder;
    split (sortOrder, reportSort, ',');
    validSortColumns (columns, sortOrder);

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

    Table table;
    table.setTableWidth (context.getWidth ());
    table.setDateFormat (context.config.get ("dateformat"));
    table.setReportName (report);

    foreach (task, tasks)
    {
      table.addRow ();
      context.hooks.trigger ("pre-display", *task);
    }

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

        char s[16];
        std::string value;
        int row = 0;
        foreach (task, tasks)
        {
          if (task->id != 0)
          {
            sprintf (s, "%d", (int) task->id);
            value = s;
          }
          else
          {
            value = "-";
          }

          context.hooks.trigger ("format-id", "id", value);
          table.addCell (row++, columnCount, value);
        }
      }

      else if (*col == "uuid")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "UUID");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        std::string value;
        int row = 0;
        foreach (task, tasks)
        {
          value = task->get ("uuid");
          context.hooks.trigger ("format-uuid", "uuid", value);
          table.addCell (row++, columnCount, value);
        }
      }

      else if (*col == "project")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Project");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        std::string value;
        int row = 0;
        foreach (task, tasks)
        {
          value = task->get ("project");
          context.hooks.trigger ("format-project", "project", value);
          table.addCell (row++, columnCount, value);
        }
      }

      else if (*col == "priority")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Pri");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        int row = 0;
        foreach (task, tasks)
        {
          std::string value = task->get ("priority");
          context.hooks.trigger ("format-priority", "priority", value);
          table.addCell (row++, columnCount, value);
        }
      }

      else if (*col == "priority_long")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Pri");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        int row = 0;
        std::string pri;
        foreach (task, tasks)
        {
          pri = task->get ("priority");

               if (pri == "H") pri = "High";   // TODO i18n
          else if (pri == "M") pri = "Medium"; // TODO i18n
          else if (pri == "L") pri = "Low";    // TODO i18n

          context.hooks.trigger ("format-priority_long", "priority", pri);
          table.addCell (row++, columnCount, pri);
        }
      }

      else if (*col == "entry" || *col == "entry_time")
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
            entered = dt.toString (context.config.get ("dateformat"));
            context.hooks.trigger ("format-entry", "entry", entered);
            table.addCell (row, columnCount, entered);
          }
        }
      }

      else if (*col == "start" || *col == "start_time")
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
            started = dt.toString (context.config.get ("dateformat"));
            context.hooks.trigger ("format-start", "start", started);
            table.addCell (row, columnCount, started);
          }
        }
      }

      else if (*col == "end" || *col == "end_time")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Completed");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        std::string ended;
        for (unsigned int row = 0; row < tasks.size(); ++row)
        {
          ended = tasks[row].get ("end");
          if (ended.length ())
          {
            Date dt (::atoi (ended.c_str ()));
            ended = dt.toString (context.config.get ("dateformat"));
            context.hooks.trigger ("format-end", "end", ended);
            table.addCell (row, columnCount, ended);
          }
        }
      }

      else if (*col == "due")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Due");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        std::string format = context.config.get ("report." + report + ".dateformat");
        if (format == "")
          format = context.config.get ("dateformat.report");
        if (format == "")
          format = context.config.get ("dateformat");

        int row = 0;
        std::string due;
        foreach (task, tasks)
        {
          std::string value = getDueDate (*task, format);
          context.hooks.trigger ("format-due", "due", value);
          table.addCell (row++, columnCount, value);
        }

        dueColumn = columnCount;
      }

      else if (*col == "countdown")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Countdown");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        std::string due;
        std::string countdown;
        Date now;
        for (unsigned int row = 0; row < tasks.size(); ++row)
        {
          due = tasks[row].get ("due");
          if (due.length ())
          {
            Date dt (::atoi (due.c_str ()));
            countdown = Duration (now - dt).format ();
            context.hooks.trigger ("format-countdown", "countdown", countdown);
            table.addCell (row, columnCount, countdown);
          }
        }
      }

      else if (*col == "countdown_compact")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Countdown");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        std::string due;
        std::string countdown;
        Date now;
        for (unsigned int row = 0; row < tasks.size(); ++row)
        {
          due = tasks[row].get ("due");
          if (due.length ())
          {
            Date dt (::atoi (due.c_str ()));
            countdown = Duration (now - dt).formatCompact ();
            context.hooks.trigger ("format-countdown_compact", "countdown_compact", countdown);
            table.addCell (row, columnCount, countdown);
          }
        }
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
            age = Duration (now - dt).format ();
            context.hooks.trigger ("format-age", "age", age);
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
            age = Duration (now - dt).formatCompact ();

            context.hooks.trigger ("format-age_compact", "age_compact", age);
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
          if (tasks[row].has ("start"))
            table.addCell (row, columnCount, context.config.get ("active.indicator"));
      }

      else if (*col == "tags")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Tags");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        int row = 0;
        std::vector <std::string> all;
        std::string tags;
        foreach (task, tasks)
        {
          task->getTags (all);
          join (tags, " ", all);
          context.hooks.trigger ("format-tags", "tags", tags);
          table.addCell (row++, columnCount, tags);
        }
      }

      else if (*col == "description_only")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Description");
        table.setColumnWidth (columnCount, Table::flexible);
        table.setColumnJustification (columnCount, Table::left);

        std::string desc;
        int row = 0;
        foreach (task, tasks)
        {
          desc = task->get ("description");
          context.hooks.trigger ("format-description_only", "description_only", desc);
          table.addCell (row++, columnCount, desc);
        }
      }

      else if (*col == "description")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Description");
        table.setColumnWidth (columnCount, Table::flexible);
        table.setColumnJustification (columnCount, Table::left);

        std::string desc;
        int row = 0;
        foreach (task, tasks)
        {
          desc = getFullDescription (*task, report);
          context.hooks.trigger ("format-description", "description", desc);
          table.addCell (row++, columnCount, desc);
        }
      }

      else if (*col == "recur")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Recur");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        for (unsigned int row = 0; row < tasks.size(); ++row)
        {
          std::string recur = tasks[row].get ("recur");
          if (recur != "")
          {
            context.hooks.trigger ("format-recur", "recur", recur);
            table.addCell (row, columnCount, recur);
          }
        }
      }

      else if (*col == "recurrence_indicator")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "R");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        for (unsigned int row = 0; row < tasks.size(); ++row)
          if (tasks[row].has ("recur"))
            table.addCell (row, columnCount, context.config.get ("recurrence.indicator"));
      }

      else if (*col == "tag_indicator")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "T");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        for (unsigned int row = 0; row < tasks.size(); ++row)
          if (tasks[row].getTagCount ())
            table.addCell (row, columnCount, context.config.get ("tag.indicator"));
      }

      else if (*col == "wait")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Wait");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::right);

        int row = 0;
        std::string wait;
        foreach (task, tasks)
        {
          wait = task->get ("wait");
          if (wait != "")
          {
            Date dt (::atoi (wait.c_str ()));
            wait = dt.toString (context.config.get ("dateformat"));
            context.hooks.trigger ("format-wait", "wait", wait);
            table.addCell (row++, columnCount, wait);
          }
        }
      }

      else if (*col == "depends")
      {
        table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Deps");
        table.setColumnWidth (columnCount, Table::minimum);
        table.setColumnJustification (columnCount, Table::left);

        int row = 0;
        std::vector <Task> blocked;
        std::vector <int> blocked_ids;
        std::string deps;
        foreach (task, tasks)
        {
          dependencyGetBlocking (*task, blocked);
          foreach (b, blocked)
            blocked_ids.push_back (b->id);

          join (deps, ",", blocked_ids);
          blocked_ids.clear ();
          blocked.clear ();

          context.hooks.trigger ("format-depends", "depends", deps);
          table.addCell (row++, columnCount, deps);
        }
      }

      // Common to all columns.
      // Add underline.
      if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
          context.config.getBoolean ("fontunderline"))
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

      // TODO This code should really be using Att::type.
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

      else if (column == "entry"      || column == "start"    || column == "wait"       ||
               column == "until"      || column == "end"      || column == "entry_time" ||
               column == "start_time" || column == "end_time")
        table.sortOn (columnIndex[column],
                      (direction == '+' ?
                        Table::ascendingDate :
                        Table::descendingDate));

      else if (column == "due")
        table.sortOn (columnIndex[column],
                      (direction == '+' ?
                        Table::ascendingDueDate :
                        Table::descendingDueDate));

      else if (column == "recur" || column == "age" || column == "age_compact")
        table.sortOn (columnIndex[column],
                      (direction == '+' ?
                        Table::ascendingPeriod :
                        Table::descendingPeriod));

      else if (column == "countdown" || column == "countdown_compact")
        table.sortOn (columnIndex[column],
                      (direction == '+' ?
                        Table::descendingPeriod :   // Yes, these are flipped.
                        Table::ascendingPeriod));   // Yes, these are flipped.

      else
        table.sortOn (columnIndex[column],
                      (direction == '+' ?
                        Table::ascendingCharacter :
                        Table::descendingCharacter));
    }

    // Now auto colorize all rows.
    if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
    {
      for (unsigned int row = 0; row < tasks.size (); ++row)
      {
        Color c (tasks[row].get ("fg") + " " + tasks[row].get ("bg"));
        autoColorize (tasks[row], c);
        table.setRowColor (row, c);
      }
    }

    // If an alternating row color is specified, notify the table.
    if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
    {
      Color alternate (context.config.get ("color.alternate"));
      if (alternate.nontrivial ())
        table.setTableAlternateColor (alternate);
    }

    // Report output can be limited by rows or lines.
    int maxrows = 0;
    int maxlines = 0;
    getLimits (report, maxrows, maxlines);

    // Adjust for fluff in the output.
    if (maxlines)
      maxlines -= (context.config.getBoolean ("blanklines") ? 2 : 0)
                + 1
                + context.headers.size ()
                + context.footnotes.size ();

    std::stringstream out;
    if (table.rowCount ())
    {
      out << optionalBlankLine ()
          << table.render (maxrows, maxlines)
          << optionalBlankLine ()
          << table.rowCount ()
          << (table.rowCount () == 1 ? " task" : " tasks");

      if (maxrows)
        out << ", " << maxrows << " shown";

      if (maxlines && maxlines < table.rowCount ())
        out << ", truncated to " << maxlines - 1 << " lines";

      out << std::endl;
    }
    else
    {
      out << "No matches."
        << std::endl;
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger (std::string ("post-") + report + "-command");
    context.hooks.trigger ("post-custom-report-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void validReportColumns (const std::vector <std::string>& columns)
{
  std::vector <std::string> bad;

  std::vector <std::string>::const_iterator it;
  for (it = columns.begin (); it != columns.end (); ++it)
    if (*it != "id"                   &&
        *it != "uuid"                 &&
        *it != "project"              &&
        *it != "priority"             &&
        *it != "priority_long"        &&
        *it != "entry"                &&
        *it != "entry_time"           &&  // TODO Deprecated
        *it != "start"                &&
        *it != "start_time"           &&  // TODO Deprecated
        *it != "end"                  &&
        *it != "end_time"             &&  // TODO Deprecated
        *it != "due"                  &&
        *it != "countdown"            &&
        *it != "countdown_compact"    &&
        *it != "age"                  &&
        *it != "age_compact"          &&
        *it != "active"               &&
        *it != "tags"                 &&
        *it != "recur"                &&
        *it != "recurrence_indicator" &&
        *it != "tag_indicator"        &&
        *it != "description_only"     &&
        *it != "description"          &&
        *it != "wait"                 &&
        *it != "depends")
      bad.push_back (*it);

  if (bad.size ())
  {
    std::string error;
    join (error, ", ", bad);
    throw std::string ("Unrecognized column name: ") + error + ".";
  }
}

////////////////////////////////////////////////////////////////////////////////
void validSortColumns (
  const std::vector <std::string>& columns,
  const std::vector <std::string>& sortColumns)
{
  std::vector <std::string> bad;
  std::vector <std::string>::const_iterator sc;
  for (sc = sortColumns.begin (); sc != sortColumns.end (); ++sc)
  {
    char direction = (*sc)[sc->length () - 1];
    if (direction != '-' && direction != '+')
      throw std::string ("Sort column '") +
            *sc +
            "' does not have a +/- ascending/descending indicator.";

    std::vector <std::string>::const_iterator co;
    for (co = columns.begin (); co != columns.end (); ++co)
      if (sc->substr (0, sc->length () - 1) == *co)
        break;

    if (co == columns.end ())
      bad.push_back (*sc);
  }

  if (bad.size ())
  {
    std::string error;
    join (error, ", ", bad);
    throw std::string ("Sort column is not part of the report: ") + error + ".";
  }
}

////////////////////////////////////////////////////////////////////////////////
// A value of zero mean unlimited.
// A value of 'page' means however many screen lines there are.
// A value of a positive integer is a row limit.
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
