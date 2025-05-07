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

#include <CmdCount.h>
#include <Filter.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
CmdCount::CmdCount() {
  _keyword = "count";
  _usage = "task <filter> count";
  _description = "Counts matching tasks";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCount::execute(std::string& output) {
  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);

  // Find number of matching tasks.  Skip recurring parent tasks.
  int count = std::count_if(filtered.begin(), filtered.end(),
                            [](const auto& task) { return task.getStatus() != Task::recurring; });
  output = format(count) + '\n';
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
