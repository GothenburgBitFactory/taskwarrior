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

#include <Color.h>
#include <Context.h>
#include <Datetime.h>
#include <TF2.h>
#include <Table.h>
#include <cmake.h>
#include <format.h>
#ifdef PRODUCT_TASKWARRIOR
#include <legacy.h>
#endif
#include <shared.h>
#include <util.h>

#define STRING_TDB2_REVERTED "Modified task reverted."

////////////////////////////////////////////////////////////////////////////////
TF2::TF2() : _loaded_tasks(false), _loaded_lines(false) {}

////////////////////////////////////////////////////////////////////////////////
TF2::~TF2() {}

////////////////////////////////////////////////////////////////////////////////
void TF2::target(const std::string& f) { _file = File(f); }

////////////////////////////////////////////////////////////////////////////////
const std::vector<std::map<std::string, std::string>>& TF2::get_tasks() {
  if (!_loaded_tasks) load_tasks();

  return _tasks;
}

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse.
//
// Note that FF1, FF2, FF3, and JSON are no longer supported.
//
// start --> [ --> Att --> ] --> end
//              ^       |
//              +-------+
//
std::map<std::string, std::string> TF2::load_task(const std::string& input) {
  std::map<std::string, std::string> data;

  // File format version 4, from 2009-5-16 - now, v1.7.1+
  // This is the parse format tried first, because it is most used.
  data.clear();

  if (input[0] == '[') {
    // Not using Pig to parse here (which would be idiomatic), because we
    // don't need to differentiate betwen utf-8 and normal characters.
    // Pig's scanning the string can be expensive.
    auto ending_bracket = input.find_last_of(']');
    if (ending_bracket != std::string::npos) {
      std::string line = input.substr(1, ending_bracket);

      if (line.length() == 0) throw std::string("Empty record in input.");

      Pig attLine(line);
      std::string name;
      std::string value;
      while (!attLine.eos()) {
        if (attLine.getUntilAscii(':', name) && attLine.skip(':') &&
            attLine.getQuoted('"', value)) {
#ifdef PRODUCT_TASKWARRIOR
          legacyAttributeMap(name);
#endif

          data[name] = decode(json::decode(value));
        }

        attLine.skip(' ');
      }

      std::string remainder;
      attLine.getRemainder(remainder);
      if (remainder.length()) throw std::string("Unrecognized characters at end of line.");
    }
  } else {
    throw std::string("Record not recognized as format 4.");
  }

  // for compatibility, include all tags in `tags` as `tag_..` attributes
  if (data.find("tags") != data.end()) {
    for (auto& tag : split(data["tags"], ',')) {
      data[Task::tag2Attr(tag)] = "x";
    }
  }

  // same for `depends` / `dep_..`
  if (data.find("depends") != data.end()) {
    for (auto& dep : split(data["depends"], ',')) {
      data[Task::dep2Attr(dep)] = "x";
    }
  }

  return data;
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   [  <- &open;
//   ]  <- &close;
const std::string TF2::decode(const std::string& value) const {
  if (value.find('&') == std::string::npos) return value;

  auto modified = str_replace(value, "&open;", "[");
  return str_replace(modified, "&close;", "]");
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_tasks() {
  Timer timer;

  if (!_loaded_lines) {
    load_lines();
  }

  // Reduce unnecessary allocations/copies.
  // Calling it on _tasks is the right thing to do even when from_gc is set.
  _tasks.reserve(_lines.size());

  int line_number = 0;  // Used for error message in catch block.
  try {
    for (auto& line : _lines) {
      ++line_number;
      auto task = load_task(line);
      _tasks.push_back(task);
    }

    _loaded_tasks = true;
  }

  catch (const std::string& e) {
    throw e + format(" in {1} at line {2}", _file._data, line_number);
  }

  Context::getContext().time_load_us += timer.total_us();
}

////////////////////////////////////////////////////////////////////////////////
void TF2::load_lines() {
  if (_file.open()) {
    if (Context::getContext().config.getBoolean("locking")) _file.lock();

    _file.read(_lines);
    _file.close();
    _loaded_lines = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
// vim: ts=2 et sw=2
