////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include <algorithm>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Context.h"
#include "Directory.h"
#include "File.h"
#include "Date.h"
#include "Duration.h"
#include "Table.h"
#include "text.h"
#include "util.h"
#include "main.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

static void countTasks (const std::vector <Task>&, const std::string&, const std::string&, int&, int&);

extern Context context;

////////////////////////////////////////////////////////////////////////////////
int shortUsage (std::string& outs)
{
  Table table;

  table.addColumn (" ");
  table.addColumn (" ");
  table.addColumn (" ");

  table.setColumnJustification (0, Table::left);
  table.setColumnJustification (1, Table::left);
  table.setColumnJustification (2, Table::left);

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::flexible);
  table.setTableWidth (context.getWidth ());
  table.setDateFormat (context.config.get ("dateformat"));

  int row = table.addRow ();
  table.addCell (row, 0, "Usage:");
  table.addCell (row, 1, "task");

  row = table.addRow ();
  table.addCell (row, 1, "task add [tags] [attrs] desc...");
  table.addCell (row, 2, "Adds a new task.");

  row = table.addRow ();
  table.addCell (row, 1, "task log [tags] [attrs] desc...");
  table.addCell (row, 2, "Adds a new task that is already completed.");

  row = table.addRow ();
  table.addCell (row, 1, "task append ID [tags] [attrs] desc...");
  table.addCell (row, 2, "Appends more description to an existing task.");

  row = table.addRow ();
  table.addCell (row, 1, "task prepend ID [tags] [attrs] desc...");
  table.addCell (row, 2, "Prepends more description to an existing task.");

  row = table.addRow ();
  table.addCell (row, 1, "task annotate ID desc...");
  table.addCell (row, 2, "Adds an annotation to an existing task.");

  row = table.addRow ();
  table.addCell (row, 1, "task denotate ID desc...");
  table.addCell (row, 2, "Deletes an annotation of an existing task.");

  row = table.addRow ();
  table.addCell (row, 1, "task ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Modifies the existing task with provided arguments.");

  row = table.addRow ();
  table.addCell (row, 1, "task ID /from/to/g");
  table.addCell (row, 2, "Performs substitution on the task description and "
                         "annotations.  The 'g' is optional, and causes "
                         "substitutions for all matching text, not just the "
                         "first occurrence.");

  row = table.addRow ();
  table.addCell (row, 1, "task ID");
  table.addCell (row, 2, "Specifying an ID without a command invokes the 'info' command.");

  row = table.addRow ();
  table.addCell (row, 1, "task edit ID");
  table.addCell (row, 2, "Launches an editor to let you modify all aspects of a task directly, therefore it is to be used carefully.");

  row = table.addRow ();
  table.addCell (row, 1, "task undo");
  table.addCell (row, 2, "Reverts the most recent action.");

#ifdef FEATURE_SHELL
  row = table.addRow ();
  table.addCell (row, 1, "task shell");
  table.addCell (row, 2, "Launches an interactive shell.");
#endif

  row = table.addRow ();
  table.addCell (row, 1, "task duplicate ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Duplicates the specified task, and allows modifications.");

  row = table.addRow ();
  table.addCell (row, 1, "task delete ID");
  table.addCell (row, 2, "Deletes the specified task.");

  row = table.addRow ();
  table.addCell (row, 1, "task info ID");
  table.addCell (row, 2, "Shows all data, metadata for specified task.");

  row = table.addRow ();
  table.addCell (row, 1, "task start ID");
  table.addCell (row, 2, "Marks specified task as started.");

  row = table.addRow ();
  table.addCell (row, 1, "task stop ID");
  table.addCell (row, 2, "Removes the 'start' time from a task.");

  row = table.addRow ();
  table.addCell (row, 1, "task done ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Marks the specified task as completed.");

  row = table.addRow ();
  table.addCell (row, 1, "task projects");
  table.addCell (row, 2, "Shows a list of all project names used, and how many tasks are in each.");

  row = table.addRow ();
  table.addCell (row, 1, "task tags");
  table.addCell (row, 2, "Shows a list of all tags used.");

  row = table.addRow ();
  table.addCell (row, 1, "task summary");
  table.addCell (row, 2, "Shows a report of task status by project.");

  row = table.addRow ();
  table.addCell (row, 1, "task timesheet [weeks]");
  table.addCell (row, 2, "Shows a weekly report of tasks completed and started.");

  row = table.addRow ();
  table.addCell (row, 1, "task history");
  table.addCell (row, 2, "Shows a report of task history, by month.  Alias to history.monthly.");

  row = table.addRow ();
  table.addCell (row, 1, "task history.annual");
  table.addCell (row, 2, "Shows a report of task history, by year.");

  row = table.addRow ();
  table.addCell (row, 1, "task ghistory");
  table.addCell (row, 2, "Shows a graphical report of task history, by month.  Alias to ghistory.monthly.");

  row = table.addRow ();
  table.addCell (row, 1, "task ghistory.annual");
  table.addCell (row, 2, "Shows a graphical report of task history, by year.");

  row = table.addRow ();
  table.addCell (row, 1, "task burndown.daily");
  table.addCell (row, 2, "Shows a graphical burndown chart, by day.");

  row = table.addRow ();
  table.addCell (row, 1, "task burndown.weekly");
  table.addCell (row, 2, "Shows a graphical burndown chart, by week.");

  row = table.addRow ();
  table.addCell (row, 1, "task burndown.monthly");
  table.addCell (row, 2, "Shows a graphical burndown chart, by month.");

  row = table.addRow ();
  table.addCell (row, 1, "task calendar [due|month year|year]");
  table.addCell (row, 2, "Shows a calendar, with due tasks marked.");

  row = table.addRow ();
  table.addCell (row, 1, "task stats");
  table.addCell (row, 2, "Shows task database statistics.");

  row = table.addRow ();
  table.addCell (row, 1, "task import");
  table.addCell (row, 2, "Imports tasks from a variety of formats.");

  row = table.addRow ();
  table.addCell (row, 1, "task export");
  table.addCell (row, 2, "Lists all tasks in CSV format.  Alias to export.csv");

  row = table.addRow ();
  table.addCell (row, 1, "task export.csv");
  table.addCell (row, 2, "Lists all tasks in CSV format.");

  row = table.addRow ();
  table.addCell (row, 1, "task export.ical");
  table.addCell (row, 2, "Lists all tasks in iCalendar format.");

  row = table.addRow ();
  table.addCell (row, 1, "task export.yaml");
  table.addCell (row, 2, "Lists all tasks in YAML format.");

  row = table.addRow ();
  table.addCell (row, 1, "task merge URL");
  table.addCell (row, 2, "Merges the specified undo.data file with the local data files.");

  row = table.addRow ();
  table.addCell (row, 1, "task push URL");
  table.addCell (row, 2, "Pushes the local *.data files to the URL.");

	row = table.addRow ();
  table.addCell (row, 1, "task pull URL");
  table.addCell (row, 2, "Overwrites the local *.data files with those found at the URL.");

  row = table.addRow ();
  table.addCell (row, 1, "task color [sample | legend]");
  table.addCell (row, 2, "Displays all possible colors, a named sample, or a "
                         "legend containing all currently defined colors.");

  row = table.addRow ();
  table.addCell (row, 1, "task count [filter]");
  table.addCell (row, 2, "Shows only the number of matching tasks.");

  row = table.addRow ();
  table.addCell (row, 1, "task version");
  table.addCell (row, 2, "Shows the task version number.");

  row = table.addRow ();
  table.addCell (row, 1, "task show [all | substring]");
  table.addCell (row, 2, "Shows the entire task configuration variables or the ones containing substring.");

  row = table.addRow ();
  table.addCell (row, 1, "task config [name [value | '']]");
  table.addCell (row, 2, "Add, modify and remove settings in the task configuration.");

  row = table.addRow ();
  table.addCell (row, 1, "task diagnostics");
  table.addCell (row, 2, "Information needed when reporting a problem.");

  row = table.addRow ();
  table.addCell (row, 1, "task help");
  table.addCell (row, 2, "Shows the long usage text.");

  // Add custom reports here...
  std::vector <std::string> all;
  context.cmd.allCustomReports (all);
  foreach (report, all)
  {
    std::string command = std::string ("task ") + *report + std::string (" [tags] [attrs] desc...");
    std::string description = context.config.get (std::string ("report.") + *report + ".description");
    if (description == "")
      description = "(missing description)";

    row = table.addRow ();
    table.addCell (row, 1, command);
    table.addCell (row, 2, description);
  }

  std::stringstream out;
  out << table.render ()
      << "\n"
      << "Documentation for taskwarrior can be found using 'man task', "
      << "'man taskrc', 'man task-tutorial', 'man task-color', 'man task-faq' "
      << "or at http://taskwarrior.org"
      << "\n"
      << "\n";

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int longUsage (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-usage-command"))
  {
    std::string shortUsageString;
    std::stringstream out;

    (void)shortUsage(shortUsageString);

    out << shortUsageString
        << "ID is the numeric identifier displayed by the 'task list' command. "
        << "You can specify multiple IDs for task commands, and multiple tasks "
        << "will be affected.  To specify multiple IDs make sure you use one "
        << "of these forms:"                                                    << "\n"
        << "  task delete 1,2,3"                                                << "\n"
        << "  task info 1-3"                                                    << "\n"
        << "  task pri:H 1,2-5,19"                                              << "\n"
        <<                                                                         "\n"
        << "Tags are arbitrary words, any quantity:"                            << "\n"
        << "  +tag               The + means add the tag"                       << "\n"
        << "  -tag               The - means remove the tag"                    << "\n"
        <<                                                                         "\n"
        << "Attributes are:"                                                    << "\n"
        << "  project:           Project name"                                  << "\n"
        << "  priority:          Priority"                                      << "\n"
        << "  due:               Due date"                                      << "\n"
        << "  recur:             Recurrence frequency"                          << "\n"
        << "  until:             Recurrence end date"                           << "\n"
        << "  fg:                Foreground color"                              << "\n"
        << "  bg:                Background color"                              << "\n"
        << "  limit:             Desired number of rows in report, or 'page'"   << "\n"
        << "  wait:              Date until task becomes pending"               << "\n"
        <<                                                                         "\n"
        << "Attribute modifiers improve filters.  Supported modifiers are:"     << "\n"
        << "  before     (synonyms under, below)"                               << "\n"
        << "  after      (synonyms over, above)"                                << "\n"
        << "  none"                                                             << "\n"
        << "  any"                                                              << "\n"
        << "  is         (synonym equals)"                                      << "\n"
        << "  isnt       (synonym not)"                                         << "\n"
        << "  has        (synonym contain)"                                     << "\n"
        << "  hasnt"                                                            << "\n"
        << "  startswith (synonym left)"                                        << "\n"
        << "  endswith   (synonym right)"                                       << "\n"
        << "  word"                                                             << "\n"
        << "  noword"                                                           << "\n"
        <<                                                                         "\n"
        << "  For example:"                                                     << "\n"
        << "    task list due.before:eom priority.not:L"                        << "\n"
        <<                                                                         "\n"
        << "The default .taskrc file can be overridden with:"                   << "\n"
        << "  task rc:<alternate file> ..."                                     << "\n"
        <<                                                                         "\n"
        << "The values in .taskrc (or alternate) can be overridden with:"       << "\n"
        << "  task ... rc.<name>:<value>"                                       << "\n"
        <<                                                                         "\n"
        << "Any command or attribute name may be abbreviated if still unique:"  << "\n"
        << "  task list project:Home"                                           << "\n"
        << "  task li       pro:Home"                                           << "\n"
        <<                                                                         "\n"
        << "Some task descriptions need to be escaped because of the shell:"    << "\n"
        << "  task add \"quoted ' quote\""                                      << "\n"
        << "  task add escaped \\' quote"                                       << "\n"
        <<                                                                         "\n"
        << "The argument -- tells taskwarrior to treat all other args as description." << "\n"
        << "  task add -- project:Home needs scheduling"                        << "\n"
        <<                                                                         "\n"
        << "Many characters have special meaning to the shell, including:"      << "\n"
        << "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~"                    << "\n"
        << "\n";

    outs = out.str();
    context.hooks.trigger ("post-usage-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// Display all information for the given task.
int handleInfo (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-info-command"))
  {
    // Get all the tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.loadPending (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    // Filter sequence.
    context.filter.applySequence (tasks, context.sequence);

    // Find the task.
    std::stringstream out;
    foreach (task, tasks)
    {
      Table table;
      table.setTableWidth (context.getWidth ());
      table.setDateFormat (context.config.get ("dateformat"));

      table.addColumn ("Name");
      table.addColumn ("Value");

      if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
          context.config.getBoolean ("fontunderline"))
      {
        table.setColumnUnderline (0);
        table.setColumnUnderline (1);
      }
      else
        table.setTableDashedUnderline ();

      table.setColumnWidth (0, Table::minimum);
      table.setColumnWidth (1, Table::flexible);

      table.setColumnJustification (0, Table::left);
      table.setColumnJustification (1, Table::left);
      Date now;

      context.hooks.trigger ("pre-display", *task);

      // id
      char svalue[12];
      std::string value;
      int row = table.addRow ();
      table.addCell (row, 0, "ID");
      sprintf (svalue, "%d", (int) task->id);
      value = svalue;
      context.hooks.trigger ("format-id", "id", value);
      table.addCell (row, 1, value);

      std::string status = ucFirst (Task::statusToText (task->getStatus ()));

      // description
      row = table.addRow ();
      table.addCell (row, 0, "Description");
      table.addCell (row, 1, getFullDescription (*task, "info"));

      Color c;
      autoColorize (*task, c);
      table.setCellColor (row, 1, c);

      // status
      row = table.addRow ();
      table.addCell (row, 0, "Status");
      table.addCell (row, 1, status);

      // project
      if (task->has ("project"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Project");
        table.addCell (row, 1, task->get ("project"));
      }

      // priority
      if (task->has ("priority"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Priority");
        value = task->get ("priority");
        context.hooks.trigger ("format-priority", "priority", value);
        table.addCell (row, 1, value);
      }

      // dependencies: blocked
      {
        std::vector <Task> blocked;
        dependencyGetBlocking (*task, blocked);
        if (blocked.size ())
        {
          std::stringstream message;
          std::vector <Task>::const_iterator it;
          for (it = blocked.begin (); it != blocked.end (); ++it)
            message << it->id << " " << it->get ("description") << "\n";

          row = table.addRow ();
          table.addCell (row, 0, "This task blocked by");
          table.addCell (row, 1, message.str ());
        }
      }

      // dependencies: blocking
      {
        std::vector <Task> blocking;
        dependencyGetBlocked (*task, blocking);
        if (blocking.size ())
        {
          std::stringstream message;
          std::vector <Task>::const_iterator it;
          for (it = blocking.begin (); it != blocking.end (); ++it)
            message << it->id << " " << it->get ("description") << "\n";

          row = table.addRow ();
          table.addCell (row, 0, "This task is blocking");
          table.addCell (row, 1, message.str ());
        }
      }

      // recur
      if (task->has ("recur"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Recurrence");
        value = task->get ("recur");
        context.hooks.trigger ("format-recur", "recur", value);
        table.addCell (row, 1, value);
      }

      // until
      if (task->has ("until"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Recur until");

        Date dt (atoi (task->get ("until").c_str ()));
        std::string format = context.config.get ("reportdateformat");
        if (format == "")
          format = context.config.get ("dateformat");

        std::string until = getDueDate (*task, format);
        table.addCell (row, 1, until);
      }

      // mask
      if (task->getStatus () == Task::recurring)
      {
        row = table.addRow ();
        table.addCell (row, 0, "Mask");
        table.addCell (row, 1, task->get ("mask"));
      }

      if (task->has ("parent"))
      {
        // parent
        row = table.addRow ();
        table.addCell (row, 0, "Parent task");
        table.addCell (row, 1, task->get ("parent"));

        // imask
        row = table.addRow ();
        table.addCell (row, 0, "Mask Index");
        table.addCell (row, 1, task->get ("imask"));
      }

      // due (colored)
      if (task->has ("due"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Due");

        std::string format = context.config.get ("reportdateformat");
        if (format == "")
          format = context.config.get ("dateformat");

        value = getDueDate (*task, format);
        context.hooks.trigger ("format-due", "due", value);
        table.addCell (row, 1, value);
      }

      // wait
      if (task->has ("wait"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Waiting until");
        Date dt (atoi (task->get ("wait").c_str ()));
        value = dt.toString (context.config.get ("dateformat"));
        context.hooks.trigger ("format-wait", "wait", value);
        table.addCell (row, 1, value);
      }

      // start
      if (task->has ("start"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Start");
        Date dt (atoi (task->get ("start").c_str ()));

        value = dt.toString (context.config.get ("dateformat"));
        context.hooks.trigger ("format-due", "due", value);
        table.addCell (row, 1, value);
      }

      // end
      if (task->has ("end"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "End");
        Date dt (atoi (task->get ("end").c_str ()));
        value = dt.toString (context.config.get ("dateformat"));
        context.hooks.trigger ("format-end", "end", value);
        table.addCell (row, 1, value);
      }

      // tags ...
      std::vector <std::string> tags;
      task->getTags (tags);
      if (tags.size ())
      {
        std::string allTags;
        join (allTags, " ", tags);

        row = table.addRow ();
        table.addCell (row, 0, "Tags");
        table.addCell (row, 1, allTags);
      }

      // uuid
      row = table.addRow ();
      table.addCell (row, 0, "UUID");
      value = task->get ("uuid");
      context.hooks.trigger ("format-uuid", "uuid", value);
      table.addCell (row, 1, value);

      // entry
      row = table.addRow ();
      table.addCell (row, 0, "Entered");
      Date dt (atoi (task->get ("entry").c_str ()));
      std::string entry = dt.toString (context.config.get ("dateformat"));

      std::string age;
      std::string created = task->get ("entry");
      if (created.length ())
      {
        Date dt (atoi (created.c_str ()));
        age = Duration (now - dt).format ();
      }

      context.hooks.trigger ("format-entry", "entry", entry);
      table.addCell (row, 1, entry + " (" + age + ")");

      // fg
      std::string color = task->get ("fg");
      if (color != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Foreground color");
        table.addCell (row, 1, color);
      }

      // bg
      color = task->get ("bg");
      if (color != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Background color");
        table.addCell (row, 1, color);
      }

      // Task::urgency
      // TODO Enable this later.  This was for testing.
      //row = table.addRow ();
      //table.addCell (row, 0, "Urgency");
      //table.addCell (row, 1, task->urgency ());

      // If an alternating row color is specified, notify the table.
      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
      {
        Color alternate (context.config.get ("color.alternate"));
        if (alternate.nontrivial ())
          table.setTableAlternateColor (alternate);
      }

      out << optionalBlankLine ()
          << table.render ()
          << "\n";
    }

    if (! tasks.size ()) {
      out << "No matches.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-info-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// Project  Remaining  Avg Age  Complete  0%                  100%
// A               12      13d       55%  XXXXXXXXXXXXX-----------
// B              109   3d 12h       10%  XXX---------------------
int handleReportSummary (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-summary-command"))
  {
    // Scan the pending tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    // Generate unique list of project names from all pending tasks.
    std::map <std::string, bool> allProjects;
    foreach (task, tasks)
      if (task->getStatus () == Task::pending)
        allProjects[task->get ("project")] = false;

    // Initialize counts, sum.
    std::map <std::string, int> countPending;
    std::map <std::string, int> countCompleted;
    std::map <std::string, double> sumEntry;
    std::map <std::string, int> counter;
    time_t now = time (NULL);

    // Initialize counters.
    foreach (project, allProjects)
    {
      countPending   [project->first] = 0;
      countCompleted [project->first] = 0;
      sumEntry       [project->first] = 0.0;
      counter        [project->first] = 0;
    }

    // Count the various tasks.
    foreach (task, tasks)
    {
      std::string project = task->get ("project");
      ++counter[project];

      if (task->getStatus () == Task::pending ||
          task->getStatus () == Task::waiting)
      {
        ++countPending[project];

        time_t entry = atoi (task->get ("entry").c_str ());
        if (entry)
          sumEntry[project] = sumEntry[project] + (double) (now - entry);
      }

      else if (task->getStatus () == Task::completed)
      {
        ++countCompleted[project];

        time_t entry = atoi (task->get ("entry").c_str ());
        time_t end   = atoi (task->get ("end").c_str ());
        if (entry && end)
          sumEntry[project] = sumEntry[project] + (double) (end - entry);
      }
    }

    // Create a table for output.
    Table table;
    table.addColumn ("Project");
    table.addColumn ("Remaining");
    table.addColumn ("Avg age");
    table.addColumn ("Complete");
    table.addColumn ("0%                        100%");

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
      table.setColumnUnderline (2);
      table.setColumnUnderline (3);
    }
    else
      table.setTableDashedUnderline ();

    table.setColumnJustification (1, Table::right);
    table.setColumnJustification (2, Table::right);
    table.setColumnJustification (3, Table::right);

    table.sortOn (0, Table::ascendingCharacter);
    table.setDateFormat (context.config.get ("dateformat"));

    Color bar_color (context.config.get ("color.summary.bar"));
    Color bg_color  (context.config.get ("color.summary.background"));

    int barWidth = 30;
    foreach (i, allProjects)
    {
      if (countPending[i->first] > 0)
      {
        int row = table.addRow ();
        table.addCell (row, 0, (i->first == "" ? "(none)" : i->first));
        table.addCell (row, 1, countPending[i->first]);
        if (counter[i->first])
          table.addCell (row, 2, Duration ((int) (sumEntry[i->first] / (double)counter[i->first])).format ());

        int c = countCompleted[i->first];
        int p = countPending[i->first];
        int completedBar = (c * barWidth) / (c + p);

        std::string bar;
        std::string subbar;
        if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        {
          bar += bar_color.colorize (std::string (           completedBar, ' '));
          bar += bg_color.colorize  (std::string (barWidth - completedBar, ' '));
        }
        else
        {
          bar += std::string (           completedBar, '=')
              +  std::string (barWidth - completedBar, ' ');
        }
        table.addCell (row, 4, bar);

        char percent[12];
        sprintf (percent, "%d%%", 100 * c / (c + p));
        table.addCell (row, 3, percent);
      }
    }

    std::stringstream out;
    if (table.rowCount ())
      out << optionalBlankLine ()
          << table.render ()
          << optionalBlankLine ()
          << table.rowCount ()
          << (table.rowCount () == 1 ? " project" : " projects")
          << "\n";
    else {
      out << "No projects.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-summary-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportTimesheet (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-timesheet-command"))
  {
    // Scan the pending tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    // Just do this once.
    int width = context.getWidth ();

    // What day of the week does the user consider the first?
    int weekStart = Date::dayOfWeek (context.config.get ("weekstart"));
    if (weekStart != 0 && weekStart != 1)
      throw std::string ("The 'weekstart' configuration variable may "
                         "only contain 'Sunday' or 'Monday'.");

    // Determine the date of the first day of the most recent report.
    Date today;
    Date start;
    start -= (((today.dayOfWeek () - weekStart) + 7) % 7) * 86400;

    // Roll back to midnight.
    start = Date (start.month (), start.day (), start.year ());
    Date end = start + (7 * 86400);

    // Determine how many reports to run.
    int quantity = 1;
    if (context.sequence.size () == 1)
      quantity = context.sequence[0];

    bool color = context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor");

    std::stringstream out;
    for (int week = 0; week < quantity; ++week)
    {
      Date endString (end);
      endString -= 86400;

      std::string title = start.toString (context.config.get ("dateformat"))
                          + " - "
                          + endString.toString (context.config.get ("dateformat"));

      Color bold (Color::nocolor, Color::nocolor, false, true, false);
      out << "\n"
          << (color ? bold.colorize (title) : title)
          << "\n";

      // Render the completed table.
      Table completed;
      completed.setTableWidth (width);
      completed.addColumn ("   ");
      completed.addColumn ("Project");
      completed.addColumn ("Due");
      completed.addColumn ("Description");

      if (color && context.config.getBoolean ("fontunderline"))
      {
        completed.setColumnUnderline (1);
        completed.setColumnUnderline (2);
        completed.setColumnUnderline (3);
      }
      else
        completed.setTableDashedUnderline ();

      completed.setColumnWidth (0, Table::minimum);
      completed.setColumnWidth (1, Table::minimum);
      completed.setColumnWidth (2, Table::minimum);
      completed.setColumnWidth (3, Table::flexible);

      completed.setColumnJustification (0, Table::left);
      completed.setColumnJustification (1, Table::left);
      completed.setColumnJustification (2, Table::right);
      completed.setColumnJustification (3, Table::left);

      foreach (task, tasks)
      {
        // If task completed within range.
        if (task->getStatus () == Task::completed)
        {
          context.hooks.trigger ("pre-display", *task);

          Date compDate (atoi (task->get ("end").c_str ()));
          if (compDate >= start && compDate < end)
          {
            int row = completed.addRow ();
            std::string format = context.config.get ("dateformat.report");
            if (format == "")
              format = context.config.get ("dateformat");
            completed.addCell (row, 1, task->get ("project"));
            completed.addCell (row, 2, getDueDate (*task, format));
            completed.addCell (row, 3, getFullDescription (*task, "timesheet"));

            if (color)
            {
              Color c (task->get ("fg") + " " + task->get ("bg"));
              autoColorize (*task, c);
              completed.setRowColor (row, c);
            }
          }
        }
      }

      out << "  Completed (" << completed.rowCount () << " tasks)\n";

      if (completed.rowCount ())
        out << completed.render ()
            << "\n";

      // Now render the started table.
      Table started;
      started.setTableWidth (width);
      started.addColumn ("   ");
      started.addColumn ("Project");
      started.addColumn ("Due");
      started.addColumn ("Description");

      if (color && context.config.getBoolean ("fontunderline"))
      {
        started.setColumnUnderline (1);
        started.setColumnUnderline (2);
        started.setColumnUnderline (3);
      }
      else
        started.setTableDashedUnderline ();

      started.setColumnWidth (0, Table::minimum);
      started.setColumnWidth (1, Table::minimum);
      started.setColumnWidth (2, Table::minimum);
      started.setColumnWidth (3, Table::flexible);

      started.setColumnJustification (0, Table::left);
      started.setColumnJustification (1, Table::left);
      started.setColumnJustification (2, Table::right);
      started.setColumnJustification (3, Table::left);
      foreach (task, tasks)
      {
        // If task started within range, but not completed withing range.
        if (task->getStatus () == Task::pending &&
            task->has ("start"))
        {
          context.hooks.trigger ("pre-display", *task);

          Date startDate (atoi (task->get ("start").c_str ()));
          if (startDate >= start && startDate < end)
          {
            int row = started.addRow ();
            std::string format = context.config.get ("dateformat.report");
            if (format == "")
              format = context.config.get ("dateformat");
            started.addCell (row, 1, task->get ("project"));
            started.addCell (row, 2, getDueDate (*task, format));
            started.addCell (row, 3, getFullDescription (*task, "timesheet"));

            if (color)
            {
              Color c (task->get ("fg") + " " + task->get ("bg"));
              autoColorize (*task, c);
              started.setRowColor (row, c);
            }
          }
        }
      }

      out << "  Started (" << started.rowCount () << " tasks)\n";

      if (started.rowCount ())
        out << started.render ()
            << "\n\n";

      // Prior week.
      start -= 7 * 86400;
      end   -= 7 * 86400;
    }

    outs = out.str ();
    context.hooks.trigger ("post-timesheet-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
std::string renderMonths (
  int firstMonth,
  int firstYear,
  const Date& today,
  std::vector <Task>& all,
  int monthsPerLine)
{
  Table table;
  table.setDateFormat (context.config.get ("dateformat"));

  // What day of the week does the user consider the first?
  int weekStart = Date::dayOfWeek (context.config.get ("weekstart"));
  if (weekStart != 0 && weekStart != 1)
    throw std::string ("The 'weekstart' configuration variable may "
                       "only contain 'Sunday' or 'Monday'.");

  // Build table for the number of months to be displayed.
  for (int i = 0 ; i < (monthsPerLine * 8); i += 8)
  {
    if (weekStart == 1)
    {
      table.addColumn (" ");
      table.addColumn ("Mo");
      table.addColumn ("Tu");
      table.addColumn ("We");
      table.addColumn ("Th");
      table.addColumn ("Fr");
      table.addColumn ("Sa");
      table.addColumn ("Su");
    }
    else
    {
      table.addColumn (" ");
      table.addColumn ("Su");
      table.addColumn ("Mo");
      table.addColumn ("Tu");
      table.addColumn ("We");
      table.addColumn ("Th");
      table.addColumn ("Fr");
      table.addColumn ("Sa");
    }

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (i + 1);
      table.setColumnUnderline (i + 2);
      table.setColumnUnderline (i + 3);
      table.setColumnUnderline (i + 4);
      table.setColumnUnderline (i + 5);
      table.setColumnUnderline (i + 6);
      table.setColumnUnderline (i + 7);
    }
    else
      table.setTableDashedUnderline ();

    table.setColumnJustification (i + 0, Table::right);
    table.setColumnJustification (i + 1, Table::right);
    table.setColumnJustification (i + 2, Table::right);
    table.setColumnJustification (i + 3, Table::right);
    table.setColumnJustification (i + 4, Table::right);
    table.setColumnJustification (i + 5, Table::right);
    table.setColumnJustification (i + 6, Table::right);
    table.setColumnJustification (i + 7, Table::right);

    // This creates a nice gap between the months.
    table.setColumnWidth (i + 0, 4);
  }

  // At most, we need 6 rows.
  table.addRow ();
  table.addRow ();
  table.addRow ();
  table.addRow ();
  table.addRow ();
  table.addRow ();

  // Set number of days per month, months to render, and years to render.
  std::vector<int> years;
  std::vector<int> months;
  std::vector<int> daysInMonth;
  int thisYear = firstYear;
  int thisMonth = firstMonth;
  for (int i = 0 ; i < monthsPerLine ; i++)
  {
    if (thisMonth < 13)
    {
      years.push_back (thisYear);
    }
    else
    {
      thisMonth -= 12;
      years.push_back (++thisYear);
    }
    months.push_back (thisMonth);
    daysInMonth.push_back (Date::daysInMonth (thisMonth++, thisYear));
  }

  int row = 0;

  Color color_today      (context.config.get ("color.calendar.today"));
  Color color_due        (context.config.get ("color.calendar.due"));
  Color color_duetoday   (context.config.get ("color.calendar.due.today"));
  Color color_overdue    (context.config.get ("color.calendar.overdue"));
  Color color_weekend    (context.config.get ("color.calendar.weekend"));
  Color color_holiday    (context.config.get ("color.calendar.holiday"));
  Color color_weeknumber (context.config.get ("color.calendar.weeknumber"));

  // Loop through months to be added on this line.
  for (int mpl = 0; mpl < monthsPerLine ; mpl++)
  {
    // Reset row counter for subsequent months
    if (mpl != 0)
      row = 0;

    // Loop through days in month and add to table.
    for (int d = 1; d <= daysInMonth[mpl]; ++d)
    {
      Date temp (months[mpl], d, years[mpl]);
      int dow = temp.dayOfWeek ();
      int woy = temp.weekOfYear (weekStart);

      if (context.config.getBoolean ("displayweeknumber"))
      {
        table.addCell (row, (8 * mpl), woy);
        if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
          table.setCellColor (row, (8 * mpl), color_weeknumber);
      }

      // Calculate column id.
      int thisCol = dow +                       // 0 = Sunday
                    (weekStart == 1 ? 0 : 1) +  // Offset for weekStart
                    (8 * mpl);                  // Columns in 1 month

      if (thisCol == (8 * mpl))
        thisCol += 7;

      table.addCell (row, thisCol, d);

      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
      {
        Color cellColor;

        // colorize weekends
        if (dow == 0 || dow == 6)
          cellColor.blend (color_weekend);

        // colorize holidays
        if (context.config.get ("calendar.holidays") != "none")
        {
          std::vector <std::string> holidays;
          context.config.all (holidays);
          foreach (hol, holidays)
            if (hol->substr (0, 8) == "holiday.")
              if (hol->substr (hol->size () - 4) == "date")
              {
                std::string value = context.config.get (*hol);
                Date holDate (value.c_str (), context.config.get ("dateformat.holiday"));
                if (holDate.day   () == d           &&
                    holDate.month () == months[mpl] &&
                    holDate.year  () == years[mpl])
                  cellColor.blend (color_holiday);
              }
        }

        // colorize today
        if (today.day   () == d                &&
            today.month () == months.at (mpl)  &&
            today.year  () == years.at  (mpl))
          cellColor.blend (color_today);

        // colorize due tasks
        if (context.config.get ("calendar.details") != "none")
        {
          context.config.set ("due", 0);
          foreach (task, all)
          {
            if (task->getStatus () == Task::pending &&
                !task->hasTag ("nocal")             &&
                task->has ("due"))
            {
              std::string due = task->get ("due");
              Date duedmy (atoi(due.c_str()));

              if (duedmy.day   () == d           &&
                  duedmy.month () == months[mpl] &&
                  duedmy.year  () == years[mpl])
              {
                switch (getDueState (due))
                {
                  case 1: // imminent
                    cellColor.blend (color_due);
                    break;

                  case 2: // today
                    cellColor.blend (color_duetoday);
                    break;

                  case 3: // overdue
                    cellColor.blend (color_overdue);
                    break;

                  case 0: // not due at all
                  default:
                    break;
                }
              }
            }
          }
        }
        table.setCellColor (row, thisCol, cellColor);
      }

      // Check for end of week, and...
      int eow = 6;
      if (weekStart == 1)
        eow = 0;
      if (dow == eow && d < daysInMonth[mpl])
        row++;
    }
  }

  return table.render ();
}

////////////////////////////////////////////////////////////////////////////////
int handleReportCalendar (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-calendar-command"))
  {
    // Each month requires 28 text columns width.  See how many will actually
    // fit.  But if a preference is specified, and it fits, use it.
    int width = context.getWidth ();
    int preferredMonthsPerLine = (context.config.getInteger ("monthsperline"));
    int monthsThatFit = width / 26;

    int monthsPerLine = monthsThatFit;
    if (preferredMonthsPerLine != 0 && preferredMonthsPerLine < monthsThatFit)
      monthsPerLine = preferredMonthsPerLine;

    // Get all the tasks.
    std::vector <Task> tasks;
    Filter filter;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.loadPending (tasks, filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    Date today;
    bool getpendingdate = false;
    int monthsToDisplay = 1;
    int mFrom = today.month ();
    int yFrom = today.year ();
    int mTo = mFrom;
    int yTo = yFrom;

    // Defaults.
    monthsToDisplay = monthsPerLine;
    mFrom = today.month ();
    yFrom = today.year ();

    // Set up a vector of commands (1), for autoComplete.
    std::vector <std::string> commandNames;
    commandNames.push_back ("calendar");

    // Set up a vector of keywords, for autoComplete.
    std::vector <std::string> keywordNames;
    keywordNames.push_back ("due");

    // Set up a vector of months, for autoComplete.
    std::vector <std::string> monthNames;
    monthNames.push_back ("january");
    monthNames.push_back ("february");
    monthNames.push_back ("march");
    monthNames.push_back ("april");
    monthNames.push_back ("may");
    monthNames.push_back ("june");
    monthNames.push_back ("july");
    monthNames.push_back ("august");
    monthNames.push_back ("september");
    monthNames.push_back ("october");
    monthNames.push_back ("november");
    monthNames.push_back ("december");

    // For autoComplete results.
    std::vector <std::string> matches;

    // Look at all args, regardless of sequence.
    int argMonth = 0;
    int argYear = 0;
    bool argWholeYear = false;
    foreach (arg, context.args)
    {
      // Some version of "calendar".
      if (autoComplete (lowerCase (*arg), commandNames, matches) == 1)
        continue;

      // "due".
      else if (autoComplete (lowerCase (*arg), keywordNames, matches) == 1)
        getpendingdate = true;

      // "y".
      else if (lowerCase (*arg) == "y")
        argWholeYear = true;

      // YYYY.
      else if (digitsOnly (*arg) && arg->length () == 4)
        argYear = atoi (arg->c_str ());

      // MM.
      else if (digitsOnly (*arg) && arg->length () <= 2)
      {
        argMonth = atoi (arg->c_str ());
        if (argMonth < 1 || argMonth > 12)
          throw std::string ("Argument '") + *arg + "' is not a valid month.";
      }

      // "January" etc.
      else if (autoComplete (lowerCase (*arg), monthNames, matches) == 1)
      {
        argMonth = Date::monthOfYear (matches[0]);
        if (argMonth == -1)
          throw std::string ("Argument '") + *arg + "' is not a valid month.";
      }

      else
        throw std::string ("Could not recognize argument '") + *arg + "'.";
    }

    // Supported combinations:
    //
    //   Command line  monthsToDisplay  mFrom  yFrom  getpendingdate
    //   ------------  ---------------  -----  -----  --------------
    //   cal             monthsPerLine  today  today           false
    //   cal y                      12  today  today           false
    //   cal due         monthsPerLine  today  today            true
    //   cal YYYY                   12      1    arg           false
    //   cal due y                  12  today  today            true
    //   cal MM YYYY     monthsPerLine    arg    arg           false
    //   cal MM YYYY y              12    arg    arg           false

    if (argWholeYear || (argYear && !argMonth && !argWholeYear))
      monthsToDisplay = 12;

    if (!argMonth && argYear)
      mFrom = 1;
    else if (argMonth && argYear)
      mFrom = argMonth;

    if (argYear)
      yFrom = argYear;

    // Now begin the data subset and rendering.
    int countDueDates = 0;
    if (getpendingdate == true) {
      // Find the oldest pending due date.
      Date oldest (12,31,2037);
      foreach (task, tasks)
      {
        if (task->getStatus () == Task::pending)
        {
          if (task->has ("due") &&
              !task->hasTag ("nocal"))
          {
            ++countDueDates;
            Date d (atoi (task->get ("due").c_str ()));
            if (d < oldest) oldest = d;
          }
        }
      }
      mFrom = oldest.month();
      yFrom = oldest.year();
    }

    mTo = mFrom + monthsToDisplay - 1;
    yTo = yFrom;
    if (mTo > 12) {
      mTo -=12;
      yTo++;
    }

    int details_yFrom = yFrom;
    int details_mFrom = mFrom;

    std::stringstream out;
    out << "\n";

    while (yFrom < yTo || (yFrom == yTo && mFrom <= mTo))
    {
      int nextM = mFrom;
      int nextY = yFrom;

      // Print month headers (cheating on the width settings, yes)
      for (int i = 0 ; i < monthsPerLine ; i++)
      {
        std::string month = Date::monthName (nextM);

        //    12345678901234567890123456 = 26 chars wide
        //                ^^             = center
        //    <------->                  = 13 - (month.length / 2) + 1
        //                      <------> = 26 - above
        //   +--------------------------+
        //   |         July 2009        |
        //   |     Mo Tu We Th Fr Sa Su |
        //   |  27        1  2  3  4  5 |
        //   |  28  6  7  8  9 10 11 12 |
        //   |  29 13 14 15 16 17 18 19 |
        //   |  30 20 21 22 23 24 25 26 |
        //   |  31 27 28 29 30 31       |
        //   +--------------------------+

        int totalWidth = 26;
        int labelWidth = month.length () + 5;  // 5 = " 2009"
        int leftGap = (totalWidth / 2) - (labelWidth / 2);
        int rightGap = totalWidth - leftGap - labelWidth;

        out << std::setw (leftGap) << ' '
            << month
            << ' '
            << nextY
            << std::setw (rightGap) << ' ';

        if (++nextM > 12)
        {
          nextM = 1;
          nextY++;
        }
      }

      out << "\n"
          << optionalBlankLine ()
          << renderMonths (mFrom, yFrom, today, tasks, monthsPerLine)
          << "\n";

      mFrom += monthsPerLine;
      if (mFrom > 12)
      {
        mFrom -= 12;
        ++yFrom;
      }
    }

    Color color_today      (context.config.get ("color.calendar.today"));
    Color color_due        (context.config.get ("color.calendar.due"));
    Color color_duetoday   (context.config.get ("color.calendar.due.today"));
    Color color_overdue    (context.config.get ("color.calendar.overdue"));
    Color color_weekend    (context.config.get ("color.calendar.weekend"));
    Color color_holiday    (context.config.get ("color.calendar.holiday"));
    Color color_weeknumber (context.config.get ("color.calendar.weeknumber"));

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("calendar.legend"))
      out << "Legend: "
          << color_today.colorize ("today")
          << ", "
          << color_due.colorize ("due")
          << ", "
          << color_duetoday.colorize ("due-today")
          << ", "
          << color_overdue.colorize ("overdue")
          << ", "
          << color_weekend.colorize ("weekend")
          << ", "
          << color_holiday.colorize ("holiday")
          << ", "
          << color_weeknumber.colorize ("weeknumber")
          << "."
          << optionalBlankLine ()
          << "\n";

    if (context.config.get ("calendar.details") == "full" || context.config.get ("calendar.holidays") == "full")
    {
      --details_mFrom;
      if (details_mFrom == 0)
      {
        details_mFrom = 12;
        --details_yFrom;
      }
      int details_dFrom = Date::daysInMonth (details_mFrom, details_yFrom);

      ++mTo;
      if (mTo == 13)
      {
        mTo = 1;
        ++yTo;
      }

      Date date_after (details_mFrom, details_dFrom, details_yFrom);
      std::string after = date_after.toString (context.config.get ("dateformat"));

      Date date_before (mTo, 1, yTo);
      std::string before = date_before.toString (context.config.get ("dateformat"));

      // Table with due date information
      if (context.config.get ("calendar.details") == "full")
      {
        std::string report = context.config.get ("calendar.details.report");
        std::string report_filter = context.config.get ("report." + report + ".filter");

        report_filter += " due.after:" + after + " due.before:" + before;
        context.config.set ("report." + report + ".filter", report_filter);

        // Display all due task in the report colorized not only the imminet ones
        context.config.set ("due", 0);

        context.args.clear ();
        context.filter.clear ();
        context.sequence.clear ();

        std::string output;
        handleCustomReport (report, output);
        out << output;
      }

      // Table with holiday information
      if (context.config.get ("calendar.holidays") == "full")
      {
        std::vector <std::string> holidays;
        context.config.all (holidays);

        Table holTable;
        holTable.setTableWidth (context.getWidth ());
        holTable.addColumn ("Date");
        holTable.addColumn ("Holiday");
        holTable.sortOn (0, Table::ascendingDueDate);

        if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
            context.config.getBoolean ("fontunderline"))
        {
          holTable.setColumnUnderline (0);
          holTable.setColumnUnderline (1);
        }
        else
          holTable.setTableDashedUnderline ();

        holTable.setColumnWidth (0, Table::minimum);
        holTable.setColumnWidth (1, Table::flexible);

        holTable.setColumnJustification (0, Table::left);
        holTable.setColumnJustification (1, Table::left);

        foreach (hol, holidays)
          if (hol->substr (0, 8) == "holiday.")
            if (hol->substr (hol->size () - 4) == "name")
            {
              std::string holName = context.config.get ("holiday." + hol->substr (8, hol->size () - 13) + ".name");
              std::string holDate = context.config.get ("holiday." + hol->substr (8, hol->size () - 13) + ".date");
              Date hDate (holDate.c_str (), context.config.get ("dateformat.holiday"));

              if (date_after < hDate && hDate < date_before)
              {
                std::string format = context.config.get ("report." +
                                                         context.config.get ("calendar.details.report") +
                                                         ".dateformat");
                if (format == "")
                  format = context.config.get ("dateformat.report");
                if (format == "")
                  format = context.config.get ("dateformat");

                int row = holTable.addRow ();
                holTable.addCell (row, 0, hDate.toString (format));
                holTable.addCell (row, 1, holName);
              }
            }

        out << optionalBlankLine ()
            << holTable.render ()
            << "\n";
      }
    }

    outs = out.str ();
    context.hooks.trigger ("post-calendar-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportStats (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-stats-command"))
  {
    std::stringstream out;

    // Go get the file sizes.
    size_t dataSize = 0;

    Directory location (context.config.get ("data.location"));
    File pending (location.data + "/pending.data");
    dataSize += pending.size ();

    File completed (location.data + "/completed.data");
    dataSize += completed.size ();

    File undo (location.data + "/undo.data");
    dataSize += undo.size ();

    std::vector <std::string> undoTxns;
    File::read (undo, undoTxns);
    int undoCount = 0;
    foreach (tx, undoTxns)
      if (tx->substr (0, 3) == "---")
        ++undoCount;

    // Get all the tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    Date now;
    time_t earliest   = time (NULL);
    time_t latest     = 1;
    int totalT        = 0;
    int deletedT      = 0;
    int pendingT      = 0;
    int completedT    = 0;
    int waitingT      = 0;
    int taggedT       = 0;
    int annotationsT  = 0;
    int recurringT    = 0;
    float daysPending = 0.0;
    int descLength    = 0;
    std::map <std::string, int> allTags;
    std::map <std::string, int> allProjects;

    std::vector <Task>::iterator it;
    for (it = tasks.begin (); it != tasks.end (); ++it)
    {
      ++totalT;
      if (it->getStatus () == Task::deleted)   ++deletedT;
      if (it->getStatus () == Task::pending)   ++pendingT;
      if (it->getStatus () == Task::completed) ++completedT;
      if (it->getStatus () == Task::recurring) ++recurringT;
      if (it->getStatus () == Task::waiting)   ++waitingT;

      time_t entry = atoi (it->get ("entry").c_str ());
      if (entry < earliest) earliest = entry;
      if (entry > latest)   latest   = entry;

      if (it->getStatus () == Task::completed)
      {
        time_t end = atoi (it->get ("end").c_str ());
        daysPending += (end - entry) / 86400.0;
      }

      if (it->getStatus () == Task::pending)
        daysPending += (now.toEpoch () - entry) / 86400.0;

      descLength += it->get ("description").length ();

      std::vector <Att> annotations;
      it->getAnnotations (annotations);
      annotationsT += annotations.size ();

      std::vector <std::string> tags;
      it->getTags (tags);
      if (tags.size ()) ++taggedT;

      foreach (t, tags)
        allTags[*t] = 0;

      std::string project = it->get ("project");
      if (project != "")
        allProjects[project] = 0;
    }

    // Create a table for output.
    Table table;
    table.setTableWidth (context.getWidth ());
    table.setTableIntraPadding (2);
    table.setDateFormat (context.config.get ("dateformat"));
    table.addColumn ("Category");
    table.addColumn ("Data");

    if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
        context.config.getBoolean ("fontunderline"))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }
    else
      table.setTableDashedUnderline ();

    table.setColumnWidth (0, Table::minimum);
    table.setColumnWidth (1, Table::flexible);

    table.setColumnJustification (0, Table::left);
    table.setColumnJustification (1, Table::left);

    int row = table.addRow ();
    table.addCell (row, 0, "Pending");
    table.addCell (row, 1, pendingT);

    row = table.addRow ();
    table.addCell (row, 0, "Waiting");
    table.addCell (row, 1, waitingT);

    row = table.addRow ();
    table.addCell (row, 0, "Recurring");
    table.addCell (row, 1, recurringT);

    row = table.addRow ();
    table.addCell (row, 0, "Completed");
    table.addCell (row, 1, completedT);

    row = table.addRow ();
    table.addCell (row, 0, "Deleted");
    table.addCell (row, 1, deletedT);

    row = table.addRow ();
    table.addCell (row, 0, "Total");
    table.addCell (row, 1, totalT);

    row = table.addRow ();
    table.addCell (row, 0, "Annotations");
    table.addCell (row, 1, annotationsT);

    row = table.addRow ();
    table.addCell (row, 0, "Unique tags");
    table.addCell (row, 1, (int)allTags.size ());

    row = table.addRow ();
    table.addCell (row, 0, "Projects");
    table.addCell (row, 1, (int)allProjects.size ());

    row = table.addRow ();
    table.addCell (row, 0, "Data size");
    table.addCell (row, 1, formatBytes (dataSize));

    row = table.addRow ();
    table.addCell (row, 0, "Undo transactions");
    table.addCell (row, 1, undoCount);

    if (totalT)
    {
      row = table.addRow ();
      table.addCell (row, 0, "Tasks tagged");

      std::stringstream value;
      value << std::setprecision (3) << (100.0 * taggedT / totalT) << "%";
      table.addCell (row, 1, value.str ());
    }

    if (tasks.size ())
    {
      Date e (earliest);
      row = table.addRow ();
      table.addCell (row, 0, "Oldest task");
      table.addCell (row, 1, e.toString (context.config.get ("dateformat")));

      Date l (latest);
      row = table.addRow ();
      table.addCell (row, 0, "Newest task");
      table.addCell (row, 1, l.toString (context.config.get ("dateformat")));

      row = table.addRow ();
      table.addCell (row, 0, "Task used for");
      table.addCell (row, 1, Duration (latest - earliest).format ());
    }

    if (totalT)
    {
      row = table.addRow ();
      table.addCell (row, 0, "Task added every");
      table.addCell (row, 1, Duration (((latest - earliest) / totalT)).format ());
    }

    if (completedT)
    {
      row = table.addRow ();
      table.addCell (row, 0, "Task completed every");
      table.addCell (row, 1, Duration ((latest - earliest) / completedT).format ());
    }

    if (deletedT)
    {
      row = table.addRow ();
      table.addCell (row, 0, "Task deleted every");
      table.addCell (row, 1, Duration ((latest - earliest) / deletedT).format ());
    }

    if (pendingT || completedT)
    {
      row = table.addRow ();
      table.addCell (row, 0, "Average time pending");
      table.addCell (row, 1, Duration ((int) ((daysPending / (pendingT + completedT)) * 86400)).format ());
    }

    if (totalT)
    {
      row = table.addRow ();
      table.addCell (row, 0, "Average desc length");
      std::stringstream value;
      value << (int) (descLength / totalT) << " characters";
      table.addCell (row, 1, value.str ());
    }

    // If an alternating row color is specified, notify the table.
    if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
    {
      Color alternate (context.config.get ("color.alternate"));
      if (alternate.nontrivial ())
        table.setTableAlternateColor (alternate);
    }

    out << optionalBlankLine ()
        << table.render ()
        << optionalBlankLine ();

    outs = out.str ();
    context.hooks.trigger ("post-stats-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void gatherNextTasks (std::vector <Task>& tasks)
{
  // For counting tasks by project.
  std::map <std::string, int> countByProject;
  std::map <int, bool> matching;
  std::vector <Task> filtered;
  Date now;

  // How many items per project?  Default 2.
  int limit = context.config.getInteger ("next");

  // due:< 1wk, pri:*
  foreach (task, tasks)
  {
    if (task->has ("due"))
    {
      Date d (atoi (task->get ("due").c_str ()));
      if (d < now + (7 * 24 * 60 * 60)) // if due:< 1wk
      {
        std::string project = task->get ("project");
        if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
        {
          ++countByProject[project];
          matching[task->id] = true;
          filtered.push_back (*task);
        }
      }
    }
  }

  // blocking, not blocked
  foreach (task, tasks)
  {
    if (dependencyIsBlocking (*task) &&
        ! dependencyIsBlocked (*task))
    {
      std::string project = task->get ("project");
      if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
      {
        ++countByProject[project];
        matching[task->id] = true;
        filtered.push_back (*task);
      }
    }
  }

  // due:*, pri:H
  foreach (task, tasks)
  {
    if (task->has ("due"))
    {
      std::string priority = task->get ("priority");
      if (priority == "H")
      {
        std::string project = task->get ("project");
        if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
        {
          ++countByProject[project];
          matching[task->id] = true;
          filtered.push_back (*task);
        }
      }
    }
  }

  // pri:H
  foreach (task, tasks)
  {
    std::string priority = task->get ("priority");
    if (priority == "H")
    {
      std::string project = task->get ("project");
      if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
      {
        ++countByProject[project];
        matching[task->id] = true;
        filtered.push_back (*task);
      }
    }
  }

  // due:*, pri:M
  foreach (task, tasks)
  {
    if (task->has ("due"))
    {
      std::string priority = task->get ("priority");
      if (priority == "M")
      {
        std::string project = task->get ("project");
        if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
        {
          ++countByProject[project];
          matching[task->id] = true;
          filtered.push_back (*task);
        }
      }
    }
  }

  // pri:M
  foreach (task, tasks)
  {
    std::string priority = task->get ("priority");
    if (priority == "M")
    {
      std::string project = task->get ("project");
      if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
      {
        ++countByProject[project];
        matching[task->id] = true;
        filtered.push_back (*task);
      }
    }
  }

  // due:*, pri:L
  foreach (task, tasks)
  {
    if (task->has ("due"))
    {
      std::string priority = task->get ("priority");
      if (priority == "L")
      {
        std::string project = task->get ("project");
        if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
        {
          ++countByProject[project];
          matching[task->id] = true;
          filtered.push_back (*task);
        }
      }
    }
  }

  // pri:L
  foreach (task, tasks)
  {
    std::string priority = task->get ("priority");
    if (priority == "L")
    {
      std::string project = task->get ("project");
      if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
      {
        ++countByProject[project];
        matching[task->id] = true;
        filtered.push_back (*task);
      }
    }
  }

  // due:, pri:
  foreach (task, tasks)
  {
    if (task->has ("due"))
    {
      std::string priority = task->get ("priority");
      if (priority == "")
      {
        std::string project = task->get ("project");
        if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
        {
          ++countByProject[project];
          matching[task->id] = true;
          filtered.push_back (*task);
        }
      }
    }
  }

  // Filler.
  foreach (task, tasks)
  {
    std::string project = task->get ("project");
    if (countByProject[project] < limit && matching.find (task->id) == matching.end ())
    {
      ++countByProject[project];
      matching[task->id] = true;
      filtered.push_back (*task);
    }
  }

  tasks = filtered;
}

///////////////////////////////////////////////////////////////////////////////
std::string getFullDescription (Task& task, const std::string& report)
{
  std::string desc = task.get ("description");
  std::string annotationDetails;

  std::vector <Att> annotations;
  task.getAnnotations (annotations);

  if (annotations.size () != 0)
  {
    std::string annotationDetails = context.config.get ("report." + report + ".annotations");
    if (annotationDetails == "")
      annotationDetails = context.config.get ("annotations");
    if (report == "info")
      annotationDetails = "full";

    if (annotationDetails == "none")
    {
      desc = "+" + desc;
    }
    else if (annotationDetails == "sparse")
    {
      if (annotations.size () > 1)
        desc = "+" + desc;
      Att anno (annotations.back());
      Date dt (atoi (anno.name ().substr (11).c_str ()));
      std::string format = context.config.get ("dateformat.annotation");
      if (format == "")
        format = context.config.get ("dateformat");
      std::string when = dt.toString (format);
      desc += "\n" + when + " " + anno.value ();
    }
    else
      foreach (anno, annotations)
      {
        Date dt (atoi (anno->name ().substr (11).c_str ()));
	std::string format = context.config.get ("dateformat.annotation");
	if (format == "")
	  format = context.config.get ("dateformat");
        std::string when = dt.toString (format);
        desc += "\n" + when + " " + anno->value ();
      }
  }

  return desc;
}

///////////////////////////////////////////////////////////////////////////////
std::string getDueDate (Task& task, const std::string& format)
{
  std::string due = task.get ("due");
  if (due.length ())
  {
    Date d (atoi (due.c_str ()));
    due = d.toString (format);
  }

  return due;
}

///////////////////////////////////////////////////////////////////////////////
std::string onProjectChange (Task& task, bool scope /* = true */)
{
  std::stringstream msg;
  std::string project = task.get ("project");

  if (project != "")
  {
    if (scope)
      msg << "The project '"
          << project
          << "' has changed.  ";

    // Count pending and done tasks, for this project.
    int count_pending = 0;
    int count_done = 0;

    std::vector <Task> all;
    Filter filter;
    context.tdb.load (all, filter);

    countTasks (all,                           project, task.get ("uuid"), count_pending, count_done);
    countTasks (context.tdb.getAllNew (),      project, "nope",            count_pending, count_done);
    countTasks (context.tdb.getAllModified (), project, "nope",            count_pending, count_done);

    int percentage = 0;
    if (count_done + count_pending > 0)
      percentage = (count_done * 100 / (count_done + count_pending));

    msg << "Project '"
        << project
        << "' is "
        << percentage
        << "% complete ("
        << count_pending
        << " of "
        << (count_pending + count_done)
        << " tasks remaining).\n";
  }

  return msg.str ();
}

///////////////////////////////////////////////////////////////////////////////
std::string onProjectChange (Task& task1, Task& task2)
{
  std::string messages = onProjectChange (task1);
  messages            += onProjectChange (task2);

  return messages;
}

///////////////////////////////////////////////////////////////////////////////
static void countTasks (
  const std::vector <Task>& all,
  const std::string& project,
  const std::string& skip,
  int& count_pending,
  int& count_done)
{
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->get ("project") == project &&
        it->get ("uuid")    != skip)
    {
      switch (it->getStatus ())
      {
      case Task::pending:
      case Task::waiting:
        ++count_pending;
        break;

      case Task::completed:
        ++count_done;
        break;

      default:
        break;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
