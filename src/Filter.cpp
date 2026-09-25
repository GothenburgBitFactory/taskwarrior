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
#include <DOM.h>
#include <Eval.h>
#include <Filter.h>
#include <Timer.h>
#include <Variant.h>
#include <format.h>
#include <shared.h>

////////////////////////////////////////////////////////////////////////////////
// Take an input set of tasks and filter into a subset.
void Filter::subset(const std::vector<Task>& input, std::vector<Task>& output) {
  Timer timer;
  const auto load_before = Context::getContext().time_load_us;
  _startCount = (int)input.size();

  Context::getContext().cli2.prepareFilter();

  filter_to_tasks(input, output);

  _endCount = (int)output.size();
  Context::getContext().debug(
      format("Filtered {1} tasks --> {2} tasks [list subset]", _startCount, _endCount));
  Context::getContext().time_filter_us +=
      timer.total_us() - (Context::getContext().time_load_us - load_before);
}

////////////////////////////////////////////////////////////////////////////////
// Take the set of all tasks and filter into a subset.
void Filter::subset(std::vector<Task>& output) {
  Timer timer;
  const auto load_before = Context::getContext().time_load_us;
  Context::getContext().cli2.prepareFilter();

  std::vector<std::pair<std::string, Lexer::Type>> precompiled;
  for (auto& a : Context::getContext().cli2._args)
    if (a.hasTag("FILTER")) precompiled.emplace_back(a.getToken(), a._lextype);

  // Shortcut indicates that only tasks in the working set are loaded.
  bool shortcut = false;

  if (precompiled.size()) {
    const auto& pending = Context::getContext().tdb2.pending_tasks();
    _startCount = (int)pending.size();

    output.clear();

    filter_to_tasks(pending, output);

    shortcut = pendingOnly();
    if (!shortcut) {
      const auto& completed = Context::getContext().tdb2.completed_tasks();
      _startCount += (int)completed.size();

      filter_to_tasks(completed, output);
    }
  } else {
    safety();

    output = Context::getContext().tdb2.all_tasks();
    _startCount = (int)output.size();
  }

  _endCount = (int)output.size();
  Context::getContext().debug(format("Filtered {1} tasks --> {2} tasks [{3}]", _startCount,
                                     _endCount, (shortcut ? "pending only" : "all tasks")));
  Context::getContext().time_filter_us +=
      timer.total_us() - (Context::getContext().time_load_us - load_before);
}

/////////////////////////////////////////////////////////////////////////////////
bool Filter::hasFilter() const {
  for (const auto& a : Context::getContext().cli2._args)
    if (a.hasTag("FILTER")) return true;

  return false;
}

/////////////////////////////////////////////////////////////////////////////////
// Evaluates a pre-parsed filter against a set of tasks and stores their indices
// from the vector. The filter is parsed with prepareFilter(), but this
// function does not call that or safety() itself - callers are expected to do so.
void Filter::filter_to_indices(const std::vector<Task>& pending, std::vector<int>& indices) const {
  std::vector<std::pair<std::string, Lexer::Type>> precompiled;
  for (auto& a : Context::getContext().cli2._args)
    if (a.hasTag("FILTER")) precompiled.emplace_back(a.getToken(), a._lextype);

  if (precompiled.empty()) {
    indices.reserve(pending.size());
    for (int i = 0; i < (int)pending.size(); ++i) indices.push_back(i);
  } else {
    Eval eval;
    eval.addSource(domSource);
    eval.debug(Context::getContext().config.getInteger("debug.parser") >= 3);
    eval.compileExpression(precompiled);
    for (int i = 0; i < (int)pending.size(); ++i) {
      auto currentTask = Context::getContext().withCurrentTask(&pending[i]);
      Variant var;
      eval.evaluateCompiledExpression(var);
      if (var.get_bool()) indices.push_back(i);
    }
    eval.debug(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Like filter_to_indices, but copies matched tasks into the output.
void Filter::filter_to_tasks(const std::vector<Task>& input, std::vector<Task>& output) const {
  std::vector<std::pair<std::string, Lexer::Type>> precompiled;
  for (auto& a : Context::getContext().cli2._args)
    if (a.hasTag("FILTER")) precompiled.emplace_back(a.getToken(), a._lextype);

  if (precompiled.empty()) {
    output = input;
  } else {
    Eval eval;
    eval.addSource(domSource);
    eval.debug(Context::getContext().config.getInteger("debug.parser") >= 3);
    eval.compileExpression(precompiled);
    for (auto& task : input) {
      auto currentTask = Context::getContext().withCurrentTask(&task);
      Variant var;
      eval.evaluateCompiledExpression(var);
      if (var.get_bool()) output.push_back(task);
    }
    eval.debug(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Recognizes pending only constraints that allow us to use the shortcut.

bool Filter::pendingOnly() const {
  if (!Context::getContext().config.getBoolean("gc")) return false;

  const auto& cli = Context::getContext().cli2;
  if (!cli._uuid_list.empty()) return false;

  std::vector<const A2*> filter_args;
  for (const auto& arg : cli._args) {
    if (!arg.hasTag("FILTER")) continue;

    filter_args.push_back(&arg);
  }

  const auto requires_pending = [&](const auto& self, size_t begin, size_t end) -> bool {
    if (begin == end) return false;
    int depth = 0;
    size_t first_close = end;
    std::vector<size_t> conjunctions;
    for (size_t i = begin; i < end; ++i) {
      const auto& arg = *filter_args[i];
      if (arg._lextype != Lexer::Type::op) continue;
      const auto& op = arg.attribute("raw");
      if (op == "(") {
        ++depth;
      } else if (op == ")") {
        if (--depth < 0) return false;
        if (depth == 0 && first_close == end) first_close = i;
      } else if (depth == 0) {
        if (op == "or" || op == "xor") return false;
        if (op == "and") conjunctions.push_back(i);
      }
    }

    if (depth != 0) return false;
    if (filter_args[begin]->_lextype == Lexer::Type::op &&
        filter_args[begin]->attribute("raw") == "(" && first_close == end - 1)
      return self(self, begin + 1, end - 1);

    if (!conjunctions.empty()) {
      bool required = false;
      for (auto boundary : conjunctions) {
        required |= self(self, begin, boundary);
        begin = boundary + 1;
      }
      return self(self, begin, end) || required;
    }

    if (end - begin != 3) return false;
    const auto& left = *filter_args[begin];
    const auto& op = *filter_args[begin + 1];
    const auto& right = *filter_args[begin + 2];
    if (left._lextype != Lexer::Type::dom || op._lextype != Lexer::Type::op) return false;
    const auto& operation = op.attribute("raw");
    const auto& value = right.attribute("raw");

    if (left.attribute("raw") == "id" && right._lextype == Lexer::Type::number &&
        (operation == "=" || operation == "==" || operation == ">=") &&
        value.find_first_not_of("0123456789") == std::string::npos &&
        value.find_first_not_of('0') != std::string::npos)
      return true;
    if (right._lextype != Lexer::Type::string) return false;
    return (left.attribute("canonical") == "status" && (operation == "=" || operation == "==") &&
            (value == "pending" || value == "waiting" || value == "recurring")) ||
           (left.attribute("raw") == "tags" && operation == "_hastag_" &&
            (value == "PENDING" || value == "ACTIVE" || value == "READY" || value == "WAITING"));
  };

  return requires_pending(requires_pending, 0, filter_args.size());
}

////////////////////////////////////////////////////////////////////////////////
// Disaster avoidance mechanism. If a !READONLY has no filter, then it can cause
// all tasks to be modified. This is usually not intended.
void Filter::safety() const {
  if (_safety) {
    bool readonly = true;
    bool filter = false;
    for (const auto& a : Context::getContext().cli2._args) {
      if (a.hasTag("CMD") && !a.hasTag("READONLY")) readonly = false;

      if (a.hasTag("FILTER")) filter = true;
    }

    if (!readonly && !filter) {
      if (!Context::getContext().config.getBoolean("allow.empty.filter"))
        throw std::string(
            "You did not specify a filter, and with the 'allow.empty.filter' value, no action is "
            "taken.");

      // If user is willing to be asked, this can be avoided.
      if (Context::getContext().config.getBoolean("confirmation") &&
          confirm("This command has no filter, and will modify all (including completed and "
                  "deleted) tasks.  Are you sure?"))
        return;

      // Sound the alarm.
      throw std::string("Command prevented from running.");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Filter::disableSafety() { _safety = false; }

////////////////////////////////////////////////////////////////////////////////
