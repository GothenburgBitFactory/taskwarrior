////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2020, Paul Beckingham, Federico Hernandez.
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
#include <CmdCalendar.h>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <Context.h>
#include <Table.h>
#include <Lexer.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include <utf8.h>
#include <main.h>

////////////////////////////////////////////////////////////////////////////////
CmdCalendar::CmdCalendar ()
{
  _keyword               = "calendar";
  _usage                 = "task          calendar [due|<month> <year>|<year>] [y]";
  _description           = "Shows a calendar, with due tasks marked";
  _read_only             = true;
  _displays_id           = true;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::graphs;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCalendar::execute (std::string& output)
{
  int rc = 0;

  // Each month requires 28 text columns width.  See how many will actually
  // fit.  But if a preference is specified, and it fits, use it.
  auto width = Context::getContext ().getWidth ();
  auto preferredMonthsPerLine = Context::getContext ().config.getInteger ("monthsperline");
  auto monthsThatFit = width / 26;

  auto monthsPerLine = monthsThatFit;
  if (preferredMonthsPerLine != 0 && preferredMonthsPerLine < monthsThatFit)
    monthsPerLine = preferredMonthsPerLine;

  // Load the pending tasks.
  handleUntil ();
  handleRecurrence ();
  auto tasks = Context::getContext ().tdb2.pending.get_tasks ();

  Datetime today;
  auto getPendingDate = false;
  auto monthsToDisplay = 1;
  auto mFrom = today.month ();
  auto yFrom = today.year ();
  auto mTo = mFrom;
  auto yTo = yFrom;

  // Defaults.
  monthsToDisplay = monthsPerLine;
  mFrom = today.month ();
  yFrom = today.year ();

  // Set up a vector of commands (1), for autoComplete.
  std::vector <std::string> commandNames {"calendar"};

  // Set up a vector of keywords, for autoComplete.
  std::vector <std::string> keywordNames {"due"};

  // Set up a vector of months, for autoComplete.
  std::vector <std::string> monthNames;
  for (int i = 1; i <= 12; ++i)
    monthNames.push_back (Lexer::lowerCase (Datetime::monthName (i)));

  // For autoComplete results.
  std::vector <std::string> matches;

  // Look at all args, regardless of sequence.
  auto argMonth = 0;
  auto argYear = 0;
  auto argWholeYear = false;

  for (auto& arg : Context::getContext ().cli2.getWords ())
  {
    // Some version of "calendar".
    if (autoComplete (Lexer::lowerCase (arg), commandNames, matches, Context::getContext ().config.getInteger ("abbreviation.minimum")) == 1)
      continue;

    // "due".
    else if (autoComplete (Lexer::lowerCase (arg), keywordNames, matches, Context::getContext ().config.getInteger ("abbreviation.minimum")) == 1)
      getPendingDate = true;

    // "y".
    else if (Lexer::lowerCase (arg) == "y")
      argWholeYear = true;

    // YYYY.
    else if (Lexer::isAllDigits (arg) && arg.length () == 4)
      argYear = strtol (arg.c_str (), nullptr, 10);

    // MM.
    else if (Lexer::isAllDigits (arg) && arg.length () <= 2)
    {
      argMonth = strtol (arg.c_str (), nullptr, 10);
      if (argMonth < 1 || argMonth > 12)
        throw format ("Argument '{1}' is not a valid month.", arg);
    }

    // "January" etc.
    else if (autoComplete (Lexer::lowerCase (arg), monthNames, matches, Context::getContext ().config.getInteger ("abbreviation.minimum")) == 1)
    {
      argMonth = Datetime::monthOfYear (matches[0]);
      if (argMonth == -1)
        throw format ("Argument '{1}' is not a valid month.", arg);
    }

    else
      throw format ("Could not recognize argument '{1}'.", arg);
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

  if (argWholeYear || (argYear && !argMonth && !argWholeYear))
    monthsToDisplay = 12;

  if (!argMonth && argYear)
    mFrom = 1;
  else if (argMonth && argYear)
    mFrom = argMonth;

  if (argYear)
    yFrom = argYear;

  // Now begin the data subset and rendering.
  auto countDueDates = 0;
  if (getPendingDate == true)
  {
    // Find the oldest pending due date.
    Datetime oldest (2037, 12, 31);
    for (auto& task : tasks)
    {
      if (task.getStatus () == Task::pending)
      {
        if (task.has ("due") &&
            !task.hasTag ("nocal"))
        {
          ++countDueDates;
          Datetime d (task.get ("due"));
          if (d < oldest) oldest = d;
        }
      }
    }
    mFrom = oldest.month();
    yFrom = oldest.year();
  }

  if (Context::getContext ().config.getBoolean ("calendar.offset"))
  {
    auto moffset = Context::getContext ().config.getInteger ("calendar.offset.value") % 12;
    auto yoffset = Context::getContext ().config.getInteger ("calendar.offset.value") / 12;
    mFrom += moffset;
    yFrom += yoffset;
    if (mFrom < 1)
    {
      mFrom += 12;
      yFrom--;
    }
    else if (mFrom > 12)
    {
      mFrom -= 12;
      yFrom++;
    }
  }

  mTo = mFrom + monthsToDisplay - 1;
  yTo = yFrom;
  if (mTo > 12)
  {
    mTo -= 12;
    yTo++;
  }

  auto details_yFrom = yFrom;
  auto details_mFrom = mFrom;

  std::stringstream out;
  out << '\n';

  while (yFrom < yTo || (yFrom == yTo && mFrom <= mTo))
  {
    auto nextM = mFrom;
    auto nextY = yFrom;

    // Print month headers (cheating on the width settings, yes)
    for (int i = 0 ; i < monthsPerLine ; i++)
    {
      auto month = Datetime::monthName (nextM);

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
      auto labelWidth = month.length () + 5;  // 5 = " 2009"
      auto leftGap = (totalWidth / 2) - (labelWidth / 2);
      auto rightGap = totalWidth - leftGap - labelWidth;

      out << std::setw (leftGap) << ' '
          << month
          << ' '
          << nextY
          << std::setw (rightGap) << ' ';

      if (++nextM > 12)
      {
        nextM = 1;
        nextY++;
      }
    }

    out << '\n'
        << optionalBlankLine ()
        << renderMonths (mFrom, yFrom, today, tasks, monthsPerLine)
        << '\n';

    mFrom += monthsPerLine;
    if (mFrom > 12)
    {
      mFrom -= 12;
      ++yFrom;
    }
  }

  Color color_today      (Context::getContext ().config.get ("color.calendar.today"));
  Color color_due        (Context::getContext ().config.get ("color.calendar.due"));
  Color color_duetoday   (Context::getContext ().config.get ("color.calendar.due.today"));
  Color color_overdue    (Context::getContext ().config.get ("color.calendar.overdue"));
  Color color_weekend    (Context::getContext ().config.get ("color.calendar.weekend"));
  Color color_holiday    (Context::getContext ().config.get ("color.calendar.holiday"));
  Color color_weeknumber (Context::getContext ().config.get ("color.calendar.weeknumber"));

  if (Context::getContext ().color () && Context::getContext ().config.getBoolean ("calendar.legend"))
    out << "Legend: "
        << color_today.colorize ("today")
        << ", "
        << color_due.colorize ("due")
        << ", "
        << color_duetoday.colorize ("due-today")
        << ", "
        << color_overdue.colorize ("overdue")
        << ", "
        << color_weekend.colorize ("weekend")
        << ", "
        << color_holiday.colorize ("holiday")
        << ", "
        << color_weeknumber.colorize ("weeknumber")
        << '.'
        << optionalBlankLine ()
        << '\n';

  if (Context::getContext ().config.get ("calendar.details") == "full" || Context::getContext ().config.get ("calendar.holidays") == "full")
  {
    --details_mFrom;
    if (details_mFrom == 0)
    {
      details_mFrom = 12;
      --details_yFrom;
    }
    int details_dFrom = Datetime::daysInMonth (details_yFrom, details_mFrom);

    ++mTo;
    if (mTo == 13)
    {
      mTo = 1;
      ++yTo;
    }

    Datetime date_after (details_yFrom, details_mFrom, details_dFrom);
    auto after = date_after.toString (Context::getContext ().config.get ("dateformat"));

    Datetime date_before (yTo, mTo, 1);
    auto before = date_before.toString (Context::getContext ().config.get ("dateformat"));

    // Table with due date information
    if (Context::getContext ().config.get ("calendar.details") == "full")
    {
      // Assert that 'report' is a valid report.
      auto report = Context::getContext ().config.get ("calendar.details.report");
      if (Context::getContext ().commands.find (report) == Context::getContext ().commands.end ())
        throw std::string ("The setting 'calendar.details.report' must contain a single report name.");

      // TODO Fix this:  cal      --> task
      //                 calendar --> taskendar

      // If the executable was "cal" or equivalent, replace it with "task".
      auto executable = Context::getContext ().cli2._original_args[0].attribute ("raw");
      auto cal = executable.find ("cal");
      if (cal != std::string::npos)
        executable = executable.substr (0, cal) + PACKAGE;

      std::vector <std::string> args;
      args.push_back ("rc:" + Context::getContext ().rc_file._data);
      args.push_back ("rc.due:0");
      args.push_back ("rc.verbose:label,affected,blank");
      if (Context::getContext ().color ())
          args.push_back ("rc._forcecolor:on");
      args.push_back ("due.after:" + after);
      args.push_back ("due.before:" + before);
      args.push_back ("-nocal");
      args.push_back (report);

      std::string output;
      ::execute (executable, args, "", output);
      out << output;
    }

    // Table with holiday information
    if (Context::getContext ().config.get ("calendar.holidays") == "full")
    {
      Table holTable;
      holTable.width (Context::getContext ().getWidth ());
      holTable.add ("Date");
      holTable.add ("Holiday");
      setHeaderUnderline (holTable);

      std::map <time_t, std::vector<std::string>> hm; // we need to store multiple holidays per day
      for (auto& it : Context::getContext ().config)
        if (it.first.substr (0, 8) == "holiday.")
          if (it.first.substr (it.first.size () - 4) == "name")
          {
            auto holName = Context::getContext ().config.get ("holiday." + it.first.substr (8, it.first.size () - 13) + ".name");
            auto holDate = Context::getContext ().config.get ("holiday." + it.first.substr (8, it.first.size () - 13) + ".date");
            Datetime hDate (holDate.c_str (), Context::getContext ().config.get ("dateformat.holiday"));

            if (date_after < hDate && hDate < date_before)
              hm[hDate.toEpoch()].push_back (holName);
          }

      auto format = Context::getContext ().config.get ("report." +
                                        Context::getContext ().config.get ("calendar.details.report") +
                                        ".dateformat");
      if (format == "")
        format = Context::getContext ().config.get ("dateformat.report");
      if (format == "")
        format = Context::getContext ().config.get ("dateformat");

      for (auto& hm_it : hm)
      {
        auto v = hm_it.second;
        Datetime hDate (hm_it.first);
        auto d = hDate.toString (format);
        for (size_t i = 0; i < v.size(); i++)
        {
          auto row = holTable.addRow ();
          holTable.set (row, 0, d);
          holTable.set (row, 1, v[i]);
        }
      }

      out << optionalBlankLine ()
          << holTable.render ()
          << '\n';
    }
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdCalendar::renderMonths (
  int firstMonth,
  int firstYear,
  const Datetime& today,
  std::vector <Task>& all,
  int monthsPerLine)
{
  // What day of the week does the user consider the first?
  auto weekStart = Datetime::dayOfWeek (Context::getContext ().config.get ("weekstart"));
  if (weekStart != 0 && weekStart != 1)
    throw std::string ("The 'weekstart' configuration variable may only contain 'Sunday' or 'Monday'.");

  // Build table for the number of months to be displayed.
  Table view;
  setHeaderUnderline (view);
  view.width (Context::getContext ().getWidth ());
  for (int i = 0 ; i < (monthsPerLine * 8); i += 8)
  {
    if (weekStart == 1)
    {
      view.add ("", false);
      view.add (utf8_substr (Datetime::dayName (1), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (2), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (3), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (4), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (5), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (6), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (0), 0, 2), false);
    }
    else
    {
      view.add ("", false);
      view.add (utf8_substr (Datetime::dayName (0), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (1), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (2), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (3), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (4), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (5), 0, 2), false);
      view.add (utf8_substr (Datetime::dayName (6), 0, 2), false);
    }
  }

  // At most, we need 6 rows.
  view.addRow ();
  view.addRow ();
  view.addRow ();
  view.addRow ();
  view.addRow ();
  view.addRow ();

  // Set number of days per month, months to render, and years to render.
  std::vector<int> years;
  std::vector<int> months;
  std::vector<int> daysInMonth;
  int thisYear = firstYear;
  int thisMonth = firstMonth;
  for (int i = 0 ; i < monthsPerLine ; i++)
  {
    if (thisMonth < 13)
    {
      years.push_back (thisYear);
    }
    else
    {
      thisMonth -= 12;
      years.push_back (++thisYear);
    }
    months.push_back (thisMonth);
    daysInMonth.push_back (Datetime::daysInMonth (thisYear, thisMonth++));
  }

  auto row = 0;

  Color color_today      (Context::getContext ().config.get ("color.calendar.today"));
  Color color_due        (Context::getContext ().config.get ("color.calendar.due"));
  Color color_duetoday   (Context::getContext ().config.get ("color.calendar.due.today"));
  Color color_overdue    (Context::getContext ().config.get ("color.calendar.overdue"));
  Color color_weekend    (Context::getContext ().config.get ("color.calendar.weekend"));
  Color color_holiday    (Context::getContext ().config.get ("color.calendar.holiday"));
  Color color_weeknumber (Context::getContext ().config.get ("color.calendar.weeknumber"));

  // Loop through months to be added on this line.
  for (int mpl = 0; mpl < monthsPerLine ; mpl++)
  {
    // Reset row counter for subsequent months
    if (mpl != 0)
      row = 0;

    // Loop through days in month and add to table.
    for (int d = 1; d <= daysInMonth[mpl]; ++d)
    {
      Datetime temp (years[mpl], months[mpl], d);
      auto dow = temp.dayOfWeek ();
      auto woy = temp.week ();

      if (Context::getContext ().config.getBoolean ("displayweeknumber"))
        view.set (row,
                  (8 * mpl),
                  // Make sure the week number is always 4 columns, space-padded.
                  format ((woy < 10 ? "   {1}" : "  {1}"), woy),
                  color_weeknumber);

      // Calculate column id.
      auto thisCol = dow +                       // 0 = Sunday
                     (weekStart == 1 ? 0 : 1) +  // Offset for weekStart
                     (8 * mpl);                  // Columns in 1 month

      if (thisCol == (8 * mpl))
        thisCol += 7;

      view.set (row, thisCol, d);

      if (Context::getContext ().color ())
      {
        Color cellColor;

        // colorize weekends
        if (dow == 0 || dow == 6)
          cellColor.blend (color_weekend);

        // colorize holidays
        if (Context::getContext ().config.get ("calendar.holidays") != "none")
        {
          for (auto& hol : Context::getContext ().config)
            if (hol.first.substr (0, 8) == "holiday.")
              if (hol.first.substr (hol.first.size () - 4) == "date")
              {
                std::string value = hol.second;
                Datetime holDate (value.c_str (), Context::getContext ().config.get ("dateformat.holiday"));
                if (holDate.day   () == d           &&
                    holDate.month () == months[mpl] &&
                    holDate.year  () == years[mpl])
                  cellColor.blend (color_holiday);
              }
        }

        // colorize today
        if (today.day   () == d                &&
            today.month () == months.at (mpl)  &&
            today.year  () == years.at  (mpl))
          cellColor.blend (color_today);

        // colorize due tasks
        if (Context::getContext ().config.get ("calendar.details") != "none")
        {
          Context::getContext ().config.set ("due", 0);
          for (auto& task : all)
          {
            if (task.getStatus () == Task::pending &&
                !task.hasTag ("nocal")             &&
                task.has ("due"))
            {
              std::string due = task.get ("due");
              Datetime duedmy (strtol (due.c_str(), nullptr, 10));

              if (duedmy.day   () == d           &&
                  duedmy.month () == months[mpl] &&
                  duedmy.year  () == years[mpl])
              {
                switch (task.getDateState ("due"))
                {
                case Task::dateNotDue:
                  break;

                case Task::dateAfterToday:
                  cellColor.blend (color_due);
                  break;

                case Task::dateEarlierToday:
                case Task::dateLaterToday:
                  cellColor.blend (color_duetoday);
                  cellColor.blend (color_duetoday);
                  break;

                case Task::dateBeforeToday:
                  cellColor.blend (color_overdue);
                  break;
                }
              }
            }
          }
        }

        view.set (row, thisCol, cellColor);
      }

      // Check for end of week, and...
      int eow = 6;
      if (weekStart == 1)
        eow = 0;
      if (dow == eow && d < daysInMonth[mpl])
        row++;
    }
  }

  return view.render ();
}

////////////////////////////////////////////////////////////////////////////////
