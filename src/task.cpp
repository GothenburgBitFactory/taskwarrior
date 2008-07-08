////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include <fstream>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Config.h"
#include "Date.h"
#include "Table.h"
#include "TDB.h"
#include "T.h"
#include "task.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

////////////////////////////////////////////////////////////////////////////////
static void shortUsage (Config& conf)
{
  Table table;
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  table.addColumn (" ");
  table.addColumn (" ");
  table.addColumn (" ");

  table.setColumnJustification (0, Table::left);
  table.setColumnJustification (1, Table::left);
  table.setColumnJustification (2, Table::left);

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::flexible);
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

  int row = table.addRow ();
  table.addCell (row, 0, "Usage:");
  table.addCell (row, 1, "task");

  row = table.addRow ();
  table.addCell (row, 1, "task add [tags] [attrs] desc...");
  table.addCell (row, 2, "Adds a new task");

  row = table.addRow ();
  table.addCell (row, 1, "task list [tags] [attrs] desc...");
  table.addCell (row, 2, "Lists all tasks matching the specified criteria");

  row = table.addRow ();
  table.addCell (row, 1, "task long [tags] [attrs] desc...");
  table.addCell (row, 2, "Lists all task, all data, matching the specified criteria");

  row = table.addRow ();
  table.addCell (row, 1, "task ls [tags] [attrs] desc...");
  table.addCell (row, 2, "Minimal listing of all tasks matching the specified criteria");

  row = table.addRow ();
  table.addCell (row, 1, "task completed [tags] [attrs] desc...");
  table.addCell (row, 2, "Chronological listing of all completed tasks matching the specified criteria");

  row = table.addRow ();
  table.addCell (row, 1, "task ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Modifies the existing task with provided arguments");

  row = table.addRow ();
  table.addCell (row, 1, "task ID /from/to/");
  table.addCell (row, 2, "Perform the substitution on the desc, for fixing mistakes");

  row = table.addRow ();
  table.addCell (row, 1, "task delete ID");
  table.addCell (row, 2, "Deletes the specified task");

  row = table.addRow ();
  table.addCell (row, 1, "task undelete ID");
  table.addCell (row, 2, "Undeletes the specified task, provided a report has not yet been run");

  row = table.addRow ();
  table.addCell (row, 1, "task info ID");
  table.addCell (row, 2, "Shows all data, metadata for specified task");

  row = table.addRow ();
  table.addCell (row, 1, "task start ID");
  table.addCell (row, 2, "Marks specified task as started, starts the clock ticking");

  row = table.addRow ();
  table.addCell (row, 1, "task done ID");
  table.addCell (row, 2, "Marks the specified task as completed");

  row = table.addRow ();
  table.addCell (row, 1, "task projects");
  table.addCell (row, 2, "Shows a list of all project names used, and how many tasks are in each");

  row = table.addRow ();
  table.addCell (row, 1, "task tags");
  table.addCell (row, 2, "Shows a list of all tags used");

  row = table.addRow ();
  table.addCell (row, 1, "task summary");
  table.addCell (row, 2, "Shows a report of task status by project");

  row = table.addRow ();
  table.addCell (row, 1, "task history");
  table.addCell (row, 2, "Shows a report of task history, by month");

  row = table.addRow ();
  table.addCell (row, 1, "task ghistory");
  table.addCell (row, 2, "Shows a graphical report of task history, by month");

  row = table.addRow ();
  table.addCell (row, 1, "task next");
  table.addCell (row, 2, "Shows the most important tasks for each project");

  row = table.addRow ();
  table.addCell (row, 1, "task calendar");
  table.addCell (row, 2, "Shows a monthly calendar, with due tasks marked");

  row = table.addRow ();
  table.addCell (row, 1, "task active");
  table.addCell (row, 2, "Shows all task that are started, but not completed");

  row = table.addRow ();
  table.addCell (row, 1, "task overdue");
  table.addCell (row, 2, "Shows all incomplete tasks that are beyond their due date");

  row = table.addRow ();
  table.addCell (row, 1, "task oldest");
  table.addCell (row, 2, "Shows the oldest tasks");

  row = table.addRow ();
  table.addCell (row, 1, "task newest");
  table.addCell (row, 2, "Shows the newest tasks");

  row = table.addRow ();
  table.addCell (row, 1, "task stats");
  table.addCell (row, 2, "Shows task database statistics");

  row = table.addRow ();
  table.addCell (row, 1, "task usage");
  table.addCell (row, 2, "Shows task command usage frequency");

  row = table.addRow ();
  table.addCell (row, 1, "task export");
  table.addCell (row, 2, "Exports all tasks as a CSV file");

  row = table.addRow ();
  table.addCell (row, 1, "task color");
  table.addCell (row, 2, "Displays all possible colors");

  row = table.addRow ();
  table.addCell (row, 1, "task version");
  table.addCell (row, 2, "Shows the task version number");

  row = table.addRow ();
  table.addCell (row, 1, "task help");
  table.addCell (row, 2, "Shows the long usage text");

  std::cout << table.render ()
            << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
static void longUsage (Config& conf)
{
  shortUsage (conf);

  std::cout
    << "ID is the numeric identifier displayed by the 'task list' command" << "\n"
    <<                                                                        "\n"
    << "Tags are arbitrary words, any quantity:"                           << "\n"
    << "  +tag               The + means add the tag"                      << "\n"
    << "  -tag               The - means remove the tag"                   << "\n"
    <<                                                                        "\n"
    << "Attributes are:"                                                   << "\n"
    << "  project:           Project name"                                 << "\n"
    << "  priority:          Priority"                                     << "\n"
    << "  due:               Due date"                                     << "\n"
    << "  fg:                Foreground color"                             << "\n"
    << "  bg:                Background color"                             << "\n"
    << "  rc:                Alternate .taskrc file"                       << "\n"
    <<                                                                        "\n"
    << "Any command or attribute name may be abbreviated if still unique:" << "\n"
    << "  task list project:Home"                                          << "\n"
    << "  task li       pro:Home"                                          << "\n"
    <<                                                                        "\n"
    << "Some task descriptions need to be escaped because of the shell:"   << "\n"
    << "  task add \"quoted ' quote\""                                     << "\n"
    << "  task add escaped \\' quote"                                      << "\n"
    <<                                                                        "\n"
    << "Many characters have special meaning to the shell, including:"     << "\n"
    << "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~"                   << "\n"
    << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void loadConfFile (int argc, char** argv, Config& conf)
{
  for (int i = 1; i < argc; ++i)
  {
    if (! strncmp (argv[i], "rc:", 3))
    {
      if (! access (&(argv[i][3]), F_OK))
      {
        std::string file = &(argv[i][3]);
        conf.load (file);
        return;
      }
      else
        throw std::string ("Could not read configuration file '") + &(argv[i][3]) + "'";
    }
  }

  struct passwd* pw = getpwuid (getuid ());
  if (!pw)
    throw std::string ("Could not read home directory from passwd file.");

  std::string file = pw->pw_dir;
  conf.createDefault (file);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
// TODO Find out what this is, and either promote it to live code, or remove it.
//  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);

  // Set up randomness.
#ifdef HAVE_SRANDOM
  srandom (time (NULL));
#else
  srand (time (NULL));
#endif

  try
  {
    // Load the config file from the home directory.  If the file cannot be
    // found, offer to create a sample one.
    Config conf;
    loadConfFile (argc, argv, conf);

    // When redirecting output to a file, do not use color, curses.
    if (!isatty (fileno (stdout)))
    {
      conf.set ("curses", "off");
      conf.set ("color",  "off");
    }

    TDB tdb;
    tdb.dataDirectory (conf.get ("data.location"));

    // Log commands, if desired.
    if (conf.get ("command.logging") == "on")
      tdb.logCommand (argc, argv);

    // Parse the command line.
    std::vector <std::string> args;
    for (int i = 1; i < argc; ++i)
      args.push_back (argv[i]);
    std::string command;
    T task;
    parse (args, command, task, conf);

         if (command == "add")                handleAdd            (tdb, task, conf);
    else if (command == "projects")           handleProjects       (tdb, task, conf);
    else if (command == "tags")               handleTags           (tdb, task, conf);
    else if (command == "list")               handleList           (tdb, task, conf);
    else if (command == "info")               handleInfo           (tdb, task, conf);
    else if (command == "undelete")           handleUndelete       (tdb, task, conf);
    else if (command == "long")               handleLongList       (tdb, task, conf);
    else if (command == "ls")                 handleSmallList      (tdb, task, conf);
    else if (command == "colors")             handleColor          (           conf);
    else if (command == "completed")          handleCompleted      (tdb, task, conf);
    else if (command == "delete")             handleDelete         (tdb, task, conf);
    else if (command == "start")              handleStart          (tdb, task, conf);
    else if (command == "done")               handleDone           (tdb, task, conf);
    else if (command == "export")             handleExport         (tdb, task, conf);
    else if (command == "version")            handleVersion        (           conf);
    else if (command == "summary")            handleReportSummary  (tdb, task, conf);
    else if (command == "next")               handleReportNext     (tdb, task, conf);
    else if (command == "history")            handleReportHistory  (tdb, task, conf);
    else if (command == "ghistory")           handleReportGHistory (tdb, task, conf);
    else if (command == "calendar")           handleReportCalendar (tdb, task, conf);
    else if (command == "active")             handleReportActive   (tdb, task, conf);
    else if (command == "overdue")            handleReportOverdue  (tdb, task, conf);
    else if (command == "oldest")             handleReportOldest   (tdb, task, conf);
    else if (command == "newest")             handleReportNewest   (tdb, task, conf);
    else if (command == "stats")              handleReportStats    (tdb, task, conf);
    else if (command == "usage")              handleReportUsage    (tdb, task, conf);
    else if (command == "" && task.getId ())  handleModify         (tdb, task, conf);
    else if (command == "help")               longUsage (conf);
    else                                      shortUsage (conf);
  }

  catch (std::string& error)
  {
    std::cout << error << std::endl;
    return -1;
  }

  catch (...)
  {
    std::cout << "Unknown error." << std::endl;
    return -2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void nag (const TDB& tdb, T& task, Config& conf)
{
  std::string nagMessage = conf.get ("nag", std::string (""));
  if (nagMessage != "")
  {
    // Load all pending.
    std::vector <T> pending;
    tdb.allPendingT (pending);

    // Restrict to matching subset.
    std::vector <int> matching;
    gatherNextTasks (tdb, task, conf, pending, matching);

    foreach (i, matching)
      if (pending[*i].getId () == task.getId ())
        return;

    std::cout << nagMessage << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Determines whether a task is overdue.  Returns
//   0 = not due at all
//   1 = imminent
//   2 = overdue
int getDueState (const std::string& due)
{
  if (due.length ())
  {
    Date dt (::atoi (due.c_str ()));
    Date now;

    if (dt < now)
      return 2;

    Date nextweek = now + 7 * 86400;
    if (dt < nextweek)
      return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Scan for recurring tasks, and generate any necessary instances of those
// tasks.
void handleRecurrence (const TDB& tdb, std::vector <T>& tasks)
{
  std::vector <T> modified;
  Date now;

  std::cout << "# handleRecurrence" << std::endl;
  std::vector <T>::iterator it;
  for (it = tasks.begin (); it != tasks.end (); ++it)
  {
    if (it->getStatus () == T::recurring)
    {
      std::cout << "# found recurring task " << it->getUUID () << std::endl;

      // This task is recurring.  While it remains hidden from view, it spawns
      // child tasks automatically, here, that are regular tasks, except they
      // have a "parent" attribute that contains the UUID of the original.

      // Generate a list of child tasks.
      std::vector <T> children;
      std::vector <T>::iterator them;
      for (them = tasks.begin (); them != tasks.end (); ++them)
        if (them->getAttribute ("parent") == it->getUUID ())
          children.push_back (*them);

      // Determine due date, recur period and until date.
      Date due (atoi (it->getAttribute ("due").c_str ()));
      std::cout << "# due=" << due.toString () << std::endl;
      std::string recur = it->getAttribute ("recur");
      std::cout << "# recur=" << recur << std::endl;

      bool specificEnd = false;
      Date until;
      if (it->getAttribute ("until") != "")
      {
        until = Date (atoi (it->getAttribute ("until").c_str ()));
        specificEnd = true;
      }

      std::cout << "# specficEnd=" << (specificEnd ? "true" : "false") << std::endl;
      if (specificEnd)
        std::cout << "# until=" << until.toString () << std::endl;

      for (Date i = due; ; i = getNextRecurrence (i, recur))
      {
        std::cout << "# i=" << i.toString () << std::endl;
        if (specificEnd && i > until)
          break;

        // Look to see if there is a gap at date "i" by scanning children.
        bool foundChild = false;
        std::vector <T>::iterator cit;
        for (cit = children.begin (); cit != children.end (); ++cit)
        {
          if (atoi (cit->getAttribute ("due").c_str ()) == i.toEpoch ())
          {
            foundChild = true;
            break;
          }
        }

// TODO A gap may be filled by a completed task.  Oh crap.

        // There is a gap, so insert a task.
        if (!foundChild)
        {
          std::cout << "# found a gap at i=" << i.toString () << std::endl;
          T rec (*it);  // Clone the parent.

          char dueDate[16];
          sprintf (dueDate, "%u", (unsigned int) i.toEpoch ());
          rec.setAttribute ("due", dueDate);
          rec.setAttribute ("parent", it->getUUID ());
          rec.setStatus (T::pending);

          std::cout << "# adding to modified" << std::endl;
          modified.push_back (rec);
          std::cout << "# adding to pending" << std::endl;
          tdb.addT (rec);
        }

        if (i > now)
        {
          std::cout << "# already 1 instance into the future, stopping" << std::endl;
          break;
        }
      }
    }
    else
      modified.push_back (*it);
  }

  tasks = modified;
}

////////////////////////////////////////////////////////////////////////////////
Date getNextRecurrence (Date& current, std::string& period)
{
  int days = convertDuration (period);

  // Some periods are difficult, because they can be vague.
  if (period == "monthly" ||
      (isdigit (period[0]) && period[period.length () - 1] == 'm'))
  {
    int m = current.month ();
    int d = current.day ();
    int y = current.year ();

    if (++m == 13) m = 1;
    while (! Date::valid (m, d, y))
      --d;

    std::cout << "# next " << current.toString () << " + " << period << " = " << m << "/" << d << "/" << y << std::endl;
    return Date (m, d, y);
  }

  if (period == "bimonthly"   ||
      period == "semimonthly" ||
      period == "quarterly"   ||
      period == "biannual"    ||
      period == "biyearly"    ||
      period == "semiannual"  ||
      (isdigit (period[0]) && (
         period[period.length () - 1] == 'm' ||
         period[period.length () - 1] == 'q')))
  {
    // TODO lots of work here...
  }

  // If the period is an 'easy' one, add it to current, and we're done.
  return current + (days * 86400);
}

////////////////////////////////////////////////////////////////////////////////
