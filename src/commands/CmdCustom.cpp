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

#include <CmdCustom.h>
#include <CmdNews.h>
#include <Context.h>
#include <Filter.h>
#include <Lexer.h>
#include <Version.h>
#include <ViewTask.h>
#include <feedback.h>
#include <format.h>
#include <legacy.h>
#include <shared.h>
#include <sort.h>
#include <util.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
CmdCustom::CmdCustom(const std::string& keyword, const std::string& usage,
                     const std::string& description) {
  _keyword = keyword;
  _usage = usage;
  _description = description;
  _read_only = true;
  _displays_id = true;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Category::report;
}

////////////////////////////////////////////////////////////////////////////////
// Whether a report uses context is defined by the report.<name>.context
// configuration variable.
//
bool CmdCustom::uses_context() const {
  auto config = Context::getContext().config;
  auto key = "report." + _keyword + ".context";

  if (config.has(key))
    return config.getBoolean(key);
  else
    return _uses_context;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCustom::execute(std::string& output) {
  auto rc = 0;

  // Load report configuration.
  auto reportColumns = Context::getContext().config.get("report." + _keyword + ".columns");
  auto reportLabels = Context::getContext().config.get("report." + _keyword + ".labels");
  auto reportSort = Context::getContext().config.get("report." + _keyword + ".sort");
  auto reportFilter = Context::getContext().config.get("report." + _keyword + ".filter");

  auto columns = split(reportColumns, ',');
  validateReportColumns(columns);

  auto labels = split(reportLabels, ',');

  if (columns.size() != labels.size() && labels.size() != 0)
    throw format("There are different numbers of columns and labels for report '{1}'.", _keyword);

  auto sortOrder = split(reportSort, ',');
  if (sortOrder.size() != 0 && sortOrder[0] != "none") validateSortColumns(sortOrder);

  // Add the report filter to any existing filter.
  if (reportFilter != "") Context::getContext().cli2.addFilter(reportFilter);

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);

  std::vector<int> sequence;
  if (sortOrder.size() && sortOrder[0] == "none") {
    // Assemble a sequence vector that represents the tasks listed in
    // Context::getContext ().cli2._uuid_ranges, in the order in which they appear. This
    // equates to no sorting, just a specified order.
    sortOrder.clear();
    for (auto& i : Context::getContext().cli2._uuid_list)
      for (unsigned int t = 0; t < filtered.size(); ++t)
        if (filtered[t].get("uuid") == i) sequence.push_back(t);
  } else {
    // There is a sortOrder, so sorting will take place, which means the initial
    // order of sequence is ascending.
    for (unsigned int i = 0; i < filtered.size(); ++i) sequence.push_back(i);

    // Sort the tasks.
    if (sortOrder.size()) sort_tasks(filtered, sequence, reportSort);
  }

  // Configure the view.
  ViewTask view;
  view.width(Context::getContext().getWidth());
  view.leftMargin(Context::getContext().config.getInteger("indent.report"));
  view.extraPadding(Context::getContext().config.getInteger("row.padding"));
  view.intraPadding(Context::getContext().config.getInteger("column.padding"));

  if (Context::getContext().color()) {
    Color label(Context::getContext().config.get("color.label"));
    view.colorHeader(label);

    Color label_sort(Context::getContext().config.get("color.label.sort"));
    view.colorSortHeader(label_sort);

    // If an alternating row color is specified, notify the table.
    Color alternate(Context::getContext().config.get("color.alternate"));
    if (alternate.nontrivial()) {
      view.colorOdd(alternate);
      view.intraColorOdd(alternate);
    }
  }

  // Capture columns that are sorted.
  std::vector<std::string> sortColumns;

  // Add the break columns, if any.
  for (const auto& so : sortOrder) {
    std::string name;
    bool ascending;
    bool breakIndicator;
    Context::getContext().decomposeSortField(so, name, ascending, breakIndicator);

    if (breakIndicator) view.addBreak(name);

    sortColumns.push_back(name);
  }

  // Add the columns and labels.
  for (unsigned int i = 0; i < columns.size(); ++i) {
    Column* c = Column::factory(columns[i], _keyword);
    if (i < labels.size()) c->setLabel(labels[i]);

    bool sort = std::find(sortColumns.begin(), sortColumns.end(), c->name()) != sortColumns.end()
                    ? true
                    : false;

    view.add(c, sort);
  }

  // How many lines taken up by table header?
  int table_header = 0;
  if (Context::getContext().verbose("label")) {
    if (Context::getContext().color() && Context::getContext().config.getBoolean("fontunderline"))
      table_header = 1;  // Underlining doesn't use extra line.
    else
      table_header = 2;  // Dashes use an extra line.
  }

  // Report output can be limited by rows or lines.
  auto maxrows = 0;
  auto maxlines = 0;
  Context::getContext().getLimits(maxrows, maxlines);

  // Adjust for fluff in the output.
  if (maxlines)
    maxlines -=
        table_header + (Context::getContext().verbose("blank") ? 1 : 0) +
        (Context::getContext().verbose("footnote") ? Context::getContext().footnotes.size() : 0) +
        (Context::getContext().verbose("affected") ? 1 : 0) +
        Context::getContext().config.getInteger("reserved.lines");  // For prompt, etc.

  // Render.
  std::stringstream out;
  if (filtered.size()) {
    view.truncateRows(maxrows);
    view.truncateLines(maxlines);

    out << optionalBlankLine() << view.render(filtered, sequence) << optionalBlankLine();

    // Print the number of rendered tasks
    if (Context::getContext().verbose("affected")) {
      out << (filtered.size() == 1 ? "1 task" : format("{1} tasks", filtered.size()));

      if (maxrows && maxrows < (int)filtered.size()) out << ", " << format("{1} shown", maxrows);

      if (maxlines && maxlines < (int)filtered.size())
        out << ", " << format("truncated to {1} lines", maxlines - table_header);

      out << '\n';
    }
  } else {
    Context::getContext().footnote("No matches.");
    rc = 1;
  }

  // Inform user about the new release highlights if not presented yet
  if (CmdNews::should_nag()) {
    std::ostringstream notice;
    Version current_version = Version::Current();
    notice << "Recently upgraded to " << current_version
           << ". "
              "Please run 'task news' to read highlights about the new release.";
    if (Context::getContext().verbose("footnote"))
      Context::getContext().footnote(notice.str());
    else if (Context::getContext().verbose("header"))
      Context::getContext().header(notice.str());
  }

  std::string location = (Context::getContext().data_dir);
  File pending_data = File(location + "/pending.data");
  if (pending_data.exists()) {
    Color warning = Color(Context::getContext().config.get("color.warning"));
    std::cerr << warning.colorize(format("Found existing '*.data' files in {1}", location)) << "\n";
    std::cerr << "  Taskwarrior's storage format changed in 3.0, requiring a manual migration.\n";
    std::cerr << "  See https://taskwarrior.org/docs/upgrade-3/. Run `task import-v2` to import\n";
    std::cerr << "  the tasks into the Taskwarrior-3.x format\n";
  }

  feedback_backlog();
  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void CmdCustom::validateReportColumns(std::vector<std::string>& columns) {
  for (auto& col : columns) legacyColumnMap(col);
}

////////////////////////////////////////////////////////////////////////////////
void CmdCustom::validateSortColumns(std::vector<std::string>& columns) {
  for (auto& col : columns) legacySortColumnMap(col);
}

////////////////////////////////////////////////////////////////////////////////
