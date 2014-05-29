////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
  STRING_DATE_SUNDAY_LONG,
  STRING_DATE_MONDAY_LONG,
  STRING_DATE_TUESDAY_LONG,
  STRING_DATE_WEDNESDAY_LONG,
  STRING_DATE_THURSDAY_LONG,
  STRING_DATE_FRIDAY_LONG,
  STRING_DATE_SATURDAY_LONG,
  "today",
  "tomorrow",
  "yesterday",
  "eow",
  "eoww",
  "eocw",
  "eocm",
  "eom",
  "eoq",
  "eoy",
  "sow",
  "soww",
  "socw",
  "socm",
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
  _t = time (NULL);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const time_t t)
{
  _t = t;
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

  _t = mktime (&t);
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

  _t = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (
  const std::string& input,
  const std::string& format /* = "m/d/Y" */,
  const bool iso            /* = true */,
  const bool epoch          /* = true */)
{
  // Before parsing according to "format", perhaps this is a relative date?
  if (isRelativeDate (input))
    return;

  // Parse a formatted date.
  Nibbler n (input);
#ifdef NIBBLER_FEATURE_DATE
  if (n.getDate (format, _t) && n.depleted ())
    return;
#endif

  // Parse an ISO date.
  if (iso && n.getDateISO (_t) && n.depleted ())
    return;

  // Perhaps it is an epoch date, in string form?
  if (epoch && isEpoch (input))
    return;

  throw ::format (STRING_DATE_INVALID_FORMAT, input, format);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (const Date& rhs)
{
  _t = rhs._t;
}

////////////////////////////////////////////////////////////////////////////////
Date::~Date ()
{
}

////////////////////////////////////////////////////////////////////////////////
time_t Date::toEpoch ()
{
  return _t;
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::toEpochString ()
{
  std::stringstream epoch;
  epoch << _t;
  return epoch.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 19980119T070000Z =  YYYYMMDDThhmmssZ
std::string Date::toISO ()
{
  struct tm* t = gmtime (&_t);

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
  return (_t / 86400.0) + 2440587.5;
}

////////////////////////////////////////////////////////////////////////////////
void Date::toEpoch (time_t& epoch)
{
  epoch = _t;
}

////////////////////////////////////////////////////////////////////////////////
void Date::toMDY (int& m, int& d, int& y)
{
  struct tm* t = localtime (&_t);

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
    case 'm': sprintf (buffer, "%d",    this->month ());                        break;
    case 'M': sprintf (buffer, "%02d",  this->month ());                        break;
    case 'd': sprintf (buffer, "%d",    this->day ());                          break;
    case 'D': sprintf (buffer, "%02d",  this->day ());                          break;
    case 'y': sprintf (buffer, "%02d",  this->year () % 100);                   break;
    case 'Y': sprintf (buffer, "%d",    this->year ());                         break;
    case 'a': sprintf (buffer, "%.3s",  Date::dayName (dayOfWeek ()).c_str ()); break;
    case 'A': sprintf (buffer, "%.10s", Date::dayName (dayOfWeek ()).c_str ()); break;
    case 'b': sprintf (buffer, "%.3s",  Date::monthName (month ()).c_str ());   break;
    case 'B': sprintf (buffer, "%.10s", Date::monthName (month ()).c_str ());   break;
    case 'v': sprintf (buffer, "%d",    Date::weekOfYear (Date::dayOfWeek (context.config.get ("weekstart")))); break;
    case 'V': sprintf (buffer, "%02d",  Date::weekOfYear (Date::dayOfWeek (context.config.get ("weekstart")))); break;
    case 'h': sprintf (buffer, "%d",    this->hour ());                         break;
    case 'H': sprintf (buffer, "%02d",  this->hour ());                         break;
    case 'n': sprintf (buffer, "%d",    this->minute ());                       break;
    case 'N': sprintf (buffer, "%02d",  this->minute ());                       break;
    case 's': sprintf (buffer, "%d",    this->second ());                       break;
    case 'S': sprintf (buffer, "%02d",  this->second ());                       break;
    case 'j': sprintf (buffer, "%d",    this->dayOfYear ());                    break;
    case 'J': sprintf (buffer, "%03d",  this->dayOfYear ());                    break;
    default:  sprintf (buffer, "%c",    c);                                     break;
    }

    formatted += buffer;
  }

  return formatted;
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfDay () const
{
  return Date (month (), day (), year ());
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfWeek () const
{
  Date sow (_t);
  sow -= (dayOfWeek () * 86400);
  return Date (sow.month (), sow.day (), sow.year ());
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfMonth () const
{
  return Date (month (), 1, year ());
}

////////////////////////////////////////////////////////////////////////////////
Date Date::startOfYear () const
{
  return Date (1, 1, year ());
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

  if (d < 1 || d > Date::daysInYear (y))
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
int Date::daysInYear (int year)
{
  return Date::leapYear (year) ? 366 : 365;
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::monthName (int month)
{
  static const char* months[12] =
  {
    STRING_DATE_JANUARY_LONG,
    STRING_DATE_FEBRUARY_LONG,
    STRING_DATE_MARCH_LONG,
    STRING_DATE_APRIL_LONG,
    STRING_DATE_MAY_LONG,
    STRING_DATE_JUNE_LONG,
    STRING_DATE_JULY_LONG,
    STRING_DATE_AUGUST_LONG,
    STRING_DATE_SEPTEMBER_LONG,
    STRING_DATE_OCTOBER_LONG,
    STRING_DATE_NOVEMBER_LONG,
    STRING_DATE_DECEMBER_LONG,
  };

  assert (month > 0);
  assert (month <= 12);
  return ucFirst (months[month - 1]);
}

////////////////////////////////////////////////////////////////////////////////
void Date::dayName (int dow, std::string& name)
{
  static const char* days[7] =
  {
    STRING_DATE_SUNDAY_LONG,
    STRING_DATE_MONDAY_LONG,
    STRING_DATE_TUESDAY_LONG,
    STRING_DATE_WEDNESDAY_LONG,
    STRING_DATE_THURSDAY_LONG,
    STRING_DATE_FRIDAY_LONG,
    STRING_DATE_SATURDAY_LONG,
  };

  name = ucFirst (days[dow]);
}

////////////////////////////////////////////////////////////////////////////////
std::string Date::dayName (int dow)
{
  static const char* days[7] =
  {
    STRING_DATE_SUNDAY_LONG,
    STRING_DATE_MONDAY_LONG,
    STRING_DATE_TUESDAY_LONG,
    STRING_DATE_WEDNESDAY_LONG,
    STRING_DATE_THURSDAY_LONG,
    STRING_DATE_FRIDAY_LONG,
    STRING_DATE_SATURDAY_LONG,
  };

  return ucFirst (days[dow]);
}

////////////////////////////////////////////////////////////////////////////////
int Date::weekOfYear (int weekStart) const
{
  struct tm* t = localtime (&_t);
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
  struct tm* t = localtime (&_t);
  return t->tm_wday;
}

////////////////////////////////////////////////////////////////////////////////
int Date::dayOfWeek (const std::string& input)
{
  std::string in = lowerCase (input);

  if (in == STRING_DATE_SUNDAY_LONG    || in == STRING_DATE_SUNDAY_SHORT)    return 0;
  if (in == STRING_DATE_MONDAY_LONG    || in == STRING_DATE_MONDAY_SHORT)    return 1;
  if (in == STRING_DATE_TUESDAY_LONG   || in == STRING_DATE_TUESDAY_SHORT)   return 2;
  if (in == STRING_DATE_WEDNESDAY_LONG || in == STRING_DATE_WEDNESDAY_SHORT) return 3;
  if (in == STRING_DATE_THURSDAY_LONG  || in == STRING_DATE_THURSDAY_SHORT)  return 4;
  if (in == STRING_DATE_FRIDAY_LONG    || in == STRING_DATE_FRIDAY_SHORT)    return 5;
  if (in == STRING_DATE_SATURDAY_LONG  || in == STRING_DATE_SATURDAY_SHORT)  return 6;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
int Date::dayOfYear () const
{
  struct tm* t = localtime (&_t);
  return t->tm_yday + 1;
}

////////////////////////////////////////////////////////////////////////////////
int Date::monthOfYear (const std::string& input)
{
  std::string in = lowerCase (input);

  if (in == STRING_DATE_JANUARY_LONG   || in == STRING_DATE_JANUARY_SHORT  )     return  1;
  if (in == STRING_DATE_FEBRUARY_LONG  || in == STRING_DATE_FEBRUARY_SHORT )     return  2;
  if (in == STRING_DATE_MARCH_LONG     || in == STRING_DATE_MARCH_SHORT    )     return  3;
  if (in == STRING_DATE_APRIL_LONG     || in == STRING_DATE_APRIL_SHORT    )     return  4;
  if (in == STRING_DATE_MAY_LONG       || in == STRING_DATE_MAY_SHORT      )     return  5;
  if (in == STRING_DATE_JUNE_LONG      || in == STRING_DATE_JUNE_SHORT     )     return  6;
  if (in == STRING_DATE_JULY_LONG      || in == STRING_DATE_JULY_SHORT     )     return  7;
  if (in == STRING_DATE_AUGUST_LONG    || in == STRING_DATE_AUGUST_SHORT   )     return  8;
  if (in == STRING_DATE_SEPTEMBER_LONG || in == STRING_DATE_SEPTEMBER_SHORT)     return  9;
  if (in == STRING_DATE_OCTOBER_LONG   || in == STRING_DATE_OCTOBER_SHORT  )     return 10;
  if (in == STRING_DATE_NOVEMBER_LONG  || in == STRING_DATE_NOVEMBER_SHORT )     return 11;
  if (in == STRING_DATE_DECEMBER_LONG  || in == STRING_DATE_DECEMBER_SHORT )     return 12;

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
    case 'v':
    case 'V':
    case 'h':
    case 'H':
    case 'n':
    case 'N':
    case 's':
    case 'S': total += 2;  break;
    case 'b':
    case 'j':
    case 'J':
    case 'a': total += 3;  break;
    case 'Y': total += 4;  break;
    case 'A':
    case 'B': total += 10; break;

    // TODO This should be a calculated character width, not necessarily 1.
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
  struct tm* t = localtime (&_t);
  return t->tm_mon + 1;
}

////////////////////////////////////////////////////////////////////////////////
int Date::week () const
{
  return Date::weekOfYear (Date::dayOfWeek (context.config.get ("weekstart")));
}

////////////////////////////////////////////////////////////////////////////////
int Date::day () const
{
  struct tm* t = localtime (&_t);
  return t->tm_mday;
}

////////////////////////////////////////////////////////////////////////////////
int Date::year () const
{
  struct tm* t = localtime (&_t);
  return t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
int Date::hour () const
{
  struct tm* t = localtime (&_t);
  return t->tm_hour;
}

////////////////////////////////////////////////////////////////////////////////
int Date::minute () const
{
  struct tm* t = localtime (&_t);
  return t->tm_min;
}

////////////////////////////////////////////////////////////////////////////////
int Date::second () const
{
  struct tm* t = localtime (&_t);
  return t->tm_sec;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator== (const Date& rhs) const
{
  return rhs._t == _t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator!= (const Date& rhs) const
{
  return rhs._t != _t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator<  (const Date& rhs) const
{
  return _t < rhs._t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator>  (const Date& rhs) const
{
  return _t > rhs._t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator<= (const Date& rhs) const
{
  return _t <= rhs._t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::operator>= (const Date& rhs) const
{
  return _t >= rhs._t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameHour (const Date& rhs) const
{
  if (this->year ()  == rhs.year ()  &&
      this->month () == rhs.month () &&
      this->day ()   == rhs.day ()   &&
      this->hour ()  == rhs.hour ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameDay (const Date& rhs) const
{
  if (this->year ()  == rhs.year ()  &&
      this->month () == rhs.month () &&
      this->day ()   == rhs.day ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameWeek (const Date& rhs) const
{
  if (this->year ()  == rhs.year () &&
      this->week () == rhs.week ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameMonth (const Date& rhs) const
{
  if (this->year ()  == rhs.year () &&
      this->month () == rhs.month ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::sameYear (const Date& rhs) const
{
  if (this->year () == rhs.year ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
Date Date::operator- (const int delta)
{
  return Date (_t - delta);
}

////////////////////////////////////////////////////////////////////////////////
Date Date::operator+ (const int delta)
{
  return Date (_t + delta);
}

////////////////////////////////////////////////////////////////////////////////
Date& Date::operator+= (const int delta)
{
  _t += (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Date& Date::operator-= (const int delta)
{
  _t -= (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
time_t Date::operator- (const Date& rhs)
{
  return _t - rhs._t;
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
  _t = yesterday._t;
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
  _t = yesterday._t;
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
  _t = tomorrow._t;
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
  _t = tomorrow._t;
}

////////////////////////////////////////////////////////////////////////////////
bool Date::isEpoch (const std::string& input)
{
  if (digitsOnly (input) &&
      input.length () <= 10 )
  {
    _t = (time_t) atoi (input.c_str ());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// If the input string looks like a relative date, determine that date, set _t
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
        found == "soww")
    {
      if (found == "eow" || found == "eoww")
        dow = 5;

      if (found == "eocw")
        dow = (Date::dayOfWeek (context.config.get ("weekstart")) + 6) % 7;

      if (found == "sow" || found == "soww")
        dow = 1;

      if (today.dayOfWeek () >= dow)
        today += (dow - today.dayOfWeek () + 7) * 86400;
      else
        today += (dow - today.dayOfWeek ()) * 86400;

      int m, d, y;
      today.toMDY (m, d, y);
      Date then (m, d, y);

      _t = then._t;
      return true;
    }

    else if (found == "socw")
    {
      //       day S M T W T F S
      //       dow 0 1 2 3 4 5 6
      // -----------------------
      // weekstart   ^
      // today1    ^
      // today2            ^
      //
      // delta1 = 6 <-- (0 - 1 + 7) % 7
      // delta2 = 3 <-- (4 - 1 + 7) % 7

      dow = Date::dayOfWeek (context.config.get ("weekstart"));
      int delta = (today.dayOfWeek () - dow + 7) % 7;
      today -= delta * 86400;

      Date then (today.month (), today.day (), today.year ());
      _t = then._t;
      return true;
    }

    else if (found == "today")
    {
      Date then (today.month (),
                 today.day (),
                 today.year ());
      _t = then._t;
      return true;
    }
    else if (found == "tomorrow")
    {
      Date then (today.month (),
                 today.day (),
                 today.year ());
      _t = then._t + 86400;
      return true;
    }
    else if (found == "yesterday")
    {
      Date then (today.month (),
                 today.day (),
                 today.year ());
      _t = then._t - 86400;
      return true;
    }
    else if (found == "eom" || found == "eocm")
    {
      Date then (today.month (),
                 daysInMonth (today.month (), today.year ()),
                 today.year ());
      _t = then._t;
      return true;
    }
    else if (found == "eoq")
    {
      int eoq_month = today.month () + 2 - (today.month () - 1) % 3;
      Date then (eoq_month,
                 daysInMonth (eoq_month, today.year ()),
                 today.year ());
      _t = then._t;
      return true;
    }
    else if (found == "eoy")
    {
      Date then (12, 31, today.year ());
      _t = then._t;
      return true;
    }
    else if (found == "socm")
    {
      int m = today.month ();
      int y = today.year ();
      Date then (m, 1, y);
      _t = then._t;
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
      _t = then._t;
      return true;
    }
    else if (found == "soq")
    {
      int m = today.month () + 3 - (today.month () - 1) % 3;
      int y = today.year ();
      if (m > 12)
      {
        m -=12;
        y++;
      }
      Date then (m , 1, y);
      _t = then._t;
      return true;
    }
    else if (found == "soy")
    {
      Date then (1, 1, today.year () + 1);
      _t = then._t;
      return true;
    }
    else if (found == "goodfriday")
    {
      Date then (Date::easter(today.year()));
      _t = then._t - 86400*2;
      return true;
    }
    else if (found == "easter")
    {
      Date then (Date::easter(today.year()));
      _t = then._t;
      return true;
    }
    else if (found == "eastermonday")
    {
      Date then (Date::easter(today.year()));
      _t = then._t + 86400;
      return true;
    }
    else if (found == "ascension")
    {
      Date then (Date::easter(today.year()));
      _t = then._t + 86400*39;
      return true;
    }
    else if (found == "pentecost")
    {
      Date then (Date::easter(today.year()));
      _t = then._t + 86400*49;
      return true;
    }
    else if (found == "midsommar")
    {
      for (int midsommar = 20; midsommar <= 26; midsommar++)
      {
        Date then (6, midsommar, today.year ());
        if (6 == then.dayOfWeek ())
        {
          _t = then._t;
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
          _t = then._t;
          return true;
        }
      }
    }
    else if (found == "now")
    {
      _t = time (NULL);
      return true;
    }
    else if (found == "later" || found == "someday")
    {
      Date then (1, 18, 2038);
      _t = then._t;
      return true;
    }
  }

  // Support "21st" to indicate the next date that is the 21st day.
  else if (in.length () <= 4 &&
           isdigit (in[0]))
  {
    int number;
    std::string ordinal;

    if (isdigit (in[1]))
    {
      number = atoi (in.substr (0, 2).c_str ());
      ordinal = lowerCase (in.substr (2));
    }
    else
    {
      number = atoi (in.substr (0, 2).c_str ());
      ordinal = lowerCase (in.substr (1));
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
          _t = then._t;
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
        _t = then._t;
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
