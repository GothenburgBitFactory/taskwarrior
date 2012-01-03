////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////


#define L10N                                           // Localization complete.

#include <sstream>
#include <Context.h>
#include <ViewText.h>
#include <text.h>
#include <i18n.h>
#include <main.h>
#include <CmdProjects.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdProjects::CmdProjects ()
{
  _keyword     = "projects";
  _usage       = "task <filter> projects";
  _description = STRING_CMD_PROJECTS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdProjects::execute (std::string& output)
{
  int rc = 0;

  // Get all the tasks.
  handleRecurrence ();
  std::vector <Task> tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("list.all.projects"))
  {
    std::vector <Task> extra = context.tdb2.completed.get_tasks ();
    std::vector <Task>::iterator task;
    for (task = extra.begin (); task != extra.end (); ++task)
      tasks.push_back (*task);
  }

  int quantity = tasks.size ();

  context.tdb2.commit ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  std::stringstream out;

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
  for (task = filtered.begin (); task != filtered.end (); ++task)
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
    view.add (Column::factory ("string",       STRING_COLUMN_LABEL_PROJECT));
    view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_TASKS));
    view.add (Column::factory ("string.right", STRING_CMD_PROJECTS_PRI_N));
    view.add (Column::factory ("string.right", STRING_CMD_PROJECTS_PRI_H));
    view.add (Column::factory ("string.right", STRING_CMD_PROJECTS_PRI_M));
    view.add (Column::factory ("string.right", STRING_CMD_PROJECTS_PRI_L));

    std::map <std::string, int>::iterator project;
    for (project = unique.begin (); project != unique.end (); ++project)
    {
      int row = view.addRow ();
      view.set (row, 0, (project->first == "" ? STRING_CMD_PROJECTS_NONE : project->first));
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
        << (number_projects == 1
              ? format (STRING_CMD_PROJECTS_SUMMARY,  number_projects)
              : format (STRING_CMD_PROJECTS_SUMMARY2, number_projects))
        << " "
        << (quantity == 1
              ? format (STRING_CMD_PROJECTS_TASK,  quantity)
              : format (STRING_CMD_PROJECTS_TASKS, quantity))
        << "\n";
  }
  else
  {
    out << STRING_CMD_PROJECTS_NO << "\n";
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionProjects::CmdCompletionProjects ()
{
  _keyword     = "_projects";
  _usage       = "task <filter> _projects";
  _description = STRING_CMD_PROJECTS_USAGE_2;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionProjects::execute (std::string& output)
{
  // Get all the tasks.
  handleRecurrence ();
  std::vector <Task> tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("list.all.projects"))
  {
    std::vector <Task> extra = context.tdb2.completed.get_tasks ();
    std::vector <Task>::iterator task;
    for (task = extra.begin (); task != extra.end (); ++task)
      tasks.push_back (*task);
  }

  context.tdb2.commit ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    unique[task->get ("project")] = 0;

  std::map <std::string, int>::iterator project;
  for (project = unique.begin (); project != unique.end (); ++project)
    if (project->first.length ())
      output += project->first + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
