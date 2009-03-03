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
#include <fstream>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
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
  table.addCell (row, 2, "Marks specified task as started");

  row = table.addRow ();
  table.addCell (row, 1, "task stop ID");
  table.addCell (row, 2, "Removes the 'start' time from a task");

  row = table.addRow ();
  table.addCell (row, 1, "task done ID");
  table.addCell (row, 2, "Marks the specified task as completed");

  row = table.addRow ();
  table.addCell (row, 1, "task undo ID");
  table.addCell (row, 2, "Marks the specified done task as pending, provided a report has not yet been run");

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
            << std::endl
            << "See http://www.beckingham.net/task.html for the latest releases and a full tutorial."
            << std::endl
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
    << "  recur:             Recurrence frequency"                         << "\n"
    << "  until:             Recurrence end date"                          << "\n"
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
    std::string dataLocation = expandPath (conf.get ("data.location"));
    tdb.dataDirectory (dataLocation);

    // Check for silly shadow file settings.
    std::string shadowFile = expandPath (conf.get ("shadow.file"));
    if (shadowFile != "")
    {
      if (shadowFile == dataLocation + "/pending.data")
        throw std::string ("Configuration variable 'shadow.file' is set to "
                           "overwrite your pending tasks.  Please change it.");

      if (shadowFile == dataLocation + "/completed.data")
        throw std::string ("Configuration variable 'shadow.file' is set to "
                           "overwrite your completed tasks.  Please change it.");
    }

    std::cout << runTaskCommand (argc, argv, tdb, conf);
  }

  catch (std::string& error)
  {
    std::cerr << error << std::endl;
    return -1;
  }

  catch (...)
  {
    std::cerr << "Unknown error." << std::endl;
    return -2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void nag (TDB& tdb, T& task, Config& conf)
{
  std::string nagMessage = conf.get ("nag", std::string (""));
  if (nagMessage != "")
  {
    // Load all pending.
    std::vector <T> pending;
    tdb.allPendingT (pending);

    // Counters.
    int overdue  = 0;
    int high     = 0;
    int medium   = 0;
    int low      = 0;

    // Scan all pending tasks.
    foreach (t, pending)
    {
      if (t->getId () != task.getId ())
      {
        if (getDueState (t->getAttribute ("due")) == 2)
          overdue++;

        std::string priority = t->getAttribute ("priority");
        if (priority.length ())
        {
          switch (priority[0])
          {
          case 'H': high++;   break;
          case 'M': medium++; break;
          case 'L': low++;    break;
          }
        }
      }
    }

    // Scan the current task.
    bool isOverdue = getDueState (task.getAttribute ("due")) == 2 ? true : false;

    char pri = ' ';
    std::string priority = task.getAttribute ("priority");
    if (priority.length ())
      pri = priority[0];

    // General form is "if there are no more deserving tasks", suppress the nag.
    std::cout << "# task.isOverdue = " << (isOverdue ? "true" : "false") << std::endl;
    std::cout << "# task.pri = "       << pri                            << std::endl;
    std::cout << "# task.overdue = "   << overdue                        << std::endl;
    std::cout << "# pending.high = "   << high                           << std::endl;
    std::cout << "# pending.medium = " << medium                         << std::endl;
    std::cout << "# pending.low = "    << low                            << std::endl;

    if (isOverdue                                         ) return;
    if (pri == 'H' && !overdue                            ) return;
    if (pri == 'M' && !overdue && !high                   ) return;
    if (pri == 'L' && !overdue && !high && !medium        ) return;
    if (pri == ' ' && !overdue && !high && !medium && !low) return;

    // All the excuses are made, all that remains is to nag the user.
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

    // rightNow is the current date + time.
    Date rightNow;
    Date midnight (rightNow.month (), rightNow.day (), rightNow.year ());

    if (dt < midnight)
      return 2;

    Date nextweek = midnight + 7 * 86400;
    if (dt < nextweek)
      return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Scans all tasks, and for any recurring tasks, determines whether any new
// child tasks need to be generated to fill gaps.
void handleRecurrence (TDB& tdb, std::vector <T>& tasks)
{
  std::vector <T> modified;

  // Look at all tasks and find any recurring ones.
  foreach (t, tasks)
  {
    if (t->getStatus () == T::recurring)
    {
      // Generate a list of due dates for this recurring task, regardless of
      // the mask.
      std::vector <Date> due;
      if (!generateDueDates (*t, due))
      {
        std::cout << "Task "
                  << t->getUUID ()
                  << " ("
                  << trim (t->getDescription ())
                  << ") is past its 'until' date, and has be deleted" << std::endl;
        tdb.deleteT (*t);
        continue;
      }

      // Get the mask from the parent task.
      std::string mask = t->getAttribute ("mask");

      // Iterate over the due dates, and check each against the mask.
      bool changed = false;
      unsigned int i = 0;
      foreach (d, due)
      {
        if (mask.length () <= i)
        {
          mask += '-';
          changed = true;

          T rec (*t);                                 // Clone the parent.
          rec.setId (tdb.nextId ());                  // Assign a unique id.
          rec.setUUID (uuid ());                      // New UUID.
          rec.setStatus (T::pending);                 // Shiny.
          rec.setAttribute ("parent", t->getUUID ()); // Remember mom.

          char dueDate[16];
          sprintf (dueDate, "%u", (unsigned int) d->toEpoch ());
          rec.setAttribute ("due", dueDate);          // Store generated due date.

          char indexMask[12];
          sprintf (indexMask, "%u", (unsigned int) i);
          rec.setAttribute ("imask", indexMask);      // Store index into mask.

          // Add the new task to the vector, for immediate use.
          modified.push_back (rec);

          // Add the new task to the DB.
          tdb.addT (rec);
        }

        ++i;
      }

      // Only modify the parent if necessary.
      if (changed)
      {
        t->setAttribute ("mask", mask);
        tdb.modifyT (*t);
      }
    }
    else
      modified.push_back (*t);
  }

  tasks = modified;
}

////////////////////////////////////////////////////////////////////////////////
// Determine a start date (due), an optional end date (until), and an increment
// period (recur).  Then generate a set of corresponding dates.
//
// Returns false if the parent recurring task is depleted.
bool generateDueDates (T& parent, std::vector <Date>& allDue)
{
  // Determine due date, recur period and until date.
  Date due (atoi (parent.getAttribute ("due").c_str ()));
  std::string recur = parent.getAttribute ("recur");

  bool specificEnd = false;
  Date until;
  if (parent.getAttribute ("until") != "")
  {
    until = Date (atoi (parent.getAttribute ("until").c_str ()));
    specificEnd = true;
  }

  Date now;
  for (Date i = due; ; i = getNextRecurrence (i, recur))
  {
    allDue.push_back (i);

    if (specificEnd && i > until)
    {
      // If i > until, it means there are no more tasks to generate, and if the
      // parent mask contains all + or X, then there never will be another task
      // to generate, and this parent task may be safely reaped.
      std::string mask = parent.getAttribute ("mask");
      if (mask.length () == allDue.size () &&
          mask.find ('-') == std::string::npos)
        return false;

      return true;
    }

    if (i > now)
      return true;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
Date getNextRecurrence (Date& current, std::string& period)
{
  int m = current.month ();
  int d = current.day ();
  int y = current.year ();

  // Some periods are difficult, because they can be vague.
  if (period == "monthly")
  {
    if (++m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  if (isdigit (period[0]) && period[period.length () - 1] == 'm')
  {
    std::string numeric = period.substr (0, period.length () - 1);
    int increment = atoi (numeric.c_str ());

    m += increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "quarterly")
  {
    m += 3;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (isdigit (period[0]) && period[period.length () - 1] == 'q')
  {
    std::string numeric = period.substr (0, period.length () - 1);
    int increment = atoi (numeric.c_str ());

    m += 3 * increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "semiannual")
  {
    m += 6;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "bimonthly")
  {
    m += 2;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "biannual"    ||
           period == "biyearly")
  {
    y += 2;

    return Date (m, d, y);
  }

  // If the period is an 'easy' one, add it to current, and we're done.
  int days = convertDuration (period);
  return current + (days * 86400);
}

////////////////////////////////////////////////////////////////////////////////
// When the status of a recurring child task changes, the parent task must
// update it's mask.
void updateRecurrenceMask (
  TDB& tdb,
  std::vector <T>& all,
  T& task)
{
  std::string parent = task.getAttribute ("parent");
  if (parent != "")
  {
    std::vector <T>::iterator it;
    for (it = all.begin (); it != all.end (); ++it)
    {
      if (it->getUUID () == parent)
      {
        unsigned int index = atoi (task.getAttribute ("imask").c_str ());
        std::string mask = it->getAttribute ("mask");
        if (mask.length () > index)
        {
          mask[index] = (task.getStatus () == T::pending)   ? '-'
                      : (task.getStatus () == T::completed) ? '+'
                      : (task.getStatus () == T::deleted)   ? 'X'
                      :                                       '?';

          it->setAttribute ("mask", mask);
          tdb.modifyT (*it);
        }
        else
        {
          std::string mask;
          for (unsigned int i = 0; i < index; ++i)
            mask += "?";

          mask += (task.getStatus () == T::pending)   ? '-'
                : (task.getStatus () == T::completed) ? '+'
                : (task.getStatus () == T::deleted)   ? 'X'
                :                                       '?';
        }

        return;  // No point continuing the loop.
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void updateShadowFile (TDB& tdb, Config& conf)
{
  try
  {
    // Determine if shadow file is enabled.
    std::string shadowFile = expandPath (conf.get ("shadow.file"));
    if (shadowFile != "")
    {
      std::string oldCurses = conf.get ("curses");
      std::string oldColor = conf.get ("color");
      conf.set ("curses", "off");
      conf.set ("color",  "off");

      // Run report.  Use shadow.command, using default.command as a fallback
      // with "list" as a default.
      std::string command = conf.get ("shadow.command",
                              conf.get ("default.command", "list"));
      std::vector <std::string> args;
      split (args, command, ' ');
      std::string result = runTaskCommand (args, tdb, conf);

      std::ofstream out (shadowFile.c_str ());
      if (out.good ())
      {
        out << result;
        out.close ();
      }
      else
        throw std::string ("Could not write file '") + shadowFile + "'";

      conf.set ("curses", oldCurses);
      conf.set ("color",  oldColor);
    }
    else
      throw std::string ("No specified shadow file '") + shadowFile + "'.";

    // Optionally display a notification that the shadow file was updated.
    if (conf.get (std::string ("shadow.notify"), false))
      std::cout << "[Shadow file '" << shadowFile << "' updated]" << std::endl;
  }

  catch (std::string& error)
  {
    std::cout << error << std::endl;
  }

  catch (...)
  {
    std::cout << "Unknown error." << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string runTaskCommand (
  int argc,
  char** argv,
  TDB& tdb,
  Config& conf,
  bool gc /* = true */,
  bool shadow /* = true */)
{
  std::vector <std::string> args;
  for (int i = 1; i < argc; ++i)
    args.push_back (argv[i]);

  return runTaskCommand (args, tdb, conf, gc, shadow);
}

////////////////////////////////////////////////////////////////////////////////
std::string runTaskCommand (
  std::vector <std::string>& args,
  TDB& tdb,
  Config& conf,
  bool gc /* = false */,
  bool shadow /* = false */)
{
  // If argc == 1 and the default.command configuration variable is set,
  // then use that, otherwise stick with argc/argv.
  std::string defaultCommand = conf.get ("default.command");
  if (args.size () == 0 && defaultCommand != "")
  {
    // Stuff the command line.
    args.clear ();
    split (args, defaultCommand, ' ');
    std::cout << "[task " << defaultCommand << "]" << std::endl;
  }

  loadCustomReports (conf);

  std::string command;
  T task;
  parse (args, command, task, conf);

  std::string out = "";

       if (command == "" && task.getId ())  {                          handleModify         (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "add")                {                          handleAdd            (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "done")               {                          handleDone           (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "export")             {                          handleExport         (tdb, task, conf         );                                           }
  else if (command == "projects")           {                    out = handleProjects       (tdb, task, conf         );                                           }
  else if (command == "tags")               {                    out = handleTags           (tdb, task, conf         );                                           }
  else if (command == "info")               {                    out = handleInfo           (tdb, task, conf         );                                           }
  else if (command == "undelete")           {                    out = handleUndelete       (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "delete")             {                    out = handleDelete         (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "start")              {                    out = handleStart          (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "stop")               {                    out = handleStop           (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "undo")               {                    out = handleUndo           (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "stats")              {                    out = handleReportStats    (tdb, task, conf         );                                           }
  else if (command == "completed")          { if (gc) tdb.gc (); out = handleCompleted      (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); } // TODO replace with Custom
  else if (command == "summary")            { if (gc) tdb.gc (); out = handleReportSummary  (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "next")               { if (gc) tdb.gc (); out = handleReportNext     (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); } // TODO replace with Custom
  else if (command == "history")            { if (gc) tdb.gc (); out = handleReportHistory  (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "ghistory")           { if (gc) tdb.gc (); out = handleReportGHistory (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "calendar")           { if (gc) tdb.gc (); out = handleReportCalendar (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); }
  else if (command == "active")             { if (gc) tdb.gc (); out = handleReportActive   (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); } // TODO replace with Custom
  else if (command == "overdue")            { if (gc) tdb.gc (); out = handleReportOverdue  (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); } // TODO replace with Custom
  else if (command == "oldest")             { if (gc) tdb.gc (); out = handleReportOldest   (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); } // TODO replace with Custom
  else if (command == "newest")             { if (gc) tdb.gc (); out = handleReportNewest   (tdb, task, conf         ); if (shadow) updateShadowFile (tdb, conf); } // TODO replace with Custom
  else if (command == "colors")             {                    out = handleColor          (           conf         );                                           }
  else if (command == "version")            {                    out = handleVersion        (           conf         );                                           }
  else if (command == "help")               {                          longUsage            (           conf         );                                           }
  else if (isCustomReport (command))        { if (gc) tdb.gc (); out = handleCustomReport   (tdb, task, conf, command); if (shadow) updateShadowFile (tdb, conf); } // New Custom reports
  else                                      {                          shortUsage           (           conf         );                                           }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
