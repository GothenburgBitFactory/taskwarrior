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

#include <CmdExport.h>
#include <Context.h>
#include <Filter.h>
#include <format.h>
#include <legacy.h>
#include <shared.h>
#include <sort.h>

////////////////////////////////////////////////////////////////////////////////
CmdExport::CmdExport() {
  _keyword = "export";
  _usage = "task <filter> export [<report>]";
  _description = "Exports tasks in JSON format";
  _read_only = true;
  _displays_id = true;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = false;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category = Command::Category::migration;
}

////////////////////////////////////////////////////////////////////////////////
int CmdExport::execute(std::string& output) {
  int rc = 0;

  auto words = Context::getContext().cli2.getWords();
  std::string selectedReport = "";

  if (words.size() == 1) {
    // Find the report matching the prompt
    for (auto& command : Context::getContext().commands) {
      if (command.second->category() == Command::Category::report &&
          closeEnough(command.second->keyword(), words[0])) {
        selectedReport = command.second->keyword();
        break;
      }
    }

    if (selectedReport.empty()) {
      throw format("Unable to find report that matches '{1}'.", words[0]);
    }
  }

  auto reportSort = Context::getContext().config.get("report." + selectedReport + ".sort");
  auto reportFilter = Context::getContext().config.get("report." + selectedReport + ".filter");

  auto sortOrder = split(reportSort, ',');
  if (sortOrder.size() != 0 && sortOrder[0] != "none") {
    validateSortColumns(sortOrder);
  }

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
    // sort_tasks requires the order array initially be identity
    for (unsigned int i = 0; i < filtered.size(); ++i) sequence.push_back(i);

    // if no sort order, sort by id
    if (!sortOrder.size()) {
      reportSort = "id,uuid";
    }

    // Sort the tasks.
    sort_tasks(filtered, sequence, reportSort);
  }

  // Export == render.
  Timer timer;

  // Obey 'limit:N'.
  int rows = 0;
  int lines = 0;
  Context::getContext().getLimits(rows, lines);
  int limit = (rows > lines ? rows : lines);

  // Is output contained within a JSON array?
  bool json_array = Context::getContext().config.getBoolean("json.array");

  // Compose output.
  if (json_array) output += "[\n";

  int counter = 0;
  for (auto& t : sequence) {
    auto task = filtered[t];
    if (counter) {
      if (json_array) output += ',';
      output += '\n';
    }

    output += task.composeJSON(true);

    ++counter;
    if (limit && counter >= limit) break;
  }

  if (filtered.size()) output += '\n';

  if (json_array) output += "]\n";

  Context::getContext().time_render_us += timer.total_us();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////

void CmdExport::validateSortColumns(std::vector<std::string>& columns) {
  for (auto& col : columns) legacySortColumnMap(col);
}
