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
#include <sstream>
#include <algorithm>
#include <limits>
#include <string.h>
#include <math.h>
#include <Context.h>
#include <Filter.h>
#include <ISO8601.h>
#include <main.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <CmdBurndown.h>

extern Context context;

// Helper macro.
#define LOC(y,x) (((y) * (_width + 1)) + (x))

////////////////////////////////////////////////////////////////////////////////
class Bar
{
public:
  Bar ();
  Bar (const Bar&);
  Bar& operator= (const Bar&);
  ~Bar ();

public:
  int _offset;                   // from left of chart
  std::string _major_label;      // x-axis label, major (year/-/month)
  std::string _minor_label;      // x-axis label, minor (month/week/day)
  int _pending;                  // Number of pending tasks in period
  int _started;                  // Number of started tasks in period
  int _done;                     // Number of done tasks in period
  int _added;                    // Number added in period
  int _removed;                  // Number removed in period
};

////////////////////////////////////////////////////////////////////////////////
Bar::Bar ()
: _offset (0)
, _major_label ("")
, _minor_label ("")
, _pending (0)
, _started (0)
, _done (0)
, _added (0)
, _removed (0)
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
  ~Chart ();

  void scan (std::vector <Task>&);
  std::string render ();

private:
  void generateBars ();
  void optimizeGrid ();
  ISO8601d quantize (const ISO8601d&);

  ISO8601d increment (const ISO8601d&);
  ISO8601d decrement (const ISO8601d&);
  void maxima ();
  void yLabels (std::vector <int>&);
  void calculateRates (std::vector <time_t>&);
  unsigned round_up_to (unsigned, unsigned);
  unsigned burndown_size (unsigned);

public:
  int _width;                     // Terminal width
  int _height;                    // Terminal height
  int _graph_width;               // Width of plot area
  int _graph_height;              // Height of plot area
  int _max_value;                 // Largest combined bar value
  int _max_label;                 // Longest y-axis label
  std::vector <int> _labels;      // Y-axis labels
  int _estimated_bars;            // Estimated bar count
  int _actual_bars;               // Calculated bar count
  std::map <time_t, Bar> _bars;   // Epoch-indexed set of bars
  ISO8601d _earliest;             // Date of earliest estimated bar
  int _carryover_done;            // Number of 'done' tasks prior to chart range
  char _period;                   // D, W, M
  std::string _title;             // Additional description
  std::string _grid;              // String representing grid of characters

  float _find_rate;               // Calculated find rate
  float _fix_rate;                // Calculated fix rate
  std::string _completion;        // Estimated completion date
};

////////////////////////////////////////////////////////////////////////////////
Chart::Chart (char type)
{
  // How much space is there to render in?  This chart will occupy the
  // maximum space, and the width drives various other parameters.
  _width = context.getWidth ();
  _height = context.getHeight () - 1;  // Allow for new line with prompt.
  _max_value = 0;
  _max_label = 1;
  _graph_height = _height - 7;
  _graph_width = _width - _max_label - 14;

  // Estimate how many 'bars' can be dsplayed.  This will help subset a
  // potentially enormous data set.
  _estimated_bars = (_width - 1 - 14) / 3;

  _actual_bars = 0;
  _period = type;
  _carryover_done = 0;

  // Rates are calculated last.
  _find_rate = 0.0;
  _fix_rate = 0.0;

  // Set the title.
  std::vector <std::string> words = context.cli2.getWords ();
  std::string filter;
  join (filter, " ", words);
  _title = "(" + filter + ")";
}

////////////////////////////////////////////////////////////////////////////////
Chart::~Chart ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Chart::scan (std::vector <Task>& tasks)
{
  generateBars ();

  // Not quantized, so that "while (xxx < now)" is inclusive.
  ISO8601d now;

  time_t epoch;
  for (auto& task : tasks)
  {
    // The entry date is when the counting starts.
    ISO8601d from = quantize (ISO8601d (task.get_date ("entry")));
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
        ISO8601d start = quantize (ISO8601d (task.get_date ("start")));
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._started;
          from = increment (from);
        }
      }
      else
      {
        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from);
        }
      }
    }

    // e--C   e--s--C
    // pppd>  pppsssd>
    else if (status == Task::completed)
    {
      // Truncate history so it starts at 'earliest' for completed tasks.
      ISO8601d end = quantize (ISO8601d (task.get_date ("end")));
      epoch = end.toEpoch ();

      if (_bars.find (epoch) != _bars.end ())
        ++_bars[epoch]._removed;

      // Maintain a running total of 'done' tasks that are off the left of the
      // chart.
      if (end < _earliest)
      {
        ++_carryover_done;
        continue;
      }

      if (task.has ("start"))
      {
        ISO8601d start = quantize (ISO8601d (task.get_date ("start")));
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from);
        }

        while (from < end)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._started;
          from = increment (from);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._done;
          from = increment (from);
        }
      }
      else
      {
        ISO8601d end = quantize (ISO8601d (task.get_date ("end")));
        while (from < end)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from);
        }

        while (from < now)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._done;
          from = increment (from);
        }
      }
    }

    // e--D   e--s--D
    // ppp    pppsss
    else if (status == Task::deleted)
    {
      // Skip old deleted tasks.
      ISO8601d end = quantize (ISO8601d (task.get_date ("end")));
      epoch = end.toEpoch ();
      if (_bars.find (epoch) != _bars.end ())
        ++_bars[epoch]._removed;

      if (end < _earliest)
        continue;

      if (task.has ("start"))
      {
        ISO8601d start = quantize (ISO8601d (task.get_date ("start")));
        while (from < start)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
          from = increment (from);
        }

        while (from < end)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._started;
          from = increment (from);
        }
      }
      else
      {
        ISO8601d end = quantize (ISO8601d (task.get_date ("end")));
        while (from < end)
        {
          epoch = from.toEpoch ();
          if (_bars.find (epoch) != _bars.end ())
            ++_bars[epoch]._pending;
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
    return std::string (STRING_CMD_BURN_TOO_SMALL) + "\n";
  }

  else if (_graph_height > 1000 || // each line is a string allloc
           _graph_width  > 1000)
  {
    return std::string (STRING_CMD_BURN_TOO_LARGE) + "\n";
  }

  if (_max_value == 0)
    context.footnote (STRING_FEEDBACK_NO_MATCH);

  // Create a grid, folded into a string.
  _grid = "";
  for (int i = 0; i < _height; ++i)
    _grid += std::string (_width, ' ') + "\n";

  // Title.
  std::string full_title;
  switch (_period)
  {
  case 'D': full_title = STRING_CMD_BURN_DAILY;   break;
  case 'W': full_title = STRING_CMD_BURN_WEEKLY;  break;
  case 'M': full_title = STRING_CMD_BURN_MONTHLY; break;
  }

  full_title += std::string (" ") + STRING_CMD_BURN_TITLE;

  if (_title.length ())
  {
    if (full_title.length () + 1 + _title.length () < (unsigned) _width)
    {
      full_title += " " + _title;
      _grid.replace (LOC (0, (_width - full_title.length ()) / 2), full_title.length (), full_title);
    }
    else
    {
      _grid.replace (LOC (0, (_width - full_title.length ()) / 2), full_title.length (), full_title);
      _grid.replace (LOC (1, (_width - _title.length ()) / 2), _title.length (), _title);
    }
  }
  else
  {
    _grid.replace (LOC (0, (_width - full_title.length ()) / 2), full_title.length (), full_title);
  }

  // Legend.
  _grid.replace (LOC (_graph_height / 2 - 1, _width - 10), 10, "DD " + leftJustify (STRING_CMD_BURN_DONE,    7));
  _grid.replace (LOC (_graph_height / 2,     _width - 10), 10, "SS " + leftJustify (STRING_CMD_BURN_STARTED, 7));
  _grid.replace (LOC (_graph_height / 2 + 1, _width - 10), 10, "PP " + leftJustify (STRING_CMD_BURN_PENDING, 7));

  // Determine y-axis labelling.
  std::vector <int> _labels;
  yLabels (_labels);
  _max_label = (int) log10 ((double) _labels[2]) + 1;

  // Draw y-axis.
  for (int i = 0; i < _graph_height; ++i)
     _grid.replace (LOC (i + 1, _max_label + 1), 1, "|");

  // Draw y-axis labels.
  char label [12];
  sprintf (label, "%*d", _max_label, _labels[2]);
  _grid.replace (LOC (1,                       _max_label - strlen (label)), strlen (label), label);
  sprintf (label, "%*d", _max_label, _labels[1]);
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
        _grid.replace (LOC (_height - 4, _max_label + 2 + ((_actual_bars - bar._offset - 1) * 3)), bar._major_label.length (), " " + bar._major_label);

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
  calculateRates (bars_in_sequence);
  char rate[12];
  if (_find_rate != 0.0)
    sprintf (rate, "%.1f/d", _find_rate);
  else
    strcpy (rate, "-");

  _grid.replace (LOC (_height - 2, _max_label + 3), 18 + strlen (rate), std::string ("Add rate:         ") + rate);

  if (_fix_rate != 0.0)
    sprintf (rate, "%.1f/d", _fix_rate);
  else
    strcpy (rate, "-");

  _grid.replace (LOC (_height - 1, _max_label + 3), 18 + strlen (rate), std::string ("Done/Delete rate: ") + rate);

  // Draw completion date.
  if (_completion.length ())
    _grid.replace (LOC (_height - 2, _max_label + 32), 22 + _completion.length (), "Estimated completion: " + _completion);

  optimizeGrid ();

  if (context.color ())
  {
    // Colorize the grid.
    Color color_pending (context.config.get ("color.burndown.pending"));
    Color color_done    (context.config.get ("color.burndown.done"));
    Color color_started (context.config.get ("color.burndown.started"));

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
ISO8601d Chart::quantize (const ISO8601d& input)
{
  if (_period == 'D') return input.startOfDay ();
  if (_period == 'W') return input.startOfWeek ();
  if (_period == 'M') return input.startOfMonth ();

  return input;
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d Chart::increment (const ISO8601d& input)
{
  // Move to the next period.
  int d = input.day ();
  int m = input.month ();
  int y = input.year ();

  int days;

  switch (_period)
  {
  case 'D':
    if (++d > ISO8601d::daysInMonth (m, y))
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
    days = ISO8601d::daysInMonth (m, y);
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

  return ISO8601d (m, d, y, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d Chart::decrement (const ISO8601d& input)
{
  // Move to the previous period.
  int d = input.day ();
  int m = input.month ();
  int y = input.year ();

  switch (_period)
  {
  case 'D':
    if (--d == 0)
    {
      if (--m == 0)
      {
        m = 12;
        --y;
      }

      d = ISO8601d::daysInMonth (m, y);
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

      d += ISO8601d::daysInMonth (m, y);
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

  return ISO8601d (m, d, y, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Do '_bars[epoch] = Bar' for every bar that may appear on a chart.
void Chart::generateBars ()
{
  Bar bar;

  // Determine the last bar date.
  ISO8601d cursor;
  switch (_period)
  {
  case 'D': cursor = ISO8601d ().startOfDay ();   break;
  case 'W': cursor = ISO8601d ().startOfWeek ();  break;
  case 'M': cursor = ISO8601d ().startOfMonth (); break;
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
        std::string month = ISO8601d::monthName (cursor.month ());
        bar._major_label = month.substr (0, 3);

        sprintf (str, "%02d", cursor.day ());
        bar._minor_label = str;
      }
      break;

    case 'W': // year/week
      sprintf (str, "%d", cursor.year ());
      bar._major_label = str;

      sprintf (str, "%02d", cursor.weekOfYear (0));
      bar._minor_label = str;
      break;

    case 'M': // year/month
      sprintf (str, "%d", cursor.year ());
      bar._major_label = str;

      sprintf (str, "%02d", cursor.month ());
      bar._minor_label = str;
      break;
    }

    bar._offset = i;
    _bars[cursor.toEpoch ()] = bar;

    // Record the earliest date, for use as a cutoff when scanning data.
    _earliest = cursor;

    // Move to the previous period.
    cursor = decrement (cursor);
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
void Chart::calculateRates (std::vector <time_t>& sequence)
{
  // If there are no current pending tasks, then it is meaningless to find
  // rates or estimated completion date.
  if (_bars[sequence.back ()]._pending == 0)
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
  switch (_period)
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
    total_added_50 += _bars[sequence[i]]._added;
    total_removed_50 += _bars[sequence[i]]._removed;
  }

  for (unsigned int i = half + quarter; i < sequence.size (); ++i)
  {
    total_added_75 += _bars[sequence[i]]._added;
    total_removed_75 += _bars[sequence[i]]._removed;
  }

  float find_rate_50 = 1.0 * total_added_50 / half_days;
  float find_rate_75 = 1.0 * total_added_75 / quarter_days;
  float fix_rate_50 = 1.0 * total_removed_50 / half_days;
  float fix_rate_75 = 1.0 * total_removed_75 / quarter_days;

  // Make configurable.
  float bias = (float) context.config.getReal ("burndown.bias");

  _find_rate = (find_rate_50 * (1.0 - bias) + find_rate_75 * bias);
  _fix_rate  = (fix_rate_50  * (1.0 - bias) + fix_rate_75 * bias);

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
        << _find_rate
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
        << _fix_rate;
  context.debug (rates.str ());

  // Estimate completion
  if (_fix_rate > _find_rate)
  {
    int current_pending = _bars[sequence.back ()]._pending;
    int remaining_days = (int) (current_pending / (_fix_rate - _find_rate));

    ISO8601d now;
    ISO8601p delta (remaining_days * 86400);
    now += delta;

    // Prefer dateformat.report over dateformat.
    std::string format = context.config.get ("dateformat.report");
    if (format == "")
      format = context.config.get ("dateformat");

    _completion = now.toString (format)
               + " ("
               + delta.format ()
               + ")";

    std::stringstream est;
    est << "Chart::calculateRates Completion: "
         << current_pending
         << " tasks / ("
         << _fix_rate
         << " - "
         << _find_rate
         << ") = "
         << remaining_days
         << " days = "
         << _completion;
    context.debug (est.str ());
  }
  else
  {
    _completion = STRING_CMD_BURN_NO_CONVERGE;
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
  const unsigned count = (unsigned) log10 (std::numeric_limits<unsigned>::max ());
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
  _description           = STRING_CMD_BURN_USAGE_M;
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
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('M');
  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownWeekly::CmdBurndownWeekly ()
{
  _keyword               = "burndown.weekly";
  _usage                 = "task <filter> burndown.weekly";
  _description           = STRING_CMD_BURN_USAGE_W;
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
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('W');
  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownDaily::CmdBurndownDaily ()
{
  _keyword               = "burndown.daily";
  _usage                 = "task <filter> burndown.daily";
  _description           = STRING_CMD_BURN_USAGE_D;
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
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Create a chart, scan the tasks, then render.
  Chart chart ('D');
  chart.scan (filtered);
  output = chart.render ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
