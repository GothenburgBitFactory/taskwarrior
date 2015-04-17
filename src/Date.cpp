////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <Variant.h>
#include <Dates.h>
#include <text.h>
#include <util.h>
#include <utf8.h>
#include <i18n.h>
#include <Context.h>

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
  const std::string& format    /* = "m/d/Y" */,
  const bool iso               /* = true */,
  const bool epoch             /* = true */)
{
  // Check first to see if this is supported as a named date.
  Variant v;
  if (namedDates (input, v))
  {
    _t = v.get_date ();
    return;
  }

  // Parse a formatted date.
  Nibbler n (input);
  n.save ();
#ifdef NIBBLER_FEATURE_DATE
  if (n.getDate (format, _t) && n.depleted ())
    return;
#endif

  // Parse an ISO date.
  n.restore ();
  if (iso && n.getDateISO (_t) && n.depleted ())
    return;

  // Perhaps it is an epoch date, in string form?
  n.restore ();
  if (epoch && isEpoch (input))
    return;

  throw ::format (STRING_DATE_INVALID_FORMAT, input, format);
}

////////////////////////////////////////////////////////////////////////////////
Date::Date (
  const std::string& input,
  std::string::size_type& i,
  const std::string& format    /* = "m/d/Y" */,
  const bool iso               /* = true */,
  const bool epoch             /* = true */)
{
  // Check first to see if this is supported as a named date.
  Variant v;
  if (namedDates (input, v))
  {
    i = v.source ().length ();
    _t = v.get_date ();
    return;
  }

  // Parse a formatted date.
  Nibbler n (input);
  n.save ();
#ifdef NIBBLER_FEATURE_DATE
  if (n.getDate (format, _t))
  {
    i = n.cursor ();
    return;
  }
#endif

  // Parse an ISO date.
  n.restore ();
  if (iso && n.getDateISO (_t))
  {
    i = n.cursor ();
    return;
  }

  // Perhaps it is an epoch date, in string form?
  n.restore ();
  if (epoch && isEpoch (input))
  {
    i = 10;
    return;
  }

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
  static int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month == 2 && Date::leapYear (year))
    return 29;

  return days[month - 1];
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

    // Calculate the width, don't assume a single character width.
    default:  total += mk_wcwidth (*i); break;
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
  if (Lexer::isAllDigits (input) &&
      input.length () <= 10 )
  {
    _t = (time_t) atoi (input.c_str ());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
