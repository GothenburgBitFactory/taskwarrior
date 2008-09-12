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
void filter (std::vector<T>& all, T& task)
{
  std::vector <T> filtered;

  // Split any description specified into words.
  std::vector <std::string> descWords;
  split (descWords, lowerCase (task.getDescription ()), ' ');

  // Get all the tags to match against.
  std::vector <std::string> tagList;
  task.getTags (tagList);

  // Get all the attributes to match against.
  std::map <std::string, std::string> attrList;
  task.getAttributes (attrList);

  // Iterate over each task, and apply selection criteria.
  for (unsigned int i = 0; i < all.size (); ++i)
  {
    T refTask (all[i]);

    // Apply description filter.
    std::string desc = lowerCase (refTask.getDescription ());
    unsigned int matches = 0;
    for (unsigned int w = 0; w < descWords.size (); ++w)
      if (desc.find (descWords[w]) != std::string::npos)
        ++matches;

    if (matches == descWords.size ())
    {
      // Apply attribute filter.
      matches = 0;
      foreach (a, attrList)
        if (a->first == "project")
        {
          if (a->second.length () <= refTask.getAttribute (a->first).length ())
            if (a->second == refTask.getAttribute (a->first).substr (0, a->second.length ()))
              ++matches;
        }
        else if (a->second == refTask.getAttribute (a->first))
          ++matches;

      if (matches == attrList.size ())
      {
        // Apply tag filter.
        matches = 0;
        for (unsigned int t = 0; t < tagList.size (); ++t)
          if (refTask.hasTag (tagList[t]))
            ++matches;

        if (matches == tagList.size ())
          filtered.push_back (refTask);
      }
    }
  }

  all = filtered;
}

////////////////////////////////////////////////////////////////////////////////
// Successively apply filters based on the task object built from the command
// line.  Tasks that match all the specified criteria are listed.
void handleList (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  bool showAge = conf.get ("showage", true);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Active");
  if (showAge) table.addColumn ("Age");
  table.addColumn ("Description");

  if (conf.get (std::string ("color"), true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
    if (showAge) table.setColumnUnderline (6);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::minimum);
  if (showAge) table.setColumnWidth (5, Table::minimum);
  table.setColumnWidth ((showAge ? 6 : 5), Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);
  if (showAge) table.setColumnJustification (5, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);
    if (refTask.getStatus () != T::pending)
      continue;

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = refTask.getAttribute ("due");
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
      due = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string active;
    if (refTask.getAttribute ("start") != "")
      active = "*";

    std::string age;
    std::string created = refTask.getAttribute ("entry");
    if (created.length ())
    {
      Date now;
      Date dt (::atoi (created.c_str ()));
      formatTimeDeltaDays (age, (time_t) (now - dt));
    }

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, refTask.getId ());
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getAttribute ("priority"));
    table.addCell (row, 3, due);
    table.addCell (row, 4, active);
    if (showAge) table.addCell (row, 5, age);
    table.addCell (row, (showAge ? 6 : 5), refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::red);
        else if (imminent)
          table.setCellFg (row, 3, Text::yellow);
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Successively apply filters based on the task object built from the command
// line.  Tasks that match all the specified criteria are listed.  Show a narrow
// list that works better on mobile devices.
void handleSmallList (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::left);

  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  // Iterate over each task, and apply selection criteria.
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = refTask.getAttribute ("due");
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
      due = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string active;
    if (refTask.getAttribute ("start") != "")
      active = "*";

    std::string age;
    std::string created = refTask.getAttribute ("entry");
    if (created.length ())
    {
      Date now;
      Date dt (::atoi (created.c_str ()));
      formatTimeDeltaDays (age, (time_t) (now - dt));
    }

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, refTask.getId ());
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getAttribute ("priority"));
    table.addCell (row, 3, refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::red);
        else if (imminent)
          table.setCellFg (row, 3, Text::yellow);
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Successively apply filters based on the task object built from the command
// line.  Tasks that match all the specified criteria are listed.
void handleCompleted (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.completedT (tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("Done");
  table.addColumn ("Project");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (1, Table::left);
  table.setColumnJustification (2, Table::left);

  table.sortOn (0, Table::ascendingDate);

  // Iterate over each task, and apply selection criteria.
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);

    // Now format the matching task.
    Date end (::atoi (refTask.getAttribute ("end").c_str ()));

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();

    table.addCell (row, 0, end.toString (conf.get ("dateformat", "m/d/Y")));
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Display all information for the given task.
void handleInfo (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get all the tasks.
  std::vector <T> tasks;
  tdb.allPendingT (tasks);

  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

  table.addColumn ("Name");
  table.addColumn ("Value");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);

  table.setColumnJustification (0, Table::left);
  table.setColumnJustification (1, Table::left);

  // Find the task.
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);

    if (refTask.getId () == task.getId ())
    {
      Date now;

      int row = table.addRow ();
      table.addCell (row, 0, "ID");
      table.addCell (row, 1, refTask.getId ());

      row = table.addRow ();
      table.addCell (row, 0, "Status");
      table.addCell (row, 1, (  refTask.getStatus () == T::pending   ? "Pending"
                              : refTask.getStatus () == T::completed ? "Completed"
                              : refTask.getStatus () == T::deleted   ? "Deleted"
                              : refTask.getStatus () == T::recurring ? "Recurring"
                              : ""));

      row = table.addRow ();
      table.addCell (row, 0, "Description");
      table.addCell (row, 1, refTask.getDescription ());

      if (refTask.getAttribute ("project") != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Project");
        table.addCell (row, 1, refTask.getAttribute ("project"));
      }

      if (refTask.getAttribute ("priority") != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Priority");
        table.addCell (row, 1, refTask.getAttribute ("priority"));
      }

      if (refTask.getStatus () == T::recurring)
      {
        row = table.addRow ();
        table.addCell (row, 0, "Recurrence");
        table.addCell (row, 1, refTask.getAttribute ("recur"));

        row = table.addRow ();
        table.addCell (row, 0, "Recur until");
        table.addCell (row, 1, refTask.getAttribute ("until"));

        row = table.addRow ();
        table.addCell (row, 0, "Mask");
        table.addCell (row, 1, refTask.getAttribute ("mask"));
      }

      if (refTask.getAttribute ("parent") != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Parent task");
        table.addCell (row, 1, refTask.getAttribute ("parent"));

        row = table.addRow ();
        table.addCell (row, 0, "Mask Index");
        table.addCell (row, 1, refTask.getAttribute ("imask"));
      }

      // due (colored)
      bool imminent = false;
      bool overdue = false;
      std::string due = refTask.getAttribute ("due");
      if (due != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Due");

        Date dt (::atoi (due.c_str ()));
        due = dt.toString (conf.get ("dateformat", "m/d/Y"));
        table.addCell (row, 1, due);

        if (due.length ())
        {
          overdue = (dt < now) ? true : false;
          Date nextweek = now + 7 * 86400;
          imminent = dt < nextweek ? true : false;

          if (conf.get ("color", true))
          {
            if (overdue)
              table.setCellFg (row, 1, Text::red);
            else if (imminent)
              table.setCellFg (row, 1, Text::yellow);
          }
        }
      }

      // start
      if (refTask.getAttribute ("start") != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "Start");
        Date dt (::atoi (refTask.getAttribute ("start").c_str ()));
        table.addCell (row, 1, dt.toString (conf.get ("dateformat", "m/d/Y")));
      }

      // end
      if (refTask.getAttribute ("end") != "")
      {
        row = table.addRow ();
        table.addCell (row, 0, "End");
        Date dt (::atoi (refTask.getAttribute ("end").c_str ()));
        table.addCell (row, 1, dt.toString (conf.get ("dateformat", "m/d/Y")));
      }

      // tags ...
      std::vector <std::string> tags;
      refTask.getTags (tags);
      if (tags.size ())
      {
        std::string allTags;
        join (allTags, " ", tags);

        row = table.addRow ();
        table.addCell (row, 0, "Tags");
        table.addCell (row, 1, allTags);
      }

      row = table.addRow ();
      table.addCell (row, 0, "UUID");
      table.addCell (row, 1, refTask.getUUID ());

      row = table.addRow ();
      table.addCell (row, 0, "Entered");
      Date dt (::atoi (refTask.getAttribute ("entry").c_str ()));
      std::string entry = dt.toString (conf.get ("dateformat", "m/d/Y"));

      std::string age;
      std::string created = refTask.getAttribute ("entry");
      if (created.length ())
      {
        Date dt (::atoi (created.c_str ()));
        formatTimeDeltaDays (age, (time_t) (now - dt));
      }

      table.addCell (row, 1, entry + " (" + age + ")");
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Successively apply filters based on the task object built from the command
// line.  Tasks that match all the specified criteria are listed.
void handleLongList (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get all the tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  bool showAge = conf.get ("showage", true);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Entry");
  table.addColumn ("Start");
  table.addColumn ("Due");
  if (showAge) table.addColumn ("Age");
  table.addColumn ("Tags");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
    table.setColumnUnderline (6);
    table.setColumnUnderline (7);
    if (showAge) table.setColumnUnderline (8);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::minimum);
  table.setColumnWidth (5, Table::minimum);
  if (showAge) table.setColumnWidth (6, Table::minimum);
  table.setColumnWidth ((showAge ? 7 : 6), Table::minimum);
  table.setColumnWidth ((showAge ? 8 : 7), Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);
  table.setColumnJustification (4, Table::right);
  table.setColumnJustification (5, Table::right);
  if (showAge) table.setColumnJustification (6, Table::right);

  table.sortOn (5, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  // Iterate over each task, and apply selection criteria.
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);

    Date now;

    std::string started = refTask.getAttribute ("start");
    if (started.length ())
    {
      Date dt (::atoi (started.c_str ()));
      started = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string entered = refTask.getAttribute ("entry");
    if (entered.length ())
    {
      Date dt (::atoi (entered.c_str ()));
      entered = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = refTask.getAttribute ("due");
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
      due = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string age;
    std::string created = refTask.getAttribute ("entry");
    if (created.length ())
    {
      Date dt (::atoi (created.c_str ()));
      formatTimeDeltaDays (age, (time_t) (now - dt));
    }

    // Make a list of tags.
    std::string tags;
    std::vector <std::string> all;
    refTask.getTags (all);
    join (tags, " ", all);

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, refTask.getId ());
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getAttribute ("priority"));
    table.addCell (row, 3, entered);
    table.addCell (row, 4, started);
    table.addCell (row, 5, due);
    if (showAge) table.addCell (row, 6, age);
    table.addCell (row, (showAge ? 7 : 6), tags);
    table.addCell (row, (showAge ? 8 : 7), refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::red);
        else if (imminent)
          table.setCellFg (row, 3, Text::yellow);
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Project  Tasks  Avg Age  Status
// A           12      13d  XXXXXXXX------
// B          109   3d 12h  XX------------
void handleReportSummary (TDB& tdb, T& task, Config& conf)
{
  // Generate unique list of project names.
  tdb.gc ();
  std::map <std::string, bool> allProjects;
  std::vector <T> pending;
  tdb.allPendingT (pending);
  handleRecurrence (tdb, pending);
  filter (pending, task);
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    T task (pending[i]);
    allProjects[task.getAttribute ("project")] = false;
  }

  std::vector <T> completed;
  tdb.allCompletedT (completed);
  filter (completed, task);
  for (unsigned int i = 0; i < completed.size (); ++i)
  {
    T task (completed[i]);
    allProjects[task.getAttribute ("project")] = false;
  }

  // Initialize counts, sum.
  std::map <std::string, int> countPending;
  std::map <std::string, int> countCompleted;
  std::map <std::string, double> sumEntry;
  std::map <std::string, int> counter;
  time_t now = time (NULL);

  foreach (i, allProjects)
  {
    countPending   [i->first] = 0;
    countCompleted [i->first] = 0;
    sumEntry       [i->first] = 0.0;
    counter        [i->first] = 0;
  }

  // Count the pending tasks.
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    T task (pending[i]);
    std::string project = task.getAttribute ("project");
    if (task.getStatus () == T::pending)
      ++countPending[project];

    time_t entry = ::atoi (task.getAttribute ("entry").c_str ());
    if (entry)
    {
      sumEntry[project] = sumEntry[project] + (double) (now - entry);
      ++counter[project];
    }
  }

  // Count the completed tasks.
  for (unsigned int i = 0; i < completed.size (); ++i)
  {
    T task (completed[i]);
    std::string project = task.getAttribute ("project");
    countCompleted[project] = countCompleted[project] + 1;
    ++counter[project];

    time_t entry = ::atoi (task.getAttribute ("entry").c_str ());
    time_t end   = ::atoi (task.getAttribute ("end").c_str ());
    if (entry && end)
      sumEntry[project] = sumEntry[project] + (double) (end - entry);
  }

  // Create a table for output.
  Table table;
  table.addColumn ("Project");
  table.addColumn ("Remaining");
  table.addColumn ("Avg age");
  table.addColumn ("Complete");
  table.addColumn ("0%                        100%");

  if (conf.get ("color", true))
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
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

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
        formatTimeDeltaDays (age, (time_t) (sumEntry[i->first] / counter[i->first]));
        table.addCell (row, 2, age);
      }

      int c = countCompleted[i->first];
      int p = countPending[i->first];
      int completedBar = (c * barWidth) / (c + p);

      std::string bar;
      if (conf.get ("color", true))
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

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " project" : " projects")
              << std::endl;
  else
    std::cout << "No projects." << std::endl;
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
void handleReportNext (TDB& tdb, T& task, Config& conf)
{
  // Load all pending.
  tdb.gc ();
  std::vector <T> pending;
  tdb.allPendingT (pending);
  handleRecurrence (tdb, pending);
  filter (pending, task);

  // Restrict to matching subset.
  std::vector <int> matching;
  gatherNextTasks (tdb, task, conf, pending, matching);

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  tdb.gc ();

  // Get the pending tasks.
  std::vector <T> tasks;
  tdb.pendingT (tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  bool showAge = conf.get ("showage", true);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Active");
  if (showAge) table.addColumn ("Age");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
    if (showAge) table.setColumnUnderline (6);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::minimum);
  if (showAge) table.setColumnWidth (5, Table::minimum);
  table.setColumnWidth ((showAge ? 6 : 5), Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);
  if (showAge) table.setColumnJustification (5, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  // Iterate over each task, and apply selection criteria.
  foreach (i, matching)
  {
    T refTask (pending[*i]);
    Date now;

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = refTask.getAttribute ("due");
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
      due = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string active;
    if (refTask.getAttribute ("start") != "")
      active = "*";

    std::string age;
    std::string created = refTask.getAttribute ("entry");
    if (created.length ())
    {
      Date dt (::atoi (created.c_str ()));
      formatTimeDeltaDays (age, (time_t) (now - dt));
    }

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, refTask.getId ());
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getAttribute ("priority"));
    table.addCell (row, 3, due);
    table.addCell (row, 4, active);
    if (showAge) table.addCell (row, 5, age);
    table.addCell (row, (showAge ? 6 : 5), refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::red);
        else if (imminent)
          table.setCellFg (row, 3, Text::yellow);
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Year Month    Added Completed Deleted
// 2006 November    87        63      14
// 2006 December    21         6       1
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

void handleReportHistory (TDB& tdb, T& task, Config& conf)
{
  std::map <time_t, int> groups;
  std::map <time_t, int> addedGroup;
  std::map <time_t, int> completedGroup;
  std::map <time_t, int> deletedGroup;

  // Scan the pending tasks.
  tdb.gc ();
  std::vector <T> pending;
  tdb.allPendingT (pending);
  handleRecurrence (tdb, pending);
  filter (pending, task);
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    T task (pending[i]);
    time_t epoch = monthlyEpoch (task.getAttribute ("entry"));
    if (epoch)
    {
      groups[epoch] = 0;

      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      if (task.getStatus () == T::deleted)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));

        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }
  }

  // Scan the completed tasks.
  std::vector <T> completed;
  tdb.allCompletedT (completed);
  filter (completed, task);
  for (unsigned int i = 0; i < completed.size (); ++i)
  {
    T task (completed[i]);
    time_t epoch = monthlyEpoch (task.getAttribute ("entry"));
    if (epoch)
    {
      groups[epoch] = 0;

      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      epoch = monthlyEpoch (task.getAttribute ("end"));
      if (task.getStatus () == T::deleted)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));
        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }
  }

  // Now build the table.
  Table table;
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("Year");
  table.addColumn ("Month");
  table.addColumn ("Added");
  table.addColumn ("Completed");
  table.addColumn ("Deleted");
  table.addColumn ("Net");

  if (conf.get ("color", true))
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
    if (conf.get ("color", true) && net)
      table.setCellFg (row, 5, net > 0 ? Text::red: Text::green);
  }

  if (table.rowCount ())
  {
    table.addRow ();
    row = table.addRow ();

    table.addCell (row, 1, "Average");
    if (conf.get ("color", true)) table.setRowFg (row, Text::bold);
    table.addCell (row, 2, totalAdded / (table.rowCount () - 2));
    table.addCell (row, 3, totalCompleted / (table.rowCount () - 2));
    table.addCell (row, 4, totalDeleted / (table.rowCount () - 2));
    table.addCell (row, 5, (totalAdded - totalCompleted - totalDeleted) / (table.rowCount () - 2));
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << std::endl;
  else
    std::cout << "No tasks." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void handleReportGHistory (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif
  int widthOfBar = width - 15;   // strlen ("2008 September ")

  std::map <time_t, int> groups;
  std::map <time_t, int> addedGroup;
  std::map <time_t, int> completedGroup;
  std::map <time_t, int> deletedGroup;

  // Scan the pending tasks.
  tdb.gc ();
  std::vector <T> pending;
  tdb.allPendingT (pending);
  handleRecurrence (tdb, pending);
  filter (pending, task);
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    T task (pending[i]);
    time_t epoch = monthlyEpoch (task.getAttribute ("entry"));
    if (epoch)
    {
      groups[epoch] = 0;

      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      if (task.getStatus () == T::deleted)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));

        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }
  }

  // Scan the completed tasks.
  std::vector <T> completed;
  tdb.allCompletedT (completed);
  filter (completed, task);
  for (unsigned int i = 0; i < completed.size (); ++i)
  {
    T task (completed[i]);
    time_t epoch = monthlyEpoch (task.getAttribute ("entry"));
    if (epoch)
    {
      groups[epoch] = 0;

      if (addedGroup.find (epoch) != addedGroup.end ())
        addedGroup[epoch] = addedGroup[epoch] + 1;
      else
        addedGroup[epoch] = 1;

      epoch = monthlyEpoch (task.getAttribute ("end"));
      if (task.getStatus () == T::deleted)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));
        if (completedGroup.find (epoch) != completedGroup.end ())
          completedGroup[epoch] = completedGroup[epoch] + 1;
        else
          completedGroup[epoch] = 1;
      }
    }
  }

  // Now build the table.
  Table table;
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("Year");
  table.addColumn ("Month");
  table.addColumn ("Added/Completed/Deleted");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
  }
  else
    table.setTableDashedUnderline ();

  // Determine the longest line.
  int maxLine = 0;
  foreach (i, groups)
  {
    int line = addedGroup[i->first] + completedGroup[i->first] + deletedGroup[i->first];

    if (line > maxLine)
      maxLine = line;
  }

  if (maxLine > 0)
  {
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

      std::string bar;
      if (conf.get ("color", true))
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

        bar  = Text::colorize (Text::black, Text::on_red,  aBar);
        bar += Text::colorize (Text::black, Text::on_green, cBar);
        bar += Text::colorize (Text::black, Text::on_yellow,    dBar);
      }
      else
      {
        std::string aBar = ""; while (aBar.length () < addedBar)     aBar += "+";
        std::string cBar = ""; while (cBar.length () < completedBar) cBar += "X";
        std::string dBar = ""; while (dBar.length () < deletedBar)   dBar += "-";

        bar = aBar + cBar + dBar;
      }

      table.addCell (row, 2, bar);
    }
  }

  if (table.rowCount ())
  {
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << std::endl;

    if (conf.get ("color", true))
      std::cout << "Legend: "
                << Text::colorize (Text::black, Text::on_red, "added")
                << ", "
                << Text::colorize (Text::black, Text::on_green, "completed")
                << ", "
                << Text::colorize (Text::black, Text::on_yellow, "deleted")
                << optionalBlankLine (conf)
                << std::endl;
    else
      std::cout << "Legend: + added, X completed, - deleted" << std::endl;
  }
  else
    std::cout << "No tasks." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// A summary of the command usage.  Not useful to users, but used to display
// usage statistics for feedback.
//
// 2006-12-04 19:59:43 "task list"
//
void handleReportUsage (const TDB& tdb, T& task, Config& conf)
{
  if (conf.get ("command.logging") == "on")
  {
    std::map <std::string, int> usage;
    std::vector <std::string> all;
    tdb.logRead (all);
    for (unsigned int i = 0; i < all.size (); ++i)
    {
      // 0123456789012345678901
      //                      v 21
      // 2006-12-04 19:59:43 "task list"
      std::string command = all[i].substr (21, all[i].length () - 22);

      // Parse as a command line.
      std::vector <std::string> args;
      split (args, command, " ");

      try
      {
        T task;
        std::string commandName;
        parse (args, commandName, task, conf);

        usage[commandName]++;
      }

      // Deliberately ignore errors from parsing the command log, as there may
      // be commands from a prior version of task in there, which were
      // abbreviated, and are now ambiguous.
      catch (...) {}
    }

    // Now render the table.
    Table table;
    table.addColumn ("Command");
    table.addColumn ("Frequency");

    if (conf.get ("color", true))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }
  else
    table.setTableDashedUnderline ();

    table.setColumnJustification (1, Table::right);
    table.sortOn (1, Table::descendingNumeric);
    table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

    foreach (i, usage)
    {
      int row = table.addRow ();
      table.addCell (row, 0, (i->first == "" ? "(modify)" : i->first));
      table.addCell (row, 1, i->second);
    }

    if (table.rowCount ())
      std::cout << optionalBlankLine (conf)
                << table.render ()
                << std::endl;
    else
      std::cout << "No usage." << std::endl;
  }
  else
    std::cout << "Command logging is not enabled, so no history has been kept."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
std::string renderMonths (
  int firstMonth,
  int firstYear,
  const Date& today,
  std::vector <T>& all,
  Config& conf)
{
  Table table;
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  int monthsPerLine = (conf.get ("monthsperline", 1));

  // Build table for the number of months to be displayed.
  for (int i = 0 ; i < (monthsPerLine * 8); i += 8)
  {
    table.addColumn (" ");
    table.addColumn ("Su");
    table.addColumn ("Mo");
    table.addColumn ("Tu");
    table.addColumn ("We");
    table.addColumn ("Th");
    table.addColumn ("Fr");
    table.addColumn ("Sa");

    if (conf.get ("color", true))
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
  for (int c = 0; c < monthsPerLine ; c++)
  {
    // Reset row counter for subsequent months
    if (c != 0)
      row = 0;

    // Loop through days in month and add to table.
    for (int d = 1; d <= daysInMonth.at (c); ++d)
    {
      Date temp (months.at (c), d, years.at (c));
      int dow = temp.dayOfWeek ();
      int thisCol = dow + 1 + (8 * c);

      table.addCell (row, thisCol, d);

      if (conf.get ("color", true)         &&
          today.day ()   == d              &&
          today.month () == months.at (c)  &&
          today.year ()  == years.at (c))
        table.setCellFg (row, thisCol, Text::cyan);

      std::vector <T>::iterator it;
      for (it = all.begin (); it != all.end (); ++it)
      {
        Date due (::atoi (it->getAttribute ("due").c_str ()));

        if (conf.get ("color", true)      &&
            due.day ()   == d             &&
            due.month () == months.at (c) &&
            due.year ()  == years.at (c))
        {
          table.setCellFg (row, thisCol, Text::black);
          table.setCellBg (row, thisCol, due < today ? Text::on_red : Text::on_yellow);
        }
      }

      // Check for end of week, and...
      if (dow == 6 && d < daysInMonth.at (c))
        row++;
    }
  }

  return table.render ();
}

////////////////////////////////////////////////////////////////////////////////
void handleReportCalendar (TDB& tdb, T& task, Config& conf)
{
  // Load all the pending tasks.
  tdb.gc ();
  std::vector <T> pending;
  tdb.allPendingT (pending);
  handleRecurrence (tdb, pending);
  filter (pending, task);

  // Find the oldest pending due date.
  Date oldest;
  Date newest;
  std::vector <T>::iterator it;
  for (it = pending.begin (); it != pending.end (); ++it)
  {
    if (it->getAttribute ("due") != "")
    {
      Date d (::atoi (it->getAttribute ("due").c_str ()));

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

  std::cout << std::endl;
  std::string output;

  int monthsPerLine = (conf.get ("monthsperline", 1));

  while (yFrom < yTo || (yFrom == yTo && mFrom <= mTo))
  {
    int nextM = mFrom;
    int nextY = yFrom;

    // Print month headers (cheating on the width settings, yes)
    for (int i = 0 ; i < monthsPerLine ; i++)
    {
      std::string month = Date::monthName (nextM);
      int left = (18 - month.length ()) / 2 + 1;
      int right = 18 - left - month.length ();

      std::cout << std::setw (left) << ' '
                << month
                << ' '
                << nextY
                << std::setw (right) << ' ';

      if (++nextM > 12)
      {
        nextM = 1;
        nextY++;
      }
    }

    std::cout << std::endl
              << optionalBlankLine (conf)
              << renderMonths (mFrom, yFrom, today, pending, conf)
              << std::endl;

    mFrom += monthsPerLine;
    if (mFrom > 12)
    {
      mFrom -= 12;
      ++yFrom;
    }
  }

  std::cout << "Legend: "
            << Text::colorize (Text::cyan, Text::nocolor, "today")
            << ", "
            << Text::colorize (Text::black, Text::on_yellow, "due")
            << ", "
            << Text::colorize (Text::black, Text::on_red, "overdue")
            << "."
            << optionalBlankLine (conf)
            << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void handleReportActive (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get all the tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.pendingT (tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  // Iterate over each task, and apply selection criteria.
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);
    if (refTask.getAttribute ("start") != "")
    {
      Date now;
      bool imminent = false;
      bool overdue = false;
      std::string due = refTask.getAttribute ("due");
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
        due = dt.toString (conf.get ("dateformat", "m/d/Y"));
      }

      // All criteria match, so add refTask to the output table.
      int row = table.addRow ();
      table.addCell (row, 0, refTask.getId ());
      table.addCell (row, 1, refTask.getAttribute ("project"));
      table.addCell (row, 2, refTask.getAttribute ("priority"));
      table.addCell (row, 3, due);
      table.addCell (row, 4, refTask.getDescription ());

      if (conf.get ("color", true))
      {
        Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
        Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
        autoColorize (refTask, fg, bg);
        table.setRowFg (row, fg);
        table.setRowBg (row, bg);

        if (fg == Text::nocolor)
        {
          if (overdue)
            table.setCellFg (row, 3, Text::red);
          else if (imminent)
            table.setCellFg (row, 3, Text::yellow);
        }
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No active tasks." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void handleReportOverdue (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get all the tasks.
  std::vector <T> tasks;
  tdb.pendingT (tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  Date now;

  // Iterate over each task, and apply selection criteria.
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);
    std::string due;
    if ((due = refTask.getAttribute ("due")) != "")
    {
      if (due.length ())
      {
        Date dt (::atoi (due.c_str ()));
        due = dt.toString (conf.get ("dateformat", "m/d/Y"));

        // If overdue.
        if (dt < now)
        {
          // All criteria match, so add refTask to the output table.
          int row = table.addRow ();
          table.addCell (row, 0, refTask.getId ());
          table.addCell (row, 1, refTask.getAttribute ("project"));
          table.addCell (row, 2, refTask.getAttribute ("priority"));
          table.addCell (row, 3, due);
          table.addCell (row, 4, refTask.getDescription ());

          if (conf.get ("color", true))
          {
            Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
            Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
            autoColorize (refTask, fg, bg);
            table.setRowFg (row, fg);
            table.setRowBg (row, bg);

            if (fg == Text::nocolor)
              table.setCellFg (row, 3, Text::red);
          }
        }
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No overdue tasks." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Successively apply filters based on the task object built from the command
// line.  Tasks that match all the specified criteria are listed.
void handleReportOldest (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  bool showAge = conf.get ("showage", true);
  unsigned int quantity = conf.get ("oldest", 10);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Active");
  if (showAge) table.addColumn ("Age");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
    if (showAge) table.setColumnUnderline (6);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::minimum);
  if (showAge) table.setColumnWidth (5, Table::minimum);
  table.setColumnWidth ((showAge ? 6 : 5), Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);
  if (showAge) table.setColumnJustification (5, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

  for (unsigned int i = 0; i < min (quantity, tasks.size ()); ++i)
  {
    T refTask (tasks[i]);
    Date now;

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = refTask.getAttribute ("due");
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
      due = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string active;
    if (refTask.getAttribute ("start") != "")
      active = "*";

    std::string age;
    std::string created = refTask.getAttribute ("entry");
    if (created.length ())
    {
      Date dt (::atoi (created.c_str ()));
      formatTimeDeltaDays (age, (time_t) (now - dt));
    }

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, refTask.getId ());
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getAttribute ("priority"));
    table.addCell (row, 3, due);
    table.addCell (row, 4, active);
    if (showAge) table.addCell (row, 5, age);
    table.addCell (row, (showAge ? 6 : 5), refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::red);
        else if (imminent)
          table.setCellFg (row, 3, Text::yellow);
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// Successively apply filters based on the task object built from the command
// line.  Tasks that match all the specified criteria are listed.
void handleReportNewest (TDB& tdb, T& task, Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
  tdb.gc ();
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);
  filter (tasks, task);

  initializeColorRules (conf);

  bool showAge = conf.get ("showage", true);
  int quantity = conf.get ("newest", 10);

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.addColumn ("ID");
  table.addColumn ("Project");
  table.addColumn ("Pri");
  table.addColumn ("Due");
  table.addColumn ("Active");
  if (showAge) table.addColumn ("Age");
  table.addColumn ("Description");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
    table.setColumnUnderline (2);
    table.setColumnUnderline (3);
    table.setColumnUnderline (4);
    table.setColumnUnderline (5);
    if (showAge) table.setColumnUnderline (6);
  }
  else
    table.setTableDashedUnderline ();

  table.setColumnWidth (0, Table::minimum);
  table.setColumnWidth (1, Table::minimum);
  table.setColumnWidth (2, Table::minimum);
  table.setColumnWidth (3, Table::minimum);
  table.setColumnWidth (4, Table::minimum);
  if (showAge) table.setColumnWidth (5, Table::minimum);
  table.setColumnWidth ((showAge ? 6 : 5), Table::flexible);

  table.setColumnJustification (0, Table::right);
  table.setColumnJustification (3, Table::right);
  if (showAge) table.setColumnJustification (5, Table::right);

  table.sortOn (3, Table::ascendingDate);
  table.sortOn (2, Table::descendingPriority);
  table.sortOn (1, Table::ascendingCharacter);

  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

  int total = tasks.size ();
  for (int i = total - 1; i >= max (0, total - quantity); --i)
  {
    T refTask (tasks[i]);
    Date now;

    // Now format the matching task.
    bool imminent = false;
    bool overdue = false;
    std::string due = refTask.getAttribute ("due");
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
      due = dt.toString (conf.get ("dateformat", "m/d/Y"));
    }

    std::string active;
    if (refTask.getAttribute ("start") != "")
      active = "*";

    std::string age;
    std::string created = refTask.getAttribute ("entry");
    if (created.length ())
    {
      Date dt (::atoi (created.c_str ()));
      formatTimeDeltaDays (age, (time_t) (now - dt));
    }

    // All criteria match, so add refTask to the output table.
    int row = table.addRow ();
    table.addCell (row, 0, refTask.getId ());
    table.addCell (row, 1, refTask.getAttribute ("project"));
    table.addCell (row, 2, refTask.getAttribute ("priority"));
    table.addCell (row, 3, due);
    table.addCell (row, 4, active);
    if (showAge) table.addCell (row, 5, age);
    table.addCell (row, (showAge ? 6 : 5), refTask.getDescription ());

    if (conf.get ("color", true))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::red);
        else if (imminent)
          table.setCellFg (row, 3, Text::yellow);
      }
    }
  }

  if (table.rowCount ())
    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << table.rowCount ()
              << (table.rowCount () == 1 ? " task" : " tasks")
              << std::endl;
  else
    std::cout << "No matches."
              << std::endl;
}


////////////////////////////////////////////////////////////////////////////////
void handleReportStats (TDB& tdb, T& task, Config& conf)
{
  // Get all the tasks.
  std::vector <T> tasks;
  tdb.allT (tasks);
  filter (tasks, task);

  Date now;
  time_t earliest   = time (NULL);
  time_t latest     = 1;
  int totalT        = 0;
  int deletedT      = 0;
  int pendingT      = 0;
  int completedT    = 0;
  int taggedT       = 0;
  int recurringT    = 0;
  float daysPending = 0.0;
  int descLength    = 0;

  std::vector <T>::iterator it;
  for (it = tasks.begin (); it != tasks.end (); ++it)
  {
    ++totalT;
    if (it->getStatus () == T::deleted)   ++deletedT;
    if (it->getStatus () == T::pending)   ++pendingT;
    if (it->getStatus () == T::completed) ++completedT;
    if (it->getStatus () == T::recurring) ++recurringT;

    time_t entry = ::atoi (it->getAttribute ("entry").c_str ());
    if (entry < earliest) earliest = entry;
    if (entry > latest)   latest   = entry;

    if (it->getStatus () == T::completed)
    {
      time_t end = ::atoi (it->getAttribute ("end").c_str ());
      daysPending += (end - entry) / 86400.0;
    }

    if (it->getStatus () == T::pending)
      daysPending += (now - entry) / 86400.0;

    descLength += it->getDescription ().length ();

    std::vector <std::string> tags;
    it->getTags (tags);
    if (tags.size ()) ++taggedT;
  }

  std::cout << "Pending               " << pendingT   << std::endl
            << "Recurring             " << recurringT << std::endl
            << "Completed             " << completedT << std::endl
            << "Deleted               " << deletedT   << std::endl
            << "Total                 " << totalT     << std::endl;

  if (tasks.size ())
  {
    Date e (earliest);
    std::cout << "Oldest task           " << e.toString (conf.get ("dateformat", "m/d/Y")) << std::endl;
    Date l (latest);
    std::cout << "Newest task           " << l.toString (conf.get ("dateformat", "m/d/Y")) << std::endl;
    std::cout << "Task used for         " << formatSeconds (latest - earliest) << std::endl;
  }

  if (totalT)
    std::cout << "Task added every      " << formatSeconds ((latest - earliest) / totalT)     << std::endl;

  if (completedT)
    std::cout << "Task completed every  " << formatSeconds ((latest - earliest) / completedT) << std::endl;

  if (deletedT)
    std::cout << "Task deleted every    " << formatSeconds ((latest - earliest) / deletedT)   << std::endl;

  if (pendingT || completedT)
    std::cout << "Average time pending  "
              << formatSeconds ((int) ((daysPending / (pendingT + completedT)) * 86400))
              << std::endl;

  if (totalT)
  {
    std::cout << "Average desc length   " << (int) (descLength / totalT) << " characters" << std::endl;
    std::cout << "Tasks tagged          " << std::setprecision (3) << (100.0 * taggedT / totalT) << "%" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
void gatherNextTasks (
  const TDB& tdb,
  T& task,
  Config& conf,
  std::vector <T>& pending,
  std::vector <int>& all)
{
  // For counting tasks by project.
  std::map <std::string, int> countByProject;
  std::map <int, bool> matching;

  Date now;

  // How many items per project?  Default 3.
  int limit = conf.get ("next", 3);

  // due:< 1wk, pri:*
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string due = pending[i].getAttribute ("due");
      if (due != "")
      {
        Date d (::atoi (due.c_str ()));
        if (d < now + (7 * 24 * 60 * 60)) // if due:< 1wk
        {
          std::string project = pending[i].getAttribute ("project");
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
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string due = pending[i].getAttribute ("due");
      if (due != "")
      {
        std::string priority = pending[i].getAttribute ("priority");
        if (priority == "H")
        {
          std::string project = pending[i].getAttribute ("project");
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
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string priority = pending[i].getAttribute ("priority");
      if (priority == "H")
      {
        std::string project = pending[i].getAttribute ("project");
        if (countByProject[project] < limit && matching.find (i) == matching.end ())
        {
          ++countByProject[project];
          matching[i] = true;
        }
      }
    }
  }

  // due:*, pri:M
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string due = pending[i].getAttribute ("due");
      if (due != "")
      {
        std::string priority = pending[i].getAttribute ("priority");
        if (priority == "M")
        {
          std::string project = pending[i].getAttribute ("project");
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
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string priority = pending[i].getAttribute ("priority");
      if (priority == "M")
      {
        std::string project = pending[i].getAttribute ("project");
        if (countByProject[project] < limit && matching.find (i) == matching.end ())
        {
          ++countByProject[project];
          matching[i] = true;
        }
      }
    }
  }

  // due:*, pri:L
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string due = pending[i].getAttribute ("due");
      if (due != "")
      {
        std::string priority = pending[i].getAttribute ("priority");
        if (priority == "L")
        {
          std::string project = pending[i].getAttribute ("project");
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
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string priority = pending[i].getAttribute ("priority");
      if (priority == "L")
      {
        std::string project = pending[i].getAttribute ("project");
        if (countByProject[project] < limit && matching.find (i) == matching.end ())
        {
          ++countByProject[project];
          matching[i] = true;
        }
      }
    }
  }

  // due:, pri:
  for (unsigned int i = 0; i < pending.size (); ++i)
  {
    if (pending[i].getStatus () == T::pending)
    {
      std::string due = pending[i].getAttribute ("due");
      if (due == "")
      {
        std::string priority = pending[i].getAttribute ("priority");
        if (priority == "")
        {
          std::string project = pending[i].getAttribute ("project");
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

////////////////////////////////////////////////////////////////////////////////
