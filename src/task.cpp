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
#include <sstream>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Context.h"
#include "Date.h"
#include "Duration.h"
#include "Table.h"
#include "TDB.h"
#include "T.h"
#include "text.h"
#include "util.h"
#include "task.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

// Global context for use by all.
Context context;

////////////////////////////////////////////////////////////////////////////////
std::string shortUsage ()
{
  std::stringstream out;
  Table table;
  int width = context.config.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (context.config.get ("curses", true))
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
  table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

  int row = table.addRow ();
  table.addCell (row, 0, "Usage:");
  table.addCell (row, 1, "task");

  row = table.addRow ();
  table.addCell (row, 1, "task add [tags] [attrs] desc...");
  table.addCell (row, 2, "Adds a new task");

  row = table.addRow ();
  table.addCell (row, 1, "task append [tags] [attrs] desc...");
  table.addCell (row, 2, "Appends more description to an existing task");

  row = table.addRow ();
  table.addCell (row, 1, "task annotate ID desc...");
  table.addCell (row, 2, "Adds an annotation to an existing task");

  row = table.addRow ();
  table.addCell (row, 1, "task completed [tags] [attrs] desc...");
  table.addCell (row, 2, "Chronological listing of all completed tasks matching the specified criteria");

  row = table.addRow ();
  table.addCell (row, 1, "task ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Modifies the existing task with provided arguments");

  row = table.addRow ();
  table.addCell (row, 1, "task ID /from/to/");
  table.addCell (row, 2, "Performs one substitution on the task description, for fixing mistakes");

  row = table.addRow ();
  table.addCell (row, 1, "task ID /from/to/g");
  table.addCell (row, 2, "Performs all substitutions on the task description, for fixing mistakes");

  row = table.addRow ();
  table.addCell (row, 1, "task edit ID");
  table.addCell (row, 2, "Launches an editor to let you modify all aspects of a task directly, therefore it is to be used carefully");

  row = table.addRow ();
  table.addCell (row, 1, "task duplicate ID [tags] [attrs] [desc...]");
  table.addCell (row, 2, "Duplicates the specified task, and allows modifications");

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
  table.addCell (row, 1, "task done ID [tags] [attrs] [desc...]");
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
  table.addCell (row, 1, "task timesheet [weeks]");
  table.addCell (row, 2, "Shows a weekly report of tasks completed and started");

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
  table.addCell (row, 1, "task stats");
  table.addCell (row, 2, "Shows task database statistics");

  row = table.addRow ();
  table.addCell (row, 1, "task import");
  table.addCell (row, 2, "Imports tasks from a variety of formats");

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

  // Add custom reports here...
  std::vector <std::string> all;
  allCustomReports (all);
  foreach (report, all)
  {
    std::string command = std::string ("task ") + *report + std::string (" [tags] [attrs] desc...");
    std::string description = context.config.get (
      std::string ("report.") + *report + ".description", std::string ("(missing description)"));

    row = table.addRow ();
    table.addCell (row, 1, command);
    table.addCell (row, 2, description);
  }

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
      <<                                                                         "\n"
      << "Many characters have special meaning to the shell, including:"      << "\n"
      << "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~"                    << "\n"
      << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  // Set up randomness.
#ifdef HAVE_SRANDOM
  srandom (time (NULL));
#else
  srand (time (NULL));
#endif

  int status = 0;

  try
  {
    context.initialize (argc, argv);
    if (context.program.find ("itask") != std::string::npos)
      status = context.interactive ();
    else
      status = context.run ();

// start OBSOLETE
/*
    TDB tdb;
    std::string dataLocation = expandPath (context.config.get ("data.location"));
    tdb.dataDirectory (dataLocation);

    // Allow user override of file locking.  Solaris/NFS machines may want this.
    if (! context.config.get ("locking", true))
      tdb.noLock ();

    // Check for silly shadow file settings.
    std::string shadowFile = expandPath (context.config.get ("shadow.file"));
    if (shadowFile != "")
    {
      if (shadowFile == dataLocation + "/pending.data")
        throw std::string ("Configuration variable 'shadow.file' is set to "
                           "overwrite your pending tasks.  Please change it.");

      if (shadowFile == dataLocation + "/completed.data")
        throw std::string ("Configuration variable 'shadow.file' is set to "
                           "overwrite your completed tasks.  Please change it.");
    }

    std::cout << runTaskCommand (context.args, tdb);
*/
// end OBSOLETE
  }

  catch (std::string& error)
  {
    std::cout << error << std::endl;
    return -1;
  }

  catch (...)
  {
    std::cerr << context.stringtable.get (100, "Unknown error.") << std::endl;
    return -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
void updateShadowFile (TDB& tdb)
{
  try
  {
    // Determine if shadow file is enabled.
    std::string shadowFile = expandPath (context.config.get ("shadow.file"));
    if (shadowFile != "")
    {
      std::string oldCurses = context.config.get ("curses");
      std::string oldColor = context.config.get ("color");
      context.config.set ("curses", "off");
      context.config.set ("color",  "off");

      // Run report.  Use shadow.command, using default.command as a fallback
      // with "list" as a default.
      std::string command = context.config.get ("shadow.command",
                              context.config.get ("default.command", "list"));
      std::vector <std::string> args;
      split (args, command, ' ');
      std::string result = runTaskCommand (args, tdb);

      std::ofstream out (shadowFile.c_str ());
      if (out.good ())
      {
        out << result;
        out.close ();
      }
      else
        throw std::string ("Could not write file '") + shadowFile + "'";

      context.config.set ("curses", oldCurses);
      context.config.set ("color",  oldColor);
    }

    // Optionally display a notification that the shadow file was updated.
    if (context.config.get (std::string ("shadow.notify"), false))
      std::cout << "[Shadow file '" << shadowFile << "' updated]" << std::endl;
  }

  catch (std::string& error)
  {
    std::cerr << error << std::endl;
  }

  catch (...)
  {
    std::cerr << "Unknown error." << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
// TODO Obsolete
std::string runTaskCommand (
  int argc,
  char** argv,
  TDB& tdb,
  bool gc /* = true */,
  bool shadow /* = true */)
{
  std::vector <std::string> args;
  for (int i = 1; i < argc; ++i)
    if (strncmp (argv[i], "rc:", 3) &&
        strncmp (argv[i], "rc.", 3))
{
std::cout << "arg=" << argv[i] << std::endl;
      args.push_back (argv[i]);
}

  return runTaskCommand (args, tdb, gc, shadow);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Obsolete
std::string runTaskCommand (
  std::vector <std::string>& args,
  TDB& tdb,
  bool gc /* = false */,
  bool shadow /* = false */)
{
  // If argc == 1 and there is a default.command, use it.  Otherwise use
  // argc/argv.
  std::string defaultCommand = context.config.get ("default.command");
  if (args.size () == 0 || defaultCommand != "")
  {
    // Stuff the command line.
    args.clear ();
    split (args, defaultCommand, ' ');
    std::cout << "[task " << defaultCommand << "]" << std::endl;
  }

  loadCustomReports ();

  std::string command;
  T task;
  parse (args, command, task);

  bool gcMod  = false; // Change occurred by way of gc.
  bool cmdMod = false; // Change occurred by way of command type.
  std::string out;
/*
  // Read-only commands with no side effects.
       if (command == "export")             { out = handleExport          (tdb, task); }
  else if (command == "info")               { out = handleInfo            (tdb, task); }
  else if (command == "stats")              { out = handleReportStats     (tdb, task); }
  else if (command == "history")            { out = handleReportHistory   (tdb, task); }
  else if (command == "ghistory")           { out = handleReportGHistory  (tdb, task); }
  else if (command == "calendar")           { out = handleReportCalendar  (tdb, task); }
  else if (command == "summary")            { out = handleReportSummary   (tdb, task); }
  else if (command == "timesheet")          { out = handleReportTimesheet (tdb, task); }

  // Commands that cause updates.
  else if (command == "" && task.getId ())  { cmdMod = true; out = handleModify    (tdb, task); }
  else if (command == "add")                { cmdMod = true; out = handleAdd       (tdb, task); }
  else if (command == "append")             { cmdMod = true; out = handleAppend    (tdb, task); }
  else if (command == "annotate")           { cmdMod = true; out = handleAnnotate  (tdb, task); }
  else if (command == "done")               { cmdMod = true; out = handleDone      (tdb, task); }
  else if (command == "undelete")           { cmdMod = true; out = handleUndelete  (tdb, task); }
  else if (command == "delete")             { cmdMod = true; out = handleDelete    (tdb, task); }
  else if (command == "start")              { cmdMod = true; out = handleStart     (tdb, task); }
  else if (command == "stop")               { cmdMod = true; out = handleStop      (tdb, task); }
  else if (command == "undo")               { cmdMod = true; out = handleUndo      (tdb, task); }
  else if (command == "import")             { cmdMod = true; out = handleImport    (tdb, task); }
  else if (command == "duplicate")          { cmdMod = true; out = handleDuplicate (tdb, task); }
  else if (command == "edit")               { cmdMod = true; out = handleEdit      (tdb, task); }

  // Command that display IDs and therefore need TDB::gc first.
  else if (command == "completed")          { if (gc) gcMod = tdb.gc (); out = handleCompleted     (tdb, task); }
  else if (command == "next")               { if (gc) gcMod = tdb.gc (); out = handleReportNext    (tdb, task); }
  else if (command == "active")             { if (gc) gcMod = tdb.gc (); out = handleReportActive  (tdb, task); }
  else if (command == "overdue")            { if (gc) gcMod = tdb.gc (); out = handleReportOverdue (tdb, task); }
  else if (isCustomReport (command))        { if (gc) gcMod = tdb.gc (); out = handleCustomReport  (tdb, task, command); }

  // Only update the shadow file if such an update was not suppressed (shadow),
  // and if an actual change occurred (gcMod || cmdMod).
  if (shadow && (gcMod || cmdMod))
    updateShadowFile (tdb);
*/

  return out;
}

////////////////////////////////////////////////////////////////////////////////
