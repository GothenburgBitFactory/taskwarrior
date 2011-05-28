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
#include <text.h>
#include <CmdProjects.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdProjects::CmdProjects ()
{
  _keyword     = "projects";
  _usage       = "task projects [<filter>]";
  _description = "Shows a list of all project names used, and how many tasks are in each";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdProjects::execute (const std::string& command_line, std::string& output)
{
  int rc = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  int quantity;
  if (context.config.getBoolean ("list.all.projects"))
    quantity = context.tdb.load (tasks, context.filter);
  else
    quantity = context.tdb.loadPending (tasks, context.filter);

  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  std::map <std::string, int> high;
  std::map <std::string, int> medium;
  std::map <std::string, int> low;
  std::map <std::string, int> none;
  bool no_project = false;
  std::string project;
  std::string priority;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    project = task->get ("project");
    priority = task->get ("priority");

    unique[project] += 1;
    if (project == "")
      no_project = true;

         if (priority == "H") high[project]   += 1;
    else if (priority == "M") medium[project] += 1;
    else if (priority == "L") low[project]    += 1;
    else                      none[project]   += 1;
  }

  if (unique.size ())
  {
    // Render a list of project names from the map.
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string",       "Project"));
    view.add (Column::factory ("string.right", "Tasks"));
    view.add (Column::factory ("string.right", "Pri:None"));
    view.add (Column::factory ("string.right", "Pri:L"));
    view.add (Column::factory ("string.right", "Pri:M"));
    view.add (Column::factory ("string.right", "Pri:H"));

    std::map <std::string, int>::iterator project;
    for (project = unique.begin (); project != unique.end (); ++project)
    {
      int row = view.addRow ();
      view.set (row, 0, (project->first == "" ? "(none)" : project->first));
      view.set (row, 1, project->second);
      view.set (row, 2, none[project->first]);
      view.set (row, 3, low[project->first]);
      view.set (row, 4, medium[project->first]);
      view.set (row, 5, high[project->first]);
    }

    int number_projects = unique.size ();
    if (no_project)
      --number_projects;

    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ()
        << number_projects
        << (number_projects == 1 ? " project" : " projects")
        << " (" << quantity << (quantity == 1 ? " task" : " tasks") << ")\n";
  }
  else
  {
    out << "No projects.\n";
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionProjects::CmdCompletionProjects ()
{
  _keyword     = "_projects";
  _usage       = "task _projects [<filter>]";
  _description = "Shows only a list of all project names used";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionProjects::execute (const std::string& command_line, std::string& output)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));

  Filter filter;
  if (context.config.getBoolean ("complete.all.projects"))
    context.tdb.load (tasks, filter);
  else
    context.tdb.loadPending (tasks, filter);

  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    unique[task->get ("project")] = 0;

  std::map <std::string, int>::iterator project;
  for (project = unique.begin (); project != unique.end (); ++project)
    if (project->first.length ())
      output += project->first + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
