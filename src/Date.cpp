////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include "task.h"
#include "Date.h"

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
Date::Date (const std::string& mdy, const std::string format /* = "m/d/Y" */)
{
  int month = 0;
  int day   = 0;
  int year  = 0;

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
          mdy[i + 0] == '1'                                             &&
          (mdy[i + 1] == '0' || mdy[i + 1] == '1' || mdy[i + 1] == '2'))
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

      if (i + 1 < mdy.length ()  &&
          (mdy[i + 0] == '1' || mdy[i + 0] == '2' || mdy[i + 0] == '3') &&
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
std::string Date::toString (const std::string& format /*= "m/d/Y"*/)
{
  std::string formatted;
  for (unsigned int i = 0; i < format.length (); ++i)
  {
    switch (format[i])
    {
    case 'm':
      {
        char m[3];
        sprintf (m, "%d", this->month ());
        formatted += m;
      }
      break;

    case 'M':
      {
        char m[3];
        sprintf (m, "%02d", this->month ());
        formatted += m;
      }
      break;

    case 'd':
      {
        char d[3];
        sprintf (d, "%d", this->day ());
        formatted += d;
      }
      break;

    case 'D':
      {
        char d[3];
        sprintf (d, "%02d", this->day ());
        formatted += d;
      }
      break;

    case 'y':
      {
        char y[3];
        sprintf (y, "%02d", this->year () % 100);
        formatted += y;
      }
      break;

    case 'Y':
      {
        char y[5];
        sprintf (y, "%d", this->year ());
        formatted += y;
      }
      break;

    default:
      formatted += format[i];
      break;
    }
  }

  return formatted;
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
  return months[month -1];
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
int Date::dayOfWeek () const
{
  struct tm* t = localtime (&mT);
  return t->tm_wday;
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
