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
void filterSequence (std::vector<T>& all, T& task)
{
  std::vector <int> sequence = task.getAllIds ();

  std::vector <T> filtered;
  std::vector <T>::iterator t;
  for (t = all.begin (); t != all.end (); ++t)
  {
    std::vector <int>::iterator s;
    for (s = sequence.begin (); s != sequence.end (); ++s)
      if (t->getId () == *s)
        filtered.push_back (*t);
  }

  if (sequence.size () != filtered.size ())
  {
    std::vector <int> filteredSequence;
    std::vector <T>::iterator fs;
    for (fs = filtered.begin (); fs != filtered.end (); ++fs)
      filteredSequence.push_back (fs->getId ());

    std::vector <int> left;
    std::vector <int> right;
    listDiff (filteredSequence, sequence, left, right);
    if (left.size ())
      throw std::string ("Sequence filtering error - please report this error");

    if (right.size ())
    {
      std::stringstream out;
      out << "Task";

      if (right.size () > 1) out << "s";

      std::vector <int>::iterator s;
      for (s = right.begin (); s != right.end (); ++s)
        out << " " << *s;

      out << " not found";
      throw out.str ();
    }
  }

  all = filtered;
}

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
      {
        if (a->first == "project")
        {
          if (a->second.length () <= refTask.getAttribute (a->first).length ())
            if (a->second == refTask.getAttribute (a->first).substr (0, a->second.length ()))
              ++matches;
/*
  TODO Attempt at allowing "pri:!H", thwarted by a lack of coffee and the
       validation of "!H" as a priority value.  To be revisited soon.
          {
            if (a->second[0] == '!')  // Inverted search.
            {
              if (a->second.substr (1, std::string::npos) != refTask.getAttribute (a->first).substr (0, a->second.length ()))
                ++matches;
            }
            else
            {
              if (a->second == refTask.getAttribute (a->first).substr (0, a->second.length ()))
                ++matches;
            }
          }
*/
        }
        else if (a->second == refTask.getAttribute (a->first))
          ++matches;
/*
        else
        {
          if (a->second[0] == '!')  // Inverted search.
          {
            if (a->second.substr (1, std::string::npos) != refTask.getAttribute (a->first))
              ++matches;
          }
          else
          {
            if (a->second == refTask.getAttribute (a->first))
              ++matches;
          }
        }
*/
      }

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
std::string handleCompleted (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
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

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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

  // Note: There is deliberately no sorting.  The original sorting was on the
  //       end date.  Tasks are appended to completed.data naturally sorted by
  //       the end date, so that sequence is assumed to remain unchanged, and
  //       relied upon here.

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

    std::string description = refTask.getDescription ();
    std::string when;
    std::map <time_t, std::string> annotations;
    refTask.getAnnotations (annotations);
    foreach (anno, annotations)
    {
      Date dt (anno->first);
      when = dt.toString (conf.get ("dateformat", "m/d/Y"));
      description += "\n" + when + " " + anno->second;
    }
    table.addCell (row, 2, description);

    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg, conf);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);
    }
  }

  if (table.rowCount ())
    out << optionalBlankLine (conf)
        << table.render ()
        << optionalBlankLine (conf)
        << table.rowCount ()
        << (table.rowCount () == 1 ? " task" : " tasks")
        << std::endl;
  else
    out << "No matches."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Display all information for the given task.
std::string handleInfo (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
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

  // Find the task.
  int count = 0;
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T refTask (tasks[i]);

    if (refTask.getId () == task.getId () || task.sequenceContains (refTask.getId ()))
    {
      ++count;

      Table table;
      table.setTableWidth (width);
      table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

      table.addColumn ("Name");
      table.addColumn ("Value");

      if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
      table.addCell (row, 1, refTask.getId ());

      std::string status = refTask.getStatus () == T::pending   ? "Pending"
                         : refTask.getStatus () == T::completed ? "Completed"
                         : refTask.getStatus () == T::deleted   ? "Deleted"
                         : refTask.getStatus () == T::recurring ? "Recurring"
                         : "";
      if (refTask.getAttribute ("parent") != "")
        status += " (Recurring)";

      row = table.addRow ();
      table.addCell (row, 0, "Status");
      table.addCell (row, 1, status);

      std::string description = refTask.getDescription ();
      std::string when;
      std::map <time_t, std::string> annotations;
      refTask.getAnnotations (annotations);
      foreach (anno, annotations)
      {
        Date dt (anno->first);
        when = dt.toString (conf.get ("dateformat", "m/d/Y"));
        description += "\n" + when + " " + anno->second;
      }

      row = table.addRow ();
      table.addCell (row, 0, "Description");
      table.addCell (row, 1, description);

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

      if (refTask.getStatus () == T::recurring ||
          refTask.getAttribute ("parent") != "")
      {
        if (refTask.getAttribute ("recur") != "")
        {
          row = table.addRow ();
          table.addCell (row, 0, "Recurrence");
          table.addCell (row, 1, refTask.getAttribute ("recur"));
        }

        if (refTask.getAttribute ("until") != "")
        {
          row = table.addRow ();
          table.addCell (row, 0, "Recur until");
          table.addCell (row, 1, refTask.getAttribute ("until"));
        }

        if (refTask.getAttribute ("mask") != "")
        {
          row = table.addRow ();
          table.addCell (row, 0, "Mask");
          table.addCell (row, 1, refTask.getAttribute ("mask"));
        }

        if (refTask.getAttribute ("parent") != "")
        {
          row = table.addRow ();
          table.addCell (row, 0, "Parent task");
          table.addCell (row, 1, refTask.getAttribute ("parent"));
        }

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

          if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
          {
            if (overdue)
              table.setCellFg (row, 1, Text::colorCode (conf.get ("color.overdue", "red")));
            else if (imminent)
              table.setCellFg (row, 1, Text::colorCode (conf.get ("color.due", "yellow")));
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

      out << optionalBlankLine (conf)
          << table.render ()
          << std::endl;
    }
  }

  if (! count)
    out << "No matches." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Project  Remaining  Avg Age  Complete  0%                  100%
// A               12      13d       55%  XXXXXXXXXXXXX-----------
// B              109   3d 12h       10%  XXX---------------------
std::string handleReportSummary (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  std::vector <T> tasks;
  tdb.allT (tasks);
  handleRecurrence (tdb, tasks);
  filter (tasks, task);

  // Generate unique list of project names from all pending tasks.
  std::map <std::string, bool> allProjects;
  foreach (t, tasks)
    if (t->getStatus () == T::pending)
      allProjects[t->getAttribute ("project")] = false;

  // Initialize counts, sum.
  std::map <std::string, int> countPending;
  std::map <std::string, int> countCompleted;
  std::map <std::string, double> sumEntry;
  std::map <std::string, int> counter;
  time_t now = time (NULL);

  // Initialize counters.
  foreach (i, allProjects)
  {
    countPending   [i->first] = 0;
    countCompleted [i->first] = 0;
    sumEntry       [i->first] = 0.0;
    counter        [i->first] = 0;
  }

  // Count the various tasks.
  foreach (t, tasks)
  {
    std::string project = t->getAttribute ("project");
    ++counter[project];

    if (t->getStatus () == T::pending)
    {
      ++countPending[project];

      time_t entry = ::atoi (t->getAttribute ("entry").c_str ());
      if (entry)
        sumEntry[project] = sumEntry[project] + (double) (now - entry);
    }

    else if (t->getStatus () == T::completed)
    {
      ++countCompleted[project];

      time_t entry = ::atoi (t->getAttribute ("entry").c_str ());
      time_t end   = ::atoi (task.getAttribute ("end").c_str ());
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

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
      if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
    out << optionalBlankLine (conf)
        << table.render ()
        << optionalBlankLine (conf)
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
std::string handleReportNext (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Load all pending.
  std::vector <T> pending;
  tdb.allPendingT (pending);
  handleRecurrence (tdb, pending);
  filter (pending, task);

  // Restrict to matching subset.
  std::vector <int> matching;
  gatherNextTasks (tdb, task, conf, pending, matching);

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Get the pending tasks.
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
  table.addColumn ("Active");
  table.addColumn ("Age");
  table.addColumn ("Description");

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
    table.addCell (row, 5, age);

    std::string description = refTask.getDescription ();
    std::string when;
    std::map <time_t, std::string> annotations;
    refTask.getAnnotations (annotations);
    foreach (anno, annotations)
    {
      Date dt (anno->first);
      when = dt.toString (conf.get ("dateformat", "m/d/Y"));
      description += "\n" + when + " " + anno->second;
    }

    table.addCell (row, 6, description);

    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
    {
      Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
      Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
      autoColorize (refTask, fg, bg, conf);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (overdue)
          table.setCellFg (row, 3, Text::colorCode (conf.get ("color.overdue", "red")));
        else if (imminent)
          table.setCellFg (row, 3, Text::colorCode (conf.get ("color.due", "yellow")));
      }
    }
  }

  if (table.rowCount ())
    out << optionalBlankLine (conf)
        << table.render ()
        << optionalBlankLine (conf)
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

std::string handleReportHistory (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  std::map <time_t, int> groups;
  std::map <time_t, int> addedGroup;
  std::map <time_t, int> completedGroup;
  std::map <time_t, int> deletedGroup;

  // Scan the pending tasks.
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
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));
        groups[epoch] = 0;

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
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));
        groups[epoch] = 0;

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

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
    if ((conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false)) && net)
      table.setCellFg (row, 5, net > 0 ? Text::red: Text::green);
  }

  if (table.rowCount ())
  {
    table.addRow ();
    row = table.addRow ();

    table.addCell (row, 1, "Average");
    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false)) table.setRowFg (row, Text::bold);
    table.addCell (row, 2, totalAdded / (table.rowCount () - 2));
    table.addCell (row, 3, totalCompleted / (table.rowCount () - 2));
    table.addCell (row, 4, totalDeleted / (table.rowCount () - 2));
    table.addCell (row, 5, (totalAdded - totalCompleted - totalDeleted) / (table.rowCount () - 2));
  }

  if (table.rowCount ())
    out << optionalBlankLine (conf)
        << table.render ()
        << std::endl;
  else
    out << "No tasks." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportGHistory (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif
  int widthOfBar = width - 15;   // 15 == strlen ("2008 September ")

  std::map <time_t, int> groups;
  std::map <time_t, int> addedGroup;
  std::map <time_t, int> completedGroup;
  std::map <time_t, int> deletedGroup;

  // Scan the pending tasks.
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
        groups[epoch] = 0;

        if (deletedGroup.find (epoch) != deletedGroup.end ())
          deletedGroup[epoch] = deletedGroup[epoch] + 1;
        else
          deletedGroup[epoch] = 1;
      }
      else if (task.getStatus () == T::completed)
      {
        epoch = monthlyEpoch (task.getAttribute ("end"));
        groups[epoch] = 0;

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
  table.addColumn ("Number Added/Completed/Deleted");

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
      if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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

  if (table.rowCount ())
  {
    out << optionalBlankLine (conf)
        << table.render ()
        << std::endl;

    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
      out << "Legend: "
          << Text::colorize (Text::black, Text::on_red, "added")
          << ", "
          << Text::colorize (Text::black, Text::on_green, "completed")
          << ", "
          << Text::colorize (Text::black, Text::on_yellow, "deleted")
          << optionalBlankLine (conf)
          << std::endl;
    else
      out << "Legend: + added, X completed, - deleted" << std::endl;
  }
  else
    out << "No tasks." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string renderMonths (
  int firstMonth,
  int firstYear,
  const Date& today,
  std::vector <T>& all,
  Config& conf,
  int monthsPerLine)
{
  Table table;
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

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

    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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

      if ((conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false)) &&
          today.day ()   == d              &&
          today.month () == months.at (c)  &&
          today.year ()  == years.at (c))
        table.setCellFg (row, thisCol, Text::cyan);

      std::vector <T>::iterator it;
      for (it = all.begin (); it != all.end (); ++it)
      {
        Date due (::atoi (it->getAttribute ("due").c_str ()));

        if ((conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false)) &&
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
std::string handleReportCalendar (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Each month requires 23 text columns width.  See how many will actually
  // fit.  But if a preference is specified, and it fits, use it.
  int preferredMonthsPerLine = (conf.get (std::string ("monthsperline"), 0));
  int monthsThatFit = width / 23;

  int monthsPerLine = monthsThatFit;
  if (preferredMonthsPerLine != 0 && preferredMonthsPerLine < monthsThatFit)
    monthsPerLine = preferredMonthsPerLine;

  // Load all the pending tasks.
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
      int left = (18 - month.length ()) / 2 + 1;
      int right = 18 - left - month.length ();

      out << std::setw (left) << ' '
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

    out << std::endl
        << optionalBlankLine (conf)
        << renderMonths (mFrom, yFrom, today, pending, conf, monthsPerLine)
        << std::endl;

    mFrom += monthsPerLine;
    if (mFrom > 12)
    {
      mFrom -= 12;
      ++yFrom;
    }
  }

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
    out << "Legend: "
        << Text::colorize (Text::cyan, Text::nocolor, "today")
        << ", "
        << Text::colorize (Text::black, Text::on_yellow, "due")
        << ", "
        << Text::colorize (Text::black, Text::on_red, "overdue")
        << "."
        << optionalBlankLine (conf)
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportActive (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
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

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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

      std::string description = refTask.getDescription ();
      std::string when;
      std::map <time_t, std::string> annotations;
      refTask.getAnnotations (annotations);
      foreach (anno, annotations)
      {
        Date dt (anno->first);
        when = dt.toString (conf.get ("dateformat", "m/d/Y"));
        description += "\n" + when + " " + anno->second;
      }

      table.addCell (row, 4, description);

      if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
      {
        Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
        Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
        autoColorize (refTask, fg, bg, conf);
        table.setRowFg (row, fg);
        table.setRowBg (row, bg);

        if (fg == Text::nocolor)
        {
          if (overdue)
            table.setCellFg (row, 3, Text::colorCode (conf.get ("color.overdue", "red")));
          else if (imminent)
            table.setCellFg (row, 3, Text::colorCode (conf.get ("color.due", "yellow")));
        }
      }
    }
  }

  if (table.rowCount ())
    out << optionalBlankLine (conf)
        << table.render ()
        << optionalBlankLine (conf)
        << table.rowCount ()
        << (table.rowCount () == 1 ? " task" : " tasks")
        << std::endl;
  else
    out << "No active tasks." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportOverdue (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
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

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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

          std::string description = refTask.getDescription ();
          std::string when;
          std::map <time_t, std::string> annotations;
          refTask.getAnnotations (annotations);
          foreach (anno, annotations)
          {
            Date dt (anno->first);
            when = dt.toString (conf.get ("dateformat", "m/d/Y"));
            description += "\n" + when + " " + anno->second;
          }

          table.addCell (row, 4, description);

          if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
          {
            Text::color fg = Text::colorCode (refTask.getAttribute ("fg"));
            Text::color bg = Text::colorCode (refTask.getAttribute ("bg"));
            autoColorize (refTask, fg, bg, conf);
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
    out << optionalBlankLine (conf)
        << table.render ()
        << optionalBlankLine (conf)
        << table.rowCount ()
        << (table.rowCount () == 1 ? " task" : " tasks")
        << std::endl;
  else
    out << "No overdue tasks." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleReportStats (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

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
  int annotationsT  = 0;
  int recurringT    = 0;
  float daysPending = 0.0;
  int descLength    = 0;
  std::map <std::string, int> allTags;
  std::map <std::string, int> allProjects;

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

    annotationsT += it->getAnnotationCount ();

    std::vector <std::string> tags;
    it->getTags (tags);
    if (tags.size ()) ++taggedT;

    foreach (t, tags)
      allTags[*t] = 0;

    std::string project = it->getAttribute ("project");
    if (project != "")
      allProjects[project] = 0;
  }

  out << "Pending               " << pendingT   << std::endl
      << "Recurring             " << recurringT << std::endl
      << "Completed             " << completedT << std::endl
      << "Deleted               " << deletedT   << std::endl
      << "Total                 " << totalT     << std::endl;

  out << "Annotations           " << annotationsT << std::endl;
  out << "Unique tags           " << allTags.size () << std::endl;
  out << "Projects              " << allProjects.size () << std::endl;

  if (totalT)
    out << "Tasks tagged          " << std::setprecision (3) << (100.0 * taggedT / totalT) << "%" << std::endl;

  if (tasks.size ())
  {
    Date e (earliest);
    out << "Oldest task           " << e.toString (conf.get ("dateformat", "m/d/Y")) << std::endl;
    Date l (latest);
    out << "Newest task           " << l.toString (conf.get ("dateformat", "m/d/Y")) << std::endl;
    out << "Task used for         " << formatSeconds (latest - earliest) << std::endl;
  }

  if (totalT)
    out << "Task added every      " << formatSeconds ((latest - earliest) / totalT)     << std::endl;

  if (completedT)
    out << "Task completed every  " << formatSeconds ((latest - earliest) / completedT) << std::endl;

  if (deletedT)
    out << "Task deleted every    " << formatSeconds ((latest - earliest) / deletedT)   << std::endl;

  if (pendingT || completedT)
    out << "Average time pending  "
              << formatSeconds ((int) ((daysPending / (pendingT + completedT)) * 86400))
              << std::endl;

  if (totalT)
    out << "Average desc length   " << (int) (descLength / totalT) << " characters" << std::endl;

  return out.str ();
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
// This report will eventually become the one report that many others morph into
// via the .taskrc file.
std::string handleCustomReport (
  TDB& tdb,
  T& task,
  Config& conf,
  const std::string& report)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Load report configuration.
  std::string columnList = conf.get ("report." + report + ".columns");
  std::vector <std::string> columns;
  split (columns, columnList, ',');
  validReportColumns (columns);

  std::string labelList = conf.get ("report." + report + ".labels");
  std::vector <std::string> labels;
  split (labels, labelList, ',');

  if (columns.size () != labels.size () && labels.size () != 0)
    throw std::string ("There are a different number of columns than labels ") +
          "for report '" + report + "'.  Please correct this.";

  std::map <std::string, std::string> columnLabels;
  if (labels.size ())
    for (unsigned int i = 0; i < columns.size (); ++i)
      columnLabels[columns[i]] = labels[i];

  std::string sortList   = conf.get ("report." + report + ".sort");
  std::vector <std::string> sortOrder;
  split (sortOrder, sortList, ',');
  validSortColumns (columns, sortOrder);

  std::string filterList = conf.get ("report." + report + ".filter");
  std::vector <std::string> filterArgs;
  split (filterArgs, filterList, ' ');

  // Load all pending tasks.
  std::vector <T> tasks;
  tdb.allPendingT (tasks);
  handleRecurrence (tdb, tasks);

  // Apply filters.
  {
    std::string ignore;
    T filterTask;
    parse (filterArgs, ignore, filterTask, conf);

    filter (tasks, filterTask);  // Filter from custom report
    filter (tasks, task);        // Filter from command line
  }

  // Initialize colorization for subsequent auto colorization.
  initializeColorRules (conf);

  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

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
        table.addCell (row, columnCount, tasks[row].getId ());
    }

    else if (*col == "uuid")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "UUID");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].getUUID ());
    }

    else if (*col == "project")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Project");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].getAttribute ("project"));
    }

    else if (*col == "priority")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Pri");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::left);

      for (unsigned int row = 0; row < tasks.size(); ++row)
        table.addCell (row, columnCount, tasks[row].getAttribute ("priority"));
    }

    else if (*col == "entry")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "Added");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      std::string entered;
      for (unsigned int row = 0; row < tasks.size(); ++row)
      {
        entered = tasks[row].getAttribute ("entry");
        if (entered.length ())
        {
          Date dt (::atoi (entered.c_str ()));
          entered = dt.toString (conf.get ("dateformat", "m/d/Y"));
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
        started = tasks[row].getAttribute ("start");
        if (started.length ())
        {
          Date dt (::atoi (started.c_str ()));
          started = dt.toString (conf.get ("dateformat", "m/d/Y"));
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
        due = tasks[row].getAttribute ("due");
        if (due.length ())
        {
          Date dt (::atoi (due.c_str ()));
          due = dt.toString (conf.get ("dateformat", "m/d/Y"));
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
        created = tasks[row].getAttribute ("entry");
        if (created.length ())
        {
          Date dt (::atoi (created.c_str ()));
          formatTimeDeltaDays (age, (time_t) (now - dt));
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
        if (tasks[row].getAttribute ("start") != "")
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
        table.addCell (row, columnCount, tasks[row].getDescription ());
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
        description = tasks[row].getDescription ();
        std::map <time_t, std::string> annotations;
        tasks[row].getAnnotations (annotations);
        foreach (anno, annotations)
        {
          Date dt (anno->first);
          when = dt.toString (conf.get ("dateformat", "m/d/Y"));
          description += "\n" + when + " " + anno->second;
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
        table.addCell (row, columnCount, tasks[row].getAttribute ("recur"));
    }

    else if (*col == "recurrence_indicator")
    {
      table.addColumn (columnLabels[*col] != "" ? columnLabels[*col] : "R");
      table.setColumnWidth (columnCount, Table::minimum);
      table.setColumnJustification (columnCount, Table::right);

      for (unsigned int row = 0; row < tasks.size (); ++row)
        table.addCell (row, columnCount,
                       tasks[row].getAttribute ("recur") != "" ? "R" : "");
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
    if (conf.get (std::string ("color"), true) || conf.get (std::string ("_forcecolor"), false))
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
    overdue = false;
    due = tasks[row].getAttribute ("due");
    if (due.length ())
    {
      switch (getDueState (due))
      {
      case 2: overdue = true;  break;
      case 1: imminent = true; break;
      case 0:
      default:                 break;
      }
    }

    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
    {
      Text::color fg = Text::colorCode (tasks[row].getAttribute ("fg"));
      Text::color bg = Text::colorCode (tasks[row].getAttribute ("bg"));
      autoColorize (tasks[row], fg, bg, conf);
      table.setRowFg (row, fg);
      table.setRowBg (row, bg);

      if (fg == Text::nocolor)
      {
        if (dueColumn != -1)
        {
          if (overdue)
            table.setCellFg (row, columnCount, Text::colorCode (conf.get ("color.overdue", "red")));
          else if (imminent)
            table.setCellFg (row, columnCount, Text::colorCode (conf.get ("color.due", "yellow")));
        }
      }
    }
  }

  int maximum = conf.get (std::string ("report.") + report + ".limit", (int)0);

  std::stringstream out;
  if (table.rowCount ())
    out << optionalBlankLine (conf)
        << table.render (maximum)
        << optionalBlankLine (conf)
        << table.rowCount ()
        << (table.rowCount () == 1 ? " task" : " tasks")
        << std::endl;
  else
    out << "No matches."
        << std::endl;

  return out.str ();
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
        *it != "entry"                &&
        *it != "start"                &&
        *it != "due"                  &&
        *it != "age"                  &&
        *it != "active"               &&
        *it != "tags"                 &&
        *it != "recur"                &&
        *it != "recurrence_indicator" &&
        *it != "tag_indicator"        &&
        *it != "description_only"     &&
        *it != "description")
      bad.push_back (*it);

  if (bad.size ())
  {
    std::string error;
    join (error, ", ", bad);
    throw std::string ("Unrecognized column name: ") + error;
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
    throw std::string ("Sort column is not part of the report: ") + error;
  }
}

////////////////////////////////////////////////////////////////////////////////
