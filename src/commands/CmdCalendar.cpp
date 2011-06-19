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
#include <iomanip>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <main.h>
#include <CmdCalendar.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCalendar::CmdCalendar ()
{
  _keyword     = "calendar";
  _usage       = "task calendar [due|month year|year]";
  _description = "Shows a calendar, with due tasks marked.";
  _read_only   = true;
  _displays_id = true;
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

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.loadPending (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  Date today;
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
  monthNames.push_back ("january");
  monthNames.push_back ("february");
  monthNames.push_back ("march");
  monthNames.push_back ("april");
  monthNames.push_back ("may");
  monthNames.push_back ("june");
  monthNames.push_back ("july");
  monthNames.push_back ("august");
  monthNames.push_back ("september");
  monthNames.push_back ("october");
  monthNames.push_back ("november");
  monthNames.push_back ("december");

  // For autoComplete results.
  std::vector <std::string> matches;

  // Look at all args, regardless of sequence.
  int argMonth = 0;
  int argYear = 0;
  bool argWholeYear = false;
/*
  std::vector <std::string> args = context.args.list ();
  std::vector <std::string>::iterator arg;
  for (arg = args.begin (); arg != args.end (); ++arg)
  {
    // Some version of "calendar".
    if (autoComplete (lowerCase (*arg), commandNames, matches) == 1)
      continue;

    // "due".
    else if (autoComplete (lowerCase (*arg), keywordNames, matches) == 1)
      getpendingdate = true;

    // "y".
    else if (lowerCase (*arg) == "y")
      argWholeYear = true;

    // YYYY.
    else if (digitsOnly (*arg) && arg->length () == 4)
      argYear = strtol (arg->c_str (), NULL, 10);

    // MM.
    else if (digitsOnly (*arg) && arg->length () <= 2)
    {
      argMonth = strtol (arg->c_str (), NULL, 10);
      if (argMonth < 1 || argMonth > 12)
        throw std::string ("Argument '") + *arg + "' is not a valid month.";
    }

    // "January" etc.
    else if (autoComplete (lowerCase (*arg), monthNames, matches) == 1)
    {
      argMonth = Date::monthOfYear (matches[0]);
      if (argMonth == -1)
        throw std::string ("Argument '") + *arg + "' is not a valid month.";
    }

    else
      throw std::string ("Could not recognize argument '") + *arg + "'.";
  }
*/

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
    Date oldest (12,31,2037);
    std::vector <Task>::iterator task;
    for (task = filtered.begin (); task != filtered.end (); ++task)
    {
      if (task->getStatus () == Task::pending)
      {
        if (task->has ("due") &&
            !task->hasTag ("nocal"))
        {
          ++countDueDates;
          Date d (strtol (task->get ("due").c_str (), NULL, 10));
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
      std::string month = Date::monthName (nextM);

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
        << renderMonths (mFrom, yFrom, today, filtered, monthsPerLine)
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
    int details_dFrom = Date::daysInMonth (details_mFrom, details_yFrom);

    ++mTo;
    if (mTo == 13)
    {
      mTo = 1;
      ++yTo;
    }

    Date date_after (details_mFrom, details_dFrom, details_yFrom);
    std::string after = date_after.toString (context.config.get ("dateformat"));

    Date date_before (mTo, 1, yTo);
    std::string before = date_before.toString (context.config.get ("dateformat"));

    // Table with due date information
    if (context.config.get ("calendar.details") == "full")
    {
      std::string report = context.config.get ("calendar.details.report");
      std::string report_filter = context.config.get ("report." + report + ".filter");

      report_filter += " due.after:" + after + " due.before:" + before;
      context.config.set ("report." + report + ".filter", report_filter);

      // Display all due task in the report colorized not only the imminet ones
      context.config.set ("due", 0);

      std::string output;
      context.commands[report]->execute (output);
      out << output;
    }

    // Table with holiday information
    if (context.config.get ("calendar.holidays") == "full")
    {
      std::vector <std::string> holidays;
      context.config.all (holidays);

      ViewText holTable;
      holTable.width (context.getWidth ());
      holTable.add (Column::factory ("string", "Date"));
      holTable.add (Column::factory ("string", "Holiday"));

      std::vector <std::string>::iterator it;
      for (it = holidays.begin (); it != holidays.end (); ++it)
        if (it->substr (0, 8) == "holiday.")
          if (it->substr (it->size () - 4) == "name")
          {
            std::string holName = context.config.get ("holiday." + it->substr (8, it->size () - 13) + ".name");
            std::string holDate = context.config.get ("holiday." + it->substr (8, it->size () - 13) + ".date");
            Date hDate (holDate.c_str (), context.config.get ("dateformat.holiday"));

            if (date_after < hDate && hDate < date_before)
            {
              std::string format = context.config.get ("report." +
                                                       context.config.get ("calendar.details.report") +
                                                       ".dateformat");
              if (format == "")
                format = context.config.get ("dateformat.report");
              if (format == "")
                format = context.config.get ("dateformat");

              int row = holTable.addRow ();
              holTable.set (row, 0, hDate.toString (format));
              holTable.set (row, 1, holName);
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
  const Date& today,
  std::vector <Task>& all,
  int monthsPerLine)
{
  // What day of the week does the user consider the first?
  int weekStart = Date::dayOfWeek (context.config.get ("weekstart"));
  if (weekStart != 0 && weekStart != 1)
    throw std::string ("The 'weekstart' configuration variable may "
                       "only contain 'Sunday' or 'Monday'.");

  // Build table for the number of months to be displayed.
  ViewText view;
  view.width (context.getWidth ());
  for (int i = 0 ; i < (monthsPerLine * 8); i += 8)
  {
    if (weekStart == 1)
    {
      view.add (Column::factory ("string.right", "    "));
      view.add (Column::factory ("string.right", "Mo"));
      view.add (Column::factory ("string.right", "Tu"));
      view.add (Column::factory ("string.right", "We"));
      view.add (Column::factory ("string.right", "Th"));
      view.add (Column::factory ("string.right", "Fr"));
      view.add (Column::factory ("string.right", "Sa"));
      view.add (Column::factory ("string.right", "Su"));
    }
    else
    {
      view.add (Column::factory ("string.right", "    "));
      view.add (Column::factory ("string.right", "Su"));
      view.add (Column::factory ("string.right", "Mo"));
      view.add (Column::factory ("string.right", "Tu"));
      view.add (Column::factory ("string.right", "We"));
      view.add (Column::factory ("string.right", "Th"));
      view.add (Column::factory ("string.right", "Fr"));
      view.add (Column::factory ("string.right", "Sa"));
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
    daysInMonth.push_back (Date::daysInMonth (thisMonth++, thisYear));
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
      Date temp (months[mpl], d, years[mpl]);
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
          std::vector <std::string> holidays;
          context.config.all (holidays);
          std::vector <std::string>::iterator hol;
          for (hol = holidays.begin (); hol != holidays.end (); ++hol)
            if (hol->substr (0, 8) == "holiday.")
              if (hol->substr (hol->size () - 4) == "date")
              {
                std::string value = context.config.get (*hol);
                Date holDate (value.c_str (), context.config.get ("dateformat.holiday"));
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
          std::vector <Task>::iterator task;
          for (task = all.begin (); task != all.end (); ++task)
          {
            if (task->getStatus () == Task::pending &&
                !task->hasTag ("nocal")             &&
                task->has ("due"))
            {
              std::string due = task->get ("due");
              Date duedmy (strtol (due.c_str(), NULL, 10));

              if (duedmy.day   () == d           &&
                  duedmy.month () == months[mpl] &&
                  duedmy.year  () == years[mpl])
              {
                switch (getDueState (due))
                {
                  case 1: // imminent
                    cellColor.blend (color_due);
                    break;

                  case 2: // today
                    cellColor.blend (color_duetoday);
                    break;

                  case 3: // overdue
                    cellColor.blend (color_overdue);
                    break;

                  case 0: // not due at all
                  default:
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
