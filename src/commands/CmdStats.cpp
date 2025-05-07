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

#include <CmdStats.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Filter.h>
#include <Table.h>
#include <format.h>
#include <stdlib.h>
#include <util.h>

#include <iomanip>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdStats::CmdStats() {
  _keyword = "stats";
  _usage = "task <filter> stats";
  _description = "Shows task database statistics";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = false;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStats::execute(std::string& output) {
  int rc = 0;
  std::stringstream out;

  std::string dateformat = Context::getContext().config.get("dateformat");

  // Count the possible reverts.
  int undoCount = Context::getContext().tdb2.num_reverts_possible();

  // Count the backlog transactions.
  int numLocalChanges = Context::getContext().tdb2.num_local_changes();

  // Get all the tasks.
  Filter filter;
  std::vector<Task> all = Context::getContext().tdb2.all_tasks();
  std::vector<Task> filtered;
  filter.subset(all, filtered);

  Datetime now;
  time_t earliest = time(nullptr);
  time_t latest = 1;
  int totalT = 0;
  int deletedT = 0;
  int pendingT = 0;
  int completedT = 0;
  int waitingT = 0;
  int taggedT = 0;
  int annotationsT = 0;
  int recurringT = 0;
  int blockingT = 0;
  int blockedT = 0;
  float daysPending = 0.0;
  int descLength = 0;
  std::map<std::string, int> allTags;
  std::map<std::string, int> allProjects;

  for (auto& task : filtered) {
    ++totalT;

    Task::status status = task.getStatus();
    switch (status) {
      case Task::deleted:
        ++deletedT;
        break;
      case Task::pending:
        ++pendingT;
        break;
      case Task::completed:
        ++completedT;
        break;
      case Task::recurring:
        ++recurringT;
        break;
      case Task::waiting:
        ++waitingT;
        break;
    }

    if (task.is_blocked) ++blockedT;
    if (task.is_blocking) ++blockingT;

    time_t entry = strtoll(task.get("entry").c_str(), nullptr, 10);
    if (entry < earliest) earliest = entry;
    if (entry > latest) latest = entry;

    if (status == Task::completed) {
      time_t end = strtoll(task.get("end").c_str(), nullptr, 10);
      daysPending += (end - entry) / 86400.0;
    }

    if (status == Task::pending) daysPending += (now.toEpoch() - entry) / 86400.0;

    descLength += task.get("description").length();
    annotationsT += task.getAnnotations().size();

    auto tags = task.getTags();
    if (tags.size()) ++taggedT;

    for (auto& tag : tags) allTags[tag] = 0;

    std::string project = task.get("project");
    if (project != "") allProjects[project] = 0;
  }

  // Create a table for output.
  Table view;
  view.width(Context::getContext().getWidth());
  view.intraPadding(2);
  view.add("Category");
  view.add("Data");
  setHeaderUnderline(view);

  int row = view.addRow();
  view.set(row, 0, "Pending");
  view.set(row, 1, pendingT);

  row = view.addRow();
  view.set(row, 0, "Waiting");
  view.set(row, 1, waitingT);

  row = view.addRow();
  view.set(row, 0, "Recurring");
  view.set(row, 1, recurringT);

  row = view.addRow();
  view.set(row, 0, "Completed");
  view.set(row, 1, completedT);

  row = view.addRow();
  view.set(row, 0, "Deleted");
  view.set(row, 1, deletedT);

  row = view.addRow();
  view.set(row, 0, "Total");
  view.set(row, 1, totalT);

  row = view.addRow();
  view.set(row, 0, "Annotations");
  view.set(row, 1, annotationsT);

  row = view.addRow();
  view.set(row, 0, "Unique tags");
  view.set(row, 1, (int)allTags.size());

  row = view.addRow();
  view.set(row, 0, "Projects");
  view.set(row, 1, (int)allProjects.size());

  row = view.addRow();
  view.set(row, 0, "Blocked tasks");
  view.set(row, 1, blockedT);

  row = view.addRow();
  view.set(row, 0, "Blocking tasks");
  view.set(row, 1, blockingT);

  row = view.addRow();
  view.set(row, 0, "Undo transactions");
  view.set(row, 1, undoCount);

  row = view.addRow();
  view.set(row, 0, "Sync backlog transactions");
  view.set(row, 1, numLocalChanges);

  if (totalT) {
    row = view.addRow();
    view.set(row, 0, "Tasks tagged");

    std::stringstream value;
    value << std::setprecision(3) << (100.0 * taggedT / totalT) << '%';
    view.set(row, 1, value.str());
  }

  if (filtered.size()) {
    Datetime e(earliest);
    row = view.addRow();
    view.set(row, 0, "Oldest task");
    view.set(row, 1, e.toString(dateformat));

    Datetime l(latest);
    row = view.addRow();
    view.set(row, 0, "Newest task");
    view.set(row, 1, l.toString(dateformat));

    row = view.addRow();
    view.set(row, 0, "Task used for");
    view.set(row, 1, Duration(latest - earliest).formatVague());
  }

  if (totalT) {
    row = view.addRow();
    view.set(row, 0, "Task added every");
    view.set(row, 1, Duration(((latest - earliest) / totalT)).formatVague());
  }

  if (completedT) {
    row = view.addRow();
    view.set(row, 0, "Task completed every");
    view.set(row, 1, Duration((latest - earliest) / completedT).formatVague());
  }

  if (deletedT) {
    row = view.addRow();
    view.set(row, 0, "Task deleted every");
    view.set(row, 1, Duration((latest - earliest) / deletedT).formatVague());
  }

  if (pendingT || completedT) {
    row = view.addRow();
    view.set(row, 0, "Average time pending");
    view.set(row, 1,
             Duration((int)((daysPending / (pendingT + completedT)) * 86400)).formatVague());
  }

  if (totalT) {
    row = view.addRow();
    view.set(row, 0, "Average desc length");
    view.set(row, 1, format("{1} characters", (int)(descLength / totalT)));
  }

  // If an alternating row color is specified, notify the table.
  if (Context::getContext().color()) {
    Color alternate(Context::getContext().config.get("color.alternate"));
    if (alternate.nontrivial()) {
      view.colorOdd(alternate);
      view.intraColorOdd(alternate);
      view.extraColorOdd(alternate);
    }
  }

  out << optionalBlankLine() << view.render() << optionalBlankLine();

  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
