////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include <Context.h>
#include <Date.h>
#include <Duration.h>
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
  std::vector <Task> tasks = context.tdb2.pending.get_tasks ();
  Date now;

  // Look at all tasks and find any recurring ones.
  std::vector <Task>::iterator t;
  for (t = tasks.begin (); t != tasks.end (); ++t)
  {
    if (t->getStatus () == Task::recurring)
    {
      // Generate a list of due dates for this recurring task, regardless of
      // the mask.
      std::vector <Date> due;
      if (!generateDueDates (*t, due))
      {
        // Determine the end date.
        t->setStatus (Task::deleted);
        context.tdb2.modify (*t);
        context.footnote (onExpiration (*t));
        continue;
      }

      // Get the mask from the parent task.
      std::string mask = t->get ("mask");

      // Iterate over the due dates, and check each against the mask.
      bool changed = false;
      unsigned int i = 0;
      std::vector <Date>::iterator d;
      for (d = due.begin (); d != due.end (); ++d)
      {
        if (mask.length () <= i)
        {
          changed = true;

          Task rec (*t);                         // Clone the parent.
          rec.setStatus (Task::pending);         // Change the status.
          rec.id = context.tdb2.next_id ();      // New ID.
          rec.set ("uuid", uuid ());             // New UUID.
          rec.set ("parent", t->get ("uuid"));   // Remember mom.
          rec.setEntry ();                       // New entry date.

          char dueDate[16];
          sprintf (dueDate, "%u", (unsigned int) d->toEpoch ());
          rec.set ("due", dueDate);              // Store generated due date.

          if (t->has ("wait"))
          {
            Date old_wait (t->get_date ("wait"));
            Date old_due (t->get_date ("due"));
            Date due (*d);
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
        t->set ("mask", mask);
        context.tdb2.modify (*t);
      }
    }

    // Non-recurring tasks expire too.
    else
    {
      if (t->has ("until") &&
          Date (t->get_date ("until")) < now)
      {
        t->setStatus (Task::deleted);
        context.tdb2.modify(*t);
        context.footnote (onExpiration (*t));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Determine a start date (due), an optional end date (until), and an increment
// period (recur).  Then generate a set of corresponding dates.
//
// Returns false if the parent recurring task is depleted.
bool generateDueDates (Task& parent, std::vector <Date>& allDue)
{
  // Determine due date, recur period and until date.
  Date due (parent.get_date ("due"));
  std::string recur = parent.get ("recur");

  bool specificEnd = false;
  Date until;
  if (parent.get ("until") != "")
  {
    until = Date (parent.get ("until"));
    specificEnd = true;
  }

  int recurrence_limit = context.config.getInteger ("recurrence.limit");
  int recurrence_counter = 0;
  Date now;
  for (Date i = due; ; i = getNextRecurrence (i, recur))
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
Date getNextRecurrence (Date& current, std::string& period)
{
  int m = current.month ();
  int d = current.day ();
  int y = current.year ();

  // Some periods are difficult, because they can be vague.
  if (period == "monthly")
  {
    if (++m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
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

  else if (isdigit (period[0]) && period[period.length () - 1] == 'm')
  {
    std::string numeric = period.substr (0, period.length () - 1);
    int increment = atoi (numeric.c_str ());

    m += increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "quarterly")
  {
    m += 3;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (isdigit (period[0]) && period[period.length () - 1] == 'q')
  {
    std::string numeric = period.substr (0, period.length () - 1);
    int increment = atoi (numeric.c_str ());

    m += 3 * increment;
    while (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "semiannual")
  {
    m += 6;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "bimonthly")
  {
    m += 2;
    if (m > 12)
    {
       m -= 12;
       ++y;
    }

    while (! Date::valid (m, d, y))
      --d;

    return Date (m, d, y);
  }

  else if (period == "biannual"    ||
           period == "biyearly")
  {
    y += 2;

    return Date (m, d, y);
  }

  else if (period == "annual" ||
           period == "yearly")
  {
    y += 1;

    // If the due data just happens to be 2/29 in a leap year, then simply
    // incrementing y is going to create an invalid date.
    if (m == 2 && d == 29)
      d = 28;

    return Date (m, d, y);
  }

  // If the period is an 'easy' one, add it to current, and we're done.
  // If it throws an error, the duration was not recognized.
  int secs = 0;
  Duration du (period);
  secs = du;

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
// Determines whether a task is overdue.  Returns
//   0 = not due at all
//   1 = imminent
//   2 = today
//   3 = overdue
int getDueState (const std::string& due)
{
  if (due.length ())
  {
    Date dt (::atoi (due.c_str ()));

    // rightNow is the current date + time.
    static Date rightNow;
    Date thisDay (rightNow.month (), rightNow.day (), rightNow.year ());

    if (dt < rightNow)
      return 3;

    if (rightNow.sameDay (dt))
      return 2;

    int imminentperiod = context.config.getInteger ("due");

    if (imminentperiod == 0)
      return 1;

    Date imminentDay = thisDay + imminentperiod * 86400;
    if (dt < imminentDay)
      return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Determines whether a task is overdue.  Returns
//   0 = not due at all
//   1 = imminent
//   2 = today
//   3 = overdue
int getDueState (const Date& due)
{
  // rightNow is the current date + time.
  static Date rightNow;
  Date thisDay (rightNow.month (), rightNow.day (), rightNow.year ());

  if (due < rightNow)
    return 3;

  if (rightNow.sameDay (due))
    return 2;

  int imminentperiod = context.config.getInteger ("due");

  if (imminentperiod == 0)
    return 1;

  Date imminentDay = thisDay + imminentperiod * 86400;
  if (due < imminentDay)
    return 1;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Returns a Boolean indicator as to whether a nag message was generated, so
// that commands can control the number of nag messages displayed (ie one is
// enough).
bool nag (Task& task)
{
  // Special tag overrides nagging.
  if (task.hasTag ("nonag"))
    return false;

  std::string nagMessage = context.config.get ("nag");
  if (nagMessage != "")
  {
    // Load all pending tasks.
    std::vector <Task> tasks = context.tdb2.pending.get_tasks ();

    // Counters.
    int overdue    = 0;
    int high       = 0;
    int medium     = 0;
    int low        = 0;
    bool isOverdue = false;
    char pri       = ' ';

    // Scan all pending tasks.
    std::vector <Task>::iterator t;
    for (t = tasks.begin (); t != tasks.end (); ++t)
    {
      if (t->id == task.id)
      {
        if (getDueState (t->get ("due")) == 3)
          isOverdue = true;

        std::string priority = t->get ("priority");
        if (priority.length ())
          pri = priority[0];
      }
      else if (t->getStatus () == Task::pending)
      {
        if (getDueState (t->get ("due")) == 3)
          overdue++;

        std::string priority = t->get ("priority");
        if (priority.length ())
        {
          switch (priority[0])
          {
          case 'H': high++;   break;
          case 'M': medium++; break;
          case 'L': low++;    break;
          }
        }
      }
    }

    // General form is "if there are no more deserving tasks", suppress the nag.
    if (isOverdue                                          ) return false;
    if (pri == 'H' && !overdue                             ) return false;
    if (pri == 'M' && !overdue && !high                    ) return false;
    if (pri == 'L' && !overdue && !high && !medium         ) return false;
    if (pri == ' ' && !overdue && !high && !medium && !low ) return false;
    if (task.is_blocking && !task.is_blocked               ) return false;

    // All the excuses are made, all that remains is to nag the user.
    context.footnote (nagMessage);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
