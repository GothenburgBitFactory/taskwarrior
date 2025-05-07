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

#include <CmdHistory.h>
#include <Context.h>
#include <Datetime.h>
#include <Filter.h>
#include <Table.h>
#include <format.h>
#include <util.h>

#include <sstream>

#define STRING_CMD_HISTORY_YEAR "Year"
#define STRING_CMD_HISTORY_MONTH "Month"
#define STRING_CMD_HISTORY_DAY "Day"
#define STRING_CMD_HISTORY_ADDED "Added"
#define STRING_CMD_HISTORY_COMP "Completed"
#define STRING_CMD_HISTORY_DEL "Deleted"

////////////////////////////////////////////////////////////////////////////////
template <class HistoryStrategy>
CmdHistoryBase<HistoryStrategy>::CmdHistoryBase() {
  _keyword = HistoryStrategy::keyword;
  _usage = HistoryStrategy::usage;
  _description = HistoryStrategy::description;

  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = true;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
template <class HistoryStrategy>
void CmdHistoryBase<HistoryStrategy>::outputGraphical(std::string& output) {
  auto widthOfBar = Context::getContext().getWidth() - HistoryStrategy::labelWidth;

  // Now build the view.
  Table view;
  setHeaderUnderline(view);
  view.width(Context::getContext().getWidth());

  HistoryStrategy::setupTableDates(view);

  view.add("Number Added/Completed/Deleted", true, false);  // Fixed.

  Color color_add(Context::getContext().config.get("color.history.add"));
  Color color_done(Context::getContext().config.get("color.history.done"));
  Color color_delete(Context::getContext().config.get("color.history.delete"));
  Color label(Context::getContext().config.get("color.label"));

  // Determine the longest line, and the longest "added" line.
  auto maxAddedLine = 0;
  auto maxRemovedLine = 0;
  for (auto& i : groups) {
    if (completedGroup[i.first] + deletedGroup[i.first] > maxRemovedLine)
      maxRemovedLine = completedGroup[i.first] + deletedGroup[i.first];

    if (addedGroup[i.first] > maxAddedLine) maxAddedLine = addedGroup[i.first];
  }

  auto maxLine = maxAddedLine + maxRemovedLine;
  if (maxLine > 0) {
    unsigned int leftOffset = (widthOfBar * maxAddedLine) / maxLine;

    time_t priorTime = 0;
    auto row = 0;
    for (auto& i : groups) {
      row = view.addRow();

      HistoryStrategy::insertRowDate(view, row, i.first, priorTime);
      priorTime = i.first;

      unsigned int addedBar = (widthOfBar * addedGroup[i.first]) / maxLine;
      unsigned int completedBar = (widthOfBar * completedGroup[i.first]) / maxLine;
      unsigned int deletedBar = (widthOfBar * deletedGroup[i.first]) / maxLine;

      std::string bar;
      if (Context::getContext().color()) {
        std::string aBar;
        if (addedGroup[i.first]) {
          aBar = format(addedGroup[i.first]);
          while (aBar.length() < addedBar) aBar = ' ' + aBar;
        }

        std::string cBar;
        if (completedGroup[i.first]) {
          cBar = format(completedGroup[i.first]);
          while (cBar.length() < completedBar) cBar = ' ' + cBar;
        }

        std::string dBar;
        if (deletedGroup[i.first]) {
          dBar = format(deletedGroup[i.first]);
          while (dBar.length() < deletedBar) dBar = ' ' + dBar;
        }

        bar += std::string(leftOffset - aBar.length(), ' ');
        bar += color_add.colorize(aBar);
        bar += color_done.colorize(cBar);
        bar += color_delete.colorize(dBar);
      } else {
        std::string aBar;
        while (aBar.length() < addedBar) aBar += '+';
        std::string cBar;
        while (cBar.length() < completedBar) cBar += 'X';
        std::string dBar;
        while (dBar.length() < deletedBar) dBar += '-';

        bar += std::string(leftOffset - aBar.length(), ' ');
        bar += aBar + cBar + dBar;
      }

      view.set(row, HistoryStrategy::dateFieldCount + 0, bar);
    }
  }

  std::stringstream out;
  if (view.rows()) {
    out << optionalBlankLine() << view.render() << '\n';

    if (Context::getContext().color())
      out << format("Legend: {1}, {2}, {3}", color_add.colorize(STRING_CMD_HISTORY_ADDED),
                    color_done.colorize(STRING_CMD_HISTORY_COMP),
                    color_delete.colorize(STRING_CMD_HISTORY_DEL))
          << optionalBlankLine() << '\n';
    else
      out << "Legend: + Added, X Completed, - Deleted\n";
  } else {
    Context::getContext().footnote("No tasks.");
    rc = 1;
  }

  output = out.str();
}

////////////////////////////////////////////////////////////////////////////////
template <class HistoryStrategy>
void CmdHistoryBase<HistoryStrategy>::outputTabular(std::string& output) {
  Table view;
  setHeaderUnderline(view);
  view.width(Context::getContext().getWidth());

  HistoryStrategy::setupTableDates(view);

  view.add(STRING_CMD_HISTORY_ADDED, false);
  view.add(STRING_CMD_HISTORY_COMP, false);
  view.add(STRING_CMD_HISTORY_DEL, false);
  view.add("Net", false);

  auto totalAdded = 0;
  auto totalCompleted = 0;
  auto totalDeleted = 0;

  auto row = 0;
  time_t lastTime = 0;
  for (auto& i : groups) {
    row = view.addRow();

    totalAdded += addedGroup[i.first];
    totalCompleted += completedGroup[i.first];
    totalDeleted += deletedGroup[i.first];

    HistoryStrategy::insertRowDate(view, row, i.first, lastTime);
    lastTime = i.first;

    auto net = 0;

    if (addedGroup.find(i.first) != addedGroup.end()) {
      view.set(row, HistoryStrategy::dateFieldCount + 0, addedGroup[i.first]);
      net += addedGroup[i.first];
    }

    if (completedGroup.find(i.first) != completedGroup.end()) {
      view.set(row, HistoryStrategy::dateFieldCount + 1, completedGroup[i.first]);
      net -= completedGroup[i.first];
    }

    if (deletedGroup.find(i.first) != deletedGroup.end()) {
      view.set(row, HistoryStrategy::dateFieldCount + 2, deletedGroup[i.first]);
      net -= deletedGroup[i.first];
    }

    Color net_color;
    if (Context::getContext().color() && net)
      net_color = net > 0 ? Color(Color::red) : Color(Color::green);

    view.set(row, HistoryStrategy::dateFieldCount + 3, net, net_color);
  }

  if (view.rows()) {
    row = view.addRow();
    view.set(row, 1, " ");
    row = view.addRow();

    Color row_color;
    if (Context::getContext().color())
      row_color = Color(Color::nocolor, Color::nocolor, false, true, false);

    view.set(row, HistoryStrategy::dateFieldCount - 1, "Average", row_color);
    view.set(row, HistoryStrategy::dateFieldCount + 0, totalAdded / (view.rows() - 2), row_color);
    view.set(row, HistoryStrategy::dateFieldCount + 1, totalCompleted / (view.rows() - 2),
             row_color);
    view.set(row, HistoryStrategy::dateFieldCount + 2, totalDeleted / (view.rows() - 2), row_color);
    view.set(row, HistoryStrategy::dateFieldCount + 3,
             (totalAdded - totalCompleted - totalDeleted) / (view.rows() - 2), row_color);
  }

  std::stringstream out;
  if (view.rows())
    out << optionalBlankLine() << view.render() << '\n';
  else {
    Context::getContext().footnote("No tasks.");
    rc = 1;
  }

  output = out.str();
}

////////////////////////////////////////////////////////////////////////////i
class MonthlyHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfMonth(); }

  static void setupTableDates(Table& view) {
    view.add(STRING_CMD_HISTORY_YEAR, true);
    view.add(STRING_CMD_HISTORY_MONTH, true);
  }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    if (y != last_y) view.set(row, 0, y);

    view.set(row, 1, Datetime::monthName(m));
  }

  static constexpr const char* keyword = "history.monthly";
  static constexpr const char* usage = "task <filter> history.monthly";
  static constexpr const char* description = "Shows a report of task history, by month";
  static constexpr unsigned int dateFieldCount = 2;
  static constexpr bool graphical = false;
  static constexpr unsigned int labelWidth = 0;  // unused.
};

////////////////////////////////////////////////////////////////////////////////
template <class HistoryStrategy>
int CmdHistoryBase<HistoryStrategy>::execute(std::string& output) {
  rc = 0;

  // TODO is this necessary?
  groups.clear();
  addedGroup.clear();
  deletedGroup.clear();
  completedGroup.clear();

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);

  for (auto& task : filtered) {
    Datetime entry(task.get_date("entry"));

    Datetime end;
    if (task.has("end")) end = Datetime(task.get_date("end"));

    auto epoch = HistoryStrategy::getRelevantDate(entry).toEpoch();
    groups[epoch] = 0;

    // Every task has an entry date, but exclude templates.
    if (task.getStatus() != Task::recurring) ++addedGroup[epoch];

    // All deleted tasks have an end date.
    if (task.getStatus() == Task::deleted) {
      epoch = HistoryStrategy::getRelevantDate(end).toEpoch();
      groups[epoch] = 0;
      ++deletedGroup[epoch];
    }

    // All completed tasks have an end date.
    else if (task.getStatus() == Task::completed) {
      epoch = HistoryStrategy::getRelevantDate(end).toEpoch();
      groups[epoch] = 0;
      ++completedGroup[epoch];
    }
  }

  // Now build the view.
  if (HistoryStrategy::graphical)
    this->outputGraphical(output);
  else
    this->outputTabular(output);

  return rc;
}

////////////////////////////////////////////////////////////////////////////i
class MonthlyGHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfMonth(); }

  static void setupTableDates(Table& view) {
    view.add(STRING_CMD_HISTORY_YEAR, true);
    view.add(STRING_CMD_HISTORY_MONTH, true);
  }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    if (y != last_y) view.set(row, 0, y);

    view.set(row, 1, Datetime::monthName(m));
  }

  static constexpr const char* keyword = "ghistory.monthly";
  static constexpr const char* usage = "task <filter> ghistory.monthly";
  static constexpr const char* description = "Shows a graphical report of task history, by month";
  static constexpr unsigned int dateFieldCount = 2;
  static constexpr bool graphical = true;
  static constexpr unsigned int labelWidth = 15;  // length '2017 September ' = 15
};

////////////////////////////////////////////////////////////////////////////i
class AnnualGHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfYear(); }

  static void setupTableDates(Table& view) { view.add(STRING_CMD_HISTORY_YEAR, true); }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    if (y != last_y) view.set(row, 0, y);
  }

  static constexpr const char* keyword = "ghistory.annual";
  static constexpr const char* usage = "task <filter> ghistory.annual";
  static constexpr const char* description = "Shows a graphical report of task history, by year";
  static constexpr unsigned int dateFieldCount = 1;
  static constexpr bool graphical = true;
  static constexpr unsigned int labelWidth = 5;  // length '2017 ' = 5
};

////////////////////////////////////////////////////////////////////////////i
class AnnualHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfYear(); }

  static void setupTableDates(Table& view) { view.add(STRING_CMD_HISTORY_YEAR, true); }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    if (y != last_y) view.set(row, 0, y);
  }

  static constexpr const char* keyword = "history.annual";
  static constexpr const char* usage = "task <filter> history.annual";
  static constexpr const char* description = "Shows a report of task history, by year";
  static constexpr unsigned int dateFieldCount = 1;
  static constexpr bool graphical = false;
  static constexpr unsigned int labelWidth = 0;  // unused.
};

////////////////////////////////////////////////////////////////////////////i
class DailyHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfDay(); }

  static void setupTableDates(Table& view) {
    view.add(STRING_CMD_HISTORY_YEAR, true);
    view.add(STRING_CMD_HISTORY_MONTH, true);
    view.add(STRING_CMD_HISTORY_DAY, false);
  }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    bool y_changed = (y != last_y) || (lastTime == 0);
    bool m_changed = (m != last_m) || (lastTime == 0);

    if (y_changed) view.set(row, 0, y);

    if (y_changed || m_changed) view.set(row, 1, Datetime::monthName(m));

    view.set(row, 2, d);
  }

  static constexpr const char* keyword = "history.daily";
  static constexpr const char* usage = "task <filter> history.daily";
  static constexpr const char* description = "Shows a report of task history, by day";
  static constexpr unsigned int dateFieldCount = 3;
  static constexpr bool graphical = false;
  static constexpr unsigned int labelWidth = 0;  // unused.
};

////////////////////////////////////////////////////////////////////////////i
class DailyGHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfDay(); }

  static void setupTableDates(Table& view) {
    view.add(STRING_CMD_HISTORY_YEAR, true);
    view.add(STRING_CMD_HISTORY_MONTH, true);
    view.add(STRING_CMD_HISTORY_DAY, false);
  }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    bool y_changed = (y != last_y) || (lastTime == 0);
    bool m_changed = (m != last_m) || (lastTime == 0);

    if (y_changed) view.set(row, 0, y);

    if (y_changed || m_changed) view.set(row, 1, Datetime::monthName(m));

    view.set(row, 2, d);
  }

  static constexpr const char* keyword = "ghistory.daily";
  static constexpr const char* usage = "task <filter> ghistory.daily";
  static constexpr const char* description = "Shows a graphical report of task history, by day";
  static constexpr unsigned int dateFieldCount = 3;
  static constexpr bool graphical = true;
  static constexpr unsigned int labelWidth = 19;  // length '2017 September Day ' = 19
};

////////////////////////////////////////////////////////////////////////////i
class WeeklyHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfWeek(); }

  static void setupTableDates(Table& view) {
    view.add(STRING_CMD_HISTORY_YEAR, true);
    view.add(STRING_CMD_HISTORY_MONTH, true);
    view.add(STRING_CMD_HISTORY_DAY, false);
  }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    bool y_changed = (y != last_y) || (lastTime == 0);
    bool m_changed = (m != last_m) || (lastTime == 0);

    if (y_changed) view.set(row, 0, y);

    if (y_changed || m_changed) view.set(row, 1, Datetime::monthName(m));

    view.set(row, 2, d);
  }

  static constexpr const char* keyword = "history.weekly";
  static constexpr const char* usage = "task <filter> history.weekly";
  static constexpr const char* description = "Shows a report of task history, by week";
  static constexpr unsigned int dateFieldCount = 3;
  static constexpr bool graphical = false;
  static constexpr unsigned int labelWidth = 0;  // unused.
};

////////////////////////////////////////////////////////////////////////////i
class WeeklyGHistoryStrategy {
 public:
  static Datetime getRelevantDate(const Datetime& dt) { return dt.startOfWeek(); }

  static void setupTableDates(Table& view) {
    view.add(STRING_CMD_HISTORY_YEAR, true);
    view.add(STRING_CMD_HISTORY_MONTH, true);
    view.add(STRING_CMD_HISTORY_DAY, false);
  }

  static void insertRowDate(Table& view, int row, time_t rowTime, time_t lastTime) {
    Datetime dt(rowTime);
    int m, d, y;
    dt.toYMD(y, m, d);

    Datetime last_dt(lastTime);
    int last_m, last_d, last_y;
    last_dt.toYMD(last_y, last_m, last_d);

    bool y_changed = (y != last_y) || (lastTime == 0);
    bool m_changed = (m != last_m) || (lastTime == 0);

    if (y_changed) view.set(row, 0, y);

    if (y_changed || m_changed) view.set(row, 1, Datetime::monthName(m));

    view.set(row, 2, d);
  }

  static constexpr const char* keyword = "ghistory.weekly";
  static constexpr const char* usage = "task <filter> ghistory.weekly";
  static constexpr const char* description = "Shows a graphical report of task history, by week";
  static constexpr unsigned int dateFieldCount = 3;
  static constexpr bool graphical = true;
  static constexpr unsigned int labelWidth = 19;  // length '2017 September Day ' = 19
};

// Explicit instantiations, avoiding cpp-inclusion or implementation in header
template class CmdHistoryBase<DailyHistoryStrategy>;
template class CmdHistoryBase<WeeklyHistoryStrategy>;
template class CmdHistoryBase<MonthlyHistoryStrategy>;
template class CmdHistoryBase<AnnualHistoryStrategy>;
template class CmdHistoryBase<DailyGHistoryStrategy>;
template class CmdHistoryBase<WeeklyGHistoryStrategy>;
template class CmdHistoryBase<MonthlyGHistoryStrategy>;
template class CmdHistoryBase<AnnualGHistoryStrategy>;

////////////////////////////////////////////////////////////////////////////////
