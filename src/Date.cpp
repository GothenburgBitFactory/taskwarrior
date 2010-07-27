////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include <iomanip>
#include <sstream>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "Date.h"
#include "text.h"
#include "util.h"
#include "Context.h"

extern Context context;

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
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = d;
  t.tm_mon   = m - 1;
  t.tm_year  = y - 1900;

  mT = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const int m,  const int d,  const int y,
            const int hr, const int mi, const int se)
{
  // Error if not valid.
  struct tm t = {0};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = d;
  t.tm_mon   = m - 1;
  t.tm_year  = y - 1900;
  t.tm_hour  = hr;
  t.tm_min   = mi;
  t.tm_sec   = se;

  mT = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const std::string& input, const std::string& format /* = "m/d/Y" */)
{
  int month  = 0;
  int day    = 0;
  int year   = -1;   // So we can check later.
  int hour   = 0;
  int minute = 0;
  int second = 0;

  // Perhaps it is an epoch date, in string form?
  if (isEpoch (input))
    return;

  // Before parsing according to "format", perhaps this is a relative date?
  if (isRelativeDate (input))
    return;

  unsigned int i = 0; // Index into input.

  // Format may include: mMdDyYVaAbBhHNS
  //
  // Note that the format should never include T or Z, as that interferes with
  // the potential parsing for ISO dates constructed from the above format.
  for (unsigned int f = 0; f < format.length (); ++f)
  {
    switch (format[f])
    {
    // Single or double digit.
    case 'm':
      if (i >= input.length () ||
          ! isdigit (input[i]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (m).";
      }

      if (i + 1 < input.length ()                    &&
          (input[i + 0] == '0' || input[i + 0] == '1') &&
          isdigit (input[i + 1]))
      {
        month = atoi (input.substr (i, 2).c_str ());
        i += 2;
      }
      else
      {
        month = atoi (input.substr (i, 1).c_str ());
        ++i;
      }
      break;

    case 'd':
      if (i >= input.length () ||
          ! isdigit (input[i]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (d).";
      }

      if (i + 1 < input.length ()                                                                    &&
          (input[i + 0] == '0' || input[i + 0] == '1' || input[i + 0] == '2' || input[i + 0] == '3') &&
          isdigit (input[i + 1]))
      {
        day = atoi (input.substr (i, 2).c_str ());
        i += 2;
      }
      else
      {
        day = atoi (input.substr (i, 1).c_str ());
        ++i;
      }
      break;

    // Double digit.
    case 'y':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (y).";
      }

      year = atoi (input.substr (i, 2).c_str ()) + 2000;
      i += 2;
      break;

    case 'M':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (M).";
      }

      month = atoi (input.substr (i, 2).c_str ());
      i += 2;
      break;

    case 'D':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (D).";
      }

      day = atoi (input.substr (i, 2).c_str ());
      i += 2;
      break;

    case 'V':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (V).";
      }

      i += 2;
      break;

    // Quadruple digit.
    case 'Y':
      if (i + 3 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]) ||
          ! isdigit (input[i + 2]) ||
          ! isdigit (input[i + 3]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (Y).";
      }

      year = atoi (input.substr (i, 4).c_str ());
      i += 4;
      break;

    // Short names with 3 characters
    case 'a':
      if (i + 2 >= input.length () ||
          isdigit (input[i + 0]) ||
          isdigit (input[i + 1]) ||
          isdigit (input[i + 2]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (a).";
      }

      i += 3;
      break;

    case 'b':
      if (i + 2 >= input.length () ||
          isdigit (input[i + 0]) ||
          isdigit (input[i + 1]) ||
          isdigit (input[i + 2]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (b).";
      }

      month = Date::monthOfYear (input.substr (i, 3).c_str());
      i += 3;
      break;

    // Long names
    case 'A':
      if (i + 2 >= input.length () ||
          isdigit (input[i + 0]) ||
          isdigit (input[i + 1]) ||
          isdigit (input[i + 2]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (A).";
      }

      i += Date::dayName( Date::dayOfWeek (input.substr (i, 3).c_str()) ).size();
      break;

    case 'B':
      if (i + 2 >= input.length () ||
          isdigit (input[i + 0]) ||
          isdigit (input[i + 1]) ||
          isdigit (input[i + 2]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (B).";
      }

      month = Date::monthOfYear (input.substr (i, 3).c_str());
      i += Date::monthName(month).size();
      break;

    // Single or double digit.
    case 'h':
      if (i >= input.length () ||
          ! isdigit (input[i]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (h).";
      }

      if (i + 1 < input.length ()                    &&
          (input[i + 0] == '0' || input[i + 0] == '1' || input[i + 0] == '2') &&
          isdigit (input[i + 1]))
      {
        hour = atoi (input.substr (i, 2).c_str ());
        i += 2;
      }
      else
      {
        hour = atoi (input.substr (i, 1).c_str ());
        ++i;
      }
      break;

    case 'H':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (H).";
      }

      hour = atoi (input.substr (i, 2).c_str ());
      i += 2;
      break;

    case 'N':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (N).";
      }

      minute = atoi (input.substr (i, 2).c_str ());
      i += 2;
      break;

    case 'S':
      if (i + 1 >= input.length () ||
          ! isdigit (input[i + 0]) ||
          ! isdigit (input[i + 1]))
      {
        throw std::string ("\"") + input + "\" is not a valid date (S).";
      }

      second = atoi (input.substr (i, 2).c_str ());
      i += 2;
      break;

    default:
      if (i >= input.length () ||
          input[i] != format[f])
      {
        throw std::string ("\"") + input + "\" is not a valid date (DEFAULT).";
      }
      ++i;
      break;
    }
  }

  // Default the year to the current year, for formats that lack Y/y.
  if (year == -1)
  {
    time_t now = time (NULL);
    struct tm* default_year = localtime (&now);
    year = default_year->tm_year + 1900;
  }

  if (i < input.length ())
    throw std::string ("\"") + input + "\" is not a valid date in " + format + " format.";

  if (!valid (month, day, year))
    throw std::string ("\"") + input + "\" is not a valid date (VALID).";

  // Convert to epoch.
  struct tm t = {0};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = day;
  t.tm_mon   = month - 1;
  t.tm_year  = year - 1900;
  t.tm_hour  = hour;
  t.tm_min   = minute;
  t.tm_sec   = second;

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
std::string Date::toEpochString ()
{
  std::stringstream epoch;
  epoch << mT;
  return epoch.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 19980119T070000Z =  YYYYMMDDThhmmssZ
std::string Date::toISO ()
{
  struct tm* t = gmtime (&mT);

  std::stringstream iso;
  iso << std::setw (4) << std::setfill ('0') << t->tm_year + 1900
      << std::setw (2) << std::setfill ('0') << t->tm_mon + 1
      << std::setw (2) << std::setfill ('0') << t->tm_mday
      << "T"
      << std::setw (2) << std::setfill ('0') << t->tm_hour
      << std::setw (2) << std::setfill ('0') << t->tm_min
      << std::setw (2) << std::setfill ('0') << t->tm_sec
      << "Z";

  return iso.str ();
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
    case 'm': sprintf (buffer, "%d",   this->month ());                        break;
    case 'M': sprintf (buffer, "%02d", this->month ());                        break;
    case 'd': sprintf (buffer, "%d",   this->day ());                          break;
    case 'D': sprintf (buffer, "%02d", this->day ());                          break;
    case 'y': sprintf (buffer, "%02d", this->year () % 100);                   break;
    case 'Y': sprintf (buffer, "%d",   this->year ());                         break;
    case 'a': sprintf (buffer, "%.3s", Date::dayName (dayOfWeek ()).c_str ()); break;
    case 'A': sprintf (buffer, "%s",   Date::dayName (dayOfWeek ()).c_str ()); break;
    case 'b': sprintf (buffer, "%.3s", Date::monthName (month ()).c_str ());   break;
    case 'B': sprintf (buffer, "%.9s", Date::monthName (month ()).c_str ());   break;
    case 'V': sprintf (buffer, "%02d", Date::weekOfYear (Date::dayOfWeek (context.config.get ("weekstart")))); break;
    case 'h': sprintf (buffer, "%d",   this->hour ());                         break;
    case 'H': sprintf (buffer, "%02d", this->hour ());                         break;
    case 'N': sprintf (buffer, "%02d", this->minute ());                       break;
    case 'S': sprintf (buffer, "%02d", this->second ());                       break;
    default:  sprintf (buffer, "%c",   c);                                     break;
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
bool Date::valid (const int m, const int d, const int y, const int hr,
                  const int mi, const int se)
{
  if (hr < 0 || hr > 23)
    return false;

  if (mi < 0 || mi > 59)
    return false;

  if (se < 0 || se > 59)
    return false;

  return Date::valid (m, d, y);
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

  // (year % 4 == 0) && (year % 100 !=0)  OR
  // (year % 400 == 0)
  // are leapyears

  if (((!(year % 4)) && (year % 100)) || (!(year % 400))) ly = true;

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

  int weekNumber = atoi (weekStr);

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

  if (in == "sunday"    || in == "sun")   return 0;
  if (in == "monday"    || in == "mon")   return 1;
  if (in == "tuesday"   || in == "tue")   return 2;
  if (in == "wednesday" || in == "wed")   return 3;
  if (in == "thursday"  || in == "thu")   return 4;
  if (in == "friday"    || in == "fri")   return 5;
  if (in == "saturday"  || in == "sat")   return 6;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
int Date::monthOfYear (const std::string& input)
{
  std::string in = lowerCase (input);

  if (in == "january"   || in == "jan")     return  1;
  if (in == "february"  || in == "feb")     return  2;
  if (in == "march"     || in == "mar")     return  3;
  if (in == "april"     || in == "apr")     return  4;
  if (in == "may"       || in == "may")     return  5;
  if (in == "june"      || in == "jun")     return  6;
  if (in == "july"      || in == "jul")     return  7;
  if (in == "august"    || in == "aug")     return  8;
  if (in == "september" || in == "sep")     return  9;
  if (in == "october"   || in == "oct")     return 10;
  if (in == "november"  || in == "nov")     return 11;
  if (in == "december"  || in == "dec")     return 12;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
time_t Date::easter (int year)
{
  int Y = year;
  int a = Y % 19;
  int b = Y / 100;
  int c = Y % 100;
  int d = b / 4;
  int e = b % 4;
  int f = (b + 8) / 25;
  int g = (b - f + 1) / 3;
  int h = (19 * a + b - d - g + 15) % 30;
  int i = c / 4;
  int k = c % 4;
  int L = (32 + 2 * e + 2 * i - h - k) % 7;
  int m = (a + 11 * h + 22 * L) / 451;
  int month = (h + L - 7 * m + 114) / 31;
  int day = ((h + L - 7 * m + 114) % 31) + 1;
  struct tm t = {0};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = day;
  t.tm_mon   = month - 1;
  t.tm_year  = year - 1900;
  return mktime (&t);
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
int Date::hour () const
{
  struct tm* t = localtime (&mT);
  return t->tm_hour;
}

////////////////////////////////////////////////////////////////////////////////
int Date::minute () const
{
  struct tm* t = localtime (&mT);
  return t->tm_min;
}

////////////////////////////////////////////////////////////////////////////////
int Date::second () const
{
  struct tm* t = localtime (&mT);
  return t->tm_sec;
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
bool Date::sameHour (const Date& rhs)
{
  if (this->year ()  == rhs.year ()  &&
      this->month () == rhs.month () &&
      this->day ()   == rhs.day ()   &&
      this->hour ()  == rhs.hour ())
    return true;

  return false;
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
Date Date::operator- (const int delta)
{
  return Date (mT - delta);
}

////////////////////////////////////////////////////////////////////////////////
Date Date::operator+ (const int delta)
{
  return Date (mT + delta);
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
bool Date::isEpoch (const std::string& input)
{
  if (digitsOnly (input)    &&
      input.length () >   8 &&
      input.length () <= 10 )
  {
    mT = (time_t) atoi (input.c_str ());
    return true;
  }

  return false;
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
  supported.push_back ("sow");
  supported.push_back ("som");
  supported.push_back ("soy");
  supported.push_back ("goodfriday");
  supported.push_back ("easter");
  supported.push_back ("eastermonday");
  supported.push_back ("ascension");
  supported.push_back ("pentecost");
  supported.push_back ("midsommar");
  supported.push_back ("midsommarafton");

  std::vector <std::string> matches;
  if (autoComplete (in, supported, matches) == 1)
  {
    std::string found = matches[0];

    // If day name.
    int dow;
    if ((dow = Date::dayOfWeek (found)) != -1 ||
        found == "eow"  ||
        found == "eocw" ||
        found == "sow")
    {
      if (found == "eow")
        dow = 5;

      if (found == "sow")
        dow =Date::dayOfWeek (context.config.get ("weekstart"));

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
    else if (found == "som")
    {
      Date then (today.month (),
                 1,
                 today.year ());
      mT = then.mT;
      return true;
    }
    else if (found == "soy")
    {
      Date then (1, 1, today.year ());
      mT = then.mT;
      return true;
    }
    else if (found == "goodfriday")
    {
      Date then (Date::easter(today.year()));
      mT = then.mT - 86400*2;
      return true;
    }
    else if (found == "easter")
    {
      Date then (Date::easter(today.year()));
      mT = then.mT;
      return true;
    }
    else if (found == "eastermonday")
    {
      Date then (Date::easter(today.year()));
      mT = then.mT + 86400;
      return true;
    }
    else if (found == "ascension")
    {
      Date then (Date::easter(today.year()));
      mT = then.mT + 86400*39;
      return true;
    }
    else if (found == "pentecost")
    {
      Date then (Date::easter(today.year()));
      mT = then.mT + 86400*49;
      return true;
    }
    else if (found == "midsommar")
    {
      for (int midsommar = 20; midsommar <= 26; midsommar++)
      {
        Date then (6, midsommar, today.year ());
        if (6 == then.dayOfWeek ())
        {
          mT = then.mT;
          return true;
        }
      }
    }
    else if (found == "midsommarafton")
    {
      for (int midsommar = 19; midsommar <= 25; midsommar++)
      {
        Date then (6, midsommar, today.year ());
        if (5 == then.dayOfWeek ())
        {
          mT = then.mT;
          return true;
        }
      }
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
      number = atoi (input.substr (0, 2).c_str ());
      ordinal = lowerCase (input.substr (2));
    }
    else
    {
      number = atoi (input.substr (0, 2).c_str ());
      ordinal = lowerCase (input.substr (1));
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
