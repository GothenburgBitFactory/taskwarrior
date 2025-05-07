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

#include <CmdCalendar.h>
#include <Context.h>
#include <Lexer.h>
#include <Table.h>
#include <format.h>
#include <shared.h>
#include <stdlib.h>
#include <utf8.h>
#include <util.h>

#include <iomanip>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
CmdCalendar::CmdCalendar() {
  _keyword = "calendar";
  _usage = "task          calendar [due|<month> <year>|<year>] [y]";
  _description = "Shows a calendar, with due tasks marked";
  _read_only = true;
  _displays_id = true;
  _needs_gc = true;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCalendar::execute(std::string& output) {
  int rc = 0;

  auto& config = Context::getContext().config;

  // Each month requires 28 text columns width.  See how many will actually
  // fit.  But if a preference is specified, and it fits, use it.
  auto width = Context::getContext().getWidth();
  int preferredMonthsPerLine;

  if (config.has("calendar.monthsperline"))
    preferredMonthsPerLine = config.getInteger("calendar.monthsperline");
  else
    // Legacy configuration variable value
    preferredMonthsPerLine = config.getInteger("monthsperline");

  auto monthsThatFit = width / 26;

  auto monthsPerLine = monthsThatFit;
  if (preferredMonthsPerLine != 0 && preferredMonthsPerLine < monthsThatFit)
    monthsPerLine = preferredMonthsPerLine;

  // Load the pending tasks.
  auto tasks = Context::getContext().tdb2.pending_tasks();

  Datetime today;
  auto getPendingDate = false;
  auto monthsToDisplay = 1;
  auto mFrom = today.month();
  auto yFrom = today.year();
  auto mTo = mFrom;
  auto yTo = yFrom;

  // Defaults.
  monthsToDisplay = monthsPerLine;
  mFrom = today.month();
  yFrom = today.year();

  // Set up a vector of commands (1), for autoComplete.
  std::vector<std::string> commandNames{"calendar"};

  // Set up a vector of keywords, for autoComplete.
  std::vector<std::string> keywordNames{"due"};

  // Set up a vector of months, for autoComplete.
  std::vector<std::string> monthNames;
  for (int i = 1; i <= 12; ++i) monthNames.push_back(Lexer::lowerCase(Datetime::monthName(i)));

  // For autoComplete results.
  std::vector<std::string> matches;

  // Look at all args, regardless of sequence.
  auto argMonth = 0;
  auto argYear = 0;
  auto argWholeYear = false;

  for (auto& arg : Context::getContext().cli2.getWords()) {
    // Some version of "calendar".
    if (autoComplete(Lexer::lowerCase(arg), commandNames, matches,
                     config.getInteger("abbreviation.minimum")) == 1)
      continue;

    // "due".
    else if (autoComplete(Lexer::lowerCase(arg), keywordNames, matches,
                          config.getInteger("abbreviation.minimum")) == 1)
      getPendingDate = true;

    // "y".
    else if (Lexer::lowerCase(arg) == "y")
      argWholeYear = true;

    // YYYY.
    else if (Lexer::isAllDigits(arg) && arg.length() == 4)
      argYear = strtol(arg.c_str(), nullptr, 10);

    // MM.
    else if (Lexer::isAllDigits(arg) && arg.length() <= 2) {
      argMonth = strtol(arg.c_str(), nullptr, 10);
      if (argMonth < 1 || argMonth > 12) throw format("Argument '{1}' is not a valid month.", arg);
    }

    // "January" etc.
    else if (autoComplete(Lexer::lowerCase(arg), monthNames, matches,
                          config.getInteger("abbreviation.minimum")) == 1) {
      argMonth = Datetime::monthOfYear(matches[0]);
      if (argMonth == -1) throw format("Argument '{1}' is not a valid month.", arg);
    }

    else
      throw format("Could not recognize argument '{1}'.", arg);
  }

  // Supported combinations:
  //
  //   Command line  monthsToDisplay  mFrom  yFrom  getPendingDate
  //   ------------  ---------------  -----  -----  --------------
  //   cal             monthsPerLine  today  today           false
  //   cal y                      12  today  today           false
  //   cal due         monthsPerLine  today  today            true
  //   cal YYYY                   12      1    arg           false
  //   cal due y                  12  today  today            true
  //   cal MM YYYY     monthsPerLine    arg    arg           false
  //   cal MM YYYY y              12    arg    arg           false

  if (argWholeYear || (argYear && !argMonth && !argWholeYear)) monthsToDisplay = 12;

  if (!argMonth && argYear)
    mFrom = 1;
  else if (argMonth && argYear)
    mFrom = argMonth;

  if (argYear) yFrom = argYear;

  // Now begin the data subset and rendering.
  if (getPendingDate == true) {
    // Find the oldest pending due date.
    Datetime oldest(9999, 12, 31);
    for (auto& task : tasks) {
      auto status = task.getStatus();
      if (status == Task::pending || status == Task::waiting) {
        if (task.has("due") && !task.hasTag("nocal")) {
          Datetime d(task.get("due"));
          if (d < oldest) oldest = d;
        }
      }
    }

    // Default to current month if no due date is present
    if (oldest != Datetime(9999, 12, 31)) {
      mFrom = oldest.month();
      yFrom = oldest.year();
    }
  }

  if (config.getBoolean("calendar.offset")) {
    auto moffset = config.getInteger("calendar.offset.value") % 12;
    auto yoffset = config.getInteger("calendar.offset.value") / 12;
    mFrom += moffset;
    yFrom += yoffset;
    if (mFrom < 1) {
      mFrom += 12;
      yFrom--;
    } else if (mFrom > 12) {
      mFrom -= 12;
      yFrom++;
    }
  }

  mTo = mFrom + monthsToDisplay - 1;
  yTo = yFrom;
  if (mTo > 12) {
    mTo -= 12;
    yTo++;
  }

  auto details_yFrom = yFrom;
  auto details_mFrom = mFrom;

  std::stringstream out;
  out << '\n';

  while (yFrom < yTo || (yFrom == yTo && mFrom <= mTo)) {
    auto nextM = mFrom;
    auto nextY = yFrom;

    // Print month headers (cheating on the width settings, yes)
    for (int i = 0; i < monthsPerLine; i++) {
      auto month = Datetime::monthName(nextM);

      //    12345678901234567890123456 = 26 chars wide
      //                ^^             = center
      //    <------->                  = 13 - (month.length / 2) + 1
      //                      <------> = 26 - above
      //   +--------------------------+
      //   |         July 2009        |
      //   |     Mo Tu We Th Fr Sa Su |
      //   |  27        1  2  3  4  5 |
      //   |  28  6  7  8  9 10 11 12 |
      //   |  29 13 14 15 16 17 18 19 |
      //   |  30 20 21 22 23 24 25 26 |
      //   |  31 27 28 29 30 31       |
      //   +--------------------------+

      auto totalWidth = 26;
      auto labelWidth = month.length() + 5;  // 5 = " 2009"
      auto leftGap = (totalWidth / 2) - (labelWidth / 2);
      auto rightGap = totalWidth - leftGap - labelWidth;

      out << std::setw(leftGap) << ' ' << month << ' ' << nextY << std::setw(rightGap) << ' ';

      if (++nextM > 12) {
        nextM = 1;
        nextY++;
      }
    }

    out << '\n'
        << optionalBlankLine() << renderMonths(mFrom, yFrom, today, tasks, monthsPerLine) << '\n';

    mFrom += monthsPerLine;
    if (mFrom > 12) {
      mFrom -= 12;
      ++yFrom;
    }
  }

  Color color_today(config.get("color.calendar.today"));
  Color color_due(config.get("color.calendar.due"));
  Color color_duetoday(config.get("color.calendar.due.today"));
  Color color_overdue(config.get("color.calendar.overdue"));
  Color color_weekend(config.get("color.calendar.weekend"));
  Color color_holiday(config.get("color.calendar.holiday"));
  Color color_scheduled(config.get("color.calendar.scheduled"));
  Color color_weeknumber(config.get("color.calendar.weeknumber"));

  if (Context::getContext().color() && config.getBoolean("calendar.legend")) {
    out << "Legend: " << color_today.colorize("today") << ", " << color_weekend.colorize("weekend")
        << ", ";

    // If colorizing due dates, print legend
    if (config.get("calendar.details") != "none")
      out << color_due.colorize("due") << ", " << color_duetoday.colorize("due-today") << ", "
          << color_overdue.colorize("overdue") << ", " << color_scheduled.colorize("scheduled")
          << ", ";

    // If colorizing holidays, print legend
    if (config.get("calendar.holidays") != "none") out << color_holiday.colorize("holiday") << ", ";

    out << color_weeknumber.colorize("weeknumber") << '.' << optionalBlankLine() << '\n';
  }

  if (config.get("calendar.details") == "full" || config.get("calendar.holidays") == "full") {
    --details_mFrom;
    if (details_mFrom == 0) {
      details_mFrom = 12;
      --details_yFrom;
    }
    int details_dFrom = Datetime::daysInMonth(details_yFrom, details_mFrom);

    ++mTo;
    if (mTo == 13) {
      mTo = 1;
      ++yTo;
    }

    Datetime date_after(details_yFrom, details_mFrom, details_dFrom);
    auto after = date_after.toString(config.get("dateformat"));

    Datetime date_before(yTo, mTo, 1);
    auto before = date_before.toString(config.get("dateformat"));

    // Table with due date information
    if (config.get("calendar.details") == "full") {
      // Assert that 'report' is a valid report.
      auto report = config.get("calendar.details.report");
      if (Context::getContext().commands.find(report) == Context::getContext().commands.end())
        throw std::string(
            "The setting 'calendar.details.report' must contain a single report name.");

      // TODO Fix this:  cal      --> task
      //                 calendar --> taskendar

      // If the executable was "cal" or equivalent, replace it with "task".
      auto executable = Context::getContext().cli2._original_args[0].attribute("raw");
      auto cal = executable.find("cal");
      if (cal != std::string::npos) executable = executable.substr(0, cal) + PACKAGE;

      std::vector<std::string> args;
      args.push_back("rc:" + Context::getContext().rc_file._data);
      args.push_back("rc.due:0");
      args.push_back("rc.verbose:label,affected,blank");
      if (Context::getContext().color()) args.push_back("rc._forcecolor:on");
      args.push_back("due.after:" + after);
      args.push_back("due.before:" + before);
      args.push_back("-nocal");
      args.push_back(report);

      std::string output;
      ::execute(executable, args, "", output);
      out << output;
    }

    // Table with holiday information
    if (config.get("calendar.holidays") == "full") {
      Table holTable;
      holTable.width(Context::getContext().getWidth());
      holTable.add("Date");
      holTable.add("Holiday");
      setHeaderUnderline(holTable);

      auto dateFormat = config.get("dateformat.holiday");

      std::map<time_t, std::vector<std::string>> hm;  // we need to store multiple holidays per day
      for (auto& it : config)
        if (it.first.substr(0, 8) == "holiday.")
          if (it.first.substr(it.first.size() - 4) == "name") {
            auto holName = it.second;
            auto date = config.get("holiday." + it.first.substr(8, it.first.size() - 13) + ".date");
            auto start =
                config.get("holiday." + it.first.substr(8, it.first.size() - 13) + ".start");
            auto end = config.get("holiday." + it.first.substr(8, it.first.size() - 13) + ".end");
            if (!date.empty()) {
              Datetime holDate(date.c_str(), dateFormat);

              if (date_after < holDate && holDate < date_before)
                hm[holDate.toEpoch()].push_back(holName);
            }
            if (!start.empty() && !end.empty()) {
              Datetime holStart(start.c_str(), dateFormat);
              Datetime holEnd(end.c_str(), dateFormat);

              if (date_after < holStart && holStart < date_before)
                hm[holStart.toEpoch()].push_back("Start of " + holName);
              if (date_after < holEnd && holEnd < date_before)
                hm[holEnd.toEpoch()].push_back("End of " + holName);
            }
          }

      auto format = config.get("report." + config.get("calendar.details.report") + ".dateformat");
      if (format == "") format = config.get("dateformat.report");
      if (format == "") format = config.get("dateformat");

      for (auto& hm_it : hm) {
        auto v = hm_it.second;
        Datetime hDate(hm_it.first);
        auto d = hDate.toString(format);
        for (const auto& i : v) {
          auto row = holTable.addRow();
          holTable.set(row, 0, d);
          holTable.set(row, 1, i);
        }
      }

      out << optionalBlankLine() << holTable.render() << '\n';
    }
  }

  output = out.str();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdCalendar::renderMonths(int firstMonth, int firstYear, const Datetime& today,
                                      std::vector<Task>& all, int monthsPerLine) {
  auto& config = Context::getContext().config;
  auto weekStart = Datetime::weekstart;

  // Build table for the number of months to be displayed.
  Table view;
  setHeaderUnderline(view);
  view.width(Context::getContext().getWidth());
  for (int i = 0; i < (monthsPerLine * 8); i += 8) {
    if (weekStart == 1) {
      view.add("", false);
      view.add(utf8_substr(Datetime::dayName(1), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(2), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(3), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(4), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(5), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(6), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(0), 0, 2), false);
    } else {
      view.add("", false);
      view.add(utf8_substr(Datetime::dayName(0), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(1), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(2), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(3), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(4), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(5), 0, 2), false);
      view.add(utf8_substr(Datetime::dayName(6), 0, 2), false);
    }
  }

  // At most, we need 6 rows.
  view.addRow();
  view.addRow();
  view.addRow();
  view.addRow();
  view.addRow();
  view.addRow();

  // Set number of days per month, months to render, and years to render.
  std::vector<int> years;
  std::vector<int> months;
  std::vector<int> daysInMonth;
  int thisYear = firstYear;
  int thisMonth = firstMonth;
  for (int i = 0; i < monthsPerLine; i++) {
    if (thisMonth < 13) {
      years.push_back(thisYear);
    } else {
      thisMonth -= 12;
      years.push_back(++thisYear);
    }
    months.push_back(thisMonth);
    daysInMonth.push_back(Datetime::daysInMonth(thisYear, thisMonth++));
  }

  auto row = 0;

  Color color_today(config.get("color.calendar.today"));
  Color color_due(config.get("color.calendar.due"));
  Color color_duetoday(config.get("color.calendar.due.today"));
  Color color_overdue(config.get("color.calendar.overdue"));
  Color color_weekend(config.get("color.calendar.weekend"));
  Color color_holiday(config.get("color.calendar.holiday"));
  Color color_scheduled(config.get("color.calendar.scheduled"));
  Color color_weeknumber(config.get("color.calendar.weeknumber"));

  // Loop through months to be added on this line.
  for (int mpl = 0; mpl < monthsPerLine; mpl++) {
    // Reset row counter for subsequent months
    if (mpl != 0) row = 0;

    // Loop through days in month and add to table.
    for (int d = 1; d <= daysInMonth[mpl]; ++d) {
      Datetime date(years[mpl], months[mpl], d);
      auto dow = date.dayOfWeek();
      auto woy = date.week();

      if (config.getBoolean("displayweeknumber"))
        view.set(row, (8 * mpl),
                 // Make sure the week number is always 4 columns, space-padded.
                 format((woy < 10 ? "   {1}" : "  {1}"), woy), color_weeknumber);

      // Calculate column id.
      auto thisCol = dow +                       // 0 = Sunday
                     (weekStart == 1 ? 0 : 1) +  // Offset for weekStart
                     (8 * mpl);                  // Columns in 1 month

      if (thisCol == (8 * mpl)) thisCol += 7;

      view.set(row, thisCol, d);

      if (Context::getContext().color()) {
        Color cellColor;

        // colorize weekends
        if (dow == 0 || dow == 6) cellColor.blend(color_weekend);

        // colorize holidays
        if (config.get("calendar.holidays") != "none") {
          auto dateFormat = config.get("dateformat.holiday");
          for (auto& hol : config) {
            if (hol.first.substr(0, 8) == "holiday.") {
              if (hol.first.substr(hol.first.size() - 4) == "date") {
                auto value = hol.second;
                Datetime holDate(value.c_str(), dateFormat);
                if (holDate.sameDay(date)) cellColor.blend(color_holiday);
              }

              if (hol.first.substr(hol.first.size() - 5) == "start" &&
                  config.has("holiday." + hol.first.substr(8, hol.first.size() - 14) + ".end")) {
                auto start = hol.second;
                auto end =
                    config.get("holiday." + hol.first.substr(8, hol.first.size() - 14) + ".end");
                Datetime holStart(start.c_str(), dateFormat);
                Datetime holEnd(end.c_str(), dateFormat);
                if (holStart <= date && date <= holEnd) cellColor.blend(color_holiday);
              }
            }
          }
        }

        // colorize today
        if (today.sameDay(date)) cellColor.blend(color_today);

        // colorize due and scheduled tasks
        if (config.get("calendar.details") != "none") {
          config.set("due", 0);
          config.set("scheduled", 0);
          // if a date has a task that is due on that day, the due color
          // takes precedence over the scheduled color
          bool coloredWithDue = false;
          for (auto& task : all) {
            auto status = task.getStatus();
            if ((status == Task::pending || status == Task::waiting) && !task.hasTag("nocal")) {
              if (task.has("scheduled") && !coloredWithDue) {
                std::string scheduled = task.get("scheduled");
                Datetime scheduleddmy(strtoll(scheduled.c_str(), nullptr, 10));

                if (scheduleddmy.sameDay(date)) {
                  cellColor.blend(color_scheduled);
                }
              }
              if (task.has("due")) {
                std::string due = task.get("due");
                Datetime duedmy(strtoll(due.c_str(), nullptr, 10));

                if (duedmy.sameDay(date)) {
                  coloredWithDue = true;
                  switch (task.getDateState("due")) {
                    case Task::dateNotDue:
                      break;

                    case Task::dateAfterToday:
                      cellColor.blend(color_due);
                      break;

                    case Task::dateLaterToday:
                      cellColor.blend(color_duetoday);
                      break;

                    case Task::dateEarlierToday:
                    case Task::dateBeforeToday:
                      cellColor.blend(color_overdue);
                      break;
                  }
                }
              }
            }
          }
        }

        view.set(row, thisCol, cellColor);
      }

      // Check for end of week, and...
      int eow = 6;
      if (weekStart == 1) eow = 0;
      if (dow == eow && d < daysInMonth[mpl]) row++;
    }
  }

  return view.render();
}

////////////////////////////////////////////////////////////////////////////////
