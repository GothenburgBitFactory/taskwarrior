////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <Context.h>
#include <Eval.h>
#include <Variant.h>
#include <Dates.h>
#include <Filter.h>
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
  if (context.dom.get (identifier, contextTask, value))
  {
    value.source (identifier);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
Filter::Filter ()
: _startCount (0)
, _endCount (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Filter::~Filter ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Take an input set of tasks and filter into a subset.
void Filter::subset (const std::vector <Task>& input, std::vector <Task>& output)
{
  context.timer_filter.start ();
  _startCount = (int) input.size ();

  if (context.config.getInteger ("debug.parser") >= 1)
    context.debug (context.cli.dump ("Filter::subset"));

  std::string filterExpr = context.cli.getFilter ();
  if (filterExpr.length ())
  {
    Eval eval;
    eval.ambiguity (false);
    eval.addSource (domSource);
    eval.addSource (namedDates);

    // Debug output from Eval during compilation is useful.  During evaluation
    // it is mostly noise.
    eval.debug (context.config.getInteger ("debug.parser") >= 2 ? true : false);
    eval.compileExpression (filterExpr);
    eval.debug (false);

    std::vector <Task>::const_iterator task;
    for (task = input.begin (); task != input.end (); ++task)
    {
      // Set up context for any DOM references.
      contextTask = *task;

      Variant var;
      eval.evaluateCompiledExpression (var);
      if (var.get_bool ())
        output.push_back (*task);
    }
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

  if (context.config.getInteger ("debug.parser") >= 1)
    context.debug (context.cli.dump ("Filter::subset"));

  bool shortcut = false;
  std::string filterExpr = context.cli.getFilter ();
  if (filterExpr.length ())
  {
    context.timer_filter.stop ();
    const std::vector <Task>& pending = context.tdb2.pending.get_tasks ();
    context.timer_filter.start ();
    _startCount = (int) pending.size ();

    Eval eval;
    eval.ambiguity (false);
    eval.addSource (domSource);
    eval.addSource (namedDates);

    // Debug output from Eval during compilation is useful.  During evaluation
    // it is mostly noise.
    eval.debug (context.config.getInteger ("debug.parser") >= 2 ? true : false);
    eval.compileExpression (filterExpr);
    eval.debug (false);

    output.clear ();
    std::vector <Task>::const_iterator task;

    for (task = pending.begin (); task != pending.end (); ++task)
    {
      // Set up context for any DOM references.
      contextTask = *task;

      Variant var;
      eval.debug (context.config.getInteger ("debug.parser") >= 2 ? true : false);
      eval.evaluateCompiledExpression (var);
      eval.debug (false);
      if (var.get_bool ())
        output.push_back (*task);
    }

    shortcut = pendingOnly ();
    if (! shortcut)
    {
      context.timer_filter.stop ();
      const std::vector <Task>& completed = context.tdb2.completed.get_tasks ();
      context.timer_filter.start ();
      _startCount += (int) completed.size ();

      for (task = completed.begin (); task != completed.end (); ++task)
      {
        // Set up context for any DOM references.
        contextTask = *task;

        Variant var;
        eval.debug (context.config.getInteger ("debug.parser") >= 2 ? true : false);
        eval.evaluateCompiledExpression (var);
        eval.debug (false);
        if (var.get_bool ())
          output.push_back (*task);
      }
    }
  }
  else
  {
    safety ();

    context.timer_filter.stop ();
    const std::vector <Task>& pending   = context.tdb2.pending.get_tasks ();
    const std::vector <Task>& completed = context.tdb2.completed.get_tasks ();
    context.timer_filter.start ();

    std::vector <Task>::const_iterator task;
    for (task = pending.begin (); task != pending.end (); ++task)
      output.push_back (*task);

    for (task = completed.begin (); task != completed.end (); ++task)
      output.push_back (*task);
  }

  _endCount = (int) output.size ();
  context.debug (format ("Filtered {1} tasks --> {2} tasks [{3}]", _startCount, _endCount, (shortcut ? "pending only" : "all tasks")));
  context.timer_filter.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// If the filter contains the restriction "status:pending", as the first filter
// term, then completed.data does not need to be loaded.
bool Filter::pendingOnly ()
{
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
  int countId        = 0;
  int countOr        = 0;
  int countXor       = 0;
  int countNot       = 0;

  std::vector <A>::iterator a;
  for (a = context.cli._args.begin (); a != context.cli._args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      if (a->hasTag ("ID"))                                                ++countId;
      if (a->hasTag ("OP")        && a->attribute ("raw")  == "or")        ++countOr;
      if (a->hasTag ("OP")        && a->attribute ("raw")  == "xor")       ++countXor;
      if (a->hasTag ("OP")        && a->attribute ("raw")  == "not")       ++countNot;
      if (a->hasTag ("ATTRIBUTE") && a->attribute ("name") == "status")    ++countStatus;
      if (                           a->attribute ("raw")  == "pending")   ++countPending;
      if (                           a->attribute ("raw")  == "waiting")   ++countWaiting;
      if (                           a->attribute ("raw")  == "recurring") ++countRecurring;
    }
  }

  if (countOr || countXor || countNot)
    return false;

  if (countStatus)
  {
    if (!countPending && !countWaiting && !countRecurring)
      return false;

    return true;
  }

  if (countId)
  {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Disaster avoidance mechanism. If a WRITECMD has no filter, then it can cause
// all tasks to be modified. This is usually not intended.
void Filter::safety ()
{
  std::vector <A>::iterator a;
  for (a = context.cli._args.begin (); a != context.cli._args.end (); ++a)
  {
    if (a->hasTag ("CMD"))
    {
      if (a->hasTag ("WRITECMD"))
      {
        if (context.cli.getFilter () == "")
        {
          if (! context.config.getBoolean ("allow.empty.filter"))
            throw std::string (STRING_TASK_SAFETY_ALLOW);

          // If user is willing to be asked, this can be avoided.
          if (context.config.getBoolean ("confirmation") &&
              confirm (STRING_TASK_SAFETY_VALVE))
            return;

          // Sounds the alarm.
          throw std::string (STRING_TASK_SAFETY_FAIL);
        }
      }

      // CMD was found.
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
