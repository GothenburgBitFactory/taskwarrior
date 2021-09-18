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
#include <CmdBurndown.h>
#include <sstream>
#include <map>
#include <algorithm>
#include <limits>
#include <string.h>
#include <math.h>
#include <Context.h>
#include <Filter.h>
#include <Datetime.h>
#include <Duration.h>
#include <main.h>
#include <shared.h>
#include <format.h>

// Helper macro.
#define LOC(y,x) (((y) * (_width + 1)) + (x))

////////////////////////////////////////////////////////////////////////////////
class Bar
{
public:
  Bar () = default;
  Bar (const Bar&);
  Bar& operator= (const Bar&);
  ~Bar () = default;

public:
  int _offset              {0};    // from left of chart
  std::string _major_label {""};   // x-axis label, major (year/-/month)
  std::string _minor_label {""};   // x-axis label, minor (month/week/day)
  int _pending             {0};    // Number of pending tasks in period
  int _started             {0};    // Number of started tasks in period
  int _done                {0};    // Number of done tasks in period
  int _added               {0};    // Number added in period
  int _removed             {0};    // Number removed in period
};

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
    _offset      = other._offset;
    _major_label = other._major_label;
    _minor_label = other._minor_label;
    _pending     = other._pending;
    _started     = other._started;
    _done        = other._done;
    _added       = other._added;
    _removed     = other._removed;
  }

  return *this;
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
//   PP         1  2  3  3  2  2  2  3  3  3
//   SS               2  1  1  1  1  1  1  1
//   DD                  1  1  1  1  1  1  1
//   --  ------------------------------------
//
//   5 |             SS DD          DD DD DD
//   4 |             SS SS DD DD DD SS SS SS
//   3 |             PP PP SS SS SS PP PP PP
//   2 |          PP PP PP PP PP PP PP PP PP
//   1 |       PP PP PP PP PP PP PP PP PP PP
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
  ~Chart () = default;

  void scan (std::vector <Task>&);
  void scanForPeak (std::vector <Task>&);
  std::string render ();

private:
  void generateBars ();
  void optimizeGrid ();
  Datetime quantize (const Datetime&, char);

  Datetime increment (const Datetime&, char);
  Datetime decrement (const Datetime&, char);
  void maxima ();
  void yLabels (std::vector <int>&);
  void calculateRates ();
  unsigned round_up_to (unsigned, unsigned);
  unsigned burndown_size (unsigned);

public:
  int _width                    {};        // Terminal width
  int _height                   {};        // Terminal height
  int _graph_width              {};        // Width of plot area
  int _graph_height             {};        // Height of plot area
  int _max_value                {0};      // Largest combined bar value
  int _max_label                {1};      // Longest y-axis label
  std::vector <int> _labels     {};        // Y-axis labels
  int _estimated_bars           {};        // Estimated bar count
  int _actual_bars              {0};      // Calculated bar count
  std::map <time_t, Bar> _bars  {};        // Epoch-indexed set of bars
  Datetime _earliest            {};        // Date of earliest estimated bar
  int _carryover_done           {0};      // Number of 'done' tasks prior to chart range
  char _period                  {};        // D, W, M
  std::string _grid             {};        // String representing grid of characters
  time_t _peak_epoch            {};        // Quantized (D) date of highest pending peak
  int _peak_count               {0};      // Corresponding peak pending count
  int _current_count            {0};      // Current pending count
  float _net_fix_rate           {0.0};    // Calculated fix rate
  std::string _completion       {};       // Estimated completion date
};

////////////////////////////////////////////////////////////////////////////////
Chart::Chart (char type)
{
  // How much space is there to render in?  This chart will occupy the
  // maximum space, and the width drives various other parameters.
  _width = Context::getContext ().getWidth ();
  _height = Context::getContext ().getHeight ()
          - Context::getContext ().config.getInteger ("reserved.lines")
          - 1;  // Allow for new line with prompt.
  _graph_height = _height - 7;
  _graph_width = _width - _max_label - 14;

  // Estimate how many 'bars' can be dsplayed.  This will help subset a
  // potentially enormous data set.
  _estimated_bars = (_width - 1 - 14) / 3;

  _period = type;
}

////////////////////////////////////////////////////////////////////////////////
// Scan all tasks, quantize the dates by day, and find the peak pending count
// and corresponding epoch.
void Chart::scanForPeak (std::vector <Task>& tasks)
{
  std::map <time_t, int> pending;
  _current_count = 0;

  for (auto& task : tasks)
  {
    // The entry date is when the counting starts.
    Datetime entry (task.get_date ("entry"));

    Datetime end;
    if (task.has ("end"))
      end = Datetime (task.get_date ("end"));
    else
      ++_current_count;

    while (entry < end)
    {
      time_t epoch = quantize (entry.toEpoch (), 'D').toEpoch ();
      if (pending.find (epoch) != pending.end ())
        ++pending[epoch];
      else
        pending[epoch] = 1;

      entry = increment (entry, 'D');
    }
  }

  // Find the peak and peak date.
  for (auto& count : pending)
  {
    if (count.second > _peak_count)
    {
      _peak_count = count.second;
      _peak_epoch = count.first;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Chart::scan (std::vector <Task>& tasks)
{
  generateBars ();

  // Not quantized, so that "while (xxx < now)" is inclusive.
  Datetime now;

  time_t epoch;
  auto& config = Context::getContext ().config;
  bool cumulative;
  if (config.has ("burndown.cumulative"))
  {
    cumulative = config.getBoolean ("burndown.cumulative");
  }
  else
  {
    cumulative = true;
  }

  for (auto& task : tasks)
  {
    // The entry date is when the counting starts.
    Datetime from = quantize (Datetime (task.get_date ("entry")), _period);
    epoch = from.toEpoch ();

    if (_bars.find (epoch) != _bars.end ())
      ++_bars[epoch]._added;

    // e-->   e--s-->
    // ppp>   pppsss>
    Task::status status = task.getStatus ();
    if (status == Task::pending ||
        status == Task::waiting)
    {
      if (task.has ("start"))
      {
        Datetime start = quantize (Datetime (task.get_date ("start")), _period);
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from, _period);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._started;
          from = increment (from, _period);
        }
      }
      else
      {
        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from, _period);
        }
      }
    }

    // e--C   e--s--C
    // pppd>  pppsssd>
    else if (status == Task::completed)
    {
      // Truncate history so it starts at 'earliest' for completed tasks.
      Datetime end = quantize (Datetime (task.get_date ("end")), _period);
      epoch = end.toEpoch ();

      if (_bars.find (epoch) != _bars.end ())
        ++_bars[epoch]._removed;

      while (from < end)
      {
        epoch = from.toEpoch ();
        if (_bars.find (epoch) != _bars.end ())
          ++_bars[epoch]._pending;
        from = increment (from, _period);
      }

      if (cumulative)
      {
        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._done;
          from = increment (from, _period);
        }

        // Maintain a running total of 'done' tasks that are off the left of the
        // chart.
        if (end < _earliest)
        {
          ++_carryover_done;
          continue;
        }
      }

      else
      {
		  epoch = from.toEpoch ();
        if (_bars.find (epoch) != _bars.end ())
          ++_bars[epoch]._done;
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
//   |    |                            DD DD DD DD DD DD DD DD             |
//   |    |          DD DD DD DD DD DD DD DD DD DD DD DD DD DD             |
//   |    | PP PP SS SS SS SS SS SS SS SS SS DD DD DD DD DD DD   DD Done   |
//   | 10 | PP PP PP PP PP PP SS SS SS SS SS SS DD DD DD DD DD   SS Started|
//   |    | PP PP PP PP PP PP PP PP PP PP PP SS SS SS SS DD DD   PP Pending|
//   |    | PP PP PP PP PP PP PP PP PP PP PP PP PP PP PP SS DD             |
//   |    | PP PP PP PP PP PP PP PP PP PP PP PP PP PP PP PP PP             |
//   |  0 +----------------------------------------------------            |
//   |      21 22 23 24 25 26 27 28 29 30 31 01 02 03 04 05 06             |
//   |      July                             August                        |
//   |                                                                     |
//   |      ADD rate 1.7/d           Estimated completion 8/12/2010        |
//   |      Don/Delete rate  1.3/d                                         |
//   +---------------------------------------------------------------------+
std::string Chart::render ()
{
  if (_graph_height < 5 ||     // a 4-line graph is essentially unreadable.
      _graph_width < 2)        // A single-bar graph is useless.
  {
    return std::string ("Terminal window too small to draw a graph.\n");
  }

  else if (_graph_height > 1000 || // each line is a string allloc
           _graph_width  > 1000)
  {
    return std::string ("Terminal window too large to draw a graph.\n");
  }

  if (_max_value == 0)
    Context::getContext ().footnote ("No matches.");

  // Create a grid, folded into a string.
  _grid = "";
  for (int i = 0; i < _height; ++i)
    _grid += std::string (_width, ' ') + '\n';

  // Title.
  std::string title = _period == 'D' ? "Daily"
                    : _period == 'W' ? "Weekly"
                    :                  "Monthly";
  title += std::string (" Burndown");
  _grid.replace (LOC (0, (_width - title.length ()) / 2), title.length (), title);

  // Legend.
  _grid.replace (LOC (_graph_height / 2 - 1, _width - 10), 10, "DD " + leftJustify ("Done",    7));
  _grid.replace (LOC (_graph_height / 2,     _width - 10), 10, "SS " + leftJustify ("Started", 7));
  _grid.replace (LOC (_graph_height / 2 + 1, _width - 10), 10, "PP " + leftJustify ("Pending", 7));

  // Determine y-axis labelling.
  std::vector <int> _labels;
  yLabels (_labels);
  _max_label = (int) log10 ((double) _labels[2]) + 1;

  // Draw y-axis.
  for (int i = 0; i < _graph_height; ++i)
     _grid.replace (LOC (i + 1, _max_label + 1), 1, "|");

  // Draw y-axis labels.
  char label [12];
  snprintf (label, 12, "%*d", _max_label, _labels[2]);
  _grid.replace (LOC (1,                       _max_label - strlen (label)), strlen (label), label);
  snprintf (label, 12, "%*d", _max_label, _labels[1]);
  _grid.replace (LOC (1 + (_graph_height / 2), _max_label - strlen (label)), strlen (label), label);
  _grid.replace (LOC (_graph_height + 1,       _max_label - 1),              1,              "0");

  // Draw x-axis.
  _grid.replace (LOC (_height - 6, _max_label + 1), 1, "+");
  _grid.replace (LOC (_height - 6, _max_label + 2), _graph_width, std::string (_graph_width, '-'));

  // Draw x-axis labels.
  std::vector <time_t> bars_in_sequence;
  for (auto& bar : _bars)
    bars_in_sequence.push_back (bar.first);

  std::sort (bars_in_sequence.begin (), bars_in_sequence.end ());
  std::string _major_label;
  for (auto& seq : bars_in_sequence)
  {
    Bar bar = _bars[seq];

    // If it fits within the allowed space.
    if (bar._offset < _actual_bars)
    {
      _grid.replace (LOC (_height - 5, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), bar._minor_label.length (), bar._minor_label);

      if (_major_label != bar._major_label)
        _grid.replace (LOC (_height - 4, _max_label + 2 + ((_actual_bars - bar._offset - 1) * 3)), bar._major_label.length (), ' ' + bar._major_label);

      _major_label = bar._major_label;
    }
  }

  // Draw bars.
  for (auto& seq : bars_in_sequence)
  {
    Bar bar = _bars[seq];

    // If it fits within the allowed space.
    if (bar._offset < _actual_bars)
    {
      int pending = ( bar._pending                                               * _graph_height) / _labels[2];
      int started = ((bar._pending + bar._started)                               * _graph_height) / _labels[2];
      int done    = ((bar._pending + bar._started + bar._done + _carryover_done) * _graph_height) / _labels[2];

      for (int b = 0; b < pending; ++b)
        _grid.replace (LOC (_graph_height - b, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), 2, "PP");

      for (int b = pending; b < started; ++b)
        _grid.replace (LOC (_graph_height - b, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), 2, "SS");

      for (int b = started; b < done; ++b)
        _grid.replace (LOC (_graph_height - b, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), 2, "DD");
    }
  }

  // Draw rates.
  calculateRates ();
  char rate[12];
  if (_net_fix_rate != 0.0)
    snprintf (rate, 12, "%.1f/d", _net_fix_rate);
  else
    strcpy (rate, "-");

  _grid.replace (LOC (_height - 2, _max_label + 3), 22 + strlen (rate), std::string ("Net Fix Rate:         ") + rate);

  // Draw completion date.
  if (_completion.length ())
    _grid.replace (LOC (_height - 1, _max_label + 3), 22 + _completion.length (), "Estimated completion: " + _completion);

  optimizeGrid ();

  if (Context::getContext ().color ())
  {
    // Colorize the grid.
    Color color_pending (Context::getContext ().config.get ("color.burndown.pending"));
    Color color_done    (Context::getContext ().config.get ("color.burndown.done"));
    Color color_started (Context::getContext ().config.get ("color.burndown.started"));

    // Replace DD, SS, PP with colored strings.
    std::string::size_type i;
    while ((i = _grid.find ("PP")) != std::string::npos)
      _grid.replace (i, 2, color_pending.colorize ("  "));

    while ((i = _grid.find ("SS")) != std::string::npos)
      _grid.replace (i, 2, color_started.colorize ("  "));

    while ((i = _grid.find ("DD")) != std::string::npos)
      _grid.replace (i, 2, color_done.colorize ("  "));
  }
  else
  {
    // Replace DD, SS, PP with ./+/X strings.
    std::string::size_type i;
    while ((i = _grid.find ("PP")) != std::string::npos)
      _grid.replace (i, 2, " X");

    while ((i = _grid.find ("SS")) != std::string::npos)
      _grid.replace (i, 2, " +");

    while ((i = _grid.find ("DD")) != std::string::npos)
      _grid.replace (i, 2, " .");
  }

  return _grid;
}

////////////////////////////////////////////////////////////////////////////////
// _grid =~ /\s+$//g
void Chart::optimizeGrid ()
{
  std::string::size_type ws;
  while ((ws = _grid.find (" \n")) != std::string::npos)
  {
    auto non_ws = ws;
    while (_grid[non_ws] == ' ')
      --non_ws;

    _grid.replace (non_ws + 1, ws - non_ws + 1, "\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
Datetime Chart::quantize (const Datetime& input, char period)
{
  if (period == 'D') return input.startOfDay ();
  if (period == 'W') return input.startOfWeek ();
  if (period == 'M') return input.startOfMonth ();

  return input;
}

////////////////////////////////////////////////////////////////////////////////
Datetime Chart::increment (const Datetime& input, char period)
{
  // Move to the next period.
  int d = input.day ();
  int m = input.month ();
  int y = input.year ();

  int days;

  switch (period)
  {
  case 'D':
    if (++d > Datetime::daysInMonth (y, m))
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
    days = Datetime::daysInMonth (y, m);
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

  return Datetime (y, m, d, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
Datetime Chart::decrement (const Datetime& input, char period)
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

      d = Datetime::daysInMonth (y, m);
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

      d += Datetime::daysInMonth (y, m);
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

  return Datetime (y, m, d, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Do '_bars[epoch] = Bar' for every bar that may appear on a chart.
void Chart::generateBars ()
{
  Bar bar;

  // Determine the last bar date.
  Datetime cursor;
  switch (_period)
  {
  case 'D': cursor = Datetime ().startOfDay ();   break;
  case 'W': cursor = Datetime ().startOfWeek ();  break;
  case 'M': cursor = Datetime ().startOfMonth (); break;
  }

  // Iterate and determine all the other bar dates.
  char str[12];
  for (int i = 0; i < _estimated_bars; ++i)
  {
    // Create the major and minor labels.
    switch (_period)
    {
    case 'D': // month/day
      {
        std::string month = Datetime::monthName (cursor.month ());
        bar._major_label = month.substr (0, 3);

        snprintf (str, 12, "%02d", cursor.day ());
        bar._minor_label = str;
      }
      break;

    case 'W': // year/week
      snprintf (str, 12, "%d", cursor.year ());
      bar._major_label = str;

      snprintf (str, 12, "%02d", cursor.week ());
      bar._minor_label = str;
      break;

    case 'M': // year/month
      snprintf (str, 12, "%d", cursor.year ());
      bar._major_label = str;

      snprintf (str, 12, "%02d", cursor.month ());
      bar._minor_label = str;
      break;
    }

    bar._offset = i;
    _bars[cursor.toEpoch ()] = bar;

    // Record the earliest date, for use as a cutoff when scanning data.
    _earliest = cursor;

    // Move to the previous period.
    cursor = decrement (cursor, _period);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Chart::maxima ()
{
  _max_value = 0;
  _max_label = 1;

  for (auto& bar : _bars)
  {
    // Determine _max_label.
    int total = bar.second._pending +
                bar.second._started +
                bar.second._done    +
                _carryover_done;

    // Determine _max_value.
    if (total > _max_value)
      _max_value = total;

    int length = (int) log10 ((double) total) + 1;
    if (length > _max_label)
      _max_label = length;
  }

  // How many bars can be shown?
  _actual_bars = (_width - _max_label - 14) / 3;
  _graph_width = _width - _max_label - 14;
}

////////////////////////////////////////////////////////////////////////////////
// Given the vertical chart area size (graph_height), the largest value
// (_max_value), populate a vector of labels for the y axis.
void Chart::yLabels (std::vector <int>& labels)
{
  // Calculate may Y using a nice algorithm that rounds the data.
  int high = burndown_size (_max_value);
  int half = high / 2;

  labels.push_back (0);
  labels.push_back (half);
  labels.push_back (high);
}

////////////////////////////////////////////////////////////////////////////////
void Chart::calculateRates ()
{
  // Q: Why is this equation written out as a debug message?
  // A: People are going to want to know how the rates and the completion date
  //    are calculated.  This may also help debugging.
  std::stringstream peak_message;
  peak_message << "Chart::calculateRates Maximum of "
               << _peak_count
               << " pending tasks on "
               << (Datetime (_peak_epoch).toISO ())
               << ", with currently "
               << _current_count
               << " pending tasks";
  Context::getContext ().debug (peak_message.str ());

  // If there are no current pending tasks, then it is meaningless to find
  // rates or estimated completion date.
  if (_current_count == 0)
    return;

  // If there is a net fix rate, and the peak was at least three days ago.
  Datetime now;
  Datetime peak (_peak_epoch);
  if (_peak_count > _current_count &&
      (now - peak) > 3 * 86400)
  {
    // Fixes per second.  Not a large number.
    auto fix_rate = 1.0 * (_peak_count - _current_count) / (now.toEpoch () - _peak_epoch);
    _net_fix_rate = fix_rate * 86400;

    std::stringstream rate_message;
    rate_message << "Chart::calculateRates Net reduction is "
                 << (_peak_count - _current_count)
                 << " tasks in "
                 << Duration (now.toEpoch () - _peak_epoch).formatISO ()
                 << " = "
                 << _net_fix_rate
                 << " tasks/d";
    Context::getContext ().debug (rate_message.str ());

    Duration delta (static_cast <time_t> (_current_count / fix_rate));
    Datetime end = now + delta.toTime_t ();

    // Prefer dateformat.report over dateformat.
    std::string format = Context::getContext ().config.get ("dateformat.report");
    if (format == "")
    {
      format = Context::getContext ().config.get ("dateformat");
      if (format == "")
        format = "Y-M-D";
    }

    _completion = end.toString (format)
               + " ("
               + delta.formatVague ()
               + ')';

    std::stringstream completion_message;
    completion_message << "Chart::calculateRates ("
                       << _current_count
                       << " tasks / "
                       << _net_fix_rate
                       << ") = "
                       << delta.format ()
                       << " --> "
                       << end.toISO ();
    Context::getContext ().debug (completion_message.str ());
  }
  else
  {
    _completion = "No convergence";
  }
}

////////////////////////////////////////////////////////////////////////////////
unsigned Chart::round_up_to (unsigned n, unsigned target)
{
  return n + target - (n % target);
}

////////////////////////////////////////////////////////////////////////////////
unsigned Chart::burndown_size (unsigned ntasks)
{
  // Nearest 2
  if (ntasks < 20)
    return round_up_to (ntasks, 2);

  // Nearest 10
  if (ntasks < 50)
    return round_up_to (ntasks, 10);

  // Nearest 20
  if (ntasks < 100)
    return round_up_to (ntasks, 20);

  // Choose the number from here rounded up to the nearest 10% of the next
  // highest power of 10 or half of power of 10.
  const auto count = (unsigned) log10 (static_cast<double>(std::numeric_limits<unsigned>::max ()));
  unsigned half = 500;
  unsigned full = 1000;

  // We start at two because we handle 5, 10, 50, and 100 above.
  for (unsigned i = 2; i < count; ++i)
  {
    if (ntasks < half)
      return round_up_to (ntasks, half / 10);

    if (ntasks < full)
      return round_up_to (ntasks, full / 10);

    half *= 10;
    full *= 10;
  }

  // Round up to max of unsigned.
  return std::numeric_limits<unsigned>::max ();
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownMonthly::CmdBurndownMonthly ()
{
  _keyword               = "burndown.monthly";
  _usage                 = "task <filter> burndown.monthly";
  _description           = "Shows a graphical burndown chart, by month";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownMonthly::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks, applying any filter.
  handleUntil ();
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('M');
  chart.scanForPeak (filtered);
  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownWeekly::CmdBurndownWeekly ()
{
  _keyword               = "burndown.weekly";
  _usage                 = "task <filter> burndown.weekly";
  _description           = "Shows a graphical burndown chart, by week";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownWeekly::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks, applying any filter.
  handleUntil ();
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('W');
  chart.scanForPeak (filtered);
  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownDaily::CmdBurndownDaily ()
{
  _keyword               = "burndown.daily";
  _usage                 = "task <filter> burndown.daily";
  _description           = "Shows a graphical burndown chart, by day";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownDaily::execute (std::string& output)
{
  int rc = 0;

  // Scan the pending tasks, applying any filter.
  handleUntil ();
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('D');
  chart.scanForPeak (filtered);
  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
