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

#include <Context.h>
#include <test.h>

#include <string>
#include <vector>

int TEST_NAME(int, char**) {
  struct WidthCase {
    const char* name;
    bool detection;
    int terminalWidth;
    int defaultWidth;
    bool avoidLastColumn;
    int expected;
  };
  const std::vector<WidthCase> cases = {
      {"detected width", true, 120, 80, false, 120},
      {"detected width overrides zero default", true, 120, 0, false, 120},
      {"detected width overrides negative default", true, 120, -80, false, 120},
      {"avoid last detected column", true, 120, 0, true, 119},
      {"disabled detection uses default", false, 120, 80, false, 80},
      {"disabled detection uses unlimited zero default", false, 120, 0, false, 65536},
      {"disabled detection uses unlimited negative default", false, 120, -80, false, 65536},
      {"avoid last column does not shrink default", false, 120, 80, true, 80},
      {"unusable detected width uses default", true, 0, 80, false, 80},
      {"unusable detected width uses unlimited zero default", true, 0, 0, false, 65536},
      {"unusable detected width uses unlimited negative default", true, 0, -80, false, 65536},
      {"avoid last column does not shrink fallback", true, 0, 80, true, 80},
      {"detected width overrides small default", true, 120, 1, false, 120},
      {"disabled detection preserves small default", false, 120, 1, false, 1},
  };

  UnitTest test(static_cast<int>(cases.size()));
  Context context;
  Context::setContext(&context);
  for (const auto& item : cases) {
    context.config.set("detection", item.detection ? "on" : "off");
    context.config.set("defaultwidth", std::to_string(item.defaultWidth));
    context.config.set("avoidlastcolumn", item.avoidLastColumn ? "on" : "off");
    context.terminal_width = item.terminalWidth;
    context.terminal_height = 24;
    test.is(context.getWidth(), item.expected, item.name);
  }
  return 0;
}