////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <iostream> // TODO Remove
#include <cmake.h>
#include <Context.h>
#include <Eval.h>
#include <E9.h>
#include <Variant.h>
#include <Dates.h>
#include <Filter.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Const iterator that can be derefenced into a Task by domSource.
static std::vector <Task>::const_iterator contextTask;

////////////////////////////////////////////////////////////////////////////////
static bool domSource (const std::string& identifier, Variant& value)
{
  std::string stringValue = context.dom.get (identifier, *contextTask);
  if (stringValue != identifier)
  {
    value = Variant (stringValue);
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

  A3 filt = context.a3.extract_filter ();
  filt.dump ("extract_filter");

  if (context.config.getBoolean ("debug"))
  {
    Tree* t = context.a3t.tree ();
    if (t)
      context.debug (t->dump ());
  }

  std::string filterExpr = context.a3t.getFilterExpression ();
  context.debug ("\033[1;37;42mFILTER\033[0m " + filterExpr);

  if (filterExpr.length ())
  {
    E9 e (filt);

    Eval eval;
    eval.addSource (namedDates);
    eval.addSource (domSource);
    eval.compileExpression (filterExpr);

    if (context.config.getBoolean ("debug"))
      eval.debug ();

    std::vector <Task>::const_iterator task;
    for (task = input.begin (); task != input.end (); ++task)
    {
      bool oldFilter = e.evalFilter (*task);
      if (oldFilter)
        output.push_back (*task);

      contextTask = task;

      Variant var;
      eval.evaluateCompiledExpression (var);

      if (oldFilter != var.get_bool ())
        std::cout << "# filter mismatch ID " << task->id << " UUID " << task->get ("uuid") << "\n";
    }
  }
  else
    output = input;

  context.timer_filter.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// Take the set of all tasks and filter into a subset.
void Filter::subset (std::vector <Task>& output)
{
  context.timer_filter.start ();
  A3 filt = context.a3.extract_filter ();
  filt.dump ("extract_filter");

  if (context.config.getBoolean ("debug"))
  {
    Tree* t = context.a3t.tree ();
    if (t)
      context.debug (t->dump ());
  }

  std::string filterExpr = context.a3t.getFilterExpression ();
  context.debug ("\033[1;37;42mFILTER\033[0m " + filterExpr);

  if (filterExpr.length ())
  {
    context.timer_filter.stop ();
    const std::vector <Task>& pending = context.tdb2.pending.get_tasks ();
    context.timer_filter.start ();
    E9 e (filt);

    Eval eval;
    eval.addSource (namedDates);
    eval.addSource (domSource);
    eval.compileExpression (filterExpr);

    if (context.config.getBoolean ("debug"))
      eval.debug ();

    output.clear ();
    std::vector <Task>::const_iterator task;

    for (task = pending.begin (); task != pending.end (); ++task)
    {
      bool oldFilter = e.evalFilter (*task);
      if (oldFilter)
        output.push_back (*task);

      contextTask = task;

      Variant var;
      eval.evaluateCompiledExpression (var);

      if (oldFilter != var.get_bool ())
        std::cout << "# filter mismatch ID " << task->id << " UUID " << task->get ("uuid") << "\n";
    }

    if (! shortcut ())
    {
      context.timer_filter.stop ();
      const std::vector <Task>& completed = context.tdb2.completed.get_tasks (); // TODO Optional
      context.timer_filter.start ();

      for (task = completed.begin (); task != completed.end (); ++task)
      {
        bool oldFilter = e.evalFilter (*task);
        if (oldFilter)
          output.push_back (*task);

        contextTask = task;

        Variant var;
        eval.evaluateCompiledExpression (var);

        if (oldFilter != var.get_bool ())
          std::cout << "# filter mismatch ID " << task->id << " UUID " << task->get ("uuid") << "\n";
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

  context.timer_filter.stop ();
}

////////////////////////////////////////////////////////////////////////////////
// If the filter contains the restriction "status:pending", as the first filter
// term, then completed.data does not need to be loaded.
bool Filter::shortcut ()
{
/*
  // Postfix: <status> <"pending"> <=>
  //                 0           1   2
  if (filter.size ()                  >= 3                 &&
      filter[0]._raw                  == "status"          &&
      filter[1]._raw.find ("pending") != std::string::npos &&
      filter[2]._raw                  == "=")
  {
    context.debug ("Command::filter skipping completed.data (status:pending only)");
    return true;
  }

  // Shortcut: If the filter contains no 'or' or 'xor' operators, IDs and no UUIDs.
  int countId   = 0;
  int countUUID = 0;
  int countOr   = 0;
  int countXor  = 0;
  std::vector <Arg>::const_iterator i;
  for (i = filter.begin (); i != filter.end (); ++i)
  {
    if (i->_category == Arg::cat_op)
    {
      if (i->_raw == "or")  ++countOr;
      if (i->_raw == "xor") ++countXor;

    }
    else if (i->_category == Arg::cat_id)   ++countId;
    else if (i->_category == Arg::cat_uuid) ++countUUID;
  }

  if (countOr   == 0 &&
      countXor  == 0 &&
      countUUID == 0 &&
      countId    > 0)
  {
    context.debug ("Command::filter skipping completed.data (IDs, no OR, no XOR, no UUID)");
    return true;
  }
*/

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Disaster avoidance mechanism.
void Filter::safety ()
{
/*
  if (! _read_only)
  {
    A3 write_filter = context.a3.extract_filter ();
    if (!write_filter.size ())  // Potential disaster.
    {
      // If user is willing to be asked, this can be avoided.
      if (context.config.getBoolean ("confirmation") &&
          confirm (STRING_TASK_SAFETY_VALVE))
        return;

      // No.
      throw std::string (STRING_TASK_SAFETY_FAIL);
    }
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
