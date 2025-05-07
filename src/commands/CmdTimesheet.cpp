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

#include <CmdTimesheet.h>
#include <Context.h>
#include <Datetime.h>
#include <Filter.h>
#include <Table.h>
#include <format.h>
#include <rules.h>
#include <util.h>

#include <algorithm>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdTimesheet::CmdTimesheet() {
  _keyword = "timesheet";
  _usage = "task [filter] timesheet";
  _description = "Summary of completed and started tasks";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = false;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::report;
}

////////////////////////////////////////////////////////////////////////////////
// Whether a the timesheet uses context is defined by the
// report.timesheet.context configuration variable.
//
bool CmdTimesheet::uses_context() const {
  auto config = Context::getContext().config;
  auto key = "report.timesheet.context";

  if (config.has(key))
    return config.getBoolean(key);
  else
    return _uses_context;
}

////////////////////////////////////////////////////////////////////////////////
int CmdTimesheet::execute(std::string& output) {
  int rc = 0;

  // Detect a filter.
  bool hasFilter{false};
  for (auto& a : Context::getContext().cli2._args) {
    if (a.hasTag("FILTER")) {
      hasFilter = true;
      break;
    }
  }

  if (!hasFilter) {
    auto defaultFilter = Context::getContext().config.get("report.timesheet.filter");
    if (defaultFilter == "")
      defaultFilter = "(+PENDING and start.after:now-4wks) or (+COMPLETED and end.after:now-4wks)";
    Context::getContext().cli2.addFilter(defaultFilter);
  }

  // Apply filter to get a set of tasks.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);

  // Subset the tasks to only those that are either completed or started.
  // The _key attribute is represents either the 'start' or 'end' date.
  int num_completed = 0;
  int num_started = 0;
  std::vector<Task> shown;
  for (auto& task : filtered) {
    if (task.getStatus() == Task::completed) {
      task.set("_key", task.get("end"));
      ++num_completed;
    }

    if (task.getStatus() == Task::pending && task.has("start")) {
      task.set("_key", task.get("start"));
      ++num_started;
    }

    shown.push_back(task);
  }

  // Sort tasks by _key.
  std::sort(shown.begin(), shown.end(),
            [](const Task& a, const Task& b) { return a.get("_key") < b.get("_key"); });

  // Render the completed table.
  Table table;
  table.width(Context::getContext().getWidth());
  if (Context::getContext().config.getBoolean("obfuscate")) table.obfuscate();
  table.add("Wk");
  table.add("Date");
  table.add("Day");
  table.add("ID");
  table.add("Action");
  table.add("Project");
  table.add("Due");
  table.add("Task");
  setHeaderUnderline(table);

  auto dateformat = Context::getContext().config.get("dateformat");

  int previous_week = -1;
  std::string previous_date = "";
  std::string previous_day = "";
  int weekCounter = 0;
  Color week_color;
  for (auto& task : shown) {
    Datetime key(task.get_date("_key"));

    std::string label = task.has("end") ? "Completed" : task.has("start") ? "Started" : "";

    auto week = key.week();
    auto date = key.toString(dateformat);
    auto due = task.has("due") ? Datetime(task.get("due")).toString(dateformat) : "";
    auto day = Datetime::dayNameShort(key.dayOfWeek());

    Color task_color;
    autoColorize(task, task_color);

    // Add a blank line between weeks.
    if (week != previous_week && previous_week != -1) {
      auto row = table.addRowEven();
      table.set(row, 0, " ");
    }

    // Keep track of unique week numbers.
    if (week != previous_week) ++weekCounter;

    // User-defined oddness.
    int row;
    if (weekCounter % 2)
      row = table.addRowOdd();
    else
      row = table.addRowEven();

    // If the data doesn't change, it doesn't get shown.
    table.set(row, 0, (week != previous_week ? format("W{1}", week) : ""));
    table.set(row, 1, (date != previous_date ? date : ""));
    table.set(row, 2, (day != previous_day ? day : ""));
    table.set(row, 3, task.identifier(true));
    table.set(row, 4, label);
    table.set(row, 5, task.get("project"));
    table.set(row, 6, due);
    table.set(row, 7, task.get("description"), task_color);

    previous_week = week;
    previous_date = date;
    previous_day = day;
  }

  // Render the table.
  std::stringstream out;
  if (table.rows()) out << optionalBlankLine() << table.render() << '\n';

  if (Context::getContext().verbose("affected"))
    out << format("{1} completed, {2} started.", num_completed, num_started) << '\n';

  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
