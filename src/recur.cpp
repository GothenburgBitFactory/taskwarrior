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
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include <Context.h>
#include <Lexer.h>
#include <ISO8601.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

// Global context for use by all.
extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Scans all tasks, and for any recurring tasks, determines whether any new
// child tasks need to be generated to fill gaps.
void handleRecurrence ()
{
  // Recurrence can be disabled.
  // Note: This is currently a workaround for TD-44, TW-1520.
  if (! context.config.getBoolean ("recurrence"))
    return;

  auto tasks = context.tdb2.pending.get_tasks ();
  ISO8601d now;

  // Look at all tasks and find any recurring ones.
  for (auto& t : tasks)
  {
    if (t.getStatus () == Task::recurring)
    {
      // Generate a list of due dates for this recurring task, regardless of
      // the mask.
      std::vector <ISO8601d> due;
      if (!generateDueDates (t, due))
      {
        // Determine the end date.
        t.setStatus (Task::deleted);
        context.tdb2.modify (t);
        context.footnote (onExpiration (t));
        continue;
      }

      // Get the mask from the parent task.
      std::string mask = t.get ("mask");

      // Iterate over the due dates, and check each against the mask.
      bool changed = false;
      unsigned int i = 0;
      for (auto& d : due)
      {
        if (mask.length () <= i)
        {
          changed = true;

          Task rec (t);                          // Clone the parent.
          rec.setStatus (Task::pending);         // Change the status.
          rec.id = context.tdb2.next_id ();      // New ID.
          rec.set ("uuid", uuid ());             // New UUID.
          rec.set ("parent", t.get ("uuid"));    // Remember mom.
          rec.setAsNow ("entry");                // New entry date.

          char dueDate[16];
          sprintf (dueDate, "%u", (unsigned int) d.toEpoch ());
          rec.set ("due", dueDate);              // Store generated due date.

          if (t.has ("wait"))
          {
            ISO8601d old_wait (t.get_date ("wait"));
            ISO8601d old_due (t.get_date ("due"));
            ISO8601d due (d);
            sprintf (dueDate, "%u", (unsigned int) (due + (old_wait - old_due)).toEpoch ());
            rec.set ("wait", dueDate);
            rec.setStatus (Task::waiting);
            mask += 'W';
          }
          else
          {
            mask += '-';
            rec.setStatus (Task::pending);
          }

          char indexMask[12];
          sprintf (indexMask, "%u", (unsigned int) i);
          rec.set ("imask", indexMask);          // Store index into mask.

          rec.remove ("mask");                   // Remove the mask of the parent.

          // Add the new task to the DB.
          context.tdb2.add (rec);
        }

        ++i;
      }

      // Only modify the parent if necessary.
      if (changed)
      {
        t.set ("mask", mask);
        context.tdb2.modify (t);

        if (context.verbose ("recur"))
          context.footnote (format (STRING_RECUR_CREATE, t.get ("description")));
      }
    }

    // Non-recurring tasks expire too.
    else
    {
      if (t.has ("until") &&
          ISO8601d (t.get_date ("until")) < now)
      {
        t.setStatus (Task::deleted);
        context.tdb2.modify(t);
        context.footnote (onExpiration (t));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Determine a start date (due), an optional end date (until), and an increment
// period (recur).  Then generate a set of corresponding dates.
//
// Returns false if the parent recurring task is depleted.
bool generateDueDates (Task& parent, std::vector <ISO8601d>& allDue)
{
  // Determine due date, recur period and until date.
  ISO8601d due (parent.get_date ("due"));
  if (due._date == 0)
    return false;

  std::string recur = parent.get ("recur");

  bool specificEnd = false;
  ISO8601d until;
  if (parent.get ("until") != "")
  {
    until = ISO8601d (parent.get ("until"));
    specificEnd = true;
  }

  int recurrence_limit = context.config.getInteger ("recurrence.limit");
  int recurrence_counter = 0;
  ISO8601d now;
  for (ISO8601d i = due; ; i = getNextRecurrence (i, recur))
  {
    allDue.push_back (i);

    if (specificEnd && i > until)
    {
      // If i > until, it means there are no more tasks to generate, and if the
      // parent mask contains all + or X, then there never will be another task
      // to generate, and this parent task may be safely reaped.
      std::string mask = parent.get ("mask");
      if (mask.length () == allDue.size () &&
          mask.find ('-') == std::string::npos)
        return false;

      return true;
    }

    if (i > now)
      ++recurrence_counter;

    if (recurrence_counter >= recurrence_limit)
      return true;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d getNextRecurrence (ISO8601d& current, std::string& period)
{
  int m = current.month ();
  int d = current.day ();
  int y = current.year ();
  int ho = current.hour ();
  int mi = current.minute ();
  int se = current.second ();

  // Some periods are difficult, because they can be vague.
  if (period == "monthly" ||
      period == "P1M")
  {
    if (++m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y, ho, mi, se);
  }

  else if (period == "weekdays")
  {
    int dow = current.dayOfWeek ();
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

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y, ho, mi, se);
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

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y);
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

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y, ho, mi, se);
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

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y, ho, mi, se);
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

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y, ho, mi, se);
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

    while (! ISO8601d::valid (m, d, y))
      --d;

    return ISO8601d (m, d, y, ho, mi, se);
  }

  else if (period == "biannual" ||
           period == "biyearly" ||
           period == "P2Y")
  {
    y += 2;

    return ISO8601d (m, d, y, ho, mi, se);
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

    return ISO8601d (m, d, y, ho, mi, se);
  }

  // Add the period to current, and we're done.
  int secs = 0;
  std::string::size_type idx = 0;
  ISO8601p p;
  if (! p.parse (period, idx))
    throw std::string (format (STRING_TASK_VALID_RECUR, period));

  secs = (time_t) p;
  return current + secs;
}

////////////////////////////////////////////////////////////////////////////////
// When the status of a recurring child task changes, the parent task must
// update it's mask.
void updateRecurrenceMask (Task& task)
{
  std::string uuid = task.get ("parent");
  Task parent;

  if (uuid != "" &&
      context.tdb2.get (uuid, parent))
  {
    unsigned int index = strtol (task.get ("imask").c_str (), NULL, 10);
    std::string mask = parent.get ("mask");
    if (mask.length () > index)
    {
      mask[index] = (task.getStatus () == Task::pending)   ? '-'
                  : (task.getStatus () == Task::completed) ? '+'
                  : (task.getStatus () == Task::deleted)   ? 'X'
                  : (task.getStatus () == Task::waiting)   ? 'W'
                  :                                          '?';
    }
    else
    {
      std::string mask;
      for (unsigned int i = 0; i < index; ++i)
        mask += "?";

      mask += (task.getStatus () == Task::pending)   ? '-'
            : (task.getStatus () == Task::completed) ? '+'
            : (task.getStatus () == Task::deleted)   ? 'X'
            : (task.getStatus () == Task::waiting)   ? 'W'
            :                                          '?';
    }

    parent.set ("mask", mask);
    context.tdb2.modify (parent);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Returns a Boolean indicator as to whether a nag message was generated, so
// that commands can control the number of nag messages displayed (ie one is
// enough).
//
// Otherwise generates a nag message, if one is defined, if there are tasks of
// higher urgency.
bool nag (Task& task)
{
  // Special tag overrides nagging.
  if (task.hasTag ("nonag"))
    return false;

  std::string nagMessage = context.config.get ("nag");
  if (nagMessage != "")
  {
    // Scan all pending, non-recurring tasks.
    auto pending = context.tdb2.pending.get_tasks ();
    for (auto& t : pending)
    {
      if ((t.getStatus () == Task::pending ||
           t.getStatus () == Task::waiting) &&
          t.urgency () > task.urgency ())
      {
        context.footnote (nagMessage);
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
