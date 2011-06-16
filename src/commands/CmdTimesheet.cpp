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

#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <Expression.h>
#include <ViewText.h>
#include <Date.h>
#include <main.h>
#include <CmdTimesheet.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdTimesheet::CmdTimesheet ()
{
  _keyword     = "timesheet";
  _usage       = "task timesheet [weeks]";
  _description = "Shows a weekly report of tasks completed and started.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdTimesheet::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Filter.
  Arguments f = context.args.extract_read_only_filter ();
  Expression e (f);

  std::vector <Task> filtered;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    if (e.eval (*task))
      filtered.push_back (*task);

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
/*
  TODO Need some command line parsing.
  if (context.sequence.size () == 1)
    quantity = context.sequence[0];
*/

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

    for (task = filtered.begin (); task != filtered.end (); ++task)
    {
      // If task completed within range.
      if (task->getStatus () == Task::completed)
      {
        Date compDate (strtol (task->get ("end").c_str (), NULL, 10));
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

    for (task = filtered.begin (); task != filtered.end (); ++task)
    {
      // If task started within range, but not completed withing range.
      if (task->getStatus () == Task::pending &&
          task->has ("start"))
      {
        Date startDate (strtol (task->get ("start").c_str (), NULL, 10));
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

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
