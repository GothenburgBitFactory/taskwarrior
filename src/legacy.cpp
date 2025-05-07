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
#include <format.h>

#include <sstream>

#define STRING_LEGACY_PRIORITY "Legacy attribute found.  Please change '{1}' to '{2}'."

////////////////////////////////////////////////////////////////////////////////
void legacyColumnMap(std::string& name) {
  // 2014-01-26: priority_long        --> priority.long        Mapping removed
  // 2014-01-26: entry_time           --> entry                Mapping removed
  // 2014-01-26: start_time           --> start                Mapping removed
  // 2014-01-26: end_time             --> end                  Mapping removed
  // 2014-01-26: countdown            --> due.countdown        Mapping removed
  // 2014-01-26: countdown_compact    --> due.countdown        Mapping removed
  // 2014-01-26: age                  --> entry.age            Mapping removed
  // 2014-01-26: age_compact          --> entry.age            Mapping removed
  // 2014-01-26: active               --> start.active         Mapping removed
  // 2014-01-26: recurrence_indicator --> recur.indicator      Mapping removed
  // 2014-01-26: tag_indicator        --> tags.indicator       Mapping removed
  // 2014-01-26: description_only     --> description.desc     Mapping removed

  // One-time initialization, on demand.
  static std::map<std::string, std::string> legacyMap{{"priority.", "priority"}};

  // If a legacy column was used, complain about it, but modify it anyway.
  auto found = legacyMap.find(name);
  if (found != legacyMap.end()) {
    Context::getContext().footnote(format(STRING_LEGACY_PRIORITY, name, found->second));
    name = found->second;
  }
}

////////////////////////////////////////////////////////////////////////////////
void legacySortColumnMap(std::string& name) {
  // 2014-01-26: priority_long        --> priority             Mapping removed
  // 2014-01-26: entry_time           --> entry                Mapping removed
  // 2014-01-26: start_time           --> start                Mapping removed
  // 2014-01-26: end_time             --> end                  Mapping removed
  // 2014-01-26: countdown            --> due                  Mapping removed
  // 2014-01-26: countdown_compact    --> due                  Mapping removed
  // 2014-01-26: age                  --> entry                Mapping removed
  // 2014-01-26: age_compact          --> entry                Mapping removed
  // 2014-01-26: active               --> start                Mapping removed
  // 2014-01-26: recurrence_indicator --> recur                Mapping removed
  // 2014-01-26: tag_indicator        --> tags                 Mapping removed
  // 2014-01-26: description_only     --> description          Mapping removed

  // One-time initialization, on demand.
  static std::map<std::string, std::string> legacyMap{{"priority.", "priority"}};

  // If a legacy column was used, complain about it, but modify it anyway.
  auto found = legacyMap.find(name);
  if (found != legacyMap.end()) {
    Context::getContext().footnote(format(STRING_LEGACY_PRIORITY, name, found->second));
    name = found->second;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedVariables() {
  std::vector<std::string> deprecated;
  for (auto& it : Context::getContext().config) {
    // 2014-07-04: report.*.limit removed.
    // 2016-02-24: alias._query removed.

    // Deprecated in 2.5.0.
    // report.*.annotations
    if (it.first.length() > 19 && it.first.substr(0, 7) == "report." &&
        it.first.substr(it.first.length() - 12) == ".annotations")
      deprecated.push_back(it.first);

    // Deprecated in 2.5.0.
    if (it.first == "next" || it.first == "annotations" || it.first == "export.ical.class")
      deprecated.push_back(it.first);

    // Deprecated in 2.5.0.
    if (it.first == "urgency.inherit.coefficient") deprecated.push_back(it.first);
  }

  std::stringstream out;
  if (deprecated.size()) {
    out << "Your .taskrc file contains variables that are deprecated:\n";

    for (const auto& dep : deprecated) out << "  " << dep << "\n";

    out << "\n";
  }

  return out.str();
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedColumns() {
  std::vector<std::string> deprecated;
  for (auto& it : Context::getContext().config) {
    if (it.first.find("report") == 0) {
      // Deprecated in 2.0.0
      std::string value = Context::getContext().config.get(it.first);
      if (value.find("entry_time") != std::string::npos ||
          value.find("start_time") != std::string::npos ||
          value.find("end_time") != std::string::npos)
        deprecated.push_back(it.first);
    }
  }

  std::stringstream out;
  out << "\n";

  if (deprecated.size()) {
    out << "Your .taskrc file contains reports with deprecated columns.  Please check for "
           "entry_time, start_time or end_time in:\n";

    for (const auto& dep : deprecated)
      out << "  " << dep << "=" << Context::getContext().config.get(dep) << "\n";

    out << "\n";
  }

  return out.str();
}

////////////////////////////////////////////////////////////////////////////////
void legacyAttributeMap(std::string& name) {
  // TW-1274, 2.4.0
  if (name == "modification") name = "modified";
}

////////////////////////////////////////////////////////////////////////////////
