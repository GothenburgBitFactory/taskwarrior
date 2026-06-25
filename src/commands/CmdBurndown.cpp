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
// cmake.h include header must come first

#include <CmdBurndown.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Filter.h>
#include <format.h>
#include <math.h>
#include <shared.h>
#include <string.h>

#include <algorithm>
#include <limits>
#include <map>
#include <sstream>
#include <unordered_map>

// Helper macro.
#define LOC(y, x) (((y) * (_width + 1)) + (x))

////////////////////////////////////////////////////////////////////////////////
class Bar {
 public:
  Bar() = default;
  Bar(const Bar&);
  Bar& operator=(const Bar&);
  ~Bar() = default;

 public:
  int _offset{0};                // from left of chart
  std::string _major_label{""};  // x-axis label, major (year/-/month)
  std::string _minor_label{""};  // x-axis label, minor (month/week/day)
  int _pending{0};               // Number of pending tasks in period
  int _started{0};               // Number of started tasks in period
  int _done{0};                  // Number of done tasks in period
  int _added{0};                 // Number added in period
  int _removed{0};               // Number removed in period
};

////////////////////////////////////////////////////////////////////////////////
Bar::Bar(const Bar& other) { *this = other; }

////////////////////////////////////////////////////////////////////////////////
Bar& Bar::operator=(const Bar& other) {
  if (this != &other) {
    _offset = other._offset;
    _major_label = other._major_label;
    _minor_label = other._minor_label;
    _pending = other._pending;
    _started = other._started;
    _done = other._done;
    _added = other._added;
    _removed = other._removed;
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

struct TaskEpochRange;

class Chart {
 public:
  Chart(char);
  Chart(const Chart&);             // Unimplemented
  Chart& operator=(const Chart&);  // Unimplemented
  ~Chart() = default;

  void accumulateTasks(const std::vector<Task>&, const std::vector<TaskEpochRange>&);
  void generateBars();
  void finalize();
  void buildEpochRange(time_t, time_t);
  std::string render();

  static Datetime quantize(const Datetime&, char);

 private:
  void optimizeGrid();

  Datetime increment(const Datetime&, char);
  Datetime decrement(const Datetime&, char);
  void maxima();
  void yLabels(std::vector<int>&);
  void calculateRates();
  unsigned round_up_to(unsigned, unsigned);
  unsigned burndown_size(unsigned);

 public:
  int _width{};                                       // Terminal width
  int _height{};                                      // Terminal height
  int _graph_width{};                                 // Width of plot area
  int _graph_height{};                                // Height of plot area
  int _max_value{0};                                  // Largest combined bar value
  int _max_label{1};                                  // Longest y-axis label
  std::vector<int> _labels{};                         // Y-axis labels
  int _estimated_bars{};                              // Estimated bar count
  int _actual_bars{0};                                // Calculated bar count
  std::map<time_t, Bar> _bars{};                      // Epoch-indexed set of bars
  Datetime _earliest{};                               // Date of earliest estimated bar
  int _carryover_done{0};                             // Number of 'done' tasks prior to chart range
  char _period{};                                     // D, W, M
  std::string _grid{};                                // String representing grid of characters
  std::vector<time_t> _epochs{};                      // All the epochs.
  std::vector<int> _pending_counts{};                 // Pending counts by epoch.
  std::unordered_map<time_t, size_t> _epoch_index{};  // A map from the bar epoch
                                                      // to its index in _epochs/_pending_counts
  time_t _peak_epoch{};                               // Date of highest pending peak.
  int _peak_count{0};                                 // Corresponding peak pending count
  int _current_count{0};                              // Current pending count
  float _net_fix_rate{0.0};                           // Calculated fix rate
  std::string _completion{};                          // Estimated completion date
};

////////////////////////////////////////////////////////////////////////////////
Chart::Chart(char type) {
  // How much space is there to render in?  This chart will occupy the
  // maximum space, and the width drives various other parameters.
  _width = Context::getContext().getWidth();
  _height = Context::getContext().getHeight() -
            Context::getContext().config.getInteger("reserved.lines") -
            1;  // Allow for new line with prompt.
  _graph_height = _height - 7;
  _graph_width = _width - _max_label - 14;

  // Estimate how many 'bars' can be dsplayed.  This will help subset a
  // potentially enormous data set.
  _estimated_bars = (_width - 1 - 14) / 3;

  _period = type;
}

////////////////////////////////////////////////////////////////////////////////
// Result of findTaskEpochRange. This includes the broad range of bars the task
// can touch, as well as the quantized epochs (so we only have to quantize once.)
struct TaskEpochRange {
  time_t first_epoch;  // the leftmost bar this task touches
  time_t last_epoch;   // the rightmost bar this task touches
  time_t entry_epoch;  // the quantized date at which the task was added
  time_t end_epoch;    // the quantized date at which the task was closed
  bool has_end;
};

/////////////////////////////////////////////////////////////////////////////////
// Compute the epoch range and the quantized epochs for a given task. This is
// used by scan epochs{} to size the global range, and by accumulateTasks() to
// skip tasks which don't contribute to the chart.
// Per-task counts are computed in accumulateTasks() from their entry_epoch and
// end_epoch. last_epoch is a different thing, because in cumulative mode, it
// must extend until today - hence the last epoch and end epoch are different -
// end is the date at which the task was completed.
static TaskEpochRange findTaskEpochRange(const Task& task, Datetime now_quantized, char period,
                                         bool cumulative) {
  time_t entry_epoch = Chart::quantize(Datetime(task.get_date("entry")), period).toEpoch();
  Task::status status = task.getStatus();

  if (status == Task::pending || status == Task::waiting) {
    // Pending tasks span entry-end/today. The today period is inclusive of the
    // unquantized period, because 'today' will not be over at the point at which
    // the report is ran.
    bool has_end = task.has("end");
    time_t last = has_end ? Chart::quantize(Datetime(task.get_date("end")), period).toEpoch()
                          : now_quantized.toEpoch();
    time_t end_epoch = has_end ? last : 0;
    return {entry_epoch, last, entry_epoch, end_epoch, has_end};
  }

  if (status == Task::completed) {
    // The range for completed tasks also cover entry-end/today. We use min for
    // the leftmost bar, and max for the rightmost. In cumulative mode, the
    // rightmost epoch that is relevant is today, so this is included. In
    // non-cumulative mode, the rightmost relevant bar is end.
    // We also cover the case where there has been some kind of bug where
    // the entry date is later than the end date. In this case, the leftmost
    // relevant bar is the end date, and the rightmost is today in cumulative
    // mode and entry in non-cumulative mode.
    time_t end_epoch = Chart::quantize(Datetime(task.get_date("end")), period).toEpoch();
    time_t leftmost = std::min(entry_epoch, end_epoch);
    if (cumulative)
      return {leftmost, std::max(end_epoch, now_quantized.toEpoch()), entry_epoch, end_epoch, true};
    return {leftmost, std::max(end_epoch, entry_epoch), entry_epoch, end_epoch, true};
  }

  // Deleted/recurring tasks are irrelevant.
  return {1, 0, entry_epoch, 0, false};
}

////////////////////////////////////////////////////////////////////////////////
void Chart::accumulateTasks(const std::vector<Task>& tasks,
                            const std::vector<TaskEpochRange>& ranges) {
  Datetime now;
  Datetime now_quantized = quantize(now, _period);
  auto& config = Context::getContext().config;
  bool cumulative =
      config.has("burndown.cumulative") ? config.getBoolean("burndown.cumulative") : true;

  for (size_t i = 0; i < tasks.size(); ++i) {
    const auto& r = ranges[i];
    const auto& task = tasks[i];

    if (r.first_epoch > r.last_epoch) continue;

    if (!r.has_end) ++_current_count;

    // This is the marker for entry dates.
    auto added_bar = _bars.find(r.entry_epoch);
    if (added_bar != _bars.end()) ++added_bar->second._added;

    Task::status status = task.getStatus();
    if (status == Task::pending || status == Task::waiting) {
      // Looks up the task's entry/last epoch in the bar array. _epochs_index
      // then maps this to the positions in _epochs/_pending_counts.
      auto entry_idx = _epoch_index.find(r.entry_epoch);
      if (entry_idx == _epoch_index.end()) continue;
      auto last_idx = _epoch_index.find(r.last_epoch);
      if (last_idx == _epoch_index.end()) continue;

      // Count the task as pending for every bar through entry-last epoch,
      // inclusive of the incomplete 'today' period.
      for (auto i = entry_idx->second, last = last_idx->second; i <= last; ++i) {
        ++_pending_counts[i];
        _bars[_epochs[i]]._pending++;
      }

      // If the task has a start attribute, bars from that point through last_epoch
      // are changed from pending to started.
      if (task.has("start")) {
        time_t start_epoch = quantize(Datetime(task.get_date("start")), _period).toEpoch();
        auto start_idx = _epoch_index.find(start_epoch);
        if (start_idx != _epoch_index.end()) {
          for (auto i = start_idx->second, last = last_idx->second; i <= last; ++i) {
            // Removes it from the pending count, and counts it as started instead.
            --_pending_counts[i];
            _bars[_epochs[i]]._pending--;
            ++_bars[_epochs[i]]._started;
          }
        }
      }
    } else if (status == Task::completed) {
      auto entry_idx = _epoch_index.find(r.entry_epoch);
      auto end_idx = _epoch_index.find(r.end_epoch);

      // We want to exclude the completion day as pending, otherwise it would
      // double the count.
      if (entry_idx != _epoch_index.end() && end_idx != _epoch_index.end()) {
        for (auto i = entry_idx->second, end = end_idx->second; i < end; ++i) {
          ++_pending_counts[i];
          _bars[_epochs[i]]._pending++;
        }
      }

      // We remove tasks at the end_epoch.
      auto removed_bar = _bars.find(r.end_epoch);
      if (removed_bar != _bars.end()) ++removed_bar->second._removed;

      if (cumulative) {
        // Just to restate: in cumulative mode, we have to extend to today.
        auto now_idx = _epoch_index.find(now_quantized.toEpoch());
        if (now_idx != _epoch_index.end()) {
          time_t done_start_epoch = std::max(r.entry_epoch, r.end_epoch);
          auto done_start_idx = _epoch_index.find(done_start_epoch);
          if (done_start_idx != _epoch_index.end()) {
            for (auto i = done_start_idx->second, now = now_idx->second; i <= now; ++i) {
              ++_bars[_epochs[i]]._done;
            }
          }
        }

        // Carry the count over if it's outside the visible chart.
        if (end_idx == _epoch_index.end()) ++_carryover_done;
      } else {
        // Finally: in non-cumulative mode each done task gets only one point
        // on the bar.
        time_t done_epoch = std::max(r.entry_epoch, r.end_epoch);
        auto done_bar = _bars.find(done_epoch);
        if (done_bar != _bars.end()) ++done_bar->second._done;
      }
    }
  }
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
std::string Chart::render() {
  if (_graph_height < 5 ||  // a 4-line graph is essentially unreadable.
      _graph_width < 2)     // A single-bar graph is useless.
  {
    return std::string("Terminal window too small to draw a graph.\n");
  }

  else if (_graph_height > 1000 ||  // each line is a string allloc
           _graph_width > 1000) {
    return std::string("Terminal window too large to draw a graph.\n");
  }

  if (_max_value == 0) Context::getContext().footnote("No matches.");

  // Create a grid, folded into a string.
  _grid = "";
  for (int i = 0; i < _height; ++i) _grid += std::string(_width, ' ') + '\n';

  // Title.
  std::string title = _period == 'D' ? "Daily" : _period == 'W' ? "Weekly" : "Monthly";
  title += std::string(" Burndown");
  _grid.replace(LOC(0, (_width - title.length()) / 2), title.length(), title);

  // Legend.
  _grid.replace(LOC(_graph_height / 2 - 1, _width - 10), 10, "DD " + leftJustify("Done", 7));
  _grid.replace(LOC(_graph_height / 2, _width - 10), 10, "SS " + leftJustify("Started", 7));
  _grid.replace(LOC(_graph_height / 2 + 1, _width - 10), 10, "PP " + leftJustify("Pending", 7));

  // Determine y-axis labelling.
  std::vector<int> _labels;
  yLabels(_labels);
  _max_label = (int)log10((double)_labels[2]) + 1;

  // Draw y-axis.
  for (int i = 0; i < _graph_height; ++i) _grid.replace(LOC(i + 1, _max_label + 1), 1, "|");

  // Draw y-axis labels.
  char label[12];
  snprintf(label, 12, "%*d", _max_label, _labels[2]);
  _grid.replace(LOC(1, _max_label - strlen(label)), strlen(label), label);
  snprintf(label, 12, "%*d", _max_label, _labels[1]);
  _grid.replace(LOC(1 + (_graph_height / 2), _max_label - strlen(label)), strlen(label), label);
  _grid.replace(LOC(_graph_height + 1, _max_label - 1), 1, "0");

  // Draw x-axis.
  _grid.replace(LOC(_height - 6, _max_label + 1), 1, "+");
  _grid.replace(LOC(_height - 6, _max_label + 2), _graph_width, std::string(_graph_width, '-'));

  // Draw x-axis labels. _bars is a map where the keys are each epochs time_t, ie. we have
  // already sorted.
  std::string _major_label;
  for (const auto& [epoch, bar] : _bars) {
    // If it fits within the allowed space.
    if (bar._offset < _actual_bars) {
      _grid.replace(LOC(_height - 5, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)),
                    bar._minor_label.length(), bar._minor_label);

      if (_major_label != bar._major_label)
        _grid.replace(LOC(_height - 4, _max_label + 2 + ((_actual_bars - bar._offset - 1) * 3)),
                      bar._major_label.length(), ' ' + bar._major_label);

      _major_label = bar._major_label;
    }
  }

  // Draw bars.
  for (const auto& [epoch, bar] : _bars) {
    // If it fits within the allowed space.
    if (bar._offset < _actual_bars) {
      int pending = (bar._pending * _graph_height) / _labels[2];
      int started = ((bar._pending + bar._started) * _graph_height) / _labels[2];
      int done = ((bar._pending + bar._started + bar._done + _carryover_done) * _graph_height) /
                 _labels[2];

      for (int b = 0; b < pending; ++b)
        _grid.replace(
            LOC(_graph_height - b, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), 2,
            "PP");

      for (int b = pending; b < started; ++b)
        _grid.replace(
            LOC(_graph_height - b, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), 2,
            "SS");

      for (int b = started; b < done; ++b)
        _grid.replace(
            LOC(_graph_height - b, _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3)), 2,
            "DD");
    }
  }

  // Draw rates.
  calculateRates();
  char rate[12];
  if (_net_fix_rate != 0.0)
    snprintf(rate, 12, "%.1f/d", _net_fix_rate);
  else
    strcpy(rate, "-");

  _grid.replace(LOC(_height - 2, _max_label + 3), 22 + strlen(rate),
                std::string("Net Fix Rate:         ") + rate);

  // Draw completion date.
  if (_completion.length())
    _grid.replace(LOC(_height - 1, _max_label + 3), 22 + _completion.length(),
                  "Estimated completion: " + _completion);

  optimizeGrid();

  if (Context::getContext().color()) {
    // Colorize the grid.
    Color color_pending(Context::getContext().config.get("color.burndown.pending"));
    Color color_done(Context::getContext().config.get("color.burndown.done"));
    Color color_started(Context::getContext().config.get("color.burndown.started"));

    // Replace DD, SS, PP with colored strings.
    std::string::size_type i;
    while ((i = _grid.find("PP")) != std::string::npos)
      _grid.replace(i, 2, color_pending.colorize("  "));

    while ((i = _grid.find("SS")) != std::string::npos)
      _grid.replace(i, 2, color_started.colorize("  "));

    while ((i = _grid.find("DD")) != std::string::npos)
      _grid.replace(i, 2, color_done.colorize("  "));
  } else {
    // Replace DD, SS, PP with ./+/X strings.
    std::string::size_type i;
    while ((i = _grid.find("PP")) != std::string::npos) _grid.replace(i, 2, " X");

    while ((i = _grid.find("SS")) != std::string::npos) _grid.replace(i, 2, " +");

    while ((i = _grid.find("DD")) != std::string::npos) _grid.replace(i, 2, " .");
  }

  return _grid;
}

////////////////////////////////////////////////////////////////////////////////
// _grid =~ /\s+$//g
void Chart::optimizeGrid() {
  std::string::size_type ws;
  while ((ws = _grid.find(" \n")) != std::string::npos) {
    auto non_ws = ws;
    while (_grid[non_ws] == ' ') --non_ws;

    _grid.replace(non_ws + 1, ws - non_ws + 1, "\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
Datetime Chart::quantize(const Datetime& input, char period) {
  if (period == 'D') return input.startOfDay();
  if (period == 'W') return input.startOfWeek();
  if (period == 'M') return input.startOfMonth();

  return input;
}

/////////////////////////////////////////////////////////////////////////////////
// We pre-compute every bar epoch between the earliest and last epoch, one period
// at a time. We build three arrays indexed by their linear position.
void Chart::buildEpochRange(time_t earliest_epoch, time_t latest_epoch) {
  _epochs.clear();
  _pending_counts.clear();
  _epoch_index.clear();

  Datetime cursor(earliest_epoch);
  Datetime end(latest_epoch);
  while (cursor <= end) {
    time_t ep = cursor.toEpoch();
    _epoch_index[ep] = _epochs.size();  // _epochs_index is a reverse lookup -
                                        // given an epoch, give us its position.
                                        // Used by accumulateTasks().
    _epochs.push_back(ep);              // These are the epochs themselves in ascending order.
    _pending_counts.push_back(0);       // This tracks the pending count per epoch.
    cursor = increment(cursor, _period);
  }
}

////////////////////////////////////////////////////////////////////////////////
Datetime Chart::increment(const Datetime& input, char period) {
  // Move to the next period.
  int d = input.day();
  int m = input.month();
  int y = input.year();

  int days;

  switch (period) {
    case 'D':
      if (++d > Datetime::daysInMonth(y, m)) {
        d = 1;

        if (++m == 13) {
          m = 1;
          ++y;
        }
      }
      break;

    case 'W':
      d += 7;
      days = Datetime::daysInMonth(y, m);
      if (d > days) {
        d -= days;

        if (++m == 13) {
          m = 1;
          ++y;
        }
      }
      break;

    case 'M':
      d = 1;
      if (++m == 13) {
        m = 1;
        ++y;
      }
      break;

    default:
      break;
  }

  return Datetime(y, m, d, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
Datetime Chart::decrement(const Datetime& input, char period) {
  // Move to the previous period.
  int d = input.day();
  int m = input.month();
  int y = input.year();

  switch (period) {
    case 'D':
      if (--d == 0) {
        if (--m == 0) {
          m = 12;
          --y;
        }

        d = Datetime::daysInMonth(y, m);
      }
      break;

    case 'W':
      d -= 7;
      if (d < 1) {
        if (--m == 0) {
          m = 12;
          y--;
        }

        d += Datetime::daysInMonth(y, m);
      }
      break;

    case 'M':
      d = 1;
      if (--m == 0) {
        m = 12;
        --y;
      }
      break;
  }

  return Datetime(y, m, d, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Do '_bars[epoch] = Bar' for every bar that may appear on a chart.
void Chart::generateBars() {
  Bar bar;

  // Determine the last bar date.
  Datetime cursor;
  switch (_period) {
    case 'D':
      cursor = Datetime().startOfDay();
      break;
    case 'W':
      cursor = Datetime().startOfWeek();
      break;
    case 'M':
      cursor = Datetime().startOfMonth();
      break;
  }

  // Iterate and determine all the other bar dates.
  char str[12];
  for (int i = 0; i < _estimated_bars; ++i) {
    // Create the major and minor labels.
    switch (_period) {
      case 'D':  // month/day
      {
        std::string month = Datetime::monthName(cursor.month());
        bar._major_label = month.substr(0, 3);

        snprintf(str, 12, "%02d", cursor.day());
        bar._minor_label = str;
      } break;

      case 'W':  // year/week
        snprintf(str, 12, "%d", cursor.year());
        bar._major_label = str;

        snprintf(str, 12, "%02d", cursor.week());
        bar._minor_label = str;
        break;

      case 'M':  // year/month
        snprintf(str, 12, "%d", cursor.year());
        bar._major_label = str;

        snprintf(str, 12, "%02d", cursor.month());
        bar._minor_label = str;
        break;
    }

    bar._offset = i;
    _bars[cursor.toEpoch()] = bar;

    // Record the earliest date, for use as a cutoff when scanning data.
    _earliest = cursor;

    // Move to the previous period.
    cursor = decrement(cursor, _period);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Once period data has been accumulated, we want to find the peak count.
// We use this to size the bar for rendering.
void Chart::finalize() {
  _peak_count = 0;
  _peak_epoch = 0;
  for (size_t i = 0; i < _epochs.size(); ++i) {
    if (_pending_counts[i] > _peak_count) {
      _peak_count = _pending_counts[i];
      _peak_epoch = _epochs[i];
    }
  }

  maxima();
}

///////////////////////////////////////////////////////////////////////////////

void Chart::maxima() {
  _max_value = 0;
  _max_label = 1;

  for (auto& bar : _bars) {
    // Determine _max_label.
    int total = bar.second._pending + bar.second._started + bar.second._done + _carryover_done;

    // Determine _max_value.
    if (total > _max_value) _max_value = total;

    int length = (int)log10((double)total) + 1;
    if (length > _max_label) _max_label = length;
  }

  // How many bars can be shown?
  _actual_bars = (_width - _max_label - 14) / 3;
  _graph_width = _width - _max_label - 14;
}

////////////////////////////////////////////////////////////////////////////////
// Given the vertical chart area size (graph_height), the largest value
// (_max_value), populate a vector of labels for the y axis.
void Chart::yLabels(std::vector<int>& labels) {
  // Calculate may Y using a nice algorithm that rounds the data.
  int high = burndown_size(_max_value);
  int half = high / 2;

  labels.push_back(0);
  labels.push_back(half);
  labels.push_back(high);
}

////////////////////////////////////////////////////////////////////////////////
void Chart::calculateRates() {
  // Q: Why is this equation written out as a debug message?
  // A: People are going to want to know how the rates and the completion date
  //    are calculated.  This may also help debugging.
  std::stringstream peak_message;
  peak_message << "Chart::calculateRates Maximum of " << _peak_count << " pending tasks on "
               << (Datetime(_peak_epoch).toISO()) << ", with currently " << _current_count
               << " pending tasks";
  Context::getContext().debug(peak_message.str());

  // If there are no current pending tasks, then it is meaningless to find
  // rates or estimated completion date.
  if (_current_count == 0) return;

  // If there is a net fix rate, and the peak was at least three days ago.
  Datetime now;
  Datetime peak(_peak_epoch);
  if (_peak_count > _current_count && (now - peak) > 3 * 86400) {
    // Fixes per second.  Not a large number.
    auto fix_rate = 1.0 * (_peak_count - _current_count) / (now.toEpoch() - _peak_epoch);
    _net_fix_rate = fix_rate * 86400;

    std::stringstream rate_message;
    rate_message << "Chart::calculateRates Net reduction is " << (_peak_count - _current_count)
                 << " tasks in " << Duration(now.toEpoch() - _peak_epoch).formatISO() << " = "
                 << _net_fix_rate << " tasks/d";
    Context::getContext().debug(rate_message.str());

    Duration delta(static_cast<time_t>(_current_count / fix_rate));
    Datetime end = now + delta.toTime_t();

    // Prefer dateformat.report over dateformat.
    std::string format = Context::getContext().config.get("dateformat.report");
    if (format == "") {
      format = Context::getContext().config.get("dateformat");
      if (format == "") format = "Y-M-D";
    }

    _completion = end.toString(format) + " (" + delta.formatVague() + ')';

    std::stringstream completion_message;
    completion_message << "Chart::calculateRates (" << _current_count << " tasks / "
                       << _net_fix_rate << ") = " << delta.format() << " --> " << end.toISO();
    Context::getContext().debug(completion_message.str());
  } else {
    _completion = "No convergence";
  }
}

////////////////////////////////////////////////////////////////////////////////
unsigned Chart::round_up_to(unsigned n, unsigned target) { return n + target - (n % target); }

////////////////////////////////////////////////////////////////////////////////
unsigned Chart::burndown_size(unsigned ntasks) {
  // Nearest 2
  if (ntasks < 20) return round_up_to(ntasks, 2);

  // Nearest 10
  if (ntasks < 50) return round_up_to(ntasks, 10);

  // Nearest 20
  if (ntasks < 100) return round_up_to(ntasks, 20);

  // Choose the number from here rounded up to the nearest 10% of the next
  // highest power of 10 or half of power of 10.
  const auto count = (unsigned)log10(static_cast<double>(std::numeric_limits<unsigned>::max()));
  unsigned half = 500;
  unsigned full = 1000;

  // We start at two because we handle 5, 10, 50, and 100 above.
  for (unsigned i = 2; i < count; ++i) {
    if (ntasks < half) return round_up_to(ntasks, half / 10);

    if (ntasks < full) return round_up_to(ntasks, full / 10);

    half *= 10;
    full *= 10;
  }

  // Round up to max of unsigned.
  return std::numeric_limits<unsigned>::max();
}

/////////////////////////////////////////////////////////////////////////////////
// This combines the work from the old CmdBurndown functions into one main function.
// period sets the chart granularity. output is the rendered chart.
static int runBurndown(char period, std::string& output) {
  // First we build the chart's time window. generateBars() populates _bars with
  // epoch keys and labels. buildEpochRange() deals with tasks outside of the
  // chart window.
  Chart chart(period);
  chart.generateBars();

  // Detects if the user is using filters.
  bool has_filter = false;
  for (const auto& a : Context::getContext().cli2._args)
    if (a.hasTag("FILTER")) {
      has_filter = true;
      break;
    }

  // We determine the global window across all tasks that can contribute to the
  // chart.
  Datetime now_quantized = Chart::quantize(Datetime(), period);
  time_t earliest = now_quantized.toEpoch();
  time_t latest = 0;
  bool cumulative;
  auto& cfg = Context::getContext().config;
  cumulative = cfg.has("burndown.cumulative") ? cfg.getBoolean("burndown.cumulative") : true;

  auto scan_epochs = [&](const std::vector<Task>& tasks, std::vector<TaskEpochRange>& ranges) {
    for (const auto& task : tasks) {
      auto r = findTaskEpochRange(task, now_quantized, period, cumulative);
      ranges.push_back(r);
      if (r.first_epoch > r.last_epoch) continue;
      if (r.first_epoch < earliest) earliest = r.first_epoch;
      if (r.last_epoch > latest) latest = r.last_epoch;
    }
  };

  if (has_filter) {
    Filter filter;
    std::vector<Task> filtered;
    filter.subset(filtered);
    std::vector<TaskEpochRange> ranges;
    scan_epochs(filtered, ranges);
    chart.buildEpochRange(earliest, latest);
    chart.accumulateTasks(filtered, ranges);
  } else {
    const auto& pending = Context::getContext().tdb2.pending_tasks();
    const auto& completed = Context::getContext().tdb2.completed_tasks();
    std::vector<TaskEpochRange> pending_ranges, completed_ranges;
    scan_epochs(pending, pending_ranges);
    scan_epochs(completed, completed_ranges);
    chart.buildEpochRange(earliest, latest);
    chart.accumulateTasks(pending, pending_ranges);
    chart.accumulateTasks(completed, completed_ranges);
  }

  chart.finalize();
  output = chart.render();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdBurndownMonthly::CmdBurndownMonthly() {
  _keyword = "burndown.monthly";
  _usage = "task <filter> burndown.monthly";
  _description = "Shows a graphical burndown chart, by month";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownMonthly::execute(std::string& output) { return runBurndown('M', output); }

////////////////////////////////////////////////////////////////////////////////
CmdBurndownWeekly::CmdBurndownWeekly() {
  _keyword = "burndown.weekly";
  _usage = "task <filter> burndown.weekly";
  _description = "Shows a graphical burndown chart, by week";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownWeekly::execute(std::string& output) { return runBurndown('W', output); }

////////////////////////////////////////////////////////////////////////////////
CmdBurndownDaily::CmdBurndownDaily() {
  _keyword = "burndown.daily";
  _usage = "task <filter> burndown.daily";
  _description = "Shows a graphical burndown chart, by day";
  _read_only = true;
  _displays_id = false;
  _needs_gc = true;
  _needs_recur_update = true;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdBurndownDaily::execute(std::string& output) { return runBurndown('D', output); }

////////////////////////////////////////////////////////////////////////////////
