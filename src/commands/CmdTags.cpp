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

#include <CmdTags.h>
#include <Context.h>
#include <Filter.h>
#include <Table.h>
#include <format.h>
#include <util.h>

#include <sstream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
CmdTags::CmdTags() {
  _keyword = "tags";
  _usage = "task <filter> tags";
  _description = "Shows a list of all tags used";
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
int CmdTags::execute(std::string& output) {
  int rc = 0;
  std::stringstream out;

  // Get all the tasks.
  auto tasks = Context::getContext().tdb2.pending_tasks();

  if (Context::getContext().config.getBoolean("list.all.tags"))
    for (auto& task : Context::getContext().tdb2.completed_tasks()) tasks.push_back(task);

  int quantity = tasks.size();

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(tasks, filtered);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map<std::string, int> unique;
  for (auto& task : filtered) {
    for (auto& tag : task.getTags())
      if (unique.find(tag) != unique.end())
        unique[tag]++;
      else
        unique[tag] = 1;
  }

  if (unique.size()) {
    // Render a list of tags names from the map.
    Table view;
    view.width(Context::getContext().getWidth());
    view.add("Tag");
    view.add("Count", false);
    setHeaderUnderline(view);

    Color bold;
    if (Context::getContext().color()) bold = Color("bold");

    bool special = false;
    for (auto& i : unique) {
      // Highlight the special tags.
      special = (Context::getContext().color() && (i.first == "nocolor" || i.first == "nonag" ||
                                                   i.first == "nocal" || i.first == "next"))
                    ? true
                    : false;

      int row = view.addRow();
      view.set(row, 0, i.first, special ? bold : Color());
      view.set(row, 1, i.second, special ? bold : Color());
    }

    out << optionalBlankLine() << view.render() << optionalBlankLine();

    if (unique.size() == 1)
      Context::getContext().footnote("1 tag");
    else
      Context::getContext().footnote(format("{1} tags", unique.size()));

    if (quantity == 1)
      Context::getContext().footnote("(1 task)");
    else
      Context::getContext().footnote(format("({1} tasks)", quantity));

    out << '\n';
  } else {
    Context::getContext().footnote("No tags.");
    rc = 1;
  }

  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionTags::CmdCompletionTags() {
  _keyword = "_tags";
  _usage = "task <filter> _tags";
  _description = "Shows only a list of all tags used, for autocompletion purposes";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = false;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionTags::execute(std::string& output) {
  // Get all the tasks.
  auto tasks = Context::getContext().tdb2.pending_tasks();

  if (Context::getContext().config.getBoolean("complete.all.tags"))
    for (auto& task : Context::getContext().tdb2.completed_tasks()) tasks.push_back(task);

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(tasks, filtered);

  // Scan all the tasks for their tags, building a map using tag
  // names as keys.
  std::map<std::string, int> unique;
  for (auto& task : filtered)
    for (auto& tag : task.getTags()) unique[tag] = 0;

  // Add built-in tags to map.
  unique["nocolor"] = 0;
  unique["nonag"] = 0;
  unique["nocal"] = 0;
  unique["next"] = 0;
  unique["ACTIVE"] = 0;
  unique["ANNOTATED"] = 0;
  unique["BLOCKED"] = 0;
  unique["BLOCKING"] = 0;
  unique["CHILD"] = 0;  // 2017-01-07: Deprecated in 2.6.0
  unique["COMPLETED"] = 0;
  unique["DELETED"] = 0;
  unique["DUE"] = 0;
  unique["DUETODAY"] = 0;  // 2016-03-29: Deprecated in 2.6.0
  unique["INSTANCE"] = 0;
  unique["LATEST"] = 0;
  unique["MONTH"] = 0;
  unique["ORPHAN"] = 0;
  unique["OVERDUE"] = 0;
  unique["PARENT"] = 0;  // 2017-01-07: Deprecated in 2.6.0
  unique["PENDING"] = 0;
  unique["PRIORITY"] = 0;
  unique["PROJECT"] = 0;
  unique["QUARTER"] = 0;
  unique["READY"] = 0;
  unique["SCHEDULED"] = 0;
  unique["TAGGED"] = 0;
  unique["TEMPLATE"] = 0;
  unique["TODAY"] = 0;
  unique["TOMORROW"] = 0;
  unique["UDA"] = 0;
  unique["UNBLOCKED"] = 0;
  unique["UNTIL"] = 0;
  unique["WAITING"] = 0;
  unique["WEEK"] = 0;
  unique["YEAR"] = 0;
  unique["YESTERDAY"] = 0;

  // If you update the above list, update src/commands/CmdInfo.cpp and src/Task.cpp as well.

  std::stringstream out;
  for (auto& it : unique) out << it.first << '\n';

  output = out.str();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
