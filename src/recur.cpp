////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////
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

#include "Context.h"
#include "Date.h"
#include "Duration.h"
#include "text.h"
#include "util.h"
#include "main.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

// Global context for use by all.
extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Scans all tasks, and for any recurring tasks, determines whether any new
// child tasks need to be generated to fill gaps.
void handleRecurrence (std::vector <Task>& tasks)
{
/*
  std::vector <T> modified;

  // Look at all tasks and find any recurring ones.
  foreach (t, tasks)
  {
    if (t->getStatus () == T::recurring)
    {
      // Generate a list of due dates for this recurring task, regardless of
      // the mask.
      std::vector <Date> due;
      if (!generateDueDates (*t, due))
      {
        std::cout << "Task "
                  << t->getUUID ()
                  << " ("
                  << trim (t->getDescription ())
                  << ") is past its 'until' date, and has be deleted" << std::endl;

        // Determine the end date.
        char endTime[16];
        sprintf (endTime, "%u", (unsigned int) time (NULL));
        t->setAttribute ("end", endTime);
        t->setStatus (T::deleted);
        tdb.modifyT (*t);
        continue;
      }

      // Get the mask from the parent task.
      std::string mask = t->getAttribute ("mask");

      // Iterate over the due dates, and check each against the mask.
      bool changed = false;
      unsigned int i = 0;
      foreach (d, due)
      {
        if (mask.length () <= i)
        {
          mask += '-';
          changed = true;

          T rec (*t);                                 // Clone the parent.
          rec.setId (tdb.nextId ());                  // Assign a unique id.
          rec.setUUID (uuid ());                      // New UUID.
          rec.setStatus (T::pending);                 // Shiny.
          rec.setAttribute ("parent", t->getUUID ()); // Remember mom.

          char dueDate[16];
          sprintf (dueDate, "%u", (unsigned int) d->toEpoch ());
          rec.setAttribute ("due", dueDate);          // Store generated due date.

          char indexMask[12];
          sprintf (indexMask, "%u", (unsigned int) i);
          rec.setAttribute ("imask", indexMask);      // Store index into mask.

          // Add the new task to the vector, for immediate use.
          modified.push_back (rec);

          // Add the new task to the DB.
          tdb.addT (rec);
        }

        ++i;
      }

      // Only modify the parent if necessary.
      if (changed)
      {
        t->setAttribute ("mask", mask);
        tdb.modifyT (*t);
      }
    }
    else
      modified.push_back (*t);
  }

  tasks = modified;
*/
}

////////////////////////////////////////////////////////////////////////////////
// Determine a start date (due), an optional end date (until), and an increment
// period (recur).  Then generate a set of corresponding dates.
//
// Returns false if the parent recurring task is depleted.
bool generateDueDates (Task& parent, std::vector <Date>& allDue)
{
  // Determine due date, recur period and until date.
  Date due (atoi (parent.get ("due").c_str ()));
  std::string recur = parent.get ("recur");

  bool specificEnd = false;
  Date until;
  if (parent.get ("until") != "")
  {
    until = Date (atoi (parent.get ("until").c_str ()));
    specificEnd = true;
  }

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

  if (period == "weekdays")
  {
    int dow = current.dayOfWeek ();
    int days;

         if (dow == 5) days = 3;
    else if (dow == 6) days = 2;
    else               days = 1;

    return current + (days * 86400);
  }

  if (isdigit (period[0]) && period[period.length () - 1] == 'm')
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
  int days = 0;
  try { Duration du (period); days = du; }
  catch (...) { days = 0; }

  return current + (days * 86400);
}

////////////////////////////////////////////////////////////////////////////////
// When the status of a recurring child task changes, the parent task must
// update it's mask.
void updateRecurrenceMask (
//  TDB& tdb,
  std::vector <Task>& all,
  Task& task)
{
/*
  std::string parent = task.getAttribute ("parent");
  if (parent != "")
  {
    std::vector <T>::iterator it;
    for (it = all.begin (); it != all.end (); ++it)
    {
      if (it->getUUID () == parent)
      {
        unsigned int index = atoi (task.getAttribute ("imask").c_str ());
        std::string mask = it->getAttribute ("mask");
        if (mask.length () > index)
        {
          mask[index] = (task.getStatus () == T::pending)   ? '-'
                      : (task.getStatus () == T::completed) ? '+'
                      : (task.getStatus () == T::deleted)   ? 'X'
                      :                                       '?';

          it->setAttribute ("mask", mask);
          tdb.modifyT (*it);
        }
        else
        {
          std::string mask;
          for (unsigned int i = 0; i < index; ++i)
            mask += "?";

          mask += (task.getStatus () == T::pending)   ? '-'
                : (task.getStatus () == T::completed) ? '+'
                : (task.getStatus () == T::deleted)   ? 'X'
                :                                       '?';
        }

        return;  // No point continuing the loop.
      }
    }
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
// Determines whether a task is overdue.  Returns
//   0 = not due at all
//   1 = imminent
//   2 = overdue
int getDueState (const std::string& due)
{
  if (due.length ())
  {
    Date dt (::atoi (due.c_str ()));

    // rightNow is the current date + time.
    Date rightNow;
    Date midnight (rightNow.month (), rightNow.day (), rightNow.year ());

    if (dt < midnight)
      return 2;

    Date nextweek = midnight + context.config.get ("due", 7) * 86400;
    if (dt < nextweek)
      return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void nag (/*TDB& tdb,*/ Task& task)
{
/*
  std::string nagMessage = context.config.get ("nag", std::string (""));
  if (nagMessage != "")
  {
    // Load all pending tasks.
    std::vector <T> pending;
    tdb.allPendingT (pending);

    // Counters.
    int overdue    = 0;
    int high       = 0;
    int medium     = 0;
    int low        = 0;
    bool isOverdue = false;
    char pri       = ' ';

    // Scan all pending tasks.
    foreach (t, pending)
    {
      if (t->getId () == task.getId ())
      {
        if (getDueState (t->getAttribute ("due")) == 2)
          isOverdue = true;

        std::string priority = t->getAttribute ("priority");
        if (priority.length ())
          pri = priority[0];
      }
      else if (t->getStatus () == T::pending)
      {
        if (getDueState (t->getAttribute ("due")) == 2)
          overdue++;

        std::string priority = t->getAttribute ("priority");
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
    if (isOverdue                                         ) return;
    if (pri == 'H' && !overdue                            ) return;
    if (pri == 'M' && !overdue && !high                   ) return;
    if (pri == 'L' && !overdue && !high && !medium        ) return;
    if (pri == ' ' && !overdue && !high && !medium && !low) return;

    // All the excuses are made, all that remains is to nag the user.
    std::cout << nagMessage << std::endl;
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
