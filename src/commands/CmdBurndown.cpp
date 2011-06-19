////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#include <sstream>
#include <algorithm>
#include <string.h>
#include <math.h>
#include <Context.h>
#include <Date.h>
#include <Duration.h>
#include <main.h>
#include <CmdBurndown.h>

extern Context context;

// Helper macro.
#define LOC(y,x) (((y) * (width + 1)) + (x))

////////////////////////////////////////////////////////////////////////////////
class Bar
{
public:
  Bar ();
  Bar (const Bar&);
  Bar& operator= (const Bar&);
  ~Bar ();

public:
  int offset;                    // from left of chart
  std::string major_label;       // x-axis label, major (year/-/month)
  std::string minor_label;       // x-axis label, minor (month/week/day)
  int pending;                   // Number of pending tasks in period
  int started;                   // Number of started tasks in period
  int done;                      // Number of done tasks in period
  int added;                     // Number added in period
  int removed;                   // Number removed in period
};

////////////////////////////////////////////////////////////////////////////////
Bar::Bar ()
: offset (0)
, major_label ("")
, minor_label ("")
, pending (0)
, started (0)
, done (0)
, added (0)
, removed (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Bar::Bar (const Bar& other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////////
Bar& Bar::operator= (const Bar& other)
{
  if (this != &other)
  {
    offset      = other.offset;
    major_label = other.major_label;
    minor_label = other.minor_label;
    pending     = other.pending;
    started     = other.started;
    done        = other.done;
    added       = other.added;
    removed     = other.removed;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Bar::~Bar ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Data gathering algorithm:
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
class Chart
{
public:
  Chart (char);
  Chart (const Chart&);              // Unimplemented
  Chart& operator= (const Chart&);   // Unimplemented
  ~Chart ();

  void description (const std::string&);
  void scan (std::vector <Task>&);
  std::string render ();

private:
  void generateBars ();
  void optimizeGrid ();
  Date quantize (const Date&);

  Date increment (const Date&);
  Date decrement (const Date&);
  void maxima ();
  void yLabels (std::vector <int>&);
  void calculateRates (std::vector <time_t>&);

public:
  int width;                     // Terminal width
  int height;                    // Terminal height
  int graph_width;               // Width of plot area
  int graph_height;              // Height of plot area
  int max_value;                 // Largest combined bar value
  int max_label;                 // Longest y-axis label
  std::vector <int> labels;      // Y-axis labels
  int estimated_bars;            // Estimated bar count
  int actual_bars;               // Calculated bar count
  std::map <time_t, Bar> bars;   // Epoch-indexed set of bars
  Date earliest;                 // Date of earliest estimated bar
  int carryover_done;            // Number of 'done' tasks prior to chart range
  char period;                   // D, W, M
  std::string title;             // Additional description
  std::string grid;              // String representing grid of characters

  float find_rate;               // Calculated find rate
  float fix_rate;                // Calculated fix rate
  std::string completion;        // Estimated completion date
};

////////////////////////////////////////////////////////////////////////////////
Chart::Chart (char type)
{
  // How much space is there to render in?  This chart will occupy the
  // maximum space, and the width drives various other parameters.
  width = context.getWidth ();
  height = context.getHeight () - 1;  // Allow for new line with prompt.
  max_value = 0;
  max_label = 1;
  graph_height = height - 7;
  graph_width = width - max_label - 14;

  // Estimate how many 'bars' can be dsplayed.  This will help subset a
  // potentially enormous data set.
  estimated_bars = (width - 1 - 14) / 3;

  actual_bars = 0;
  period = type;
  carryover_done = 0;

  // Rates are calculated last.
  find_rate = 0.0;
  fix_rate = 0.0;
}

////////////////////////////////////////////////////////////////////////////////
Chart::~Chart ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Chart::description (const std::string& value)
{
  title = value;
}

////////////////////////////////////////////////////////////////////////////////
void Chart::scan (std::vector <Task>& tasks)
{
  generateBars ();

  // Not quantized, so that "while (xxx < now)" is inclusive.
  Date now;

  time_t epoch;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    // The entry date is when the counting starts.
    Date from = quantize (Date (task->get ("entry")));
    epoch = from.toEpoch ();

    if (bars.find (epoch) != bars.end ())
      ++bars[epoch].added;

    // e-->   e--s-->
    // ppp>   pppsss>
    Task::status status = task->getStatus ();
    if (status == Task::pending ||
        status == Task::waiting)
    {
      if (task->has ("start"))
      {
        Date start = quantize (Date (task->get ("start")));
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].pending;
          from = increment (from);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].started;
          from = increment (from);
        }
      }
      else
      {
        while (from < now)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].pending;
          from = increment (from);
        }
      }
    }

    // e--C   e--s--C
    // pppd>  pppsssd>
    else if (status == Task::completed)
    {
      // Truncate history so it starts at 'earliest' for completed tasks.
      Date end = quantize (Date (task->get ("end")));
      epoch = end.toEpoch ();

      if (bars.find (epoch) != bars.end ())
        ++bars[epoch].removed;

      // Maintain a running total of 'done' tasks that are off the left of the
      // chart.
      if (end < earliest)
      {
        ++carryover_done;
        continue;
      }

      if (task->has ("start"))
      {
        Date start = quantize (Date (task->get ("start")));
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].pending;
          from = increment (from);
        }

        while (from < end)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].started;
          from = increment (from);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].done;
          from = increment (from);
        }
      }
      else
      {
        Date end = quantize (Date (task->get ("end")));
        while (from < end)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].pending;
          from = increment (from);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].done;
          from = increment (from);
        }
      }
    }

    // e--D   e--s--D
    // ppp    pppsss
    else if (status == Task::deleted)
    {
      // Skip old deleted tasks.
      Date end = quantize (Date (task->get ("end")));
      epoch = end.toEpoch ();
      if (bars.find (epoch) != bars.end ())
        ++bars[epoch].removed;

      if (end < earliest)
        continue;

      if (task->has ("start"))
      {
        Date start = quantize (Date (task->get ("start")));
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].pending;
          from = increment (from);
        }

        while (from < end)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].started;
          from = increment (from);
        }
      }
      else
      {
        Date end = quantize (Date (task->get ("end")));
        while (from < end)
        {
          epoch = from.toEpoch ();
          if (bars.find (epoch) != bars.end ()) ++bars[epoch].pending;
          from = increment (from);
        }
      }
    }
  }

  // Size the data.
  maxima ();
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
//   |      Add rate 1.7/d           Estimated completion 8/12/2010        |
//   |      Don/Delete rate  1.3/d                                         |
//   +---------------------------------------------------------------------+
std::string Chart::render ()
{
  if (graph_height < 5 ||     // a 4-line graph is essentially unreadable.
      graph_width < 2)        // A single-bar graph is useless.
  {
    return "Terminal window too small to draw a graph.\n";
  }

  if (max_value == 0)
    return "No matches.\n";

  // Create a grid, folded into a string.
  grid = "";
  for (int i = 0; i < height; ++i)
    grid += std::string (width, ' ') + "\n";

  // Title.
  std::string full_title;
  switch (period)
  {
  case 'D': full_title = "Daily";   break;
  case 'W': full_title = "Weekly";  break;
  case 'M': full_title = "Monthly"; break;
  }

  full_title += " Burndown";

  if (title.length ())
  {
    if (full_title.length () + 1 + title.length () < (unsigned) width)
    {
      full_title += " " + title;
      grid.replace (LOC (0, (width - full_title.length ()) / 2), full_title.length (), full_title);
    }
    else
    {
      grid.replace (LOC (0, (width - full_title.length ()) / 2), full_title.length (), full_title);
      grid.replace (LOC (1, (width - title.length ()) / 2), title.length (), title);
    }
  }
  else
  {
    grid.replace (LOC (0, (width - full_title.length ()) / 2), full_title.length (), full_title);
  }

  // Legend.
  grid.replace (LOC (graph_height / 2 - 1, width - 10), 10, "dd Done   ");
  grid.replace (LOC (graph_height / 2,     width - 10), 10, "ss Started");
  grid.replace (LOC (graph_height / 2 + 1, width - 10), 10, "pp Pending");

  // Determine y-axis labelling.
  std::vector <int> labels;
  yLabels (labels);
  max_label = (int) log10 ((double) labels[2]) + 1;

  // Draw y-axis.
  for (int i = 0; i < graph_height; ++i)
     grid.replace (LOC (i + 1, max_label + 1), 1, "|");

  // Draw y-axis labels.
  char label [12];
  sprintf (label, "%*d", max_label, labels[2]);
  grid.replace (LOC (1,                      max_label - strlen (label)), strlen (label), label);
  sprintf (label, "%*d", max_label, labels[1]);
  grid.replace (LOC (1 + (graph_height / 2), max_label - strlen (label)), strlen (label), label);
  grid.replace (LOC (graph_height + 1,       max_label - 1),              1,              "0");

  // Draw x-axis.
  grid.replace (LOC (height - 6, max_label + 1), 1, "+");
  grid.replace (LOC (height - 6, max_label + 2), graph_width, std::string (graph_width, '-'));

  // Draw x-axis labels.
  std::vector <time_t> bars_in_sequence;
  std::map <time_t, Bar>::iterator it;
  for (it = bars.begin (); it != bars.end (); ++it)
    bars_in_sequence.push_back (it->first);

  std::sort (bars_in_sequence.begin (), bars_in_sequence.end ());
  std::vector <time_t>::iterator seq;
  std::string major_label;
  for (seq = bars_in_sequence.begin (); seq != bars_in_sequence.end (); ++seq)
  {
    Bar bar = bars[*seq];

    // If it fits within the allowed space.
    if (bar.offset < actual_bars)
    {
      grid.replace (LOC (height - 5, max_label + 3 + ((actual_bars - bar.offset - 1) * 3)), bar.minor_label.length (), bar.minor_label);

      if (major_label != bar.major_label)
        grid.replace (LOC (height - 4, max_label + 2 + ((actual_bars - bar.offset - 1) * 3)), bar.major_label.length (), " " + bar.major_label);

      major_label = bar.major_label;
    }
  }

  // Draw bars.
  for (seq = bars_in_sequence.begin (); seq != bars_in_sequence.end (); ++seq)
  {
    Bar bar = bars[*seq];

    // If it fits within the allowed space.
    if (bar.offset < actual_bars)
    {
      int pending = (bar.pending                 * graph_height) / labels[2];
      int started = (bar.started                 * graph_height) / labels[2];
      int done    = ((bar.done + carryover_done) * graph_height) / labels[2];

      for (int b = 0; b < pending; ++b)
        grid.replace (LOC (graph_height - b, max_label + 3 + ((actual_bars - bar.offset - 1) * 3)), 2, "pp");

      for (int b = 0; b < started; ++b)
        grid.replace (LOC (graph_height - b - pending, max_label + 3 + ((actual_bars - bar.offset - 1) * 3)), 2, "ss");

      for (int b = 0; b < done; ++b)
        grid.replace (LOC (graph_height - b - pending - started, max_label + 3 + ((actual_bars - bar.offset - 1) * 3)), 2, "dd");
    }
  }

  // Draw rates.
  calculateRates (bars_in_sequence);
  char rate[12];
  if (find_rate != 0.0)
    sprintf (rate, "%.1f/d", find_rate);
  else
    strcpy (rate, "-");

  grid.replace (LOC (height - 2, max_label + 3), 11 + strlen (rate), std::string ("Add rate: ") + rate);

  if (fix_rate != 0.0)
    sprintf (rate, "%.1f/d", fix_rate);
  else
    strcpy (rate, "-");

  grid.replace (LOC (height - 1, max_label + 3), 11 + strlen (rate), std::string ("Done/Delete rate:  ") + rate);

  // Draw completion date.
  if (completion.length ())
    grid.replace (LOC (height - 2, max_label + 27), 22 + completion.length (), "Estimated completion: " + completion);

  optimizeGrid ();

  // Colorize the grid.
  Color color_pending (context.config.get ("color.burndown.pending"));
  Color color_done    (context.config.get ("color.burndown.done"));
  Color color_started (context.config.get ("color.burndown.started"));

  // Replace dd, ss, pp with colored strings.
  std::string::size_type i;
  while ((i = grid.find ("pp")) != std::string::npos)
    grid.replace (i, 2, color_pending.colorize ("  "));

  while ((i = grid.find ("ss")) != std::string::npos)
    grid.replace (i, 2, color_started.colorize ("  "));

  while ((i = grid.find ("dd")) != std::string::npos)
    grid.replace (i, 2, color_done.colorize ("  "));

  return grid;
}

////////////////////////////////////////////////////////////////////////////////
// grid =~ /\s+$//g
void Chart::optimizeGrid ()
{
  std::string::size_type ws;
  while ((ws = grid.find (" \n")) != std::string::npos)
  {
    std::string::size_type non_ws = ws;
    while (grid[non_ws] == ' ')
      --non_ws;

    grid.replace (non_ws + 1, ws - non_ws + 1, "\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
Date Chart::quantize (const Date& input)
{
  if (period == 'D') return input.startOfDay ();
  if (period == 'W') return input.startOfWeek ();
  if (period == 'M') return input.startOfMonth ();

  return input;
}

////////////////////////////////////////////////////////////////////////////////
Date Chart::increment (const Date& input)
{
  // Move to the next period.
  int d = input.day ();
  int m = input.month ();
  int y = input.year ();

  int days;

  switch (period)
  {
  case 'D':
    if (++d > Date::daysInMonth (m, y))
    {
      d = 1;

      if (++m == 13)
      {
        m = 1;
        ++y;
      }
    }
    break;

  case 'W':
    d += 7;
    days = Date::daysInMonth (m, y);
    if (d > days)
    {
      d -= days;

      if (++m == 13)
      {
        m = 1;
        ++y;
      }
    }
    break;

  case 'M':
    d = 1;
    if (++m == 13)
    {
      m = 1;
      ++y;
    }
    break;
  }

  return Date (m, d, y, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
Date Chart::decrement (const Date& input)
{
  // Move to the previous period.
  int d = input.day ();
  int m = input.month ();
  int y = input.year ();

  switch (period)
  {
  case 'D':
    if (--d == 0)
    {
      if (--m == 0)
      {
        m = 12;
        --y;
      }

      d = Date::daysInMonth (m, y);
    }
    break;

  case 'W':
    d -= 7;
    if (d < 1)
    {
      if (--m == 0)
      {
        m = 12;
        y--;
      }

      d += Date::daysInMonth (m, y);
    }
    break;

  case 'M':
    d = 1;
    if (--m == 0)
    {
      m = 12;
      --y;
    }
    break;
  }

  return Date (m, d, y, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Do 'bars[epoch] = Bar' for every bar that may appear on a chart.
void Chart::generateBars ()
{
  Bar bar;

  // Determine the last bar date.
  Date cursor;
  switch (period)
  {
  case 'D': cursor = Date ().startOfDay ();   break;
  case 'W': cursor = Date ().startOfWeek ();  break;
  case 'M': cursor = Date ().startOfMonth (); break;
  }

  // Iterate and determine all the other bar dates.
  char str[12];
  for (int i = 0; i < estimated_bars; ++i)
  {
    // Create the major and minor labels.
    switch (period)
    {
    case 'D': // month/day
      {
        std::string month = Date::monthName (cursor.month ());
        bar.major_label = month.substr (0, 3);

        sprintf (str, "%02d", cursor.day ());
        bar.minor_label = str;
      }
      break;

    case 'W': // year/week
      sprintf (str, "%d", cursor.year ());
      bar.major_label = str;

      sprintf (str, "%02d", cursor.weekOfYear (0));
      bar.minor_label = str;
      break;

    case 'M': // year/month
      sprintf (str, "%d", cursor.year ());
      bar.major_label = str;

      sprintf (str, "%02d", cursor.month ());
      bar.minor_label = str;
      break;
    }

    bar.offset = i;
    bars[cursor.toEpoch ()] = bar;

    // Record the earliest date, for use as a cutoff when scanning data.
    earliest = cursor;

    // Move to the previous period.
    cursor = decrement (cursor);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Chart::maxima ()
{
  max_value = 0;
  max_label = 1;

  std::map <time_t, Bar>::iterator it;
  for (it = bars.begin (); it != bars.end (); it++)
  {
    // Determine max_label.
    int total = it->second.pending +
                it->second.started +
                it->second.done    +
                carryover_done;

    // Determine max_value.
    if (total > max_value)
      max_value = total;

    int length = (int) log10 ((double) total) + 1;
    if (length > max_label)
      max_label = length;
  }

  // How many bars can be shown?
  actual_bars = (width - max_label - 14) / 3;
  graph_width = width - max_label - 14;
}

////////////////////////////////////////////////////////////////////////////////
// Given the vertical chart area size (graph_height), the largest value
// (max_value), populate a vector of labels for the y axis.
void Chart::yLabels (std::vector <int>& labels)
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
  // nearest 10.  This is a poor solution.
  int high = max_value;
  int mod = high % 10;
  if (mod)
    high += 10 - mod;

  int half = high / 2;

  labels.push_back (0);
  labels.push_back (half);
  labels.push_back (high);
}

////////////////////////////////////////////////////////////////////////////////
void Chart::calculateRates (std::vector <time_t>& sequence)
{
  // If there are no current pending tasks, then it is meaningless to find
  // rates or estimated completion date.
  if (bars[sequence.back ()].pending == 0)
    return;

  // Calculate how many items we have.
  int quantity = (int) sequence.size ();
  int half     = quantity / 2;
  int quarter  = quantity / 4;

  // If the half and quarter indexes match, then there are too few data points
  // to generate any meaningful rates.
  if (half == quantity || half == 0 || quarter == 0)
  {
    context.debug ("Chart::calculateRates Insufficient data for rate calc");
    return;
  }

  // How many days do these sums represent?
  int half_days = 1;
  int quarter_days = 1;
  switch (period)
  {
  case 'D':
    half_days = half;
    quarter_days = quarter;
    break;

  case 'W':
    half_days = half * 7;
    quarter_days = quarter * 7;
    break;

  case 'M':
    half_days = half * 30;
    quarter_days = quarter * 30;
    break;
  }

  int total_added_50 = 0;
  int total_added_75 = 0;
  int total_removed_50 = 0;
  int total_removed_75 = 0;

  for (unsigned int i = half; i < sequence.size (); ++i)
  {
    total_added_50 += bars[sequence[i]].added;
    total_removed_50 += bars[sequence[i]].removed;
  }

  for (unsigned int i = half + quarter; i < sequence.size (); ++i)
  {
    total_added_75 += bars[sequence[i]].added;
    total_removed_75 += bars[sequence[i]].removed;
  }

  float find_rate_50 = 1.0 * total_added_50 / half_days;
  float find_rate_75 = 1.0 * total_added_75 / quarter_days;
  float fix_rate_50 = 1.0 * total_removed_50 / half_days;
  float fix_rate_75 = 1.0 * total_removed_75 / quarter_days;

  // Make configurable.
  float bias = (float) context.config.getReal ("burndown.bias");

  find_rate = (find_rate_50 * (1.0 - bias) + find_rate_75 * bias);
  fix_rate  = (fix_rate_50  * (1.0 - bias) + fix_rate_75 * bias);

  // Q: Why is this equation written out as a debug message?
  // A: People are going to want to know how the rates and the completion date
  //    are calculated.  This may also help debugging.
  std::stringstream rates;
  rates << "Chart::calculateRates find rate: "
        << "("
        << total_added_50
        << " added / "
        << half_days
        << " days) * (1.0 - "
        << bias
        << ") + ("
        << total_added_75
        << " added / "
        << quarter_days
        << " days) * "
        << bias
        << ") = "
        << find_rate
        << "\nChart::calculateRates fix rate: "
        << "("
        << total_removed_50
        << " removed / "
        << half_days
        << " days) * (1.0 - "
        << bias
        << ") + ("
        << total_removed_75
        << " added / "
        << quarter_days
        << " days) * "
        << bias
        << ") = "
        << fix_rate;
  context.debug (rates.str ());

  // Estimate completion
  if (fix_rate > find_rate)
  {
    int current_pending = bars[sequence.back ()].pending;
    int remaining_days = (int) (current_pending / (fix_rate - find_rate));

    Date now;
    Duration delta (remaining_days * 86400);
    now += delta;

    completion = now.toString (context.config.get ("dateformat"))
               + " ("
               + delta.format ()
               + ")";

    std::stringstream est;
    est << "Chart::calculateRates Completion: "
         << current_pending
         << " tasks / ("
         << fix_rate
         << " - "
         << find_rate
         << ") = "
         << remaining_days
         << " days = "
         << completion;
    context.debug (est.str ());
  }
  else
  {
    completion = "No convergence";
  }
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownMonthly::CmdBurndownMonthly ()
{
  _keyword     = "burndown.monthly";
  _usage       = "task burndown.monthly [<filter>]";
  _description = "Shows a graphical burndown chart, by month.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownMonthly::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks, applying any filter.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('M');

  // Use any filter as a title.
  if (context.args.size () > 2)
  {
    std::string combined = "("
                         + context.args.extract_read_only_filter ().combine ()
                         + ")";
    chart.description (combined);
  }

  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownWeekly::CmdBurndownWeekly ()
{
  _keyword     = "burndown.weekly";
  _usage       = "task burndown.weekly [<filter>]";
  _description = "Shows a graphical burndown chart, by week.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownWeekly::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks, applying any filter.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('W');

  // Use any filter as a title.
  if (context.args.size () > 2)
  {
    std::string combined = "("
                         + context.args.extract_read_only_filter ().combine ()
                         + ")";
    chart.description (combined);
  }

  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownDaily::CmdBurndownDaily ()
{
  _keyword     = "burndown.daily";
  _usage       = "task burndown.daily [<filter>]";
  _description = "Shows a graphical burndown chart, by day.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownDaily::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks, applying any filter.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('D');

  // Use any filter as a title.
  if (context.args.size () > 2)
  {
    std::string combined = "("
                         + context.args.extract_read_only_filter ().combine ()
                         + ")";
    chart.description (combined);
  }

  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
