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

#include <ColUDA.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <format.h>
#include <shared.h>
#include <stdlib.h>
#include <utf8.h>

////////////////////////////////////////////////////////////////////////////////
ColumnUDAString::ColumnUDAString() {
  _name = "<uda>";  // Gets overwritten at runtime.
  _style = "default";
  _label = "";
  _modifiable = true;
  _uda = true;
  _hyphenate = true;
  _styles = {_style, "indicator"};
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUDAString::validate(const std::string& value) const {
  // No restrictions.
  if (_values.size() == 0) return true;

  // Look for exact match value.
  for (auto& i : _values)
    if (i == value) return true;

  // Fail if not found.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnUDAString::measure(Task& task, unsigned int& minimum, unsigned int& maximum) {
  minimum = maximum = 0;
  if (task.has(_name)) {
    if (_style == "default") {
      std::string value = task.get(_name);
      if (value != "") {
        auto stripped = Color::strip(value);
        maximum = longestLine(stripped);
        minimum = longestWord(stripped);
      }
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      minimum = maximum = utf8_width(indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUDAString::render(std::vector<std::string>& lines, Task& task, int width, Color& color) {
  if (task.has(_name)) {
    if (_style == "default") {
      std::string value = task.get(_name);
      std::vector<std::string> raw;
      wrapText(raw, value, width, _hyphenate);

      for (auto& i : raw) renderStringLeft(lines, width, color, i);
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      renderStringRight(lines, width, color, indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
ColumnUDANumeric::ColumnUDANumeric() {
  _name = "<uda>";
  _type = "numeric";
  _style = "default";
  _label = "";
  _uda = true;
  _styles = {_style, "indicator"};
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUDANumeric::validate(const std::string& value) const {
  // No restrictions.
  if (_values.size() == 0) return true;

  // Look for exact match value.
  for (auto& i : _values)
    if (i == value) return true;

  // Fail if not found.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnUDANumeric::measure(Task& task, unsigned int& minimum, unsigned int& maximum) {
  minimum = maximum = 0;
  if (task.has(_name)) {
    if (_style == "default") {
      auto value = task.get(_name);
      if (value != "") minimum = maximum = value.length();
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      minimum = maximum = utf8_width(indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUDANumeric::render(std::vector<std::string>& lines, Task& task, int width,
                              Color& color) {
  if (task.has(_name)) {
    if (_style == "default") {
      auto value = task.get(_name);
      renderStringRight(lines, width, color, value);
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      renderStringRight(lines, width, color, indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
ColumnUDADate::ColumnUDADate() {
  _name = "<uda>";
  _type = "date";
  _style = "default";
  _label = "";
  _uda = true;
  _styles = {_style, "indicator"};
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUDADate::validate(const std::string& value) const {
  // No restrictions.
  if (_values.size() == 0) return true;

  // Look for exact match value.
  for (auto& i : _values)
    if (i == value) return true;

  // Fail if not found.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnUDADate::measure(Task& task, unsigned int& minimum, unsigned int& maximum) {
  minimum = maximum = 0;
  if (task.has(_name)) {
    if (_style == "default") {
      auto value = task.get(_name);
      if (value != "") {
        // Determine the output date format, which uses a hierarchy of definitions.
        //   rc.report.<report>.dateformat
        //   rc.dateformat.report
        //   rc.dateformat
        Datetime date(strtoll(value.c_str(), nullptr, 10));
        auto format = Context::getContext().config.get("report." + _report + ".dateformat");
        if (format == "") format = Context::getContext().config.get("dateformat.report");
        if (format == "") format = Context::getContext().config.get("dateformat");

        minimum = maximum = Datetime::length(format);
      }
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      minimum = maximum = utf8_width(indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUDADate::render(std::vector<std::string>& lines, Task& task, int width, Color& color) {
  if (task.has(_name)) {
    if (_style == "default") {
      auto value = task.get(_name);

      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat.
      auto format = Context::getContext().config.get("report." + _report + ".dateformat");
      if (format == "") {
        format = Context::getContext().config.get("dateformat.report");
        if (format == "") format = Context::getContext().config.get("dateformat");
      }

      renderStringLeft(lines, width, color,
                       Datetime(strtoll(value.c_str(), nullptr, 10)).toString(format));
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      renderStringRight(lines, width, color, indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
ColumnUDADuration::ColumnUDADuration() {
  _name = "<uda>";
  _type = "duration";
  _style = "default";
  _label = "";
  _uda = true;
  _styles = {_style, "indicator"};
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUDADuration::validate(const std::string& value) const {
  // No restrictions.
  if (_values.size() == 0) return true;

  // Look for exact match value.
  for (auto& i : _values)
    if (i == value) return true;

  // Fail if not found.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnUDADuration::measure(Task& task, unsigned int& minimum, unsigned int& maximum) {
  minimum = maximum = 0;
  if (task.has(_name)) {
    if (_style == "default") {
      auto value = task.get(_name);
      if (value != "") minimum = maximum = Duration(value).formatISO().length();
    } else if (_style == "indicator") {
      if (task.has(_name)) {
        auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
        if (indicator == "") indicator = "U";

        minimum = maximum = utf8_width(indicator);
      } else
        minimum = maximum = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUDADuration::render(std::vector<std::string>& lines, Task& task, int width,
                               Color& color) {
  if (task.has(_name)) {
    if (_style == "default") {
      auto value = task.get(_name);
      renderStringRight(lines, width, color, Duration(value).formatISO());
    } else if (_style == "indicator") {
      auto indicator = Context::getContext().config.get("uda." + _name + ".indicator");
      if (indicator == "") indicator = "U";

      renderStringRight(lines, width, color, indicator);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
ColumnUDAUUID::ColumnUDAUUID() {
  _name = "<uda>";
  _type = "uuid";
  _style = "long";
  _label = "";
  _modifiable = true;
  _uda = true;
  _styles = {"long", "short"};
  _examples = {"f30cb9c3-3fc0-483f-bfb2-3bf134f00694", "f30cb9c3"};
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUDAUUID::validate(const std::string& input) const {
  Lexer lex(input);
  std::string token;
  Lexer::Type type;
  return lex.isUUID(token, type, true);
}

////////////////////////////////////////////////////////////////////////////////
