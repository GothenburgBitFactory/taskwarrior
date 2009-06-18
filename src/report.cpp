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
std::string shortUsage ()
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
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

  int row = table.addRow ();
  table.addCell (row, 0, "Usage:");
  table.addCell (row, 1, "task");

  row = table.addRow ();
  table.addCell (row, 1, "task add [tags] [attrs] desc...");
  table.addCell (row, 2, "Adds a new task.");

  row = table.addRow ();
  table.addCell (row, 1, "task append [tags] [attrs] desc...");
  table.addCell (row, 2, "Appends more description to an existing task.");

  row = table.addRow ();
  table.addCell (row, 1, "task annotate ID desc...");
  table.addCell (row, 2, "Adds an annotation to an existing task.");

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
  table.addCell (row, 1, "task edit ID");
  table.addCell (row, 2, "Launches an editor to let you modify all aspects of a task directly, therefore it is to be used carefully.");

  row = table.addRow ();
  table.addCell (row, 1, "task duplicate ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Duplicates the specified task, and allows modifications.");

  row = table.addRow ();
  table.addCell (row, 1, "task delete ID");
  table.addCell (row, 2, "Deletes the specified task.");

  row = table.addRow ();
  table.addCell (row, 1, "task undelete ID");
  table.addCell (row, 2, "Undeletes the specified task, provided a report has not yet been run.");

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
  table.addCell (row, 1, "task undo ID");
  table.addCell (row, 2, "Marks the specified done task as pending, provided a report has not yet been run.");

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
  table.addCell (row, 2, "Shows a report of task history, by month.");

  row = table.addRow ();
  table.addCell (row, 1, "task ghistory");
  table.addCell (row, 2, "Shows a graphical report of task history, by month.");

  row = table.addRow ();
  table.addCell (row, 1, "task calendar");
  table.addCell (row, 2, "Shows a monthly calendar, with due tasks marked.");

  row = table.addRow ();
  table.addCell (row, 1, "task stats");
  table.addCell (row, 2, "Shows task database statistics.");

  row = table.addRow ();
  table.addCell (row, 1, "task import");
  table.addCell (row, 2, "Imports tasks from a variety of formats.");

  row = table.addRow ();
  table.addCell (row, 1, "task export");
  table.addCell (row, 2, "Lists all tasks as a CSV file.");

  row = table.addRow ();
  table.addCell (row, 1, "task color");
  table.addCell (row, 2, "Displays all possible colors.");

  row = table.addRow ();
  table.addCell (row, 1, "task version");
  table.addCell (row, 2, "Shows the task version number.");

  row = table.addRow ();
  table.addCell (row, 1, "task help");
  table.addCell (row, 2, "Shows the long usage text.");

  // Add custom reports here...
  std::vector <std::string> all;
  context.cmd.allCustomReports (all);
  foreach (report, all)
  {
    std::string command = std::string ("task ") + *report + std::string (" [tags] [attrs] desc...");
    std::string description = context.config.get (
      std::string ("report.") + *report + ".description", std::string ("(missing description)"));

    row = table.addRow ();
    table.addCell (row, 1, command);
    table.addCell (row, 2, description);
  }

  std::stringstream out;
  out << table.render ()
      << std::endl
      << "See http://taskwarrior.org/wiki/taskwarrior/Download for the latest "
      << "releases and a full tutorial.  New releases containing fixes and "
      << "enhancements are made frequently.  Join in the discussion of task, "
      << "present and future, at http://taskwarrior.org"
      << std::endl
      << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string longUsage ()
{
  std::stringstream out;
  out << shortUsage ()
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
      << "  limit:             Desired number of rows in report"              << "\n"
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
      << "The argument -- tells task to treat all other args as description." << "\n"
      << "  task add -- project:Home needs scheduling"                        << "\n"
      <<                                                                         "\n"
      << "Many characters have special meaning to the shell, including:"      << "\n"
      << "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~"                    << "\n"
      << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Display all information for the given task.
std::string handleInfo ()
{
  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);
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
    table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

    table.addColumn ("Name");
    table.addColumn ("Value");

    if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
        context.config.get (std::string ("fontunderline"), "true"))
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

    int row = table.addRow ();
    table.addCell (row, 0, "ID");
    table.addCell (row, 1, task->id);

    std::string status = ucFirst (Task::statusToText (task->getStatus ()));

    if (task->has ("parent"))
      status += " (Recurring)";

    row = table.addRow ();
    table.addCell (row, 0, "Status");
    table.addCell (row, 1, status);

    row = table.addRow ();
    table.addCell (row, 0, "Description");
    table.addCell (row, 1, getFullDescription (*task));

    if (task->has ("project"))
    {
      row = table.addRow ();
      table.addCell (row, 0, "Project");
      table.addCell (row, 1, task->get ("project"));
    }

    if (task->has ("priority"))
    {
      row = table.addRow ();
      table.addCell (row, 0, "Priority");
      table.addCell (row, 1, task->get ("priority"));
    }

    if (task->getStatus () == Task::recurring ||
        task->has ("parent"))
    {
      if (task->has ("recur"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Recurrence");
        table.addCell (row, 1, task->get ("recur"));
      }

      if (task->has ("until"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Recur until");
        table.addCell (row, 1, task->get ("until"));
      }

      if (task->has ("mask"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Mask");
        table.addCell (row, 1, task->get ("mask"));
      }

      if (task->has ("parent"))
      {
        row = table.addRow ();
        table.addCell (row, 0, "Parent task");
        table.addCell (row, 1, task->get ("parent"));
      }

      row = table.addRow ();
      table.addCell (row, 0, "Mask Index");
      table.addCell (row, 1, task->get ("imask"));
    }

    // due (colored)
    bool imminent = false;
    bool overdue = false;
    if (task->has ("due"))
    {
      row = table.addRow ();
      table.addCell (row, 0, "Due");

      Date dt (::atoi (task->get ("due").c_str ()));
      std::string due = getDueDate (*task);
      table.addCell (row, 1, due);

      overdue = (dt < now) ? true : false;
      Date nextweek = now + 7 * 86400;
      imminent = dt < nextweek ? true : false;

      if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
      {
        if (overdue)
          table.setCellFg (row, 1, Text::colorCode (context.config.get ("color.overdue", "red")));
        else if (imminent)
          table.setCellFg (row, 1, Text::colorCode (context.config.get ("color.due", "yellow")));
      }
    }

    // start
    if (task->has ("start"))
    {
      row = table.addRow ();
      table.addCell (row, 0, "Start");
      Date dt (::atoi (task->get ("start").c_str ()));
      table.addCell (row, 1, dt.toString (context.config.get ("dateformat", "m/d/Y")));
    }

    // end
    if (task->has ("end"))
    {
      row = table.addRow ();
      table.addCell (row, 0, "End");
      Date dt (::atoi (task->get ("end").c_str ()));
      table.addCell (row, 1, dt.toString (context.config.get ("dateformat", "m/d/Y")));
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
    table.addCell (row, 1, task->get ("uuid"));

    // entry
    row = table.addRow ();
    table.addCell (row, 0, "Entered");
    Date dt (::atoi (task->get ("entry").c_str ()));
    std::string entry = dt.toString (context.config.get ("dateformat", "m/d/Y"));

    std::string age;
    std::string created = task->get ("entry");
    if (created.length ())
    {
      Date dt (::atoi (created.c_str ()));
      age = formatSeconds ((time_t) (now - dt));
    }

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

    out << optionalBlankLine ()
        << table.render ()
        << std::endl;
  }

  if (! tasks.size ())
    out << "No matches." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Project  Remaining  Avg Age  Complete  0%                  100%
// A               12      13d       55%  XXXXXXXXXXXXX-----------
// B              109   3d 12h       10%  XXX---------------------
std::string handleReportSummary ()
{
  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.load (tasks, context.filter);
  handleRecurrence (tasks);
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

    if (task->getStatus () == Task::pending)
    {
      ++countPending[project];

      time_t entry = ::atoi (task->get ("entry").c_str ());
      if (entry)
        sumEntry[project] = sumEntry[project] + (double) (now - entry);
    }

    else if (task->getStatus () == Task::completed)
    {
      ++countCompleted[project];

      time_t entry = ::atoi (task->get ("entry").c_str ());
      time_t end   = ::atoi (task->get ("end").c_str ());
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

  if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
      context.config.get (std::string ("fontunderline"), "true"))
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
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

  int barWidth = 30;
  foreach (i, allProjects)
  {
    if (countPending[i->first] > 0)
    {
      int row = table.addRow ();
      table.addCell (row, 0, (i->first == "" ? "(none)" : i->first));
      table.addCell (row, 1, countPending[i->first]);
      if (counter[i->first])
      {
        std::string age;
        age = formatSeconds ((time_t) (sumEntry[i->first] / counter[i->first]));
        table.addCell (row, 2, age);
      }

      int c = countCompleted[i->first];
      int p = countPending[i->first];
      int completedBar = (c * barWidth) / (c + p);

      std::string bar;
      if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
      {
        bar = "\033[42m";
        for (int b = 0; b < completedBar; ++b)
          bar += " ";

        bar += "\033[40m";
        for (int b = 0; b < barWidth - completedBar; ++b)
          bar += " ";

        bar += "\033[0m";
      }
      else
      {
        for (int b = 0; b < completedBar; ++b)
          bar += "=";

        for (int b = 0; b < barWidth - completedBar; ++b)
          bar += " ";
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
        << std::endl;
  else
    out << "No projects." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// A summary of the most important pending tasks.
//
// For every project, pull important tasks to present as an 'immediate' task
// list.  This hides the overwhelming quantity of other tasks.
//
// Present at most three tasks for every project.  Select the tasks using
// these criteria:
//   - due:< 1wk, pri:*
//   - due:*, pri:H
//   - pri:H
//   - due:*, pri:M
//   - pri:M
//   - due:*, pri:L
//   - pri:L
//   - due:*, pri:*        <-- everything else
//
// Make the "three" tasks a configurable number
//
std::string handleReportNext ()
{
  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Restrict to matching subset.
  std::vector <int> matching;
  gatherNextTasks (tasks, matching);

  initializeColorRules ();

  // Create a table for output.
  Table table;
  table.setTableWidth (context.getWidth ());
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Active");
  table.addColumn ("Age");
  table.addColumn ("Description");

  if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
      context.config.get (std::string ("fontunderline"), "true"))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
    table.setColumnUnderline (6);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::minimum);
  table.setColumnWidth (5, Table::minimum);
  table.setColumnWidth (6, Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);
  table.setColumnJustification (5, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  // Iterate over each task, and apply selection criteria.
  foreach (i, matching)
  {
    Date now;

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = tasks[*i].get ("due");
    if (due.length ())
    {
      switch (getDueState (due))
      {
      case 2: overdue = true;  break;
      case 1: imminent = true; break;
      case 0:
      default:                 break;
      }

      Date dt (::atoi (due.c_str ()));
      due = dt.toString (context.config.get ("dateformat", "m/d/Y"));
    }

    std::string active;
    if (tasks[*i].has ("start"))
      active = "*";

    std::string age;
    std::string created = tasks[*i].get ("entry");
    if (created.length ())
    {
      Date dt (::atoi (created.c_str ()));
      age = formatSeconds ((time_t) (now - dt));
    }

    // All criteria match, so add tasks[*i] to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, tasks[*i].id);
    table.addCell (row, 1, tasks[*i].get ("project"));
    table.addCell (row, 2, tasks[*i].get ("priority"));
    table.addCell (row, 3, due);
    table.addCell (row, 4, active);
    table.addCell (row, 5, age);
    table.addCell (row, 6, getFullDescription (tasks[*i]));

    if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
    {
      Text::color fg = Text::colorCode (tasks[*i].get ("fg"));
      Text::color bg = Text::colorCode (tasks[*i].get ("bg"));
      autoColorize (tasks[*i], fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::colorCode (context.config.get ("color.overdue", "red")));
        else if (imminent)
          table.setCellFg (row, 3, Text::colorCode (context.config.get ("color.due", "yellow")));
      }
    }
  }

  std::stringstream out;
  if (table.rowCount ())
    out << optionalBlankLine ()
        << table.render ()
        << optionalBlankLine ()
        << table.rowCount ()
        << (table.rowCount () == 1 ? " task" : " tasks")
        << std::endl;
  else
    out << "No matches."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Year Month    Added Completed Deleted
// 2006 November    87        63      14
//      December    21         6       1
// 2007 January      3        12       0
time_t monthlyEpoch (const std::string& date)
{
  // Convert any date in epoch form to m/d/y, then convert back
  // to epoch form for the date m/1/y.
  if (date.length ())
  {
    Date d1 (::atoi (date.c_str ()));
    int m, d, y;
    d1.toMDY (m, d, y);
    Date d2 (m, 1, y);
    time_t epoch;
    d2.toEpoch (epoch);
    return epoch;
 }

  return 0;
}

std::string handleReportHistory ()
{
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.load (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  foreach (task, tasks)
  {
    time_t epoch = monthlyEpoch (task->get ("entry"));
    groups[epoch] = 0;

    // Every task has an entry date.
    if (addedGroup.find (epoch) != addedGroup.end ())
      addedGroup[epoch] = addedGroup[epoch] + 1;
    else
      addedGroup[epoch] = 1;

    // All deleted tasks have an end date.
    if (task->getStatus () == Task::deleted)
    {
      epoch = monthlyEpoch (task->get ("end"));
      groups[epoch] = 0;

      if (deletedGroup.find (epoch) != deletedGroup.end ())
        deletedGroup[epoch] = deletedGroup[epoch] + 1;
      else
        deletedGroup[epoch] = 1;
    }

    // All completed tasks have an end date.
    else if (task->getStatus () == Task::completed)
    {
      epoch = monthlyEpoch (task->get ("end"));
      groups[epoch] = 0;

      if (completedGroup.find (epoch) != completedGroup.end ())
        completedGroup[epoch] = completedGroup[epoch] + 1;
      else
        completedGroup[epoch] = 1;
    }
  }

  // Now build the table.
  Table table;
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));
  table.addColumn ("Year");
  table.addColumn ("Month");
  table.addColumn ("Added");
  table.addColumn ("Completed");
  table.addColumn ("Deleted");
  table.addColumn ("Net");

  if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
      context.config.get (std::string ("fontunderline"), "true"))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnJustification (2, Table::right);
  table.setColumnJustification (3, Table::right);
  table.setColumnJustification (4, Table::right);
  table.setColumnJustification (5, Table::right);

  int totalAdded     = 0;
  int totalCompleted = 0;
  int totalDeleted   = 0;

  int priorYear = 0;
  int row = 0;
  foreach (i, groups)
  {
    row = table.addRow ();

    totalAdded     += addedGroup     [i->first];
    totalCompleted += completedGroup [i->first];
    totalDeleted   += deletedGroup   [i->first];

    Date dt (i->first);
    int m, d, y;
    dt.toMDY (m, d, y);

    if (y != priorYear)
    {
      table.addCell (row, 0, y);
      priorYear = y;
    }
    table.addCell (row, 1, Date::monthName(m));

    int net = 0;

    if (addedGroup.find (i->first) != addedGroup.end ())
    {
      table.addCell (row, 2, addedGroup[i->first]);
      net +=addedGroup[i->first];
    }

    if (completedGroup.find (i->first) != completedGroup.end ())
    {
      table.addCell (row, 3, completedGroup[i->first]);
      net -= completedGroup[i->first];
    }

    if (deletedGroup.find (i->first) != deletedGroup.end ())
    {
      table.addCell (row, 4, deletedGroup[i->first]);
      net -= deletedGroup[i->first];
    }

    table.addCell (row, 5, net);
    if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) && net)
      table.setCellFg (row, 5, net > 0 ? Text::red: Text::green);
  }

  if (table.rowCount ())
  {
    table.addRow ();
    row = table.addRow ();

    table.addCell (row, 1, "Average");
    if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) table.setRowFg (row, Text::bold);
    table.addCell (row, 2, totalAdded     / (table.rowCount () - 2));
    table.addCell (row, 3, totalCompleted / (table.rowCount () - 2));
    table.addCell (row, 4, totalDeleted   / (table.rowCount () - 2));
    table.addCell (row, 5, (totalAdded - totalCompleted - totalDeleted) / (table.rowCount () - 2));
  }

  std::stringstream out;
  if (table.rowCount ())
    out << optionalBlankLine ()
        << table.render ()
        << std::endl;
  else
    out << "No tasks." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportGHistory ()
{
  std::map <time_t, int> groups;          // Represents any month with data
  std::map <time_t, int> addedGroup;      // Additions by month
  std::map <time_t, int> completedGroup;  // Completions by month
  std::map <time_t, int> deletedGroup;    // Deletions by month

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.load (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  foreach (task, tasks)
  {
    time_t epoch = monthlyEpoch (task->get ("entry"));
    groups[epoch] = 0;

    // Every task has an entry date.
    if (addedGroup.find (epoch) != addedGroup.end ())
      addedGroup[epoch] = addedGroup[epoch] + 1;
    else
      addedGroup[epoch] = 1;

    // All deleted tasks have an end date.
    if (task->getStatus () == Task::deleted)
    {
      epoch = monthlyEpoch (task->get ("end"));
      groups[epoch] = 0;

      if (deletedGroup.find (epoch) != deletedGroup.end ())
        deletedGroup[epoch] = deletedGroup[epoch] + 1;
      else
        deletedGroup[epoch] = 1;
    }

    // All completed tasks have an end date.
    else if (task->getStatus () == Task::completed)
    {
      epoch = monthlyEpoch (task->get ("end"));
      groups[epoch] = 0;

      if (completedGroup.find (epoch) != completedGroup.end ())
        completedGroup[epoch] = completedGroup[epoch] + 1;
      else
        completedGroup[epoch] = 1;
    }
  }

  int widthOfBar = context.getWidth () - 15;   // 15 == strlen ("2008 September ")

  // Now build the table.
  Table table;
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));
  table.addColumn ("Year");
  table.addColumn ("Month");
  table.addColumn ("Number Added/Completed/Deleted");

  if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
      context.config.get (std::string ("fontunderline"), "true"))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
  }
  else
    table.setTableDashedUnderline ();

  // Determine the longest line, and the longest "added" line.
  int maxAddedLine = 0;
  int maxRemovedLine = 0;
  foreach (i, groups)
  {
    if (completedGroup[i->first] + deletedGroup[i->first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i->first] + deletedGroup[i->first];

    if (addedGroup[i->first] > maxAddedLine)
      maxAddedLine = addedGroup[i->first];
  }

  int maxLine = maxAddedLine + maxRemovedLine;

  if (maxLine > 0)
  {
    unsigned int leftOffset = (widthOfBar * maxAddedLine) / maxLine;

    int totalAdded     = 0;
    int totalCompleted = 0;
    int totalDeleted   = 0;

    int priorYear = 0;
    int row = 0;
    foreach (i, groups)
    {
      row = table.addRow ();

      totalAdded     += addedGroup[i->first];
      totalCompleted += completedGroup[i->first];
      totalDeleted   += deletedGroup[i->first];

      Date dt (i->first);
      int m, d, y;
      dt.toMDY (m, d, y);

      if (y != priorYear)
      {
        table.addCell (row, 0, y);
        priorYear = y;
      }
      table.addCell (row, 1, Date::monthName(m));

      unsigned int addedBar     = (widthOfBar *     addedGroup[i->first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i->first]) / maxLine;
      unsigned int deletedBar   = (widthOfBar *   deletedGroup[i->first]) / maxLine;

      std::string bar = "";
      if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
      {
        char number[24];
        std::string aBar = "";
        if (addedGroup[i->first])
        {
          sprintf (number, "%d", addedGroup[i->first]);
          aBar = number;
          while (aBar.length () < addedBar)
            aBar = " " + aBar;
        }

        std::string cBar = "";
        if (completedGroup[i->first])
        {
          sprintf (number, "%d", completedGroup[i->first]);
          cBar = number;
          while (cBar.length () < completedBar)
            cBar = " " + cBar;
        }

        std::string dBar = "";
        if (deletedGroup[i->first])
        {
          sprintf (number, "%d", deletedGroup[i->first]);
          dBar = number;
          while (dBar.length () < deletedBar)
            dBar = " " + dBar;
        }

        while (bar.length () < leftOffset - aBar.length ())
          bar += " ";

        bar += Text::colorize (Text::black, Text::on_red,    aBar);
        bar += Text::colorize (Text::black, Text::on_green,  cBar);
        bar += Text::colorize (Text::black, Text::on_yellow, dBar);
      }
      else
      {
        std::string aBar = ""; while (aBar.length () < addedBar)     aBar += "+";
        std::string cBar = ""; while (cBar.length () < completedBar) cBar += "X";
        std::string dBar = ""; while (dBar.length () < deletedBar)   dBar += "-";

        while (bar.length () < leftOffset - aBar.length ())
          bar += " ";

        bar += aBar + cBar + dBar;
      }

      table.addCell (row, 2, bar);
    }
  }

  std::stringstream out;
  if (table.rowCount ())
  {
    out << optionalBlankLine ()
        << table.render ()
        << std::endl;

    if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
      out << "Legend: "
          << Text::colorize (Text::black, Text::on_red, "added")
          << ", "
          << Text::colorize (Text::black, Text::on_green, "completed")
          << ", "
          << Text::colorize (Text::black, Text::on_yellow, "deleted")
          << optionalBlankLine ()
          << std::endl;
    else
      out << "Legend: + added, X completed, - deleted" << std::endl;
  }
  else
    out << "No tasks." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportTimesheet ()
{
  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.load (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Just do this once.
  int width = context.getWidth ();

  // What day of the week does the user consider the first?
  int weekStart = Date::dayOfWeek (context.config.get ("weekstart", "Sunday"));
  if (weekStart != 0 && weekStart != 1)
    throw std::string ("The 'weekstart' configuration variable may "
                       "only contain 'Sunday' or 'Monday'.");

  // Determine the date of the first day of the most recent report.
  Date today;
  Date start;
  start -= (((today.dayOfWeek () - weekStart) + 7) % 7) * 86400;

  // Roll back to midnight.
  start = Date (start.month (), start.day (), start.year ());
  Date end = start + (7 * 86400) - 1;

  // Determine how many reports to run.
  int quantity = 1;
  if (context.sequence.size () == 1)
    quantity = context.sequence[0];

  std::stringstream out;
  for (int week = 0; week < quantity; ++week)
  {
    out << std::endl
        << Text::colorize (Text::bold, Text::nocolor)
        << start.toString (context.config.get ("dateformat", "m/d/Y"))
        << " - "
        << end.toString (context.config.get ("dateformat", "m/d/Y"))
        << Text::colorize ()
        << std::endl;

    // Render the completed table.
    Table completed;
    completed.setTableWidth (width);
    completed.addColumn ("   ");
    completed.addColumn ("Project");
    completed.addColumn ("Due");
    completed.addColumn ("Description");

    completed.setColumnUnderline (1);
    completed.setColumnUnderline (2);
    completed.setColumnUnderline (3);

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
        Date compDate (::atoi (task->get ("end").c_str ()));
        if (compDate >= start && compDate < end)
        {
          int row = completed.addRow ();
          completed.addCell (row, 1, task->get ("project"));
          completed.addCell (row, 2, getDueDate (*task));
          completed.addCell (row, 3, getFullDescription (*task));

          if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
          {
            Text::color fg = Text::colorCode (task->get ("fg"));
            Text::color bg = Text::colorCode (task->get ("bg"));
            autoColorize (*task, fg, bg);
            completed.setRowFg (row, fg);
            completed.setRowBg (row, bg);
          }
        }
      }
    }

    out << "  Completed (" << completed.rowCount () << " tasks)" << std::endl;

    if (completed.rowCount ())
      out << completed.render ()
          << std::endl;

    // Now render the started table.
    Table started;
    started.setTableWidth (width);
    started.addColumn ("   ");
    started.addColumn ("Project");
    started.addColumn ("Due");
    started.addColumn ("Description");

    started.setColumnUnderline (1);
    started.setColumnUnderline (2);
    started.setColumnUnderline (3);

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
        Date startDate (::atoi (task->get ("start").c_str ()));
        if (startDate >= start && startDate < end)
        {
          int row = started.addRow ();
          started.addCell (row, 1, task->get ("project"));
          started.addCell (row, 2, getDueDate (*task));
          started.addCell (row, 3, getFullDescription (*task));

          if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
          {
            Text::color fg = Text::colorCode (task->get ("fg"));
            Text::color bg = Text::colorCode (task->get ("bg"));
            autoColorize (*task, fg, bg);
            started.setRowFg (row, fg);
            started.setRowBg (row, bg);
          }
        }
      }
    }

    out << "  Started (" << started.rowCount () << " tasks)" << std::endl;

    if (started.rowCount ())
      out << started.render ()
          << std::endl
          << std::endl;

    // Prior week.
    start -= 7 * 86400;
    end   -= 7 * 86400;
  }

  return out.str ();
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
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

  // What day of the week does the user consider the first?
  int weekStart = Date::dayOfWeek (context.config.get ("weekstart", "Sunday"));
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

    if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
        context.config.get (std::string ("fontunderline"), "true"))
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

  // Loop through months to be added on this line.
  for (int mpl = 0; mpl < monthsPerLine ; mpl++)
  {
    // Reset row counter for subsequent months
    if (mpl != 0)
      row = 0;

    // Loop through days in month and add to table.
    for (int d = 1; d <= daysInMonth.at (mpl); ++d)
    {
      Date temp (months.at (mpl), d, years.at (mpl));
      int dow = temp.dayOfWeek ();
      int woy = temp.weekOfYear (weekStart);

      if (context.config.get ("displayweeknumber", true))
        table.addCell (row, (8 * mpl), woy);

      // Calculate column id.
      int thisCol = dow +                       // 0 = Sunday
                    (weekStart == 1 ? 0 : 1) +  // Offset for weekStart
                    (8 * mpl);                  // Columns in 1 month

      if (thisCol == (8 * mpl))
        thisCol += 7;

      table.addCell (row, thisCol, d);

      if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
          today.day ()   == d              &&
          today.month () == months.at (mpl)  &&
          today.year ()  == years.at (mpl))
        table.setCellFg (row, thisCol, Text::cyan);

      std::vector <Task>::iterator it;
      for (it = all.begin (); it != all.end (); ++it)
      {
        Date due (::atoi (it->get ("due").c_str ()));

        if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
            due.day ()   == d             &&
            due.month () == months.at (mpl) &&
            due.year ()  == years.at (mpl))
        {
          table.setCellFg (row, thisCol, Text::black);
          table.setCellBg (row, thisCol, due < today ? Text::on_red : Text::on_yellow);
        }
      }

      // Check for end of week, and...
      int eow = 6;
      if (weekStart == 1)
        eow = 0;
      if (dow == eow && d < daysInMonth.at (mpl))
        row++;
    }
  }

  return table.render ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportCalendar ()
{
  // Each month requires 28 text columns width.  See how many will actually
  // fit.  But if a preference is specified, and it fits, use it.
  int width = context.getWidth ();
  int preferredMonthsPerLine = (context.config.get (std::string ("monthsperline"), 0));
  int monthsThatFit = width / 26;

  int monthsPerLine = monthsThatFit;
  if (preferredMonthsPerLine != 0 && preferredMonthsPerLine < monthsThatFit)
    monthsPerLine = preferredMonthsPerLine;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Find the oldest pending due date.
  Date oldest;
  Date newest;
  foreach (task, tasks)
  {
    if (task->has ("due"))
    {
      Date d (::atoi (task->get ("due").c_str ()));

      if (d < oldest) oldest = d;
      if (d > newest) newest = d;
    }
  }

  // Iterate from oldest due month, year to newest month, year.
  Date today;
  int mFrom = oldest.month ();
  int yFrom = oldest.year ();

  int mTo = newest.month ();
  int yTo = newest.year ();

  std::stringstream out;
  out << std::endl;
  std::string output;

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

    out << std::endl
        << optionalBlankLine ()
        << renderMonths (mFrom, yFrom, today, tasks, monthsPerLine)
        << std::endl;

    mFrom += monthsPerLine;
    if (mFrom > 12)
    {
      mFrom -= 12;
      ++yFrom;
    }
  }

  if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
    out << "Legend: "
        << Text::colorize (Text::cyan, Text::nocolor, "today")
        << ", "
        << Text::colorize (Text::black, Text::on_yellow, "due")
        << ", "
        << Text::colorize (Text::black, Text::on_red, "overdue")
        << "."
        << optionalBlankLine ()
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportStats ()
{
  std::stringstream out;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.load (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  Date now;
  time_t earliest   = time (NULL);
  time_t latest     = 1;
  int totalT        = 0;
  int deletedT      = 0;
  int pendingT      = 0;
  int completedT    = 0;
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

    time_t entry = ::atoi (it->get ("entry").c_str ());
    if (entry < earliest) earliest = entry;
    if (entry > latest)   latest   = entry;

    if (it->getStatus () == Task::completed)
    {
      time_t end = ::atoi (it->get ("end").c_str ());
      daysPending += (end - entry) / 86400.0;
    }

    if (it->getStatus () == Task::pending)
      daysPending += (now - entry) / 86400.0;

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
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));
  table.addColumn ("Category");
  table.addColumn ("Data");

  if ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false)) &&
      context.config.get (std::string ("fontunderline"), "true"))
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
    table.addCell (row, 1, e.toString (context.config.get ("dateformat", "m/d/Y")));

    Date l (latest);
    row = table.addRow ();
    table.addCell (row, 0, "Newest task");
    table.addCell (row, 1, l.toString (context.config.get ("dateformat", "m/d/Y")));

    row = table.addRow ();
    table.addCell (row, 0, "Task used for");
    table.addCell (row, 1, formatSeconds (latest - earliest));
  }

  if (totalT)
  {
    row = table.addRow ();
    table.addCell (row, 0, "Task added every");
    table.addCell (row, 1, formatSeconds ((latest - earliest) / totalT));
  }

  if (completedT)
  {
    row = table.addRow ();
    table.addCell (row, 0, "Task completed every");
    table.addCell (row, 1, formatSeconds ((latest - earliest) / completedT));
  }

  if (deletedT)
  {
    row = table.addRow ();
    table.addCell (row, 0, "Task deleted every");
    table.addCell (row, 1, formatSeconds ((latest - earliest) / deletedT));
  }

  if (pendingT || completedT)
  {
    row = table.addRow ();
    table.addCell (row, 0, "Average time pending");
    table.addCell (row, 1, formatSeconds ((int) ((daysPending / (pendingT + completedT)) * 86400)));
  }

  if (totalT)
  {
    row = table.addRow ();
    table.addCell (row, 0, "Average desc length");
    std::stringstream value;
    value << (int) (descLength / totalT) << " characters";
    table.addCell (row, 1, value.str ());
  }

  out << optionalBlankLine ()
      << table.render ()
      << optionalBlankLine ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
void gatherNextTasks (
  std::vector <Task>& tasks,
  std::vector <int>& all)
{
  // For counting tasks by project.
  std::map <std::string, int> countByProject;
  std::map <int, bool> matching;

  Date now;

  // How many items per project?  Default 3.
  int limit = context.config.get ("next", 3);

  // due:< 1wk, pri:*
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      if (tasks[i].has ("due"))
      {
        Date d (::atoi (tasks[i].get ("due").c_str ()));
        if (d < now + (7 * 24 * 60 * 60)) // if due:< 1wk
        {
          std::string project = tasks[i].get ("project");
          if (countByProject[project] < limit && matching.find (i) == matching.end ())
          {
            ++countByProject[project];
            matching[i] = true;
          }
        }
      }
    }
  }

  // due:*, pri:H
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      if (tasks[i].has ("due"))
      {
        std::string priority = tasks[i].get ("priority");
        if (priority == "H")
        {
          std::string project = tasks[i].get ("project");
          if (countByProject[project] < limit && matching.find (i) == matching.end ())
          {
            ++countByProject[project];
            matching[i] = true;
          }
        }
      }
    }
  }

  // pri:H
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      std::string priority = tasks[i].get ("priority");
      if (priority == "H")
      {
        std::string project = tasks[i].get ("project");
        if (countByProject[project] < limit && matching.find (i) == matching.end ())
        {
          ++countByProject[project];
          matching[i] = true;
        }
      }
    }
  }

  // due:*, pri:M
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      if (tasks[i].has ("due"))
      {
        std::string priority = tasks[i].get ("priority");
        if (priority == "M")
        {
          std::string project = tasks[i].get ("project");
          if (countByProject[project] < limit && matching.find (i) == matching.end ())
          {
            ++countByProject[project];
            matching[i] = true;
          }
        }
      }
    }
  }

  // pri:M
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      std::string priority = tasks[i].get ("priority");
      if (priority == "M")
      {
        std::string project = tasks[i].get ("project");
        if (countByProject[project] < limit && matching.find (i) == matching.end ())
        {
          ++countByProject[project];
          matching[i] = true;
        }
      }
    }
  }

  // due:*, pri:L
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      if (tasks[i].has ("due"))
      {
        std::string priority = tasks[i].get ("priority");
        if (priority == "L")
        {
          std::string project = tasks[i].get ("project");
          if (countByProject[project] < limit && matching.find (i) == matching.end ())
          {
            ++countByProject[project];
            matching[i] = true;
          }
        }
      }
    }
  }

  // pri:L
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      std::string priority = tasks[i].get ("priority");
      if (priority == "L")
      {
        std::string project = tasks[i].get ("project");
        if (countByProject[project] < limit && matching.find (i) == matching.end ())
        {
          ++countByProject[project];
          matching[i] = true;
        }
      }
    }
  }

  // due:, pri:
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    if (tasks[i].getStatus () == Task::pending)
    {
      if (tasks[i].has ("due"))
      {
        std::string priority = tasks[i].get ("priority");
        if (priority == "")
        {
          std::string project = tasks[i].get ("project");
          if (countByProject[project] < limit && matching.find (i) == matching.end ())
          {
            ++countByProject[project];
            matching[i] = true;
          }
        }
      }
    }
  }

  // Convert map to vector.
  foreach (i, matching)
    all.push_back (i->first);
}

///////////////////////////////////////////////////////////////////////////////
std::string getFullDescription (Task& task)
{
  std::string desc = task.get ("description");

  std::vector <Att> annotations;
  task.getAnnotations (annotations);
  foreach (anno, annotations)
  {
    Date dt (::atoi (anno->name ().substr (11, std::string::npos).c_str ()));
    std::string when = dt.toString (context.config.get ("dateformat", "m/d/Y"));
    desc += "\n" + when + " " + anno->value ();
  }

  return desc;
}

///////////////////////////////////////////////////////////////////////////////
std::string getDueDate (Task& task)
{
  std::string due = task.get ("due");
  if (due.length ())
  {
    Date d (::atoi (due.c_str ()));
    due = d.toString (context.config.get ("dateformat", "m/d/Y"));
  }

  return due;
}

///////////////////////////////////////////////////////////////////////////////
