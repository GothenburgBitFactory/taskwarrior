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
#include <Filter.h>
#include <algorithm>
#include <Context.h>
#include <Timer.h>
#include <DOM.h>
#include <Eval.h>
#include <Variant.h>
#include <format.h>
#include <shared.h>

////////////////////////////////////////////////////////////////////////////////
// Take an input set of tasks and filter into a subset.
void Filter::subset (const std::vector <Task>& input, std::vector <Task>& output)
{
  Timer timer;
  _startCount = (int) input.size ();

  Context::getContext ().cli2.prepareFilter ();

  std::vector <std::pair <std::string, Lexer::Type>> precompiled;
  for (auto& a : Context::getContext ().cli2._args)
    if (a.hasTag ("FILTER"))
      precompiled.emplace_back (a.getToken (), a._lextype);

  if (precompiled.size ())
  {
    Eval eval;
    eval.addSource (domSource);

    // Debug output from Eval during compilation is useful.  During evaluation
    // it is mostly noise.
    eval.debug (Context::getContext ().config.getInteger ("debug.parser") >= 3 ? true : false);
    eval.compileExpression (precompiled);

    for (auto& task : input)
    {
      // Set up context for any DOM references.
      auto currentTask = Context::getContext ().withCurrentTask(&task);

      Variant var;
      eval.evaluateCompiledExpression (var);
      if (var.get_bool ())
        output.push_back (task);
    }

    eval.debug (false);
  }
  else
    output = input;

  _endCount = (int) output.size ();
  Context::getContext ().debug (format ("Filtered {1} tasks --> {2} tasks [list subset]", _startCount, _endCount));
  Context::getContext ().time_filter_us += timer.total_us ();
}

////////////////////////////////////////////////////////////////////////////////
// Take the set of all tasks and filter into a subset.
void Filter::subset (std::vector <Task>& output)
{
  Timer timer;
  Context::getContext ().cli2.prepareFilter ();

  std::vector <std::pair <std::string, Lexer::Type>> precompiled;
  for (auto& a : Context::getContext ().cli2._args)
    if (a.hasTag ("FILTER"))
      precompiled.emplace_back (a.getToken (), a._lextype);

  // Shortcut indicates that only pending.data needs to be loaded.
  bool shortcut = false;

  if (precompiled.size ())
  {
    Timer timer_pending;
    auto pending = Context::getContext ().tdb2.pending.get_tasks ();
    Context::getContext ().time_filter_us -= timer_pending.total_us ();
    _startCount = (int) pending.size ();

    Eval eval;
    eval.addSource (domSource);

    // Debug output from Eval during compilation is useful.  During evaluation
    // it is mostly noise.
    eval.debug (Context::getContext ().config.getInteger ("debug.parser") >= 3 ? true : false);
    eval.compileExpression (precompiled);

    output.clear ();
    for (auto& task : pending)
    {
      // Set up context for any DOM references.
      auto currentTask = Context::getContext ().withCurrentTask(&task);

      Variant var;
      eval.evaluateCompiledExpression (var);
      if (var.get_bool ())
        output.push_back (task);
    }

    shortcut = pendingOnly ();
    if (! shortcut)
    {
      Timer timer_completed;
      auto completed = Context::getContext ().tdb2.completed.get_tasks ();
      Context::getContext ().time_filter_us -= timer_completed.total_us ();
      _startCount += (int) completed.size ();

      for (auto& task : completed)
      {
        // Set up context for any DOM references.
        auto currentTask = Context::getContext ().withCurrentTask(&task);

        Variant var;
        eval.evaluateCompiledExpression (var);
        if (var.get_bool ())
          output.push_back (task);
      }
    }

    eval.debug (false);
  }
  else
  {
    safety ();

    Timer pending_completed;
    for (auto& task : Context::getContext ().tdb2.pending.get_tasks ())
      output.push_back (task);

    for (auto& task : Context::getContext ().tdb2.completed.get_tasks ())
      output.push_back (task);
    Context::getContext ().time_filter_us -= pending_completed.total_us ();
  }

  _endCount = (int) output.size ();
  Context::getContext ().debug (format ("Filtered {1} tasks --> {2} tasks [{3}]", _startCount, _endCount, (shortcut ? "pending only" : "all tasks")));
  Context::getContext ().time_filter_us += timer.total_us ();
}

////////////////////////////////////////////////////////////////////////////////
bool Filter::hasFilter () const
{
  for (const auto& a : Context::getContext ().cli2._args)
    if (a.hasTag ("FILTER"))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// If the filter contains no 'or', 'xor' or 'not' operators, and only includes
// status values 'pending', 'waiting' or 'recurring', then the filter is
// guaranteed to only need data from pending.data.
bool Filter::pendingOnly () const
{
  // When GC is off, there are no shortcuts.
  if (! Context::getContext ().config.getBoolean ("gc"))
    return false;

  // To skip loading completed.data, there should be:
  // - 'status' in filter
  // - no 'completed'
  // - no 'deleted'
  // - no 'xor'
  // - no 'or'
  int countStatus    = 0;
  int countPending   = 0;
  int countWaiting   = 0;
  int countRecurring = 0;
  int countId        = (int) Context::getContext ().cli2._id_ranges.size ();
  int countUUID      = (int) Context::getContext ().cli2._uuid_list.size ();
  int countOr        = 0;
  int countXor       = 0;
  int countNot       = 0;
  bool pendingTag = false;
  bool activeTag  = false;

  for (const auto& a : Context::getContext ().cli2._args)
  {
    if (a.hasTag ("FILTER"))
    {
      std::string raw       = a.attribute ("raw");
      std::string canonical = a.attribute ("canonical");

      if (a._lextype == Lexer::Type::op  && raw == "or")           ++countOr;
      if (a._lextype == Lexer::Type::op  && raw == "xor")          ++countXor;
      if (a._lextype == Lexer::Type::op  && raw == "not")          ++countNot;
      if (a._lextype == Lexer::Type::dom && canonical == "status") ++countStatus;
      if (                                  raw == "pending")      ++countPending;
      if (                                  raw == "waiting")      ++countWaiting;
      if (                                  raw == "recurring")    ++countRecurring;
    }
  }

  for (const auto& word : Context::getContext ().cli2._original_args)
  {
    if (word.attribute ("raw") == "+PENDING") pendingTag = true;
    if (word.attribute ("raw") == "+ACTIVE")  activeTag = true;
  }


  if (countUUID)
    return false;

  if (countOr || countXor || countNot)
    return false;

  if (pendingTag || activeTag)
    return true;

  if (countStatus)
  {
    if (!countPending && !countWaiting && !countRecurring)
      return false;

    return true;
  }

  if (countId)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Disaster avoidance mechanism. If a !READONLY has no filter, then it can cause
// all tasks to be modified. This is usually not intended.
void Filter::safety () const
{
  if (_safety)
  {
    bool readonly = true;
    bool filter = false;
    for (const auto& a : Context::getContext ().cli2._args)
    {
      if (a.hasTag ("CMD") &&
          ! a.hasTag ("READONLY"))
        readonly = false;

      if (a.hasTag ("FILTER"))
        filter = true;
    }

    if (! readonly &&
        ! filter)
    {
      if (! Context::getContext ().config.getBoolean ("allow.empty.filter"))
         throw std::string ("You did not specify a filter, and with the 'allow.empty.filter' value, no action is taken.");

      // If user is willing to be asked, this can be avoided.
      if (Context::getContext ().config.getBoolean ("confirmation") &&
          confirm ("This command has no filter, and will modify all (including completed and deleted) tasks.  Are you sure?"))
        return;

      // Sound the alarm.
      throw std::string ("Command prevented from running.");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Filter::disableSafety ()
{
  _safety = false;
}

////////////////////////////////////////////////////////////////////////////////
