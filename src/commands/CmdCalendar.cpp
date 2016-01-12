////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <CmdCalendar.h>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <Lexer.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <utf8.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCalendar::CmdCalendar ()
{
  _keyword               = "calendar";
  _usage                 = "task          calendar [due|<month> <year>|<year>] [y]";
  _description           = STRING_CMD_CAL_USAGE;
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
  int width = context.getWidth ();
  int preferredMonthsPerLine = (context.config.getInteger ("monthsperline"));
  int monthsThatFit = width / 26;

  int monthsPerLine = monthsThatFit;
  if (preferredMonthsPerLine != 0 && preferredMonthsPerLine < monthsThatFit)
    monthsPerLine = preferredMonthsPerLine;

  // Load the pending tasks.
  handleRecurrence ();
  auto tasks = context.tdb2.pending.get_tasks ();

  ISO8601d today;
  bool getpendingdate = false;
  int monthsToDisplay = 1;
  int mFrom = today.month ();
  int yFrom = today.year ();
  int mTo = mFrom;
  int yTo = yFrom;

  // Defaults.
  monthsToDisplay = monthsPerLine;
  mFrom = today.month ();
  yFrom = today.year ();

  // Set up a vector of commands (1), for autoComplete.
  std::vector <std::string> commandNames;
  commandNames.push_back ("calendar");

  // Set up a vector of keywords, for autoComplete.
  std::vector <std::string> keywordNames;
  keywordNames.push_back ("due");

  // Set up a vector of months, for autoComplete.
  std::vector <std::string> monthNames;
  for (int i = 1; i <= 12; ++i)
    monthNames.push_back (Lexer::lowerCase (ISO8601d::monthName (i)));

  // For autoComplete results.
  std::vector <std::string> matches;

  // Look at all args, regardless of sequence.
  int argMonth = 0;
  int argYear = 0;
  bool argWholeYear = false;

  std::vector <std::string> words = context.cli2.getWords ();

  for (auto& arg : words)
  {
    // Some version of "calendar".
    if (autoComplete (Lexer::lowerCase (arg), commandNames, matches, context.config.getInteger ("abbreviation.minimum")) == 1)
      continue;

    // "due".
    else if (autoComplete (Lexer::lowerCase (arg), keywordNames, matches, context.config.getInteger ("abbreviation.minimum")) == 1)
      getpendingdate = true;

    // "y".
    else if (Lexer::lowerCase (arg) == "y")
      argWholeYear = true;

    // YYYY.
    else if (Lexer::isAllDigits (arg) && arg.length () == 4)
      argYear = strtol (arg.c_str (), NULL, 10);

    // MM.
    else if (Lexer::isAllDigits (arg) && arg.length () <= 2)
    {
      argMonth = strtol (arg.c_str (), NULL, 10);
      if (argMonth < 1 || argMonth > 12)
        throw format (STRING_CMD_CAL_BAD_MONTH, arg);
    }

    // "January" etc.
    else if (autoComplete (Lexer::lowerCase (arg), monthNames, matches, context.config.getInteger ("abbreviation.minimum")) == 1)
    {
      argMonth = ISO8601d::monthOfYear (matches[0]);
      if (argMonth == -1)
        throw format (STRING_CMD_CAL_BAD_MONTH, arg);
    }

    else
      throw format (STRING_CMD_CAL_BAD_ARG, arg);
  }

  // Supported combinations:
  //
  //   Command line  monthsToDisplay  mFrom  yFrom  getpendingdate
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
  int countDueDates = 0;
  if (getpendingdate == true)
  {
    // Find the oldest pending due date.
    ISO8601d oldest (12, 31, 2037);
    for (auto& task : tasks)
    {
      if (task.getStatus () == Task::pending)
      {
        if (task.has ("due") &&
            !task.hasTag ("nocal"))
        {
          ++countDueDates;
          ISO8601d d (task.get ("due"));
          if (d < oldest) oldest = d;
        }
      }
    }
    mFrom = oldest.month();
    yFrom = oldest.year();
  }

  if (context.config.getBoolean ("calendar.offset"))
  {
    int moffset = context.config.getInteger ("calendar.offset.value") % 12;
    int yoffset = context.config.getInteger ("calendar.offset.value") / 12;
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

  int details_yFrom = yFrom;
  int details_mFrom = mFrom;

  std::stringstream out;
  out << "\n";

  while (yFrom < yTo || (yFrom == yTo && mFrom <= mTo))
  {
    int nextM = mFrom;
    int nextY = yFrom;

    // Print month headers (cheating on the width settings, yes)
    for (int i = 0 ; i < monthsPerLine ; i++)
    {
      std::string month = ISO8601d::monthName (nextM);

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

      int totalWidth = 26;
      int labelWidth = month.length () + 5;  // 5 = " 2009"
      int leftGap = (totalWidth / 2) - (labelWidth / 2);
      int rightGap = totalWidth - leftGap - labelWidth;

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

    out << "\n"
        << optionalBlankLine ()
        << renderMonths (mFrom, yFrom, today, tasks, monthsPerLine)
        << "\n";

    mFrom += monthsPerLine;
    if (mFrom > 12)
    {
      mFrom -= 12;
      ++yFrom;
    }
  }

  Color color_today      (context.config.get ("color.calendar.today"));
  Color color_due        (context.config.get ("color.calendar.due"));
  Color color_duetoday   (context.config.get ("color.calendar.due.today"));
  Color color_overdue    (context.config.get ("color.calendar.overdue"));
  Color color_weekend    (context.config.get ("color.calendar.weekend"));
  Color color_holiday    (context.config.get ("color.calendar.holiday"));
  Color color_weeknumber (context.config.get ("color.calendar.weeknumber"));
  Color color_label      (context.config.get ("color.label"));

  if (context.color () && context.config.getBoolean ("calendar.legend"))
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
        << "."
        << optionalBlankLine ()
        << "\n";

  if (context.config.get ("calendar.details") == "full" || context.config.get ("calendar.holidays") == "full")
  {
    --details_mFrom;
    if (details_mFrom == 0)
    {
      details_mFrom = 12;
      --details_yFrom;
    }
    int details_dFrom = ISO8601d::daysInMonth (details_mFrom, details_yFrom);

    ++mTo;
    if (mTo == 13)
    {
      mTo = 1;
      ++yTo;
    }

    ISO8601d date_after (details_mFrom, details_dFrom, details_yFrom);
    std::string after = date_after.toString (context.config.get ("dateformat"));

    ISO8601d date_before (mTo, 1, yTo);
    std::string before = date_before.toString (context.config.get ("dateformat"));

    // Table with due date information
    if (context.config.get ("calendar.details") == "full")
    {
      // Assert that 'report' is a valid report.
      std::string report = context.config.get ("calendar.details.report");
      if (context.commands.find (report) == context.commands.end ())
        throw std::string (STRING_ERROR_DETAILS);

      // TODO Fix this:  cal      --> task
      //                 calendar --> taskendar

      // If the executable was "cal" or equivalent, replace it with "task".
      std::string executable = context.cli2._original_args[0].attribute ("raw");
      auto cal = executable.find ("cal");
      if (cal != std::string::npos)
        executable = executable.substr (0, cal) + PACKAGE;

      std::vector <std::string> args;
      args.push_back ("rc:" + context.rc_file._data);
      args.push_back ("rc.due:0");
      args.push_back ("rc.verbose:label,affected,blank");
      args.push_back ("due.after:" + after);
      args.push_back ("due.before:" + before);
      args.push_back ("-nocal");
      args.push_back (report);

      std::string output;
      ::execute (executable, args, "", output);
      out << output;
    }

    // Table with holiday information
    if (context.config.get ("calendar.holidays") == "full")
    {
      ViewText holTable;
      holTable.width (context.getWidth ());
      holTable.add (Column::factory ("string", STRING_CMD_CAL_LABEL_DATE));
      holTable.add (Column::factory ("string", STRING_CMD_CAL_LABEL_HOL));
      holTable.colorHeader (color_label);

      std::map <time_t, std::vector<std::string>> hm; // we need to store multiple holidays per day
      for (auto& it : context.config)
        if (it.first.substr (0, 8) == "holiday.")
          if (it.first.substr (it.first.size () - 4) == "name")
          {
            std::string holName = context.config.get ("holiday." + it.first.substr (8, it.first.size () - 13) + ".name");
            std::string holDate = context.config.get ("holiday." + it.first.substr (8, it.first.size () - 13) + ".date");
            ISO8601d hDate (holDate.c_str (), context.config.get ("dateformat.holiday"));

            if (date_after < hDate && hDate < date_before)
              hm[hDate.toEpoch()].push_back(holName);
          }

      std::string format = context.config.get ("report." +
                                               context.config.get ("calendar.details.report") +
                                               ".dateformat");
      if (format == "")
        format = context.config.get ("dateformat.report");
      if (format == "")
        format = context.config.get ("dateformat");

      for (auto& hm_it : hm)
      {
        std::vector <std::string> v = hm_it.second;
        ISO8601d hDate (hm_it.first);
        std::string d = hDate.toString (format);
        for (size_t i = 0; i < v.size(); i++)
        {
          int row = holTable.addRow ();
          holTable.set (row, 0, d);
          holTable.set (row, 1, v[i]);
        }
      }

      out << optionalBlankLine ()
          << holTable.render ()
          << "\n";
    }
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdCalendar::renderMonths (
  int firstMonth,
  int firstYear,
  const ISO8601d& today,
  std::vector <Task>& all,
  int monthsPerLine)
{
  // What day of the week does the user consider the first?
  int weekStart = ISO8601d::dayOfWeek (context.config.get ("weekstart"));
  if (weekStart != 0 && weekStart != 1)
    throw std::string (STRING_CMD_CAL_SUN_MON);

  // Build table for the number of months to be displayed.
  Color label (context.config.get ("color.label"));

  ViewText view;
  view.colorHeader (label);
  view.width (context.getWidth ());
  for (int i = 0 ; i < (monthsPerLine * 8); i += 8)
  {
    if (weekStart == 1)
    {
      view.add (Column::factory ("string.right", "    "));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (1), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (2), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (3), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (4), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (5), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (6), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (0), 0, 2)));
    }
    else
    {
      view.add (Column::factory ("string.right", "    "));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (0), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (1), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (2), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (3), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (4), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (5), 0, 2)));
      view.add (Column::factory ("string.right", utf8_substr (ISO8601d::dayName (6), 0, 2)));
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
    daysInMonth.push_back (ISO8601d::daysInMonth (thisMonth++, thisYear));
  }

  int row = 0;

  Color color_today      (context.config.get ("color.calendar.today"));
  Color color_due        (context.config.get ("color.calendar.due"));
  Color color_duetoday   (context.config.get ("color.calendar.due.today"));
  Color color_overdue    (context.config.get ("color.calendar.overdue"));
  Color color_weekend    (context.config.get ("color.calendar.weekend"));
  Color color_holiday    (context.config.get ("color.calendar.holiday"));
  Color color_weeknumber (context.config.get ("color.calendar.weeknumber"));

  // Loop through months to be added on this line.
  for (int mpl = 0; mpl < monthsPerLine ; mpl++)
  {
    // Reset row counter for subsequent months
    if (mpl != 0)
      row = 0;

    // Loop through days in month and add to table.
    for (int d = 1; d <= daysInMonth[mpl]; ++d)
    {
      ISO8601d temp (months[mpl], d, years[mpl]);
      int dow = temp.dayOfWeek ();
      int woy = temp.weekOfYear (weekStart);

      if (context.config.getBoolean ("displayweeknumber"))
        view.set (row, (8 * mpl), woy, color_weeknumber);

      // Calculate column id.
      int thisCol = dow +                       // 0 = Sunday
                    (weekStart == 1 ? 0 : 1) +  // Offset for weekStart
                    (8 * mpl);                  // Columns in 1 month

      if (thisCol == (8 * mpl))
        thisCol += 7;

      view.set (row, thisCol, d);

      if (context.color ())
      {
        Color cellColor;

        // colorize weekends
        if (dow == 0 || dow == 6)
          cellColor.blend (color_weekend);

        // colorize holidays
        if (context.config.get ("calendar.holidays") != "none")
        {
          for (auto& hol : context.config)
            if (hol.first.substr (0, 8) == "holiday.")
              if (hol.first.substr (hol.first.size () - 4) == "date")
              {
                std::string value = hol.second;
                ISO8601d holDate (value.c_str (), context.config.get ("dateformat.holiday"));
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
        if (context.config.get ("calendar.details") != "none")
        {
          context.config.set ("due", 0);
          for (auto& task : all)
          {
            if (task.getStatus () == Task::pending &&
                !task.hasTag ("nocal")             &&
                task.has ("due"))
            {
              std::string due = task.get ("due");
              ISO8601d duedmy (strtol (due.c_str(), NULL, 10));

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
