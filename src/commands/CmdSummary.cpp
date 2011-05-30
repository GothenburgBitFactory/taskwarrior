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
#include <Context.h>
#include <ViewText.h>
#include <Duration.h>
#include <text.h>
#include <main.h>
#include <CmdSummary.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdSummary::CmdSummary ()
{
  _keyword     = "summary";
  _usage       = "task summary";
  _description = "Shows a report of task status by project.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
// Project  Remaining  Avg Age  Complete  0%                  100%
// A               12      13d       55%  XXXXXXXXXXXXX-----------
// B              109   3d 12h       10%  XXX---------------------
int CmdSummary::execute (const std::string&, std::string& output)
{
  int rc = 0;

  // Scan the pending tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Generate unique list of project names from all pending tasks.
  std::map <std::string, bool> allProjects;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    if (task->getStatus () == Task::pending)
      allProjects[task->get ("project")] = false;

  // Initialize counts, sum.
  std::map <std::string, int> countPending;
  std::map <std::string, int> countCompleted;
  std::map <std::string, double> sumEntry;
  std::map <std::string, int> counter;
  time_t now = time (NULL);

  // Initialize counters.
  std::map <std::string, bool>::iterator project;
  for (project = allProjects.begin (); project != allProjects.end (); ++project)
  {
    countPending   [project->first] = 0;
    countCompleted [project->first] = 0;
    sumEntry       [project->first] = 0.0;
    counter        [project->first] = 0;
  }

  // Count the various tasks.
  for (task = tasks.begin (); task != tasks.end (); ++task)
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
  ViewText view;
  view.width (context.getWidth ());
  view.add (Column::factory ("string", "Project"));
  view.add (Column::factory ("string.right", "Remaining"));
  view.add (Column::factory ("string.right", "Avg age"));
  view.add (Column::factory ("string.right", "Complete"));
  view.add (Column::factory ("string", "0%                        100%"));

  Color bar_color (context.config.get ("color.summary.bar"));
  Color bg_color  (context.config.get ("color.summary.background"));

  int barWidth = 30;
  std::map <std::string, bool>::iterator i;
  for (i = allProjects.begin (); i != allProjects.end (); ++i)
  {
    if (countPending[i->first] > 0)
    {
      int row = view.addRow ();
      view.set (row, 0, (i->first == "" ? "(none)" : i->first));
      view.set (row, 1, countPending[i->first]);
      if (counter[i->first])
        view.set (row, 2, Duration ((int) (sumEntry[i->first] / (double)counter[i->first])).format ());

      int c = countCompleted[i->first];
      int p = countPending[i->first];
      int completedBar = (c * barWidth) / (c + p);

      std::string bar;
      std::string subbar;
      if (context.color ())
      {
        bar += bar_color.colorize (std::string (           completedBar, ' '));
        bar += bg_color.colorize  (std::string (barWidth - completedBar, ' '));
      }
      else
      {
        bar += std::string (           completedBar, '=')
            +  std::string (barWidth - completedBar, ' ');
      }
      view.set (row, 4, bar);

      char percent[12];
      sprintf (percent, "%d%%", 100 * c / (c + p));
      view.set (row, 3, percent);
    }
  }

  std::stringstream out;
  if (view.rows ())
    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ()
        << view.rows ()
        << (view.rows () == 1 ? " project" : " projects")
        << "\n";
  else {
    out << "No projects.\n";
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
