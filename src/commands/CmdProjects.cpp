////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#include <cmake.h>
#include <CmdProjects.h>
#include <algorithm>
#include <sstream>
#include <Context.h>
#include <Filter.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdProjects::CmdProjects ()
{
  _keyword               = "projects";
  _usage                 = "task <filter> projects";
  _description           = STRING_CMD_PROJECTS_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdProjects::execute (std::string& output)
{
  int rc = 0;

  // Get all the tasks.
  handleRecurrence ();
  auto tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("list.all.projects"))
    for (auto& task : context.tdb2.completed.get_tasks ())
      tasks.push_back (task);

  // Apply the filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (tasks, filtered);
  int quantity = filtered.size ();

  std::stringstream out;

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  bool no_project = false;
  std::string project;
  for (auto& task : filtered)
  {
    if (task.getStatus () == Task::deleted)
    {
      --quantity;
      continue;
    }

    // Increase the count for the project the task belongs to and all
    // its super-projects
    project = task.get ("project");

    std::vector <std::string> projects = extractParents (project);
    projects.push_back (project);

    for (auto& parent : projects)
      unique[parent] += 1;

    if (project == "")
      no_project = true;
  }

  if (unique.size ())
  {
    // Render a list of project names from the map.
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string",       STRING_COLUMN_LABEL_PROJECT));
    view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_TASKS));

    if (context.color ())
    {
      Color label (context.config.get ("color.label"));
      view.colorHeader (label);
    }

    std::vector <std::string> processed;
    for (auto& project : unique)
    {
      const std::vector <std::string> parents = extractParents (project.first);
      for (auto& parent : parents)
      {
        if (std::find (processed.begin (), processed.end (), parent) == processed.end ())
        {
          int row = view.addRow ();
          view.set (row, 0, indentProject (parent));
          processed.push_back (parent);
        }
      }
      int row = view.addRow ();
      view.set (row, 0, (project.first == ""
                          ? STRING_CMD_PROJECTS_NONE
                          : indentProject (project.first, "  ", '.')));
      view.set (row, 1, project.second);
      processed.push_back (project.first);
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
  _keyword               = "_projects";
  _usage                 = "task <filter> _projects";
  _description           = STRING_CMD_PROJECTS_USAGE_2;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionProjects::execute (std::string& output)
{
  // Get all the tasks.
  handleRecurrence ();
  auto tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("list.all.projects"))
    for (auto& task : context.tdb2.completed.get_tasks ())
      tasks.push_back (task);

  // Apply the filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (tasks, filtered);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  for (auto& task : filtered)
    unique[task.get ("project")] = 0;

  for (auto& project : unique)
    if (project.first.length ())
      output += project.first + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
