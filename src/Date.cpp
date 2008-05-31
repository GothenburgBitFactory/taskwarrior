////////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
//
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
Date::Date (const std::string& mdy)
{
  size_t firstSlash  = mdy.find ("/");
  size_t secondSlash = mdy.find ("/", firstSlash + 1);
  if (firstSlash != std::string::npos &&
      secondSlash != std::string::npos)
  {
    int m = ::atoi (mdy.substr (0,               firstSlash              ).c_str ());
    int d = ::atoi (mdy.substr (firstSlash  + 1, secondSlash - firstSlash).c_str ());
    int y = ::atoi (mdy.substr (secondSlash + 1, std::string::npos       ).c_str ());
    if (!valid (m, d, y))
      throw std::string ("\"") + mdy + "\" is not a valid date.";

    // Duplicate Date::Date (const int, const int, const int);
    struct tm t = {0};
    t.tm_mday = d;
    t.tm_mon = m - 1;
    t.tm_year = y - 1900;

    mT = mktime (&t);
  }
  else
    throw std::string ("\"") + mdy + "\" is not a valid date.";
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
void Date::toString (std::string& output)
{
  output = toString ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::toString (void)
{
  int m, d, y;
  toMDY (m, d, y);

  char formatted [11];
  sprintf (formatted, "%d/%d/%d", m, d, y);
  return std::string (formatted);
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
