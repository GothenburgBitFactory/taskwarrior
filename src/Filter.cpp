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
  _startCount = (int)input.size();

  Context::getContext().cli2.prepareFilter();

  filter_to_tasks(input, output);

  _endCount = (int)output.size();
  Context::getContext().debug(
      format("Filtered {1} tasks --> {2} tasks [list subset]", _startCount, _endCount));
  Context::getContext().time_filter_us += timer.total_us();
}

////////////////////////////////////////////////////////////////////////////////
// Take the set of all tasks and filter into a subset.
void Filter::subset(std::vector<Task>& output) {
  Timer timer;
  Context::getContext().cli2.prepareFilter();

  std::vector<std::pair<std::string, Lexer::Type>> precompiled;
  for (auto& a : Context::getContext().cli2._args)
    if (a.hasTag("FILTER")) precompiled.emplace_back(a.getToken(), a._lextype);

  // Shortcut indicates that only tasks in the working set are loaded.
  bool shortcut = false;

  if (precompiled.size()) {
    Timer timer_pending;
    const auto& pending = Context::getContext().tdb2.pending_tasks();
    Context::getContext().time_filter_us -= timer_pending.total_us();
    _startCount = (int)pending.size();

    output.clear();

    filter_to_tasks(pending, output);

    shortcut = pendingOnly();
    if (!shortcut) {
      Timer timer_completed;
      const auto& completed = Context::getContext().tdb2.completed_tasks();
      Context::getContext().time_filter_us -= timer_completed.total_us();
      _startCount += (int)completed.size();

      filter_to_tasks(completed, output);
    }
  } else {
    safety();

    Timer pending_completed;
    output = Context::getContext().tdb2.all_tasks();
    Context::getContext().time_filter_us -= pending_completed.total_us();
  }

  _endCount = (int)output.size();
  Context::getContext().debug(format("Filtered {1} tasks --> {2} tasks [{3}]", _startCount,
                                     _endCount, (shortcut ? "pending only" : "all tasks")));
  Context::getContext().time_filter_us += timer.total_us();
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

  Context::getContext().debug(
      format("Filtered {1} tasks --> {2} tasks [pending only]", pending.size(), indices.size()));
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
// If the filter contains no 'or', 'xor' or 'not' operators, and only includes
// status values 'pending', 'waiting' or 'recurring', then the filter is
// guaranteed to only need data from pending.data.

bool Filter::pendingOnly() const {
  if (!Context::getContext().config.getBoolean("gc")) return false;

  const auto& cli = Context::getContext().cli2;
  if (!cli._uuid_list.empty()) return false;

  std::vector<const A2*> filter_args;
  for (const auto& arg : cli._args) {
    if (!arg.hasTag("FILTER")) continue;

    const auto& raw = arg.attribute("raw");
    if (arg._lextype == Lexer::Type::op &&
        (raw == "or" || raw == "xor" || raw == "!" || raw == "not"))
      return false;

    filter_args.push_back(&arg);
  }

  for (size_t i = 0; i + 2 < filter_args.size(); ++i) {
    const auto& left = *filter_args[i];
    const auto& op = *filter_args[i + 1];
    const auto& right = *filter_args[i + 2];
    const auto& value = right.attribute("raw");

    if (left._lextype == Lexer::Type::dom && left.attribute("canonical") == "status" &&
        op._lextype == Lexer::Type::op &&
        (op.attribute("raw") == "=" || op.attribute("raw") == "==") &&
        (value == "pending" || value == "waiting" || value == "recurring"))
      return true;

    if (left._lextype == Lexer::Type::dom && left.attribute("raw") == "tags" &&
        op._lextype == Lexer::Type::op && op.attribute("raw") == "_hastag_" &&
        (value == "PENDING" || value == "ACTIVE" || value == "READY" || value == "WAITING"))
      return true;
  }

  if (!cli._id_ranges.empty()) return true;

  return false;
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
