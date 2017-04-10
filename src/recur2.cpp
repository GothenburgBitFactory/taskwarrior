////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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
#include <Datetime.h>
#include <Duration.h>
#include <Context.h>
#include <format.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Checklist:
// - last: Most recently generated instance integer. The first instance
//   generated is '1'.
// - Sync must merge duplicate N.
// - Remove recurrence.limit. Now always 1. It is not something that can be done
//   with rtype:chained tasks.
// - Handle until.

////////////////////////////////////////////////////////////////////////////////
// Given an old-style task, upgrades it perfectly.
// Note: Works for both parent and child.
static Task upgradeTask (const Task&)
{
  Task upgraded;

  // TODO Convert 'mask' to 'last' <-- length (mask)
  // TODO Convert 'parent' to 'template'.
  // TODO Convert 'imask' to 'index'.
  // TODO Add 'rtype:periodic'.
  return upgraded;
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the due date for a new instance N.
static Datetime generateNextDueDate (
  const Datetime& first,
  const std::string& period,
  const int n)
{
  auto y  = first.year ();
  auto m  = first.month ();
  auto d  = first.day ();
  auto hh = first.hour ();
  auto mm = first.minute ();
  auto ss = first.second ();

  Duration dur (period);
  auto normalized = dur.formatISO ();
  context.debug ("      period " + period + " --> " + normalized);

  if (! dur._year    &&
        dur._month   &&
      ! dur._weeks   &&
      ! dur._day     &&
      ! dur._hours   &&
      ! dur._minutes &&
      ! dur._seconds)
  {
    m += dur._month * n;
    while (m > 12)
    {
      y += 1;
      m -= 12;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, hh, mm, ss);
  }

/*
  // Some periods are difficult, because they can be vague.
  if (period == "monthly" ||
      period == "P1M")
  {
    if (++m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (period == "weekdays")
  {
    auto dow = current.dayOfWeek ();
    int days;

         if (dow == 5) days = 3;
    else if (dow == 6) days = 2;
    else               days = 1;

    return current + (days * 86400);
  }

  else if (Lexer::isDigit (period[0]) &&
           period[period.length () - 1] == 'm')
  {
    int increment = strtol (period.substr (0, period.length () - 1).c_str (), NULL, 10);

    m += increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (period[0] == 'P'                                            &&
           Lexer::isAllDigits (period.substr (1, period.length () - 2)) &&
           period[period.length () - 1] == 'M')
  {
    int increment = strtol (period.substr (0, period.length () - 1).c_str (), NULL, 10);

    m += increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d);
  }

  else if (period == "quarterly" ||
           period == "P3M")
  {
    m += 3;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (Lexer::isDigit (period[0]) && period[period.length () - 1] == 'q')
  {
    int increment = strtol (period.substr (0, period.length () - 1).c_str (), NULL, 10);

    m += 3 * increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (period == "semiannual" ||
           period == "P6M")
  {
    m += 6;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (period == "bimonthly" ||
           period == "P2M")
  {
    m += 2;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Datetime::valid (y, m, d))
      --d;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (period == "biannual" ||
           period == "biyearly" ||
           period == "P2Y")
  {
    y += 2;

    return Datetime (y, m, d, ho, mi, se);
  }

  else if (period == "annual" ||
           period == "yearly" ||
           period == "P1Y")
  {
    y += 1;

    // If the due data just happens to be 2/29 in a leap year, then simply
    // incrementing y is going to create an invalid date.
    if (m == 2 && d == 29)
      d = 28;

    return Datetime (y, m, d, ho, mi, se);
  }

  // Add the period to current, and we're done.
  std::string::size_type idx = 0;
  Duration p;
  if (! p.parse (period, idx))
    throw std::string (format (STRING_TASK_VALID_RECUR, period));

  return current + p.toTime_t ();
*/

  Datetime due;
  return due;
}

////////////////////////////////////////////////////////////////////////////////
static void synthesizeTasks (const Task&)
{
  context.debug ("synthesizeTasks start");

  // TODO 'due' = starting point
  // TODO 'recur' = frequency
  // TODO 'last' = index of most recently synthesized instance

  context.debug ("synthesizeTasks end");
}

////////////////////////////////////////////////////////////////////////////////
// Generates all necessary recurring task instances.
void handleRecurrence2 ()
{
  // Note: Disabling recurrence is currently a workaround for TD-44, TW-1520.
  if (context.config.getBoolean ("recurrence"))
    for (auto& t : context.tdb2.pending.get_tasks ())
      if (t.getStatus () == Task::recurring)
        synthesizeTasks (t);
}

////////////////////////////////////////////////////////////////////////////////
// Delete expired tasks.
void handleUntil ()
{
  Datetime now;
  auto tasks = context.tdb2.pending.get_tasks ();
  for (auto& t : tasks)
  {
    if (t.getStatus () == Task::pending &&
        t.has ("until")                 &&
        Datetime (t.get_date ("until")) < now)
    {
      t.setStatus (Task::deleted);
      context.tdb2.modify(t);
      context.footnote (onExpiration (t));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
