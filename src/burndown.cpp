////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

#include <iostream>  // TODO Remove
//#include <iomanip>
#include <sstream>
//#include <fstream>
#include <algorithm>
//#include <sys/types.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <pwd.h>
//#include <time.h>
#include <math.h>

#include "Context.h"
//#include "Directory.h"
//#include "File.h"
#include "Date.h"
//#include "Duration.h"
//#include "Table.h"
#include "text.h"
#include "util.h"
#include "main.h"

//#ifdef HAVE_LIBNCURSES
//#include <ncurses.h>
//#endif

extern Context context;

// Helper macro.
#define LOC(y,x) (((y) * (width + 1)) + (x))

////////////////////////////////////////////////////////////////////////////////
// Given the vertical chart area size (height), the largest value (value),
// populate a vector of labels for the y axis.
void calculateYAxis (std::vector <int>& labels, int height, int value)
{
/*
  double logarithm = log10 ((double) value);

  int exponent = (int) logarithm;
  logarithm -= exponent;

  int divisions = 10;
  double localMaximum = pow (10.0, exponent + 1);
  bool repeat = true;

  do
  {
    repeat = false;
    double scale = pow (10.0, exponent);

    while (value < localMaximum - scale)
    {
      localMaximum -= scale;
      --divisions;
    }

    if (divisions < 3 && exponent > 1)
    {
      divisions *= 10;
      --exponent;
      repeat = true;
    }
  }
  while (repeat);

  int division_size = localMaximum / divisions;
  for (int i = 0; i <= divisions; ++i)
    labels.push_back (i * division_size);
*/

  // For now, simply select 0, n/2 and n, where n is value rounded up to the
  // nearest 10.
  int high = value;
  int mod = high % 10;
  if (mod)
    high += 10 - mod;

  int half = high / 2;

  labels.push_back (0);
  labels.push_back (half);
  labels.push_back (high);
}

////////////////////////////////////////////////////////////////////////////////
// Graph should render like this:
//   +---------------------------------------------------------------------+
//   |                                                                     |
//   | 20 |                                                                |
//   |    |                            dd dd dd dd dd dd dd dd             |
//   |    |          dd dd dd dd dd dd dd dd dd dd dd dd dd dd             |
//   |    | pp pp ss ss ss ss ss ss ss ss ss dd dd dd dd dd dd   dd Done   |
//   | 10 | pp pp pp pp pp pp ss ss ss ss ss ss dd dd dd dd dd   ss Started|
//   |    | pp pp pp pp pp pp pp pp pp pp pp ss ss ss ss dd dd   pp Pending|
//   |    | pp pp pp pp pp pp pp pp pp pp pp pp pp pp pp ss dd             |
//   |    | pp pp pp pp pp pp pp pp pp pp pp pp pp pp pp pp pp             |
//   |  0 +----------------------------------------------------            |
//   |      21 22 23 24 25 26 27 28 29 30 31 01 02 03 04 05 06             |
//   |      July                             August                        |
//   |                                                                     |
//   |      Find rate 1.7/d      Estimated completion 8/12/2010            |
//   |      Fix rate  1.3/d                                                |
//   +---------------------------------------------------------------------+
//
//   e = entry
//   s = start
//   C = end/Completed
//   D = end/Deleted
//   > = Pending/Waiting
//
//   ID  30 31 01 02 03 04 05 06 07 08 09 10
//   --  ------------------------------------
//   1          e-----s--C
//   2             e--s-----D
//   3                e-----s-------------->
//   4                   e----------------->
//   5                               e----->
//   --  ------------------------------------
//   pp         1  2  3  3  2  2  2  3  3  3
//   ss               2  1  1  1  1  1  1  1
//   dd                  1  1  1  1  1  1  1
//   --  ------------------------------------
//
//   5 |             ss dd          dd dd dd 
//   4 |             ss ss dd dd dd ss ss ss 
//   3 |             pp pp ss ss ss pp pp pp 
//   2 |          pp pp pp pp pp pp pp pp pp 
//   1 |       pp pp pp pp pp pp pp pp pp pp 
//   0 +-------------------------------------
//       30 31 01 02 03 04 05 06 07 08 09 10
//       Oct   Nov
//
int handleReportBurndownDaily (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-burndown-command"))
  {
    std::map <time_t, int> groups;
    std::map <time_t, int> pendingGroup;
    std::map <time_t, int> startedGroup;
    std::map <time_t, int> doneGroup;

    std::map <time_t, int> addGroup;
    std::map <time_t, int> removeGroup;

    // Scan the pending tasks, applying any filter.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    // How much space is there to render in?  This chart will occupy the
    // maximum space, and the width drives various other parameters.
    int width  = context.getWidth ();
    int height = context.getHeight () - 1;    // Allow for new line with prompt.

    // Estimate how many 'bars' can be dsplayed.  This will help subset a
    // potentially enormous data set.
    unsigned int estimate = (width - 1 - 14) / 3;
    Date now;
    Date cutoff = (now - (estimate * 86400)).startOfDay ();
//    std::cout << "# cutoff " << cutoff.toString () << "\n";
//    std::cout << "# now " << now.toString () << "\n";

    time_t epoch;
    foreach (task, tasks)
    {
      // The entry date is when the counting starts.
      Date from = Date (task->get ("entry")).startOfDay ();
      addGroup[from.toEpoch ()]++;

      // e-->   e--s-->
      // ppp>   pppsss>
      Task::status status = task->getStatus ();
      if (status == Task::pending ||
          status == Task::waiting)
      {
        if (task->has ("start"))
        {
          Date start = Date (task->get ("start")).startOfDay ();
          while (from < start)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++pendingGroup[epoch];
            from++;
          }

          while (from < now)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++startedGroup[epoch];
            from++;
          }
        }
        else
        {
          while (from < now)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++pendingGroup[epoch];
            from++;
          }
        }
      }

      // e--C   e--s--C
      // pppd>  pppsssd>
      else if (status == Task::completed)
      {
        // Truncate history so it starts at 'cutoff' for completed tasks.
        Date end = Date (task->get ("end")).startOfDay ();
        removeGroup[end.toEpoch ()]++;

        if (end < cutoff)
        {
          ++doneGroup[cutoff.toEpoch ()];
          continue;
        }

        if (task->has ("start"))
        {
          Date start = Date (task->get ("start")).startOfDay ();
          while (from < start)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++pendingGroup[epoch];
            from++;
          }

          while (from < end)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++startedGroup[epoch];
            from++;
          }

          while (from < now)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++doneGroup[epoch];
            from++;
          }
        }
        else
        {
          Date end = Date (task->get ("end")).startOfDay ();
          while (from < end)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++pendingGroup[epoch];
            from++;
          }

          while (from < now)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++doneGroup[epoch];
            from++;
          }
        }
      }

      // e--D   e--s--D
      // ppp    pppsss
      else if (status == Task::deleted)
      {
        // Skip old deleted tasks.
        Date end = Date (task->get ("end")).startOfDay ();
        removeGroup[end.toEpoch ()]++;

        if (end < cutoff)
          continue;

        if (task->has ("start"))
        {
          Date start = Date (task->get ("start")).startOfDay ();
          while (from < start)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++pendingGroup[epoch];
            from++;
          }

          while (from < end)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++startedGroup[epoch];
            from++;
          }
        }
        else
        {
          Date end = Date (task->get ("end")).startOfDay ();
          while (from < end)
          {
            epoch = from.startOfDay ().toEpoch ();
            groups[epoch] = 0;
            ++pendingGroup[epoch];
            from++;
          }
        }
      }
    }

/*
    // TODO Render.
    foreach (g, groups)
    {
      std::stringstream s;
      s << Date (g->first).toISO () << " "
        << pendingGroup[g->first] << "/"
        << startedGroup[g->first] << "/"
        << doneGroup[g->first] << "\n";
      outs += s.str ();
    }
*/

    if (groups.size ())
    {
      // Horizontal Breakdown
      //
      //   >25 | xx ... xx  ll pending<
      //   ^                            left margin
      //    ^^                          max_label
      //      ^                         gap
      //       ^                        axis
      //        ^^^                     gap + bar
      //               ^^^              gap + bar
      //                  ^^            gap
      //                    ^^          legend swatch
      //                      ^         gap
      //                       ^^^^^^^  "Pending", "Started" & "Done"
      //                              ^ right margin

      // Vertical Breakdown
      //
      //   v   top margin
      //       blank line
      //   |   all remaining space
      //   -   x axis
      //   9   day/week/month
      //   9   month/-/year
      //       blank line
      //   f   find rate
      //   f   fix rate
      //   ^   bottom margin

      // What is the longest y-axis label?  This is tricky.  Having
      // optimistically estimate the number of bars to be shown, then determine
      // the longest label of the records that lie within the observable range.
      // It is important to consider that there may be zero -> bars number of
      // records that match.
      int max_label = 1;
      int max_value = 0;

      std::vector <time_t> x_axis;
      Date now;
      for (unsigned int i = 0; i < estimate; ++i)
      {
        Date x = (now - (i * 86400)).startOfDay ();
        x_axis.push_back (x.toEpoch ());

        int total = pendingGroup[x.toEpoch ()] +
                    startedGroup[x.toEpoch ()] +
                    doneGroup[x.toEpoch ()];

        if (total > max_value)
          max_value = total;

        int length = (int) log10 ((double) total) + 1;
        if (length > max_label)
          max_label = length;
      }

      std::sort (x_axis.begin (), x_axis.end ());

      // How many bars can be shown?
      unsigned int bars = (width - max_label - 14) / 3;
      int graph_width = width - max_label - 14;

      // Make them match
      while (bars < x_axis.size ())
        x_axis.erase (x_axis.begin ());

      // Determine the y-axis.
      int graph_height = height - 7;

      if (graph_height < 5 ||
          graph_width < 4)
      {
        outs = "Terminal window too small to draw a graph.\n";
        return rc;
      }

      // Determine y-axis labelling.
      std::vector <int> labels;
      calculateYAxis (labels, graph_height, max_value);
//      foreach (i, labels)
//        std::cout << "#   label " << *i << "\n";

//      std::cout << "# estimate "     << estimate       << " bars\n";
//      std::cout << "# actually "     << bars           << " bars\n";
//      std::cout << "# graph width "  << graph_width    << "\n";
//      std::cout << "# graph height " << graph_height   << "\n";
//      std::cout << "# days "         << x_axis.size () << "\n";
//      std::cout << "# max label "    << max_label      << "\n";
//      std::cout << "# max value "    << max_value      << "\n";

      // Determine the start date.
      Date start = (Date () - ((bars - 1) * 86400)).startOfDay ();
//      std::cout << "# start " << start.toISO () << "\n";

      // Compose the grid.
      std::string grid;
      for (int i = 0; i < height; ++i)
        grid += std::string (width, ' ') + "\n";

      // Draw legend.
      grid.replace (LOC (graph_height / 2 - 1, width - 10), 10, "dd Done   ");
      grid.replace (LOC (graph_height / 2,     width - 10), 10, "ss Started");
      grid.replace (LOC (graph_height / 2 + 1, width - 10), 10, "pp Pending");

      // Draw x-axis.
      grid.replace (LOC (height - 6, max_label + 1), 1, "+");
      grid.replace (LOC (height - 6, max_label + 2), graph_width, std::string (graph_width, '-'));

      int month = 0;
      Date d (start);
      for (unsigned int i = 0; i < bars; ++i)
      {
        if (month != d.month ())
          grid.replace (LOC (height - 4, max_label + 3 + (i * 3)), 3, Date::monthName (d.month ()).substr (0, 3));

        char day [3];
        sprintf (day, "%02d", d.day ());
        grid.replace (LOC (height - 5, max_label + 3 + (i * 3)), 2, day);

        month = d.month ();
        d++;
      }

      // Draw the y-axis.
      for (int i = 0; i < graph_height; ++i)
        grid.replace (LOC (i + 1, max_label + 1), 1, "|");

      char label [12];
      sprintf (label, "%*d", max_label, labels[2]);
      grid.replace (LOC (1,                      max_label - strlen (label)), strlen (label), label);
      sprintf (label, "%*d", max_label, labels[1]);
      grid.replace (LOC (1 + (graph_height / 2), max_label - strlen (label)), strlen (label), label);
      grid.replace (LOC (graph_height + 1,       max_label - 1),              1,              "0");

      // Draw the bars.
      int last_pending = 0;
      d = start;
      for (unsigned int i = 0; i < bars; ++i)
      {
        epoch = d.toEpoch ();
        int pending = (pendingGroup[epoch] * graph_height) / labels[2];
        int started = (startedGroup[epoch] * graph_height) / labels[2];
        int done    = (doneGroup[epoch]    * graph_height) / labels[2];

        // Track the latest pending count, for convergence calculation.
        last_pending = pendingGroup[epoch] + startedGroup[epoch];

        for (int b = 0; b < pending; ++b)
          grid.replace (LOC (graph_height - b, max_label + 3 + (i * 3)), 2, "pp");

        for (int b = 0; b < started; ++b)
          grid.replace (LOC (graph_height - b - pending, max_label + 3 + (i * 3)), 2, "ss");

        for (int b = 0; b < done; ++b)
          grid.replace (LOC (graph_height - b - pending - started, max_label + 3 + (i * 3)), 2, "dd");

        d++;
      }

//      std::cout << "# last pending " << last_pending << "\n";

      // Calculate and render the rates.
      // Calculate 30-day average.
      int totalAdded30 = 0;
      int totalRemoved30 = 0;
      d = (Date () - 30 * 86400).startOfDay ();
      for (unsigned int i = 0; i < 30; i++)
      {
        epoch = d.toEpoch ();

        totalAdded30   += addGroup[epoch];
        totalRemoved30 += removeGroup[epoch];

        d++;
      }

      float find_rate30 = 1.0 * totalAdded30   / x_axis.size ();
      float fix_rate30  = 1.0 * totalRemoved30 / x_axis.size ();

      // Calculate 7-day average.
      int totalAdded7 = 0;
      int totalRemoved7 = 0;
      d = (Date () - 7 * 86400).startOfDay ();
      for (unsigned int i = 0; i < 7; i++)
      {
        epoch = d.toEpoch ();

        totalAdded7   += addGroup[epoch];
        totalRemoved7 += removeGroup[epoch];

        d++;
      }

      float find_rate7 = 1.0 * totalAdded7   / x_axis.size ();
      float fix_rate7  = 1.0 * totalRemoved7 / x_axis.size ();

      // Render rates.
      char rate[12];
      sprintf (rate, "%.1f", (find_rate30 + find_rate7) / 2.0);
      grid.replace (LOC (height - 2, max_label + 3), 13 + strlen (rate), std::string ("Find rate: ") + rate + "/d");

      sprintf (rate, "%.1f", (fix_rate30 + fix_rate7) / 2.0);
      grid.replace (LOC (height - 1, max_label + 3), 13 + strlen (rate), std::string ("Fix rate:  ") + rate + "/d");

      if (last_pending == 0)
      {
        ; // Do not render an estimated completion date.
      }
      else if (find_rate7 < fix_rate7)
      {
        int current_pending = pendingGroup[Date ().startOfDay ().toEpoch ()];
        float days = 2.0 * current_pending / (fix_rate30 + fix_rate7);
        Date end;
        end += (int) (days * 86400);
        std::string formatted = end.toString (context.config.get ("dateformat"));
        grid.replace (LOC (height - 2, max_label + 27), 22 + formatted.length (), "Estimated completion: " + formatted);
      }
      else
      {
        grid.replace (LOC (height - 2, max_label + 27), 36, "Estimated completion: No convergence");
      }

      // Output the grid.
      Color color_pending (context.config.get ("color.burndown.pending"));
      Color color_done    (context.config.get ("color.burndown.done"));
      Color color_started (context.config.get ("color.burndown.started"));

      // Replace dd, ss, pp with colored strings.
      // TODO Use configurable values.
      std::string::size_type i;
      while ((i = grid.find ("pp")) != std::string::npos)
        grid.replace (i, 2, color_pending.colorize ("  "));

      while ((i = grid.find ("ss")) != std::string::npos)
        grid.replace (i, 2, color_started.colorize ("  "));

      while ((i = grid.find ("dd")) != std::string::npos)
        grid.replace (i, 2, color_done.colorize ("  "));

      outs += grid;

      context.hooks.trigger ("post-burndown-command");
    }
    else
      outs = "No matches.\n";
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportBurndownWeekly (std::string& outs)
{
  int rc = 0;

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleReportBurndownMonthly (std::string& outs)
{
  int rc = 0;

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
