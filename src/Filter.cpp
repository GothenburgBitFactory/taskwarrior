////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <Filter.h>
#include <algorithm>
#include <Context.h>
#include <DOM.h>
#include <Eval.h>
#include <Variant.h>
#include <Dates.h>
#include <i18n.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Const iterator that can be derefenced into a Task by domSource.
static Task dummy;
Task& contextTask = dummy;

////////////////////////////////////////////////////////////////////////////////
bool domSource (const std::string& identifier, Variant& value)
{
  if (getDOM (identifier, contextTask, value))
  {
    value.source (identifier);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Take an input set of tasks and filter into a subset.
void Filter::subset (const std::vector <Task>& input, std::vector <Task>& output)
{
  context.timer_filter.start ();
  _startCount = (int) input.size ();

  context.cli2.prepareFilter ();

  std::vector <std::pair <std::string, Lexer::Type>> precompiled;
  for (auto& a : context.cli2._args)
    if (a.hasTag ("FILTER"))
      precompiled.push_back (std::pair <std::string, Lexer::Type> (a.getToken (), a._lextype));

  if (precompiled.size ())
  {
    Eval eval;
    eval.addSource (domSource);
    eval.addSource (namedDates);

    // Debug output from Eval during compilation is useful.  During evaluation
    // it is mostly noise.
    eval.debug (context.config.getInteger ("debug.parser") >= 3 ? true : false);
    eval.compileExpression (precompiled);

    for (auto& task : input)
    {
      // Set up context for any DOM references.
      contextTask = task;

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
  context.debug (format ("Filtered {1} tasks --> {2} tasks [list subset]", _startCount, _endCount));
  context.timer_filter.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Take the set of all tasks and filter into a subset.
void Filter::subset (std::vector <Task>& output)
{
  context.timer_filter.start ();

  context.cli2.prepareFilter ();

  std::vector <std::pair <std::string, Lexer::Type>> precompiled;
  for (auto& a : context.cli2._args)
    if (a.hasTag ("FILTER"))
      precompiled.push_back (std::pair <std::string, Lexer::Type> (a.getToken (), a._lextype));

  // Shortcut indicates that only pending.data needs to be loaded.
  bool shortcut = false;

  if (precompiled.size ())
  {
    context.timer_filter.stop ();
    auto pending = context.tdb2.pending.get_tasks ();
    context.timer_filter.start ();
    _startCount = (int) pending.size ();

    Eval eval;
    eval.addSource (domSource);
    eval.addSource (namedDates);

    // Debug output from Eval during compilation is useful.  During evaluation
    // it is mostly noise.
    eval.debug (context.config.getInteger ("debug.parser") >= 3 ? true : false);
    eval.compileExpression (precompiled);

    output.clear ();
    for (auto& task : pending)
    {
      // Set up context for any DOM references.
      contextTask = task;

      Variant var;
      eval.evaluateCompiledExpression (var);
      if (var.get_bool ())
        output.push_back (task);
    }

    shortcut = pendingOnly ();
    if (! shortcut)
    {
      context.timer_filter.stop ();
      auto completed = context.tdb2.completed.get_tasks ();
      context.timer_filter.start ();
      _startCount += (int) completed.size ();

      for (auto& task : completed)
      {
        // Set up context for any DOM references.
        contextTask = task;

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
    context.timer_filter.stop ();

    for (auto& task : context.tdb2.pending.get_tasks ())
      output.push_back (task);

    for (auto& task : context.tdb2.completed.get_tasks ())
      output.push_back (task);

    context.timer_filter.start ();
  }

  _endCount = (int) output.size ();
  context.debug (format ("Filtered {1} tasks --> {2} tasks [{3}]", _startCount, _endCount, (shortcut ? "pending only" : "all tasks")));
  context.timer_filter.stop ();
}

////////////////////////////////////////////////////////////////////////////////
bool Filter::hasFilter () const
{
  for (const auto& a : context.cli2._args)
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
  if (! context.config.getBoolean ("gc"))
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
  int countId        = (int) context.cli2._id_ranges.size ();
  int countUUID      = (int) context.cli2._uuid_list.size ();
  int countOr        = 0;
  int countXor       = 0;
  int countNot       = 0;

  for (const auto& a : context.cli2._args)
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
      if (                                  raw == "waiting")      ++countPending;
      if (                                  raw == "recurring")    ++countPending;
    }
  }

  if (countUUID)
    return false;

  if (countOr || countXor || countNot)
    return false;

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
    for (const auto& a : context.cli2._args)
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
      if (! context.config.getBoolean ("allow.empty.filter"))
         throw std::string (STRING_TASK_SAFETY_ALLOW);

      // If user is willing to be asked, this can be avoided.
      if (context.config.getBoolean ("confirmation") &&
          confirm (STRING_TASK_SAFETY_VALVE))
        return;

      // Sound the alarm.
      throw std::string (STRING_TASK_SAFETY_FAIL);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Filter::disableSafety ()
{
  _safety = false;
}

////////////////////////////////////////////////////////////////////////////////
