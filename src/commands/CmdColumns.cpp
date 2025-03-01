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

#include <CmdColumns.h>
#include <Color.h>
#include <Context.h>
#include <Table.h>
#include <shared.h>
#include <util.h>

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
CmdColumns::CmdColumns() {
  _keyword = "columns";
  _usage = "task          columns [substring]";
  _description = "All supported columns and formatting styles";
  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category = Command::Category::config;
}

////////////////////////////////////////////////////////////////////////////////
int CmdColumns::execute(std::string& output) {
  // Obtain the arguments from the description.  That way, things like '--'
  // have already been handled.
  auto words = Context::getContext().cli2.getWords();
  if (words.size() > 1) throw std::string("You can only specify one search string.");

  // Include all columns in the table.
  std::vector<std::string> names;
  for (const auto& col : Context::getContext().columns) names.push_back(col.first);

  std::sort(names.begin(), names.end());

  // Render a list of column names, formats and examples.
  Table formats;
  formats.width(Context::getContext().getWidth());
  formats.add("Columns");
  formats.add("Type");
  formats.add("Modifiable");
  formats.add("Supported Formats");
  formats.add("Example");
  setHeaderUnderline(formats);

  for (const auto& name : names) {
    if (words.size() == 0 || find(name, words[0], false) != std::string::npos) {
      auto styles = Context::getContext().columns[name]->styles();
      auto examples = Context::getContext().columns[name]->examples();

      for (unsigned int i = 0; i < styles.size(); ++i) {
        auto row = formats.addRow();
        formats.set(row, 0, i == 0 ? name : "");
        formats.set(row, 1, i == 0 ? Context::getContext().columns[name]->type() : "");
        formats.set(row, 2,
                    i == 0 ? (Context::getContext().columns[name]->modifiable() ? "Modifiable"
                                                                                : "Read Only")
                           : "");
        formats.set(row, 3, styles[i] + (i == 0 ? "*" : ""));
        formats.set(row, 4, i < examples.size() ? examples[i] : "");
      }
    }
  }

  auto row = formats.addRow();
  formats.set(row, 0, "<uda>");
  formats.set(row, 1, "<type>");
  formats.set(row, 2, "Modifiable");
  formats.set(row, 3, "default*");

  row = formats.addRow();
  formats.set(row, 0, "");
  formats.set(row, 3, "indicator");

  output = optionalBlankLine() + formats.render() + '\n' +
           "* Means default format, and therefore optional.  For example, 'due' and "
           "'due.formatted' are equivalent.\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionColumns::CmdCompletionColumns() {
  _keyword = "_columns";
  _usage = "task          _columns";
  _description = "Displays only a list of supported columns";
  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionColumns::execute(std::string& output) {
  // Include all columns.
  std::vector<std::string> names;
  for (const auto& col : Context::getContext().columns) names.push_back(col.first);

  std::sort(names.begin(), names.end());

  // Render only the column names.
  for (const auto& name : names) output += name + '\n';

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
