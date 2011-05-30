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
#include <algorithm>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include <Context.h>
#include <Directory.h>
#include <File.h>
#include <Date.h>
#include <Duration.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <main.h>

static void countTasks (const std::vector <Task>&, const std::string&, const std::vector <Task>&, int&, int&);

extern Context context;

////////////////////////////////////////////////////////////////////////////////
int handleReportTimesheet (std::string& outs)
{
  int rc = 0;

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
        << (context.color () ? bold.colorize (title) : title)
        << "\n";

    // Render the completed table.
    ViewText completed;
    completed.width (width);
    completed.add (Column::factory ("string", "   "));
    completed.add (Column::factory ("string", "Project"));
    completed.add (Column::factory ("string.right", "Due"));
    completed.add (Column::factory ("string", "Description"));

    std::vector <Task>::iterator task;
    for (task = tasks.begin (); task != tasks.end (); ++task)
    {
      // If task completed within range.
      if (task->getStatus () == Task::completed)
      {
        Date compDate (atoi (task->get ("end").c_str ()));
        if (compDate >= start && compDate < end)
        {
          Color c (task->get ("fg") + " " + task->get ("bg"));
          if (context.color ())
            autoColorize (*task, c);

          int row = completed.addRow ();
          std::string format = context.config.get ("dateformat.report");
          if (format == "")
            format = context.config.get ("dateformat");
          completed.set (row, 1, task->get ("project"), c);
          completed.set (row, 2, getDueDate (*task, format), c);
          completed.set (row, 3, getFullDescription (*task, "timesheet"), c);
        }
      }
    }

    out << "  Completed (" << completed.rows () << " tasks)\n";

    if (completed.rows ())
      out << completed.render ()
          << "\n";

    // Now render the started table.
    ViewText started;
    started.width (width);
    started.add (Column::factory ("string", "   "));
    started.add (Column::factory ("string", "Project"));
    started.add (Column::factory ("string.right", "Due"));
    started.add (Column::factory ("string", "Description"));

    for (task = tasks.begin (); task != tasks.end (); ++task)
    {
      // If task started within range, but not completed withing range.
      if (task->getStatus () == Task::pending &&
          task->has ("start"))
      {
        Date startDate (atoi (task->get ("start").c_str ()));
        if (startDate >= start && startDate < end)
        {
          Color c (task->get ("fg") + " " + task->get ("bg"));
          if (context.color ())
            autoColorize (*task, c);

          int row = started.addRow ();
          std::string format = context.config.get ("dateformat.report");
          if (format == "")
            format = context.config.get ("dateformat");
          started.set (row, 1, task->get ("project"), c);
          started.set (row, 2, getDueDate (*task, format), c);
          started.set (row, 3, getFullDescription (*task, "timesheet"), c);

        }
      }
    }

    out << "  Started (" << started.rows () << " tasks)\n";

    if (started.rows ())
      out << started.render ()
          << "\n\n";

    // Prior week.
    start -= 7 * 86400;
    end   -= 7 * 86400;
  }

  outs = out.str ();
  return rc;
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

    countTasks (all,                           project, context.tdb.getAllModified (), count_pending, count_done);
    countTasks (context.tdb.getAllModified (), project, (std::vector <Task>) NULL,     count_pending, count_done);

    // count_done  count_pending  percentage
    // ----------  -------------  ----------
    //          0              0          0%
    //         >0              0        100%
    //          0             >0          0%
    //         >0             >0  calculated
    int percentage = 0;
    if (count_done == 0)
      percentage = 0;
    else if (count_pending == 0)
      percentage = 100;
    else
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
  const std::vector <Task>& skipTasks,
  int& count_pending,
  int& count_done)
{
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    bool skip = 0;

    if (it->get ("project") == project)
    {
      std::vector <Task>::const_iterator itSkipTasks;
      for (itSkipTasks = skipTasks.begin (); itSkipTasks != skipTasks.end (); ++itSkipTasks)
      {
        if (it->get("uuid") == itSkipTasks->get("uuid"))
        {
          skip = 1;
          break;
        }
      }
      if (skip == 0)
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
}

///////////////////////////////////////////////////////////////////////////////
