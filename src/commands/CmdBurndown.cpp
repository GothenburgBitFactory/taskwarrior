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
// cmake.h include must come first.
#include <CmdBurndown.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Filter.h>
#include <format.h>
#include <string.h>

#include <algorithm>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

// Helper macro.
#define LOC(y, x) (((y) * (_width + 1)) + (x))

////////////////////////////////////////////////////////////////////////////////
class Bar {
 public:
  Bar() = default;
  ~Bar() = default;

  int _offset{0};            // from left of chart
  std::string _major_label;  // x-axis label, major (year/-/month)
  std::string _minor_label;  // x-axis label, minor (month/week/day)
  int _pending{0};           // Number of pending tasks in period
  int _started{0};           // Number of started tasks in period
  int _done{0};              // Number of done tasks in period
};

////////////////////////////////////////////////////////////////////////////////
// Task counting pattern.
// In the original version of the burndown chart, the names here corresponded
// literally to the terms in the gathering algorithm. Following a rewrite to
// pre-compute the epochs, the relationship of these dates is more indirect
// (see code comments throughout the file), but the broad semantics remain
// similar.
//
// If you're familiar with the old version of the burndown code, note that
// we no longer count deleted tasks except for peak calculation.
//
//   e = entry
//   s = start
//   C = end/Completed
//   D = end/Deleted
//   > = Pending/Waiting
//
//   ID  30 31 01 02 03 04 05 06 07 08 09 10
//   --  ------------------------------------
//   1          e-----s--C                    completed
//   2             e--s-----D                 deleted (peak count only)
//   3                e-----s-------------->  pending, started
//   4                   e----------------->  pending
//   5                               e----->  pending
//   --  ------------------------------------
//   PP         1  1  2  2  1  1  1  2  2  2
//   SS                     1  1  1  1  1  1
//   DD                  1  1  1  1  1  1  1
//   --  ------------------------------------
// Peak         1  2  3  4  3  2  2  3  3  3

// Forward-declared so Chart can reference it in its method signatures;
// defined below findTaskEpochRange.
struct TaskEpochRange;
Datetime quantize(const Datetime&, char);

class Chart {
 public:
  Chart(char);
  Chart(const Chart&) = delete;
  Chart& operator=(const Chart&) = delete;
  ~Chart() = default;

  void accumulateTasks(const std::vector<Task>&, const std::vector<TaskEpochRange>&,
                       time_t now_epoch, bool cumulative);
  void generateBars();
  void finalize();
  std::string render();
  void buildPeakRange(time_t, time_t);

 private:
  void optimizeGrid(std::string& grid);
  Datetime increment(const Datetime&, char);
  Datetime decrement(const Datetime&, char);
  void maxima();
  std::vector<int> yLabels();
  void calculateRates();
  unsigned round_up_to(unsigned, unsigned);
  unsigned burndown_size(unsigned);

 public:
  int _width{};                            // Terminal width
  int _height{};                           // Terminal height
  int _graph_width{};                      // Width of plot area
  int _graph_height{};                     // Height of plot area
  int _max_value{0};                       // Largest value of pending+started+done
                                           // +carried over done
  int _max_label{1};                       // Longest y-axis label
  int _estimated_bars{};                   // Max number of bars that fit the terminal
  int _actual_bars{0};                     // Max no. of bars that fit given
                                           // y-axis label width
  std::map<time_t, Bar> _bars{};           // Epoch-indexed set of bars
  Datetime _earliest{};                    // Date of earliest estimated bar
  int _carryover_done{0};                  // Number of 'done' tasks prior to chart range
  char _period{};                          // D, W, M
  std::vector<time_t> _peak_day_epochs{};  // All consecutive day epochs (for peak range)
  std::vector<int> _peak_diff{};           // Difference array for peak counting,
                                           // quantized by day
  std::unordered_map<time_t, size_t> _peak_day_index{};  // a map of per-day epochs to _peak_diff
  time_t _peak_day{};                                    // Day of highest pending peak
  int _peak_count{0};                                    // Corresponding peak pending count
  int _current_count{0};                                 // Current count of tasks without end date
  float _net_fix_rate{0.0f};                             // Calculated fix rate (tasks/day)
  std::string _completion{};                             // Estimated completion date
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

  // Estimate how many 'bars' can be displayed.  This will help subset a
  // potentially enormous data set.
  _estimated_bars = (_width - 1 - 14) / 3;

  _period = type;
}

//  - Bar fields (first/last/entry/end_epoch): quantized by chart period
//    (D/W/M), used for rendered bars.
//  - Peak fields (peak_entry/peak_end): quantized by day, used for the
//    peak pending count.
struct TaskEpochRange {
  time_t first_epoch;                 // leftmost bar this task touches
  time_t last_epoch;                  // rightmost bar this task touches
  time_t entry_epoch;                 // quantized entry by period
  std::optional<time_t> end_epoch;    // quantized end by period
                                      // nullopt when task has no end
  time_t peak_entry;                  // quantized entry by day
  time_t peak_end;                    // quantized end by day, or today when no end
  std::optional<time_t> start_epoch;  // quantized start by period,
                                      // nullopt if no start
};

////////////////////////////////////////////////////////////////////////////////
// Compute the bar-epoch range and quantized attribute epochs for one task.
// Used by scan_epochs to size the peak range, and by accumulateTasks to
// pre-skip zero-contribution tasks. Per-bar contributions (pending/
// started/done) are computed in accumulateTasks from entry_epoch and
// end_epoch, because last_epoch is not always the per-bar boundary,
// eg. for a completed task in cumulative mode, last_epoch extends to today.
// now_epoch is now quantized by chart period
// now_day_epoch is now quantized by day
static TaskEpochRange findTaskEpochRange(const Task& task, time_t now_epoch, time_t now_day_epoch,
                                         char period, bool cumulative) {
  Datetime entry_date(task.get_date("entry"));
  time_t entry_epoch = quantize(entry_date, period).toEpoch();
  // The peak count is always quantized by day, regardless of chart period.
  time_t peak_entry = quantize(entry_date, 'D').toEpoch();

  std::optional<time_t> start_epoch;
  if (task.has("start")) start_epoch = quantize(Datetime(task.get_date("start")), period).toEpoch();

  // Quantize end epochs once.
  // When absent, peak_end defaults to today.
  std::optional<time_t> end_epoch;
  time_t peak_end = now_day_epoch;
  if (task.has("end")) {
    Datetime end_date(task.get_date("end"));
    end_epoch = quantize(end_date, period).toEpoch();
    peak_end = quantize(end_date, 'D').toEpoch();
  }

  Task::status status = task.getStatus();

  // Pending/waiting tasks extend to today if they lack an end.
  if (status == Task::pending || status == Task::waiting) {
    time_t last_epoch = end_epoch.value_or(now_epoch);
    return {entry_epoch, last_epoch, entry_epoch, end_epoch, peak_entry, peak_end, start_epoch};
  }

  // Completed tasks extend to end (non-cumulative) or today (cumulative).
  if (status == Task::completed) {
    time_t e = *end_epoch;                          // completed tasks always have an end
    time_t first_epoch = std::min(entry_epoch, e);  // we handle the case where
                                                    // a task has an end before
                                                    // entry for some reason
    time_t last_epoch = cumulative ? std::max(e, now_epoch) : std::max(e, entry_epoch);
    return {first_epoch, last_epoch, entry_epoch, end_epoch, peak_entry, peak_end, start_epoch};
  }

  // Deleted/recurring tasks don't contribute to bars, but they do to the
  // peak count.
  time_t last_epoch = end_epoch ? std::max(*end_epoch, entry_epoch) : entry_epoch;
  return {entry_epoch, last_epoch, entry_epoch, end_epoch, peak_entry, peak_end, start_epoch};
}

////////////////////////////////////////////////////////////////////////////////
void Chart::accumulateTasks(const std::vector<Task>& tasks,
                            const std::vector<TaskEpochRange>& ranges, time_t now_epoch,
                            bool cumulative) {
  // A helper that maps epochs to their indices, and then keeps a difference
  // array via _peak_diff. This array is then prefix-summed by finalize(),
  // giving us the _peak_count for each day. The _peak_day_index is always
  // quantized by day, regardless of the mode the chart runs in.
  auto peak_add = [&](time_t first_day, time_t last_day) {
    auto first_it = _peak_day_index.find(first_day);
    auto last_it = _peak_day_index.find(last_day);
    if (first_it == _peak_day_index.end() || last_it == _peak_day_index.end()) return;
    size_t fi = first_it->second, li = last_it->second;
    _peak_diff[fi]++;
    if (li + 1 < _peak_diff.size()) _peak_diff[li + 1]--;
  };

  // Helper function that operates on each bar in [start, last], including
  // values equal to last.
  auto for_bars_through = [&](time_t start, time_t last, auto&& fn) {
    auto it = _bars.lower_bound(start);
    for (; it != _bars.end() && it->first <= last; ++it) fn(it->second);
  };

  // Helper function that operates on each bar in [start, end],
  // excluding values equal to end.
  auto for_bars_before = [&](time_t start, time_t end_excl, auto&& fn) {
    auto it = _bars.lower_bound(start);
    for (; it != _bars.end() && it->first < end_excl; ++it) fn(it->second);
  };

  for (size_t i = 0; i < tasks.size(); ++i) {
    // ranges is the taskEpochRange returned by findTaskEpochRange().
    const auto& r = ranges[i];
    const auto& task = tasks[i];

    // Peak and _current_count are made regardless of the task status.
    peak_add(r.peak_entry, r.peak_end);
    if (!r.end_epoch) ++_current_count;

    Task::status status = task.getStatus();

    // Deleted/recurring tasks contribute to the peak and _current_count,
    // but make no contribution to the chart's bars.
    if (status == Task::deleted || status == Task::recurring) continue;

    if (status == Task::pending || status == Task::waiting) {
      // Pending tasks contribute to the pending bars through [entry, last],
      // inclusive of values equal to last. For pending tasks, last_epoch will be
      // either the now_epoch or end_epoch.
      for_bars_through(r.entry_epoch, r.last_epoch, [](Bar& b) { ++b._pending; });

      // If the task has a start date, bars from that date through last_epoch
      // change from pending to started.
      if (r.start_epoch) {
        for_bars_through(*r.start_epoch, r.last_epoch, [](Bar& b) {
          --b._pending;
          ++b._started;
        });
      }
    } else if (status == Task::completed) {
      // Completed tasks are counted as pending exclusive of their
      // end_epoch. The peak count, however, includes values equal to
      // end_epoch. See findTaskEpochRange() for details of assignment.
      for_bars_before(r.entry_epoch, *r.end_epoch, [](Bar& b) { ++b._pending; });

      if (cumulative) {
        // In cumulative mode, completed tasks range through
        // [max(entry, end), today], including values equal to today.
        // Nb: now_epoch is quantized by the period of the chart mode,
        // ie. daily/weekly/monthly.
        time_t done_start_epoch = std::max(r.entry_epoch, *r.end_epoch);
        for_bars_through(done_start_epoch, now_epoch, [](Bar& b) { ++b._done; });

        // If the end date of a completed task is before the leftmost
        // rendered bar we need to carry over the count. _earliest is simply
        // the earliest epoch in the dataset.
        if (*r.end_epoch < _earliest.toEpoch()) ++_carryover_done;
      } else {
        // In non-cumulative mode, each end date of a completed task counts
        // only for the bars of that day.
        time_t done_epoch = std::max(r.entry_epoch, *r.end_epoch);
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
  std::string grid;
  grid.reserve(static_cast<size_t>(_height) * (_width + 1));
  for (int i = 0; i < _height; ++i) grid += std::string(_width, ' ') + '\n';

  // Title.
  std::string title = _period == 'D'   ? "Daily"
                      : _period == 'W' ? "Weekly"
                      : _period == 'M' ? "Monthly"
                      : _period == 'Y' ? "Annual"
                                       : "Monthly";
  title += std::string(" Burndown");
  grid.replace(LOC(0, (_width - title.length()) / 2), title.length(), title);

  // Legend.
  grid.replace(LOC(_graph_height / 2 - 1, _width - 10), 10, "DD " + leftJustify("Done", 7));
  grid.replace(LOC(_graph_height / 2, _width - 10), 10, "SS " + leftJustify("Started", 7));
  grid.replace(LOC(_graph_height / 2 + 1, _width - 10), 10, "PP " + leftJustify("Pending", 7));

  // Determine y-axis labelling.
  auto labels = yLabels();
  // Digit count of the high y-axis label (avoids log10 FP).
  _max_label = 1;
  for (int t = labels[2]; t >= 10; t /= 10) ++_max_label;

  // Draw y-axis.
  for (int i = 0; i < _graph_height; ++i) grid.replace(LOC(i + 1, _max_label + 1), 1, "|");

  // Draw y-axis labels.
  char label[12];
  snprintf(label, 12, "%*d", _max_label, labels[2]);
  grid.replace(LOC(1, _max_label - strlen(label)), strlen(label), label);
  snprintf(label, 12, "%*d", _max_label, labels[1]);
  grid.replace(LOC(1 + (_graph_height / 2), _max_label - strlen(label)), strlen(label), label);
  grid.replace(LOC(_graph_height + 1, _max_label - 1), 1, "0");

  // Draw x-axis.
  grid.replace(LOC(_height - 6, _max_label + 1), 1, "+");
  grid.replace(LOC(_height - 6, _max_label + 2), _graph_width, std::string(_graph_width, '-'));

  // Draw x-axis labels. _bars is a std::map (already sorted by epoch).
  std::string last_major;
  for (const auto& [epoch, bar] : _bars) {
    // If it fits within the allowed space.
    if (bar._offset < _actual_bars) {
      int col = _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3);
      grid.replace(LOC(_height - 5, col), bar._minor_label.length(), bar._minor_label);

      if (last_major != bar._major_label)
        grid.replace(LOC(_height - 4, col - 1), bar._major_label.length(), ' ' + bar._major_label);

      last_major = bar._major_label;
    }
  }

  // Draw bars.
  for (const auto& [epoch, bar] : _bars) {
    // If it fits within the allowed space.
    if (bar._offset < _actual_bars) {
      int col = _max_label + 3 + ((_actual_bars - bar._offset - 1) * 3);
      int pending = (bar._pending * _graph_height) / labels[2];
      int started = ((bar._pending + bar._started) * _graph_height) / labels[2];
      int done =
          ((bar._pending + bar._started + bar._done + _carryover_done) * _graph_height) / labels[2];

      auto draw_segment = [&](int from, int to, const char* tag) {
        for (int b = from; b < to; ++b) grid.replace(LOC(_graph_height - b, col), 2, tag);
      };
      draw_segment(0, pending, "PP");
      draw_segment(pending, started, "SS");
      draw_segment(started, done, "DD");
    }
  }

  // Draw rates.
  calculateRates();
  char rate[12];
  if (_net_fix_rate != 0.0)
    snprintf(rate, 12, "%.1f/d", _net_fix_rate);
  else
    snprintf(rate, 12, "-");

  grid.replace(LOC(_height - 2, _max_label + 3), 22 + strlen(rate),
               std::string("Net Fix Rate:         ") + rate);

  // Draw completion date.
  if (_completion.length())
    grid.replace(LOC(_height - 1, _max_label + 3), 22 + _completion.length(),
                 "Estimated completion: " + _completion);

  optimizeGrid(grid);

  // Scan the grid char-by-char and replace the PP/SS/DD markers with
  // colored or plain equivalents. Non-marker characters are copied.
  const bool use_color = Context::getContext().color();
  Color color_pending(Context::getContext().config.get("color.burndown.pending"));
  Color color_done(Context::getContext().config.get("color.burndown.done"));
  Color color_started(Context::getContext().config.get("color.burndown.started"));

  std::string result;
  result.reserve(grid.size() * 2);
  std::string::size_type pos = 0;
  while (pos < grid.size()) {
    if (pos + 1 < grid.size()) {
      char c1 = grid[pos], c2 = grid[pos + 1];
      if (c1 == 'P' && c2 == 'P') {
        result += use_color ? color_pending.colorize("  ") : " X";
        pos += 2;
        continue;
      }
      if (c1 == 'S' && c2 == 'S') {
        result += use_color ? color_started.colorize("  ") : " +";
        pos += 2;
        continue;
      }
      if (c1 == 'D' && c2 == 'D') {
        result += use_color ? color_done.colorize("  ") : " .";
        pos += 2;
        continue;
      }
    }
    result += grid[pos];
    ++pos;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// grid =~ /\s+$//g
void Chart::optimizeGrid(std::string& grid) {
  std::string::size_type ws;
  while ((ws = grid.find(" \n")) != std::string::npos) {
    auto non_ws = ws;
    while (non_ws > 0 && grid[non_ws] == ' ') --non_ws;
    if (grid[non_ws] == ' ') {
      // The entire line up to ws is spaces; nothing to trim before.
      grid.replace(0, ws + 1, "\n");
      continue;
    }

    grid.replace(non_ws + 1, ws - non_ws + 1, "\n");
  }
}

////////////////////////////////////////////////////////////////////////////////
Datetime quantize(const Datetime& input, char period) {
  if (period == 'D') return input.startOfDay();
  if (period == 'W') return input.startOfWeek();
  if (period == 'M') return input.startOfMonth();
  if (period == 'Y') return input.startOfYear();

  return input;
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

    case 'Y':
      d = 1;
      m = 1;
      ++y;
      break;

    default:
      break;
  }

  return Datetime(y, m, d, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Build the epoch range used for peak counting.  The peak is always quantized
// by day regardless of chart mode.
void Chart::buildPeakRange(time_t earliest_day, time_t latest_day) {
  _peak_diff.clear();
  _peak_day_epochs.clear();
  _peak_day_index.clear();

  Datetime cursor(earliest_day);
  Datetime end(latest_day);
  while (cursor <= end) {
    time_t ep = cursor.toEpoch();
    _peak_day_index[ep] = _peak_diff.size();
    _peak_day_epochs.push_back(ep);
    _peak_diff.push_back(0);
    cursor = increment(cursor, 'D');
  }
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

    case 'Y':
      d = 1;
      m = 1;
      --y;
      break;

    default:
      break;
  }

  return Datetime(y, m, d, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Create one bar per period going backwards from today. Each bar is keyed by
// the quantized epoch of the chart mode (daily/weekly/monthly). _earliest is
// the leftmost bar's epoch, used for carrying over the done count in cumulative
// mode.
void Chart::generateBars() {
  Bar bar;  // future contributor: if you change the logic used for counting,
            // such that this function changes the counts, be mindful:
            // because we copy the struct, your state would be carried through
            // into each loop.
  Datetime cursor = quantize(Datetime(), _period);

  // Iterate backwards from today, generating labels for each bar.
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

      case 'Y':  // 2-digit year abbreviation
        snprintf(str, 12, "%02d", cursor.year() % 100);
        bar._minor_label = str;
        bar._major_label = "";
        break;
    }

    bar._offset = i;
    _bars[cursor.toEpoch()] = bar;

    // Record the earliest date, used as the cutoff for carrying over done
    // counts in cumulative mode.
    _earliest = cursor;

    // Move to the previous period.
    cursor = decrement(cursor, _period);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Compute the peak pending count from the accumulated quantized data, then
// size the bar data for rendering.
void Chart::finalize() {
  // _peak_diff is a difference array, here we prefix-sum it to get the
  // actual counts.
  _peak_count = 0;
  _peak_day = 0;
  int running = 0;
  for (size_t i = 0; i < _peak_diff.size(); ++i) {
    running += _peak_diff[i];
    if (running > _peak_count) {
      _peak_count = running;
      _peak_day = _peak_day_epochs[i];
    }
  }
  maxima();
}

////////////////////////////////////////////////////////////////////////////////
void Chart::maxima() {
  _max_value = 0;
  _max_label = 1;

  for (auto& bar : _bars) {
    // pending + started + done + carryover.
    int total = bar.second._pending + bar.second._started + bar.second._done + _carryover_done;

    // Determine _max_value.
    if (total > _max_value) _max_value = total;

    // Count of the tallest bar. Shouldn't == 0.
    if (total > 0) {
      int length = 1;
      for (int t = total; t >= 10; t /= 10) ++length;
      if (length > _max_label) _max_label = length;
    }
  }

  // How many bars can be shown?
  _actual_bars = (_width - _max_label - 14) / 3;
  _graph_width = _width - _max_label - 14;
}

////////////////////////////////////////////////////////////////////////////////
// Given the vertical chart area size (graph_height), the largest value
// (_max_value), populate a vector of labels for the y axis.
std::vector<int> Chart::yLabels() {
  // Calculate max Y using a nice algorithm that rounds the data.
  int high = burndown_size(_max_value);
  int half = high / 2;

  return {0, half, high};
}

////////////////////////////////////////////////////////////////////////////////
void Chart::calculateRates() {
  // Q: Why is this equation written out as a debug message?
  // A: People are going to want to know how the rates and the completion date
  //    are calculated.  This may also help debugging.
  std::stringstream peak_message;
  peak_message << "Chart::calculateRates Maximum of " << _peak_count << " pending tasks on "
               << (Datetime(_peak_day).toISO()) << ", with currently " << _current_count
               << " pending tasks";
  Context::getContext().debug(peak_message.str());

  // If there are no current pending tasks, then it is meaningless to find
  // rates or estimated completion date.
  if (_current_count == 0) return;

  Datetime now;  // unquantized current time - fix rate should be independent
                 // of chart quantization mode.

  // If there is a net fix rate, and the peak was at least three days ago.
  Datetime peak(_peak_day);
  if (_peak_count > _current_count && (now - peak) > 3 * 86400) {
    // Fixes per second.  Not a large number. Multiplied by 86400 to get
    // fixes per day.
    double fix_rate = 1.0 * (_peak_count - _current_count) / (now.toEpoch() - _peak_day);
    _net_fix_rate = static_cast<float>(fix_rate * 86400);

    std::stringstream rate_message;
    rate_message << "Chart::calculateRates Net reduction is " << (_peak_count - _current_count)
                 << " tasks in " << Duration(now.toEpoch() - _peak_day).formatISO() << " = "
                 << _net_fix_rate << " tasks/d";
    Context::getContext().debug(rate_message.str());

    Duration delta(static_cast<time_t>(_current_count / fix_rate));
    Datetime end = now + delta.toTime_t();

    // Prefer dateformat.report over dateformat.
    std::string format = Context::getContext().config.get("dateformat.report");
    if (format.empty()) {
      format = Context::getContext().config.get("dateformat");
      if (format.empty()) format = "Y-M-D";
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
  // highest power of 10 or half of power of 10.  UINT_MAX has 10 digits,
  // so we iterate from i=2 (we handled 5/10/50/100 above) to i=9.
  constexpr unsigned count = 10;
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

////////////////////////////////////////////////////////////////////////////////
static int runBurndown(char period, std::string& output) {
  Chart chart(period);   // size the chart
  chart.generateBars();  // create the bar map from today backwards

  const auto& args = Context::getContext().cli2._args;
  bool has_filter =
      std::any_of(args.begin(), args.end(), [](const auto& a) { return a.hasTag("FILTER"); });

  // now quantized by chart period
  const time_t now_epoch = quantize(Datetime(), period).toEpoch();
  // now quantized by day, for peak counting
  const time_t now_day_epoch = quantize(Datetime(), 'D').toEpoch();
  auto& cfg = Context::getContext().config;
  bool cumulative = cfg.has("burndown.cumulative") ? cfg.getBoolean("burndown.cumulative") : true;

  // First pass: walk all tasks to find the peak range quantized by day,
  // cache each task's taskEpochRange so accumulateTasks doesn't repeat
  // quantization work.
  // Returns {ranges, earliest_day, latest_day} for the task vector.
  struct ScanResult {
    std::vector<TaskEpochRange> ranges;
    time_t earliest_day;
    time_t latest_day;
  };
  auto scan_epochs = [&](const std::vector<Task>& tasks, time_t earliest_day_in,
                         time_t latest_day_in) -> ScanResult {
    ScanResult result;
    result.ranges.reserve(tasks.size());
    result.earliest_day = earliest_day_in;
    result.latest_day = latest_day_in;
    for (const auto& task : tasks) {
      auto r = findTaskEpochRange(task, now_epoch, now_day_epoch, period, cumulative);
      result.ranges.push_back(r);
      if (r.peak_entry < result.earliest_day) result.earliest_day = r.peak_entry;
      if (r.peak_end > result.latest_day) result.latest_day = r.peak_end;
    }
    return result;
  };

  if (has_filter) {
    Filter filter;
    std::vector<Task> filtered;
    filter.subset(filtered);
    auto sr = scan_epochs(filtered, now_day_epoch, 0);
    chart.buildPeakRange(sr.earliest_day, sr.latest_day);
    chart.accumulateTasks(filtered, sr.ranges, now_epoch, cumulative);
  } else {
    const auto& pending = Context::getContext().tdb2.pending_tasks();
    const auto& completed = Context::getContext().tdb2.completed_tasks();
    auto srp = scan_epochs(pending, now_day_epoch, 0);
    auto src = scan_epochs(completed, srp.earliest_day, srp.latest_day);
    chart.buildPeakRange(src.earliest_day, src.latest_day);
    chart.accumulateTasks(pending, srp.ranges, now_epoch, cumulative);
    chart.accumulateTasks(completed, src.ranges, now_epoch, cumulative);
  }

  chart.finalize();
  output = chart.render();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// All three CmdBurndown subclasses share the same construction parameters,
// differing only in keyword/usage/description and period.  Macro is shorter than
// 30+ lines of member initializations.
#define CMDBURNDOWN_CTOR(cls_name, period_char, period_word)            \
  cls_name::cls_name() {                                                \
    _keyword = "burndown." period_word;                                 \
    _usage = "task <filter> burndown." period_word;                     \
    _description = "Shows a graphical burndown chart, by " period_word; \
    _read_only = true;                                                  \
    _displays_id = false;                                               \
    _needs_gc = true;                                                   \
    _needs_recur_update = true;                                         \
    _uses_context = true;                                               \
    _accepts_filter = true;                                             \
    _accepts_modifications = false;                                     \
    _accepts_miscellaneous = false;                                     \
    _category = Command::Category::graphs;                              \
  }                                                                     \
  int cls_name::execute(std::string& output) { return runBurndown(period_char, output); }

CMDBURNDOWN_CTOR(CmdBurndownMonthly, 'M', "monthly")
CMDBURNDOWN_CTOR(CmdBurndownWeekly, 'W', "weekly")
CMDBURNDOWN_CTOR(CmdBurndownDaily, 'D', "daily")
CMDBURNDOWN_CTOR(CmdBurndownAnnual, 'Y', "annual")

#undef CMDBURNDOWN_CTOR
