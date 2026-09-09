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

#include <Column.h>
#include <Context.h>
#include <Task.h>
#include <shared.h>
#include <test.h>
#include <utf8.h>

#include <cstdlib>
#include <ctime>
#include <memory>

extern std::string configurationDefaults;

namespace {
struct AnnotationCase {
  std::string name;
  int width;
  int indent;
  std::string description;
  std::vector<std::string> annotations;
  std::vector<std::string> expected;
  std::string dateformat = "D";
};
}  // namespace

////////////////////////////////////////////////////////////////////////////////
int TEST_NAME(int, char**) {
  // #3914: Test column widths directly, independently of report width allocation.
  const std::vector<AnnotationCase> cases = {
      {"default indent", 12, 2, "task", {"aa bb cc dd"}, {"task", "  08 aa bb", "  cc dd"}},
      {"zero indent", 12, 0, "task", {"aa bb cc dd"}, {"task", "08 aa bb cc", "dd"}},
      {"custom indent", 12, 4, "task", {"aa bb cc dd"}, {"task", "    08 aa bb", "    cc dd"}},
      {"exact fit", 16, 2, "task", {"aa bb cc dd"}, {"task", "  08 aa bb cc dd"}},
      {"one column short", 15, 2, "task", {"aa bb cc dd"}, {"task", "  08 aa bb cc", "  dd"}},
      {"indent equals width", 2, 2, "t", {"a"}, {"t", " 0", " 8", " a"}},
      {"indent exceeds width", 2, 100, "t", {"a"}, {"t", " 0", " 8", " a"}},
      {"one-column width", 1, 2, "t", {"a"}, {"t", "0", "8", "a"}},
      {"nonpositive width", 0, 2, "t", {"a"}, {}},
      {"negative indent", 12, -2, "task", {"aa bb cc dd"}, {"task", "08 aa bb cc", "dd"}},
      {"description wraps",
       12,
       2,
       "one two three four",
       {"aa bb cc dd"},
       {"one two", "three four", "  08 aa bb", "  cc dd"}},
      {"unannotated description wraps", 12, 2, "one two three four", {}, {"one two", "three four"}},
      {"explicit and boundary newlines",
       12,
       2,
       "task\n",
       {"aa\n\nbb\n", "cc\n"},
       {"task", "", "  08 aa", "", "  bb", "", "  08 cc"}},
      {"empty description", 12, 2, "", {"aa"}, {"", "  08 aa"}},
      {"Unicode display widths", 12, 2, "task", {"åäö 界界 éé"}, {"task", "  08 åäö", "  界界 éé"}},
      {"wide glyph in one content column", 3, 2, "t", {"界"}, {"t", "  0", "  8", "  ."}},
      {"annotation date format",
       18,
       2,
       "task",
       {"aa bb cc dd"},
       {"task", "  2025-09-08 aa bb", "  cc dd"},
       "Y-M-D"},
      {"fallback date format",
       18,
       2,
       "task",
       {"aa bb cc dd"},
       {"task", "  2025-09-08 aa bb", "  cc dd"},
       ""},
  };

  UnitTest test(cases.size() * 2);
  Context context;
  Context::setContext(&context);
  unsetenv("TASKDATA");
  unsetenv("TASKRC");
  setenv("TZ", "UTC", 1);
  tzset();
  context.config.parse(configurationDefaults, 0, {TASK_RCDIR});
  context.config.set("dateformat", "Y-M-D");
  context.config.set("hyphenate", false);

  for (const auto& item : cases) {
    context.config.set("indent.annotation", std::to_string(item.indent));
    context.config.set("dateformat.annotation", item.dateformat);
    std::unique_ptr<Column> column(Column::factory("description", "rrr"));
    Task task;
    task.set("description", item.description);
    std::map<std::string, std::string> annotations;
    int timestamp = 1757332800;
    for (const auto& annotation : item.annotations)
      annotations["annotation_" + std::to_string(timestamp++)] = annotation;
    task.setAnnotations(annotations);

    Color color;
    std::vector<std::string> lines;
    column->render(lines, task, item.width, color);
    bool fits = true;
    for (auto& line : lines) {
      fits = fits && static_cast<int>(utf8_width(line)) <= item.width;
      line = rtrim(line);
    }
    test.ok(fits, item.name + ": fits column width");
    test.ok(lines == item.expected, item.name + ": rendered lines");
    if (lines != item.expected) {
      test.diag("Expected:\n" + join("\n", item.expected));
      test.diag("Actual:\n" + join("\n", lines));
    }
  }

  return 0;
}
