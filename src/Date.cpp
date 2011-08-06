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

#define L10N                                           // Localization complete.

#include <iostream> // TODO Remove.
#include <iomanip>
#include <sstream>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Nibbler.h>
#include <Date.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <Context.h>

static const char* relatives[] =
{
  "monday",
  "tuesday",
  "wednesday",
  "thursday",
  "friday",
  "saturday",
  "sunday",
  "today",
  "tomorrow",
  "yesterday",
  "eow",
  "eoww",
  "eocw",
  "eom",
  "eoq",
  "eoy",
  "sow",
  "soww",
  "socw",
  "som",
  "soq",
  "soy",
  "goodfriday",
  "easter",
  "eastermonday",
  "ascension",
  "pentecost",
  "midsommar",
  "midsommarafton",
  "now",
  "later",
  "someday",
};

#define NUM_RELATIVES   (sizeof (relatives)   / sizeof (relatives[0]))

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
  // Perhaps it is an epoch date, in string form?
  if (isEpoch (input))
    return;

  // Before parsing according to "format", perhaps this is a relative date?
  if (isRelativeDate (input))
    return;

  // Parse an ISO date.
  Nibbler n (input);
  if (n.getDateISO (mT) && n.depleted ())
    return;

  // Parse a formatted date.
  if (n.getDate (format, mT) && n.depleted ())
    return;

  throw ::format (STRING_DATE_INVALID_FORMAT, input, format);
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
double Date::toJulian ()
{
  return (mT / 86400.0) + 2440587.5;
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
const std::string Date::toString (
  const std::string& format /*= "m/d/Y" */) const
{
  // Making this local copy seems to fix a bug.  Remove the local copy and
  // you'll see segmentation faults and all kinds of gibberish.
  std::string localFormat = format;

  char buffer[12];
  std::string formatted;
  for (unsigned int i = 0; i < localFormat.length (); ++i)
  {
    int c = localFormat[i];
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
    case 'j': sprintf (buffer, "%d",   this->dayOfYear ());                    break;
    case 'J': sprintf (buffer, "%03d", this->dayOfYear ());                    break;
    default:  sprintf (buffer, "%c",   c);                                     break;
    }

    formatted += buffer;
  }

  return formatted;
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfDay () const
{
  return Date (month (), day (), year (), 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfWeek () const
{
  Date sow (mT);
  sow -= (dayOfWeek () * 86400);
  return Date (sow.month (), sow.day (), sow.year (), 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfMonth () const
{
  return Date (month (), 1, year (), 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfYear () const
{
  return Date (1, 1, year (), 0, 0, 0);
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
// Julian
bool Date::valid (const int d, const int y)
{
  // Check that the year is valid.
  if (y < 0)
    return false;

  if (d < 0 || d > 365)
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
    throw std::string (STRING_DATE_BAD_WEEKSTART);

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
int Date::dayOfYear () const
{
  struct tm* t = localtime (&mT);
  return t->tm_yday + 1;
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
int Date::length (const std::string& format)
{
  int total = 0;

  std::string::const_iterator i;
  for (i = format.begin (); i != format.end (); ++i)
  {
    switch (*i)
    {
    case 'm':
    case 'M':
    case 'd':
    case 'D':
    case 'y':
    case 'A':
    case 'b':
    case 'B':
    case 'V':
    case 'h':
    case 'H':
    case 'N':
    case 'S': total += 2; break;
    case 'a': total += 3; break;
    case 'Y': total += 4; break;
    default:  total += 1; break;
    }
  }

  return total;
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
// Prefix decrement by one day.
void Date::operator-- ()
{
  Date yesterday = startOfDay () - 1;
  yesterday = Date (yesterday.month (),
                    yesterday.day (),
                    yesterday.year (),
                    hour (),
                    minute (),
                    second ());
  mT = yesterday.mT;
}

////////////////////////////////////////////////////////////////////////////////
// Postfix decrement by one day.
void Date::operator-- (int)
{
  Date yesterday = startOfDay () - 1;
  yesterday = Date (yesterday.month (),
                    yesterday.day (),
                    yesterday.year (),
                    hour (),
                    minute (),
                    second ());
  mT = yesterday.mT;
}

////////////////////////////////////////////////////////////////////////////////
// Prefix increment by one day.
void Date::operator++ ()
{
  Date tomorrow = (startOfDay () + 90001).startOfDay ();
  tomorrow = Date (tomorrow.month (),
                   tomorrow.day (),
                   tomorrow.year (),
                   hour (),
                   minute (),
                   second ());
  mT = tomorrow.mT;
}

////////////////////////////////////////////////////////////////////////////////
// Postfix increment by one day.
void Date::operator++ (int)
{
  Date tomorrow = (startOfDay () + 90001).startOfDay ();
  tomorrow = Date (tomorrow.month (),
                   tomorrow.day (),
                   tomorrow.year (),
                   hour (),
                   minute (),
                   second ());
  mT = tomorrow.mT;
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
//   now
bool Date::isRelativeDate (const std::string& input)
{
  std::string in (lowerCase (input));
  Date today;

  std::vector <std::string> supported;
  for (unsigned int i = 0; i < NUM_RELATIVES; ++i)
    supported.push_back (relatives[i]);

  // Hard-coded 3, despite rc.abbreviation.minimum.
  std::vector <std::string> matches;
  if (autoComplete (in, supported, matches, 3) == 1)
  {
    std::string found = matches[0];

    // If day name.
    int dow;
    if ((dow = Date::dayOfWeek (found)) != -1 ||
        found == "eow"  ||
        found == "eoww" ||
        found == "eocw" ||
        found == "sow"  ||
        found == "soww" ||
        found == "socw")
    {
      if (found == "eow" || found == "eoww")
        dow = 5;

      if (found == "eocw")
        dow = (Date::dayOfWeek (context.config.get ("weekstart")) + 6) % 7;

      if (found == "sow" || found == "soww")
        dow = 1;

      if (found == "socw")
        dow = Date::dayOfWeek (context.config.get ("weekstart"));

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
    else if (found == "eoq")
    {
      int m = today.month ();
      int y = today.year ();
      int q;
      if (m <= 3)
        q = 3;
      else if (m >= 4 && m <= 6)
        q = 6;
      else if (m >= 7 && m <= 9)
        q = 9;
      else
        q = 12;
      Date then (q,
                 Date::daysInMonth (q, y),
                 y);
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
      int m = today.month () + 1;
      int y = today.year ();
      if (m > 12)
      {
        m -=12;
        y++;
      }
      Date then (m, 1, y);
      mT = then.mT;
      return true;
    }
    else if (found == "soq")
    {
      int m = today.month ();
      int y = today.year ();
      int q;
      if (m <= 3)
        q = 1;
      else if (m >= 4 && m <= 6)
        q = 4;
      else if (m >= 7 && m <= 9)
        q = 7;
      else
        q = 10;
      Date then (q,
                 1,
                 y);
      mT = then.mT;
      return true;
    }
    else if (found == "soy")
    {
      Date then (1, 1, today.year () + 1);
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
    else if (found == "now")
    {
      mT = time (NULL);
      return true;
    }
    else if (found == "later" || found == "someday")
    {
      Date then (1, 18, 2038);
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
const std::vector <std::string> Date::get_relatives ()
{
  std::vector <std::string> all;
  for (unsigned int i = 0; i < NUM_RELATIVES; ++i)
    if (strcmp (relatives[i], "-"))
      all.push_back (relatives[i]);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
