////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// cmake.h include header must come first

#include <CmdSummary.h>
#include <Context.h>
#include <Duration.h>
#include <Filter.h>
#include <Table.h>
#include <format.h>
#include <sort.h>
#include <stdlib.h>
#include <util.h>

#include <list>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdSummary::CmdSummary() {
  _keyword = "summary";
  _usage = "task <filter> summary";
  _description = "Shows a report of task status by project";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = false;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
// Project  Remaining  Avg Age  Complete  0%                  100%
// A               12      13d       55%  XXXXXXXXXXXXX-----------
// B              109   3d 12h       10%  XXX---------------------
int CmdSummary::execute(std::string& output) {
  int rc = 0;
  bool showAllProjects = Context::getContext().config.getBoolean("summary.all.projects");

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);

  // Generate unique list of project names from all pending tasks.
  std::map<std::string, bool> allProjects;
  for (auto& task : filtered)
    if (showAllProjects || task.getStatus() == Task::pending)
      allProjects[task.get("project")] = false;

  // Initialize counts, sum.
  std::map<std::string, int> countPending;
  std::map<std::string, int> countCompleted;
  std::map<std::string, double> sumEntry;
  std::map<std::string, int> counter;
  time_t now = time(nullptr);

  // Initialize counters.
  for (auto& project : allProjects) {
    countPending[project.first] = 0;
    countCompleted[project.first] = 0;
    sumEntry[project.first] = 0.0;
    counter[project.first] = 0;
  }

  // Count the various tasks.
  for (auto& task : filtered) {
    std::string project = task.get("project");
    std::vector<std::string> projects = extractParents(project);
    projects.push_back(project);

    for (auto& parent : projects) ++counter[parent];

    if (task.getStatus() == Task::pending || task.getStatus() == Task::waiting) {
      for (auto& parent : projects) {
        ++countPending[parent];

        time_t entry = strtoll(task.get("entry").c_str(), nullptr, 10);
        if (entry) sumEntry[parent] = sumEntry[parent] + (double)(now - entry);
      }
    }

    else if (task.getStatus() == Task::completed) {
      for (auto& parent : projects) {
        ++countCompleted[parent];

        time_t entry = strtoll(task.get("entry").c_str(), nullptr, 10);
        time_t end = strtoll(task.get("end").c_str(), nullptr, 10);
        if (entry && end) sumEntry[parent] = sumEntry[parent] + (double)(end - entry);
      }
    }
  }

  // Create a table for output.
  Table view;
  view.width(Context::getContext().getWidth());
  view.add("Project");
  view.add("Remaining", false);
  view.add("Avg age", false);
  view.add("Complete", false);
  view.add("0%                        100%", true, false);
  setHeaderUnderline(view);

  Color bar_color;
  Color bg_color;
  if (Context::getContext().color()) {
    bar_color = Color(Context::getContext().config.get("color.summary.bar"));
    bg_color = Color(Context::getContext().config.get("color.summary.background"));
  }

  // sort projects into sorted list
  std::list<std::pair<std::string, int>> sortedProjects;
  sort_projects(sortedProjects, allProjects);

  int barWidth = 30;
  // construct view from sorted list
  for (auto& i : sortedProjects) {
    int row = view.addRow();
    view.set(row, 0, (i.first == "" ? "(none)" : indentProject(i.first, "  ", '.')));

    view.set(row, 1, countPending[i.first]);
    if (counter[i.first])
      view.set(row, 2, Duration((int)(sumEntry[i.first] / (double)counter[i.first])).formatVague());

    int c = countCompleted[i.first];
    int p = countPending[i.first];
    int completedBar = 0;
    if (c + p) completedBar = (c * barWidth) / (c + p);

    std::string bar;
    std::string subbar;
    if (Context::getContext().color()) {
      bar += bar_color.colorize(std::string(completedBar, ' '));
      bar += bg_color.colorize(std::string(barWidth - completedBar, ' '));
    } else {
      bar += std::string(completedBar, '=') + std::string(barWidth - completedBar, ' ');
    }
    view.set(row, 4, bar);

    char percent[12] = "0%";
    if (c + p) snprintf(percent, 12, "%d%%", 100 * c / (c + p));
    view.set(row, 3, percent);
  }

  std::stringstream out;
  if (view.rows()) {
    out << optionalBlankLine() << view.render() << optionalBlankLine();

    out << format("{1} projects\n", view.rows());
  } else {
    out << "No projects.\n";
    rc = 1;
  }

  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
