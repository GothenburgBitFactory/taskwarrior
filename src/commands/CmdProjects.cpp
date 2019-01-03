////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2019, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <CmdProjects.h>
#include <algorithm>
#include <sstream>
#include <Context.h>
#include <Filter.h>
#include <Table.h>
#include <format.h>
#include <util.h>
#include <main.h>

////////////////////////////////////////////////////////////////////////////////
CmdProjects::CmdProjects ()
{
  _keyword               = "projects";
  _usage                 = "task <filter> projects";
  _description           = "Shows all project names used";
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
  handleUntil ();
  handleRecurrence ();
  auto tasks = Context::getContext ().tdb2.pending.get_tasks ();

  if (Context::getContext ().config.getBoolean ("list.all.projects"))
    for (auto& task : Context::getContext ().tdb2.completed.get_tasks ())
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
    Table view;
    view.width (Context::getContext ().getWidth ());
    view.add ("Project");
    view.add ("Tasks", false);
    setHeaderUnderline (view);

    std::vector <std::string> processed;
    int project_number_correction = 0;
    for (auto& project : unique)
    {
      // store project name
      std::string project_name = project.first;
      // catch project names including '-'
      if (project_name.find ('-') != std::string::npos)
      {
        // replace '-' with '=' which has a higher ASCII code
        // and will therefore be processed later
        std::string new_project = project_name;
        std::replace(new_project.begin (), new_project.end (), '-', '=');
        unique[new_project] = project.second;
        // account for the "additional project"
        project_number_correction++;
        continue;
      }
      // catch project names including '='
      // (assuming the user did not create any such projects himself)
      if (project_name.find ('=') != std::string::npos)
      {
        // revert '=' into '-' for correct displaying
        std::replace(project_name.begin (), project_name.end (), '=', '-');
      }
      const std::vector <std::string> parents = extractParents (project_name);
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
      view.set (row, 0, (project_name == ""
                          ? "(none)"
                          : indentProject (project_name, "  ", '.')));
      view.set (row, 1, project.second);
      processed.push_back (project_name);
    }

    int number_projects = unique.size () - project_number_correction;
    if (no_project)
      --number_projects;

    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ()
        << (number_projects == 1
              ? format ("{1} project",  number_projects)
              : format ("{1} projects", number_projects))
        << ' '
        << (quantity == 1
              ? format ("({1} task)",  quantity)
              : format ("({1} tasks)", quantity))
        << '\n';
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
  _keyword               = "_projects";
  _usage                 = "task <filter> _projects";
  _description           = "Shows only a list of all project names used";
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
  handleUntil ();
  handleRecurrence ();
  auto tasks = Context::getContext ().tdb2.pending.get_tasks ();

  if (Context::getContext ().config.getBoolean ("list.all.projects"))
    for (auto& task : Context::getContext ().tdb2.completed.get_tasks ())
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
      output += project.first + '\n';

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
