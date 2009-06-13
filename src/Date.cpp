////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <iostream>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include "Date.h"
#include "text.h"
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
// Defaults to "now".
Date::Date ()
{
  mT = time (NULL);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const time_t t)
{
  mT = t;
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const int m, const int d, const int y)
{
  // Error if not valid.
  struct tm t = {0};
  t.tm_mday = d;
  t.tm_mon = m - 1;
  t.tm_year = y - 1900;

  mT = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const std::string& mdy, const std::string& format /* = "m/d/Y" */)
{
  int month = 0;
  int day   = 0;
  int year  = 0;

  // Before parsing according to "format", perhaps this is a relative date?
  if (isRelativeDate (mdy))
    return;

  unsigned int i = 0; // Index into mdy.

  for (unsigned int f = 0; f < format.length (); ++f)
  {
    switch (format[f])
    {
    // Single or double digit.
    case 'm':
      if (i >= mdy.length () ||
          ! ::isdigit (mdy[i]))
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }

      if (i + 1 < mdy.length ()                                         &&
          (mdy[i + 0] == '0' || mdy[i + 0] == '1')                      &&
          ::isdigit (mdy[i + 1]))
      {
        month = ::atoi (mdy.substr (i, 2).c_str ());
        i += 2;
      }
      else
      {
        month = ::atoi (mdy.substr (i, 1).c_str ());
        ++i;
      }
      break;

    case 'd':
      if (i >= mdy.length () ||
          ! ::isdigit (mdy[i]))
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }

      if (i + 1 < mdy.length ()                                                              &&
          (mdy[i + 0] == '0' || mdy[i + 0] == '1' || mdy[i + 0] == '2' || mdy[i + 0] == '3') &&
          ::isdigit (mdy[i + 1]))
      {
        day = ::atoi (mdy.substr (i, 2).c_str ());
        i += 2;
      }
      else
      {
        day = ::atoi (mdy.substr (i, 1).c_str ());
        ++i;
      }
      break;

    // Double digit.
    case 'y':
      if (i + 1 >= mdy.length () ||
          ! ::isdigit (mdy[i + 0]) ||
          ! ::isdigit (mdy[i + 1]))
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }

      year = ::atoi (mdy.substr (i, 2).c_str ()) + 2000;
      i += 2;
      break;

    case 'M':
      if (i + 1 >= mdy.length () ||
          ! ::isdigit (mdy[i + 0]) ||
          ! ::isdigit (mdy[i + 1]))
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }

      month = ::atoi (mdy.substr (i, 2).c_str ());
      i += 2;
      break;

    case 'D':
      if (i + 1 >= mdy.length () ||
          ! ::isdigit (mdy[i + 0]) ||
          ! ::isdigit (mdy[i + 1]))
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }

      day = ::atoi (mdy.substr (i, 2).c_str ());
      i += 2;
      break;

    // Quadruple digit.
    case 'Y':
      if (i + 3 >= mdy.length () ||
          ! ::isdigit (mdy[i + 0]) ||
          ! ::isdigit (mdy[i + 1]) ||
          ! ::isdigit (mdy[i + 2]) ||
          ! ::isdigit (mdy[i + 3]))
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }

      year = ::atoi (mdy.substr (i, 4).c_str ());
      i += 4;
      break;

    default:
      if (i >= mdy.length () ||
          mdy[i] != format[f])
      {
        throw std::string ("\"") + mdy + "\" is not a valid date.";
      }
      ++i;
      break;
    }
  }

  if (!valid (month, day, year))
    throw std::string ("\"") + mdy + "\" is not a valid date.";

  // Duplicate Date::Date (const int, const int, const int);
  struct tm t = {0};
  t.tm_mday = day;
  t.tm_mon = month - 1;
  t.tm_year = year - 1900;

  mT = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const Date& rhs)
{
  mT = rhs.mT;
}

////////////////////////////////////////////////////////////////////////////////
Date::~Date ()
{
}

////////////////////////////////////////////////////////////////////////////////
time_t Date::toEpoch ()
{
  return mT;
}

////////////////////////////////////////////////////////////////////////////////
void Date::toEpoch (time_t& epoch)
{
  epoch = mT;
}

////////////////////////////////////////////////////////////////////////////////
void Date::toMDY (int& m, int& d, int& y)
{
  struct tm* t = localtime (&mT);

  m = t->tm_mon + 1;
  d = t->tm_mday;
  y = t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Date::toString (const std::string& format /*= "m/d/Y" */) const
{
  // Making this local copy seems to fix a bug.  Remove the local copy and you'll
  // see segmentation faults and all kinds of gibberish.
  std::string localFormat = format;

  char buffer[12];
  std::string formatted;
  for (unsigned int i = 0; i < localFormat.length (); ++i)
  {
    char c = localFormat[i];
    switch (c)
    {
    case 'm': sprintf (buffer, "%d",   this->month ());      break;
    case 'M': sprintf (buffer, "%02d", this->month ());      break;
    case 'd': sprintf (buffer, "%d",   this->day ());        break;
    case 'D': sprintf (buffer, "%02d", this->day ());        break;
    case 'y': sprintf (buffer, "%02d", this->year () % 100); break;
    case 'Y': sprintf (buffer, "%d",   this->year ());       break;
    default:  sprintf (buffer, "%c",   c);                   break;
    }

    formatted += buffer;
  }

  return formatted;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::valid (const std::string& input, const std::string& format)
{
  try
  {
    Date test (input, format);
  }

  catch (...)
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::valid (const int m, const int d, const int y)
{
  // Check that the year is valid.
  if (y < 0)
    return false;

  // Check that the month is valid.
  if (m < 1 || m > 12)
    return false;

  // Finally check that the days fall within the acceptable range for this
  // month, and whether or not this is a leap year.
  if (d < 1 || d > Date::daysInMonth (m, y))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::leapYear (int year)
{
  bool ly = false;

       if (!(year % 4))   ly = true;
  else if (!(year % 400)) ly = true;
  else if (!(year % 100)) ly = false;

  return ly;
}

////////////////////////////////////////////////////////////////////////////////
int Date::daysInMonth (int month, int year)
{
  static int days[2][12] =
  {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };

  return days[Date::leapYear (year) ? 1 : 0][month - 1];
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::monthName (int month)
{
  static const char* months[12] =
  {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
  };

  assert (month > 0);
  assert (month <= 12);
  return months[month - 1];
}

////////////////////////////////////////////////////////////////////////////////
void Date::dayName (int dow, std::string& name)
{
  static const char* days[7] =
  {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
  };

  name = days[dow];
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::dayName (int dow)
{
  static const char* days[7] =
  {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
  };

  return days[dow];
}

////////////////////////////////////////////////////////////////////////////////
int Date::weekOfYear (int weekStart) const
{
  struct tm* t = localtime (&mT);
  char   weekStr[3];

  if (weekStart == 0)
    strftime(weekStr, sizeof(weekStr), "%U", t);
  else if (weekStart == 1)
    strftime(weekStr, sizeof(weekStr), "%V", t);
  else
    throw std::string ("The 'weekstart' configuration variable may "
                       "only contain 'Sunday' or 'Monday'.");

  int weekNumber = ::atoi (weekStr);

  if (weekStart == 0)
    weekNumber += 1;

  return weekNumber;
}

////////////////////////////////////////////////////////////////////////////////
int Date::dayOfWeek () const
{
  struct tm* t = localtime (&mT);
  return t->tm_wday;
}

////////////////////////////////////////////////////////////////////////////////
int Date::dayOfWeek (const std::string& input)
{
  std::string in = lowerCase (input);

  if (in == "sunday")     return 0;
  if (in == "monday")     return 1;
  if (in == "tuesday")    return 2;
  if (in == "wednesday")  return 3;
  if (in == "thursday")   return 4;
  if (in == "friday")     return 5;
  if (in == "saturday")   return 6;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
int Date::month () const
{
  struct tm* t = localtime (&mT);
  return t->tm_mon + 1;
}

////////////////////////////////////////////////////////////////////////////////
int Date::day () const
{
  struct tm* t = localtime (&mT);
  return t->tm_mday;
}

////////////////////////////////////////////////////////////////////////////////
int Date::year () const
{
  struct tm* t = localtime (&mT);
  return t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator== (const Date& rhs)
{
  return rhs.mT == mT;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator!= (const Date& rhs)
{
  return rhs.mT != mT;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator<  (const Date& rhs)
{
  return mT < rhs.mT;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator>  (const Date& rhs)
{
  return mT > rhs.mT;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator<= (const Date& rhs)
{
  return mT <= rhs.mT;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator>= (const Date& rhs)
{
  return mT >= rhs.mT;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameDay (const Date& rhs)
{
  if (this->year ()  == rhs.year ()  &&
      this->month () == rhs.month () &&
      this->day ()   == rhs.day ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameMonth (const Date& rhs)
{
  if (this->year ()  == rhs.year () &&
      this->month () == rhs.month ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameYear (const Date& rhs)
{
  if (this->year () == rhs.year ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
Date Date::operator+ (const int delta)
{
  return Date::Date (mT + delta);
}

////////////////////////////////////////////////////////////////////////////////
Date& Date::operator+= (const int delta)
{
  mT += (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Date& Date::operator-= (const int delta)
{
  mT -= (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
time_t Date::operator- (const Date& rhs)
{
  return mT - rhs.mT;
}

////////////////////////////////////////////////////////////////////////////////
// If the input string looks like a relative date, determine that date, set mT
// and return true.
//
// What is a relative date?  All of the following should be recognizable, and
// converted to an absolute date:
//   wednesday
//   fri
//   23rd
//   today
//   tomorrow
//   yesterday
//   eow         (end of week)
//   eom         (end of month)
//   eoy         (end of year)
bool Date::isRelativeDate (const std::string& input)
{
  std::string in (lowerCase (input));
  Date today;

  std::vector <std::string> supported;
  supported.push_back ("monday");
  supported.push_back ("tuesday");
  supported.push_back ("wednesday");
  supported.push_back ("thursday");
  supported.push_back ("friday");
  supported.push_back ("saturday");
  supported.push_back ("sunday");
  supported.push_back ("today");
  supported.push_back ("tomorrow");
  supported.push_back ("yesterday");
  supported.push_back ("eow");
  supported.push_back ("eom");
  supported.push_back ("eoy");

  std::vector <std::string> matches;
  if (autoComplete (in, supported, matches) == 1)
  {
    std::string found = matches[0];

    // If day name.
    int dow;
    if ((dow = Date::dayOfWeek (found)) != -1 ||
        found == "eow")
    {
      if (found == "eow")
        dow = 5;

      if (today.dayOfWeek () >= dow)
        today += (dow - today.dayOfWeek () + 7) * 86400;
      else
        today += (dow - today.dayOfWeek ()) * 86400;

      int m, d, y;
      today.toMDY (m, d, y);
      Date then (m, d, y);

      mT = then.mT;
      return true;
    }
    else if (found == "today")
    {
      Date then (today.month (),
                 today.day (),
                 today.year ());
      mT = then.mT;
      return true;
    }
    else if (found == "tomorrow")
    {
      Date then (today.month (),
                 today.day (),
                 today.year ());
      mT = then.mT + 86400;
      return true;
    }
    else if (found == "yesterday")
    {
      Date then (today.month (),
                 today.day (),
                 today.year ());
      mT = then.mT - 86400;
      return true;
    }
    else if (found == "eom")
    {
      Date then (today.month (),
                 daysInMonth (today.month (), today.year ()),
                 today.year ());
      mT = then.mT;
      return true;
    }
    else if (found == "eoy")
    {
      Date then (12, 31, today.year ());
      mT = then.mT;
      return true;
    }
  }

  // Support "21st" to indicate the next date that is the 21st day.
  else if (input.length () <= 4 &&
           isdigit (input[0]))
  {
    int number;
    std::string ordinal;

    if (isdigit (input[1]))
    {
      number = ::atoi (input.substr (0, 2).c_str ());
      ordinal = lowerCase (input.substr (2, std::string::npos));
    }
    else
    {
      number = ::atoi (input.substr (0, 2).c_str ());
      ordinal = lowerCase (input.substr (1, std::string::npos));
    }

    // Sanity check.
    if (number <= 31)
    {
      if (ordinal == "st" ||
          ordinal == "nd" ||
          ordinal == "rd" ||
          ordinal == "th")
      {
        int m = today.month ();
        int d = today.day ();
        int y = today.year ();

        // If it is this month.
        if (d < number &&
            number <= Date::daysInMonth (m, y))
        {
          Date then (m, number, y);
          mT = then.mT;
          return true;
        }

        do
        {
          m++;

          if (m > 12)
          {
            m = 1;
            y++;
          }
        }
        while (number > Date::daysInMonth (m, y));

        Date then (m, number, y);
        mT = then.mT;
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
