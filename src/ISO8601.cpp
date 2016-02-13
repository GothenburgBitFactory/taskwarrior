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
#include <ISO8601.h>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>
#include <Lexer.h>
#include <util.h>
#ifdef PRODUCT_TASKWARRIOR
#include <Dates.h>
#endif
#include <text.h>
#include <utf8.h>
#include <i18n.h>

#define DAY    86400
#define HOUR    3600
#define MINUTE    60
#define SECOND     1

static struct
{
  std::string unit;
  int seconds;
  bool standalone;
} durations[] =
{
  // These are sorted by first character, then length, so that Nibbler::getOneOf
  // returns a maximal match.
  {"annual",     365 * DAY,    true},
  {"biannual",   730 * DAY,    true},
  {"bimonthly",   61 * DAY,    true},
  {"biweekly",    14 * DAY,    true},
  {"biyearly",   730 * DAY,    true},
  {"daily",        1 * DAY,    true},
  {"days",         1 * DAY,    false},
  {"day",          1 * DAY,    true},
  {"d",            1 * DAY,    false},
  {"fortnight",   14 * DAY,    true},
  {"hours",        1 * HOUR,   false},
  {"hour",         1 * HOUR,   true},
  {"hrs",          1 * HOUR,   false},
  {"hr",           1 * HOUR,   true},
  {"h",            1 * HOUR,   false},
  {"minutes",      1 * MINUTE, false},
  {"minute",       1 * MINUTE, true},
  {"mins",         1 * MINUTE, false},
  {"min",          1 * MINUTE, true},
  {"monthly",     30 * DAY,    true},
  {"months",      30 * DAY,    false},
  {"month",       30 * DAY,    true},
  {"mnths",       30 * DAY,    false},
  {"mths",        30 * DAY,    false},
  {"mth",         30 * DAY,    true},
  {"mos",         30 * DAY,    false},
  {"mo",          30 * DAY,    true},
  {"m",           30 * DAY,    false},
  {"quarterly",   91 * DAY,    true},
  {"quarters",    91 * DAY,    false},
  {"quarter",     91 * DAY,    true},
  {"qrtrs",       91 * DAY,    false},
  {"qrtr",        91 * DAY,    true},
  {"qtrs",        91 * DAY,    false},
  {"qtr",         91 * DAY,    true},
  {"q",           91 * DAY,    false},
  {"semiannual", 183 * DAY,    true},
  {"sennight",    14 * DAY,    false},
  {"seconds",      1 * SECOND, false},
  {"second",       1 * SECOND, true},
  {"secs",         1 * SECOND, false},
  {"sec",          1 * SECOND, true},
  {"s",            1 * SECOND, false},
  {"weekdays",     1 * DAY,    true},
  {"weekly",       7 * DAY,    true},
  {"weeks",        7 * DAY,    false},
  {"week",         7 * DAY,    true},
  {"wks",          7 * DAY,    false},
  {"wk",           7 * DAY,    true},
  {"w",            7 * DAY,    false},
  {"yearly",     365 * DAY,    true},
  {"years",      365 * DAY,    false},
  {"year",       365 * DAY,    true},
  {"yrs",        365 * DAY,    false},
  {"yr",         365 * DAY,    true},
  {"y",          365 * DAY,    false},
};

#define NUM_DURATIONS (sizeof (durations) / sizeof (durations[0]))

std::string ISO8601d::weekstart  = STRING_DATE_SUNDAY;
int ISO8601d::minimumMatchLength = 3;
bool ISO8601d::isoEnabled = true;
bool ISO8601p::isoEnabled = true;

////////////////////////////////////////////////////////////////////////////////
ISO8601d::ISO8601d ()
{
  clear ();
  _date = time (NULL);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::ISO8601d (const std::string& input, const std::string& format /*= ""*/)
{
  clear ();
  std::string::size_type start = 0;
  if (! parse (input, start, format))
    throw ::format (STRING_DATE_INVALID_FORMAT, input, format);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::ISO8601d (const time_t t)
{
  clear ();
  _date = t;
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::ISO8601d (const int m, const int d, const int y)
{
  clear ();

  // Error if not valid.
  struct tm t {};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = d;
  t.tm_mon   = m - 1;
  t.tm_year  = y - 1900;

  _date = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::ISO8601d (const int m,  const int d,  const int y,
                    const int hr, const int mi, const int se)
{
  clear ();

  // Error if not valid.
  struct tm t {};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = d;
  t.tm_mon   = m - 1;
  t.tm_year  = y - 1900;
  t.tm_hour  = hr;
  t.tm_min   = mi;
  t.tm_sec   = se;

  _date = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::~ISO8601d ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Supported:
//
//    result       ::= date 'T' time 'Z'                      # UTC
//                   | date 'T' time                          # Local
//                   | date-ext 'T' time-ext 'Z'              # UTC
//                   | date-ext 'T' time-ext offset-ext       # Specified TZ
//                   | date-ext 'T' time-ext                  # Local
//                   | date-ext                               # Local
//                   | time-ext 'Z'
//                   | time-ext offset-ext            Not needed
//                   | time-ext
//                   ;
//
//    date-ext     ::= ±YYYYY-MM-DD                   Νot needed
//                   | ±YYYYY-Www-D                   Νot needed
//                   | ±YYYYY-Www                     Νot needed
//                   | ±YYYYY-DDD                     Νot needed
//                   | YYYY-MM-DD
//                   | YYYY-DDD
//                   | YYYY-Www-D
//                   | YYYY-Www
//                   ;
//
//    time-ext     ::= hh:mm:ss[,ss]
//                   | hh:mm[,mm]
//                   | hh[,hh]                        Ambiguous (number)
//                   ;
//
//    time-utc-ext ::= hh:mm[:ss] 'Z' ;
//
//    offset-ext   ::= ±hh[:mm] ;
//
// Not yet supported:
//
//    recurrence ::=
//                 | 'R' [n] '/' designated '/' datetime-ext          # duration end
//                 | 'R' [n] '/' designated '/' datetime              # duration end
//                 | 'R' [n] '/' designated                           # duration
//                 | 'R' [n] '/' datetime-ext '/' designated          # start duration
//                 | 'R' [n] '/' datetime-ext '/' datetime-ext        # start end
//                 | 'R' [n] '/' datetime '/' designated              # start duration
//                 | 'R' [n] '/' datetime '/' datetime                # start end
//                 ;
//
bool ISO8601d::parse (
  const std::string& input,
  std::string::size_type& start,
  const std::string& format /* = "" */)
{
  auto i = start;
  Nibbler n (input.substr (i));

  // Parse epoch first, as it's the most common scenario.
  if (parse_epoch (n))
  {
    // ::validate and ::resolve are not needed in this case.
    start = n.cursor ();
    return true;
  }

  else if (parse_formatted (n, format))
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      start = n.cursor ();
      resolve ();
      return true;
    }
  }

  // Allow parse_date_time and parse_date_time_ext regardless of
  // ISO8601d::isoEnabled setting, because these formats are relied upon by
  // the 'import' command, JSON parser and hook system.
  else if (parse_date_time     (n)   || // Strictest first.
           parse_date_time_ext (n)   ||
           (ISO8601d::isoEnabled &&
            (parse_date_ext      (n) ||
             parse_time_utc_ext  (n) ||
             parse_time_off_ext  (n) ||
             parse_time_ext      (n)))) // Time last, as it is the most permissive.
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      start = n.cursor ();
      resolve ();
      return true;
    }
  }

  else if (parse_named (n))
  {
    // ::validate and ::resolve are not needed in this case.
    start = n.cursor ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void ISO8601d::clear ()
{
  _year            = 0;
  _month           = 0;
  _week            = 0;
  _weekday         = 0;
  _julian          = 0;
  _day             = 0;
  _seconds         = 0;
  _offset          = 0;
  _utc             = false;
  _date            = 0;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::parse_formatted (Nibbler& n, const std::string& format)
{
  // Short-circuit on missing format.
  if (format == "")
    return false;

  n.save ();

  int month  = -1;   // So we can check later.
  int day    = -1;
  int year   = -1;
  int hour   = -1;
  int minute = -1;
  int second = -1;

  // For parsing, unused.
  int wday   = -1;
  int week   = -1;

  for (unsigned int f = 0; f < format.length (); ++f)
  {
    switch (format[f])
    {
    case 'm':
      if (n.getDigit (month))
      {
        if (month == 0)
          n.getDigit (month);

        if (month == 1)
          if (n.getDigit (month))
            month += 10;
      }
      else
      {
        n.restore ();
        return false;
      }
      break;

    case 'M':
      if (! n.getDigit2 (month))
      {
        n.restore ();
        return false;
      }
      break;

    case 'd':
      if (n.getDigit (day))
      {
        if (day == 0)
          n.getDigit (day);

        if (day == 1 || day == 2 || day == 3)
        {
          int tens = day;
          if (n.getDigit (day))
            day += 10 * tens;
        }
      }
      else
      {
        n.restore ();
        return false;
      }
      break;

    case 'D':
      if (! n.getDigit2 (day))
      {
        n.restore ();
        return false;
      }
      break;

    case 'y':
      if (! n.getDigit2 (year))
      {
        n.restore ();
        return false;
      }
      year += 2000;
      break;

    case 'Y':
      if (! n.getDigit4 (year))
      {
        n.restore ();
        return false;
      }
      break;

    case 'h':
      if (n.getDigit (hour))
      {
        if (hour == 0)
          n.getDigit (hour);

        if (hour == 1 || hour == 2)
        {
          int tens = hour;
          if (n.getDigit (hour))
            hour += 10 * tens;
        }
      }
      else
      {
        n.restore ();
        return false;
      }
      break;

    case 'H':
      if (! n.getDigit2 (hour))
      {
        n.restore ();
        return false;
      }
      break;

    case 'n':
      if (n.getDigit (minute))
      {
        if (minute == 0)
          n.getDigit (minute);

        if (minute < 6)
        {
          int tens = minute;
          if (n.getDigit (minute))
            minute += 10 * tens;
        }
      }
      else
      {
        n.restore ();
        return false;
      }
      break;

    case 'N':
      if (! n.getDigit2 (minute))
      {
        n.restore ();
        return false;
      }
      break;

    case 's':
      if (n.getDigit (second))
      {
        if (second == 0)
          n.getDigit (second);

        if (second < 6)
        {
          int tens = second;
          if (n.getDigit (second))
            second += 10 * tens;
        }
      }
      else
      {
        n.restore ();
        return false;
      }
      break;

    case 'S':
      if (! n.getDigit2 (second))
      {
        n.restore ();
        return false;
      }
      break;

    case 'v':
      if (n.getDigit (week))
      {
        if (week == 0)
          n.getDigit (week);

        if (week < 6)
        {
          int tens = week;
          if (n.getDigit (week))
            week += 10 * tens;
        }
      }
      else
      {
        n.restore ();
        return false;
      }
      break;

    case 'V':
      if (! n.getDigit2 (week))
      {
        n.restore ();
        return false;
      }
      break;

    case 'a':
      {
        auto cursor = n.cursor ();
        wday = ISO8601d::dayOfWeek (n.str ().substr (cursor, 3));
        if (wday == -1)
        {
          n.restore ();
          return false;
        }

        n.skipN (3);
      }
      break;

    case 'A':
      {
        std::string dayName;
        if (n.getUntil (format[f + 1], dayName))
        {
          wday = ISO8601d::dayOfWeek (Lexer::lowerCase (dayName));
          if (wday == -1)
          {
            n.restore ();
            return false;
          }
        }
      }
      break;

    case 'b':
      {
        auto cursor = n.cursor ();
        month = ISO8601d::monthOfYear (n.str ().substr (cursor, 3));
        if (month == -1)
        {
          n.restore ();
          return false;
        }

        n.skipN (3);
      }
      break;

    case 'B':
      {
        std::string monthName;
        if (n.getUntil (format[f + 1], monthName))
        {
          month = ISO8601d::monthOfYear (Lexer::lowerCase (monthName));
          if (month == -1)
          {
            n.restore ();
            return false;
          }
        }
      }
      break;

    default:
      if (! n.skip (format[f]))
      {
        n.restore ();
        return false;
      }
      break;
    }
  }

  // It is possible that the format='Y-M-D', and the input is Y-M-DTH:N:SZ, and
  // this should not be considered a match.
  if (! n.depleted () && ! Lexer::isWhitespace (n.next ()))
  {
    n.restore ();
    return false;
  }

  // Missing values are filled in from the current date.
  if (year == -1)
  {
    ISO8601d now;
    year = now.year ();
    if (month == -1)
    {
      month = now.month ();
      if (day == -1)
      {
        day = now.day ();
        if (hour == -1)
        {
          hour = now.hour ();
          if (minute == -1)
          {
            minute = now.minute ();
            if (second == -1)
              second = now.second ();
          }
        }
      }
    }
  }

  // Any remaining undefined values are assigned defaults.
  if (month  == -1) month  = 1;
  if (day    == -1) day    = 1;
  if (hour   == -1) hour   = 0;
  if (minute == -1) minute = 0;
  if (second == -1) second = 0;

  _year    = year;
  _month   = month;
  _day     = day;
  _seconds = (hour * 3600) + (minute * 60) + second;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::parse_named (Nibbler& n)
{
#ifdef PRODUCT_TASKWARRIOR
  n.save ();
  std::string token;
  if (n.getUntilWS (token))
  {
    Variant v;
    if (namedDates (token, v))
    {
      _date = v.get_date ();
      return true;
    }
  }

  n.restore ();
#endif

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Valid epoch values are unsigned integers after 1980-01-01T00:00:00Z. This
// restriction means that '12' will not be identified as an epoch date.
bool ISO8601d::parse_epoch (Nibbler& n)
{
  n.save ();

  int epoch;
  if (n.getUnsignedInt (epoch) &&
      n.depleted ()            &&
      epoch >= 315532800)
  {
    _date = static_cast <time_t> (epoch);
    return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::parse_date_time (Nibbler& n)
{
  n.save ();
  int year, month, day, hour, minute, second;
  if (n.getDigit4 (year)   &&
      n.getDigit2 (month)  && month &&
      n.getDigit2 (day)    && day   &&
      n.skip      ('T')    &&
      n.getDigit2 (hour)   &&
      n.getDigit2 (minute) && minute < 60 &&
      n.getDigit2 (second) && second < 60)
  {
    if (n.skip ('Z'))
      _utc = true;

    _year    = year;
    _month   = month;
    _day     = day;
    _seconds = (((hour * 60) + minute) * 60) + second;

    return true;
  }

  _year    = 0;
  _month   = 0;
  _day     = 0;
  _seconds = 0;

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// date-ext 'T' time-ext 'Z'
// date-ext 'T' time-ext offset-ext
// date-ext 'T' time-ext
bool ISO8601d::parse_date_time_ext (Nibbler& n)
{
  n.save ();
  if (parse_date_ext (n))
  {
    if (n.skip ('T') &&
        parse_time_ext (n))
    {
      if (n.skip ('Z'))
        _utc = true;
      else if (parse_off_ext (n))
        ;

      if (! Lexer::isDigit (n.next ()))
        return true;
    }

    // Restore date_ext
    _year    = 0;
    _month   = 0;
    _week    = 0;
    _weekday = 0;
    _julian  = 0;
    _day     = 0;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// YYYY-MM-DD
// YYYY-DDD
// YYYY-Www-D
// YYYY-Www
bool ISO8601d::parse_date_ext (Nibbler& n)
{
  Nibbler backup (n);
  int year;
  if (n.getDigit4 (year) &&
      n.skip ('-'))
  {
    int month;
    int day;
    if (n.skip ('W') &&
        n.getDigit2 (_week) && _week)
    {
      if (n.skip ('-') &&
          n.getDigit (_weekday))
      {
      }

      _year = year;
      if (!Lexer::isDigit (n.next ()))
        return true;
    }
    else if (n.getDigit3 (_julian) && _julian)
    {
      _year = year;
      if (!Lexer::isDigit (n.next ()))
        return true;
    }
    else if (n.getDigit2 (month) && month &&
             n.skip ('-')        &&
             n.getDigit2 (day)   && day)
    {
      _year = year;
      _month = month;
      _day = day;
      if (!Lexer::isDigit (n.next ()))
        return true;
    }
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// ±hh[:mm]
bool ISO8601d::parse_off_ext (Nibbler& n)
{
  Nibbler backup (n);
  std::string sign;
  if (n.getN (1, sign) && (sign == "+" || sign == "-"))
  {
    int offset;
    int hh;
    int mm;
    if (n.getDigit2 (hh) && hh <= 12 &&
        !n.getDigit (mm))
    {
      offset = hh * 3600;

      if (n.skip (':'))
      {
        if (n.getDigit2 (mm) && mm < 60)
        {
          offset += mm * 60;
        }
        else
        {
          n = backup;
          return false;
        }
      }

      _offset = (sign == "-") ? -offset : offset;
      if (!Lexer::isDigit (n.next ()))
        return true;
    }
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// hh:mm[:ss]
bool ISO8601d::parse_time_ext (Nibbler& n)
{
  Nibbler backup (n);
  int seconds = 0;
  int hh;
  int mm;
  int ss;
  if (n.getDigit2 (hh) && hh <= 24 &&
      n.skip (':')     &&
      n.getDigit2 (mm) && mm < 60)
  {
    seconds = (hh * 3600) + (mm * 60);

    if (n.skip (':'))
    {
      if (n.getDigit2 (ss) && ss < 60)
      {
        seconds += ss;
        _seconds = seconds;

        if (!Lexer::isDigit (n.next ()))
          return true;
      }

      n = backup;
      return false;
    }

    _seconds = seconds;
    if (!Lexer::isDigit (n.next ()))
      return true;
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time-ext 'Z'
bool ISO8601d::parse_time_utc_ext (Nibbler& n)
{
  n.save ();
  if (parse_time_ext (n) &&
      n.skip ('Z'))
  {
    _utc = true;
    if (!Lexer::isDigit (n.next ()))
      return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time-ext offset-ext
bool ISO8601d::parse_time_off_ext  (Nibbler& n)
{
  Nibbler backup (n);
  if (parse_time_ext (n) &&
      parse_off_ext (n))
  {
    if (!Lexer::isDigit (n.next ()))
      return true;
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Validation via simple range checking.
bool ISO8601d::validate ()
{
  // _year;
  if ((_year    && (_year    <   1900 || _year    >                                  2200)) ||
      (_month   && (_month   <      1 || _month   >                                    12)) ||
      (_week    && (_week    <      1 || _week    >                                    53)) ||
      (_weekday && (_weekday <      0 || _weekday >                                     6)) ||
      (_julian  && (_julian  <      1 || _julian  >          ISO8601d::daysInYear (_year))) ||
      (_day     && (_day     <      1 || _day     > ISO8601d::daysInMonth (_month, _year))) ||
      (_seconds && (_seconds <      1 || _seconds >                                 86400)) ||
      (_offset  && (_offset  < -86400 || _offset  >                                 86400)))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// int tm_sec;       seconds (0 - 60)
// int tm_min;       minutes (0 - 59)
// int tm_hour;      hours (0 - 23)
// int tm_mday;      day of month (1 - 31)
// int tm_mon;       month of year (0 - 11)
// int tm_year;      year - 1900
// int tm_wday;      day of week (Sunday = 0)
// int tm_yday;      day of year (0 - 365)
// int tm_isdst;     is summer time in effect?
// char *tm_zone;    abbreviation of timezone name
// long tm_gmtoff;   offset from UTC in seconds
void ISO8601d::resolve ()
{
  // Don't touch the original values.
  int year    = _year;
  int month   = _month;
  int week    = _week;
  int weekday = _weekday;
  int julian  = _julian;
  int day     = _day;
  int seconds = _seconds;
  int offset  = _offset;
  bool utc    = _utc;

  // Get current time.
  time_t now = time (NULL);

  // A UTC offset needs to be accommodated.  Once the offset is subtracted,
  // only local and UTC times remain.
  if (offset)
  {
    seconds -= offset;
    now -= offset;
    utc = true;
  }

  // Get 'now' in the relevant location.
  struct tm* t_now = utc ? gmtime (&now) : localtime (&now);

  int seconds_now = (t_now->tm_hour * 3600) +
                    (t_now->tm_min  *   60) +
                     t_now->tm_sec;

  // Project forward one day if the specified seconds are earlier in the day
  // than the current seconds.
  if (year    == 0 &&
      month   == 0 &&
      day     == 0 &&
      week    == 0 &&
      weekday == 0 &&
      seconds < seconds_now)
  {
    seconds += 86400;
  }

  // Convert week + weekday --> julian.
  if (week)
  {
    julian = (week * 7) + weekday - dayOfWeek (year, 1, 4) - 3;
  }

  // Provide default values for year, month, day.
  else
  {
    // Default values for year, month, day:
    //
    // y   m   d  -->  y   m   d
    // y   m   -  -->  y   m   1
    // y   -   -  -->  y   1   1
    // -   -   -  -->  now now now
    //
    if (year == 0)
    {
      year  = t_now->tm_year + 1900;
      month = t_now->tm_mon + 1;
      day   = t_now->tm_mday;
    }
    else
    {
      if (month == 0)
      {
        month = 1;
        day   = 1;
      }
      else if (day == 0)
        day = 1;
    }
  }

  if (julian)
  {
    month = 1;
    day = julian;
  }

  struct tm t {};
  t.tm_isdst = -1;  // Requests that mktime/gmtime determine summer time effect.
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;

  if (seconds > 86400)
  {
    int days = seconds / 86400;
    t.tm_mday += days;
    seconds %= 86400;
  }

  t.tm_hour = seconds / 3600;
  t.tm_min = (seconds % 3600) / 60;
  t.tm_sec = seconds % 60;

  _date = utc ? timegm (&t) : mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
time_t ISO8601d::toEpoch () const
{
  return _date;
}

////////////////////////////////////////////////////////////////////////////////
std::string ISO8601d::toEpochString () const
{
  std::stringstream epoch;
  epoch << _date;
  return epoch.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 19980119T070000Z =  YYYYMMDDThhmmssZ
std::string ISO8601d::toISO () const
{
  struct tm* t = gmtime (&_date);

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
// 1998-01-19T07:00:00 =  YYYY-MM-DDThh:mm:ss
std::string ISO8601d::toISOLocalExtended () const
{
  struct tm* t = localtime (&_date);

  std::stringstream iso;
  iso << std::setw (4) << std::setfill ('0') << t->tm_year + 1900
      << "-"
      << std::setw (2) << std::setfill ('0') << t->tm_mon + 1
      << "-"
      << std::setw (2) << std::setfill ('0') << t->tm_mday
      << "T"
      << std::setw (2) << std::setfill ('0') << t->tm_hour
      << ":"
      << std::setw (2) << std::setfill ('0') << t->tm_min
      << ":"
      << std::setw (2) << std::setfill ('0') << t->tm_sec;

  return iso.str ();
}

////////////////////////////////////////////////////////////////////////////////
double ISO8601d::toJulian () const
{
  return (_date / 86400.0) + 2440587.5;
}

////////////////////////////////////////////////////////////////////////////////
void ISO8601d::toMDY (int& m, int& d, int& y) const
{
  struct tm* t = localtime (&_date);

  m = t->tm_mon + 1;
  d = t->tm_mday;
  y = t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
const std::string ISO8601d::toString (
  const std::string& format /*= "m/d/Y" */) const
{
  std::stringstream formatted;
  for (unsigned int i = 0; i < format.length (); ++i)
  {
    int c = format[i];
    switch (c)
    {
    case 'm': formatted                                        << this->month ();         break;
    case 'M': formatted << std::setw (2) << std::setfill ('0') << this->month ();         break;
    case 'd': formatted                                        << this->day ();           break;
    case 'D': formatted << std::setw (2) << std::setfill ('0') << this->day ();           break;
    case 'y': formatted << std::setw (2) << std::setfill ('0') << (this->year () % 100);  break;
    case 'Y': formatted                                        << this->year ();          break;
    case 'a': formatted                                        << ISO8601d::dayNameShort (dayOfWeek ()); break;
    case 'A': formatted                                        << ISO8601d::dayName (dayOfWeek ());      break;
    case 'b': formatted                                        << ISO8601d::monthNameShort (month ());   break;
    case 'B': formatted                                        << ISO8601d::monthName (month ());        break;
    case 'v': formatted                                        << ISO8601d::weekOfYear (ISO8601d::dayOfWeek (ISO8601d::weekstart)); break;
    case 'V': formatted << std::setw (2) << std::setfill ('0') << ISO8601d::weekOfYear (ISO8601d::dayOfWeek (ISO8601d::weekstart)); break;
    case 'h': formatted                                        << this->hour ();          break;
    case 'H': formatted << std::setw (2) << std::setfill ('0') << this->hour ();          break;
    case 'n': formatted                                        << this->minute ();        break;
    case 'N': formatted << std::setw (2) << std::setfill ('0') << this->minute ();        break;
    case 's': formatted                                        << this->second ();        break;
    case 'S': formatted << std::setw (2) << std::setfill ('0') << this->second ();        break;
    case 'j': formatted                                        << this->dayOfYear ();     break;
    case 'J': formatted << std::setw (3) << std::setfill ('0') << this->dayOfYear ();     break;
    default:  formatted                                        << static_cast <char> (c); break;
    }
  }

  return formatted.str ();
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d ISO8601d::startOfDay () const
{
  return ISO8601d (month (), day (), year ());
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d ISO8601d::startOfWeek () const
{
  ISO8601d sow (_date);
  sow -= (dayOfWeek () * 86400);
  return ISO8601d (sow.month (), sow.day (), sow.year ());
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d ISO8601d::startOfMonth () const
{
  return ISO8601d (month (), 1, year ());
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d ISO8601d::startOfYear () const
{
  return ISO8601d (1, 1, year ());
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::valid (const std::string& input, const std::string& format /*= ""*/)
{
  try
  {
    ISO8601d test (input, format);
  }

  catch (...)
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::valid (const int m, const int d, const int y, const int hr,
                  const int mi, const int se)
{
  if (hr < 0 || hr > 23)
    return false;

  if (mi < 0 || mi > 59)
    return false;

  if (se < 0 || se > 59)
    return false;

  return ISO8601d::valid (m, d, y);
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::valid (const int m, const int d, const int y)
{
  // Check that the year is valid.
  if (y < 0)
    return false;

  // Check that the month is valid.
  if (m < 1 || m > 12)
    return false;

  // Finally check that the days fall within the acceptable range for this
  // month, and whether or not this is a leap year.
  if (d < 1 || d > ISO8601d::daysInMonth (m, y))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Julian
bool ISO8601d::valid (const int d, const int y)
{
  // Check that the year is valid.
  if (y < 0)
    return false;

  if (d < 1 || d > ISO8601d::daysInYear (y))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Static
bool ISO8601d::leapYear (int year)
{
  return ((! (year % 4)) && (year % 100)) ||
         ! (year % 400);
}

////////////////////////////////////////////////////////////////////////////////
// Static
int ISO8601d::daysInMonth (int month, int year)
{
  static int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month == 2 && ISO8601d::leapYear (year))
    return 29;

  return days[month - 1];
}

////////////////////////////////////////////////////////////////////////////////
// Static
int ISO8601d::daysInYear (int year)
{
  return ISO8601d::leapYear (year) ? 366 : 365;
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string ISO8601d::monthName (int month)
{
  static const char* months[12] =
  {
    STRING_DATE_JANUARY,
    STRING_DATE_FEBRUARY,
    STRING_DATE_MARCH,
    STRING_DATE_APRIL,
    STRING_DATE_MAY,
    STRING_DATE_JUNE,
    STRING_DATE_JULY,
    STRING_DATE_AUGUST,
    STRING_DATE_SEPTEMBER,
    STRING_DATE_OCTOBER,
    STRING_DATE_NOVEMBER,
    STRING_DATE_DECEMBER,
  };

  assert (month > 0);
  assert (month <= 12);
  return Lexer::ucFirst (months[month - 1]);
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string ISO8601d::monthNameShort (int month)
{
  static const char* months[12] =
  {
    STRING_DATE_JANUARY,
    STRING_DATE_FEBRUARY,
    STRING_DATE_MARCH,
    STRING_DATE_APRIL,
    STRING_DATE_MAY,
    STRING_DATE_JUNE,
    STRING_DATE_JULY,
    STRING_DATE_AUGUST,
    STRING_DATE_SEPTEMBER,
    STRING_DATE_OCTOBER,
    STRING_DATE_NOVEMBER,
    STRING_DATE_DECEMBER,
  };

  assert (month > 0);
  assert (month <= 12);
  return Lexer::ucFirst (months[month - 1]).substr (0, 3);
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string ISO8601d::dayName (int dow)
{
  static const char* days[7] =
  {
    STRING_DATE_SUNDAY,
    STRING_DATE_MONDAY,
    STRING_DATE_TUESDAY,
    STRING_DATE_WEDNESDAY,
    STRING_DATE_THURSDAY,
    STRING_DATE_FRIDAY,
    STRING_DATE_SATURDAY,
  };

  return Lexer::ucFirst (days[dow]);
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string ISO8601d::dayNameShort (int dow)
{
  static const char* days[7] =
  {
    STRING_DATE_SUNDAY,
    STRING_DATE_MONDAY,
    STRING_DATE_TUESDAY,
    STRING_DATE_WEDNESDAY,
    STRING_DATE_THURSDAY,
    STRING_DATE_FRIDAY,
    STRING_DATE_SATURDAY,
  };

  return Lexer::ucFirst (days[dow]).substr (0, 3);
}

////////////////////////////////////////////////////////////////////////////////
// Static
int ISO8601d::dayOfWeek (const std::string& input)
{
  if (ISO8601d::minimumMatchLength== 0)
    ISO8601d::minimumMatchLength = 3;

       if (closeEnough (STRING_DATE_SUNDAY,    input, ISO8601d::minimumMatchLength)) return 0;
  else if (closeEnough (STRING_DATE_MONDAY,    input, ISO8601d::minimumMatchLength)) return 1;
  else if (closeEnough (STRING_DATE_TUESDAY,   input, ISO8601d::minimumMatchLength)) return 2;
  else if (closeEnough (STRING_DATE_WEDNESDAY, input, ISO8601d::minimumMatchLength)) return 3;
  else if (closeEnough (STRING_DATE_THURSDAY,  input, ISO8601d::minimumMatchLength)) return 4;
  else if (closeEnough (STRING_DATE_FRIDAY,    input, ISO8601d::minimumMatchLength)) return 5;
  else if (closeEnough (STRING_DATE_SATURDAY,  input, ISO8601d::minimumMatchLength)) return 6;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Using Zeller's Congruence.
// Static
int ISO8601d::dayOfWeek (int year, int month, int day)
{
  int adj = (14 - month) / 12;
  int m = month + 12 * adj - 2;
  int y = year - adj;
  return (day + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
}

////////////////////////////////////////////////////////////////////////////////
// Static
int ISO8601d::monthOfYear (const std::string& input)
{
  if (ISO8601d::minimumMatchLength== 0)
    ISO8601d::minimumMatchLength= 3;

       if (closeEnough (STRING_DATE_JANUARY,   input, ISO8601d::minimumMatchLength)) return 1;
  else if (closeEnough (STRING_DATE_FEBRUARY,  input, ISO8601d::minimumMatchLength)) return 2;
  else if (closeEnough (STRING_DATE_MARCH,     input, ISO8601d::minimumMatchLength)) return 3;
  else if (closeEnough (STRING_DATE_APRIL,     input, ISO8601d::minimumMatchLength)) return 4;
  else if (closeEnough (STRING_DATE_MAY,       input, ISO8601d::minimumMatchLength)) return 5;
  else if (closeEnough (STRING_DATE_JUNE,      input, ISO8601d::minimumMatchLength)) return 6;
  else if (closeEnough (STRING_DATE_JULY,      input, ISO8601d::minimumMatchLength)) return 7;
  else if (closeEnough (STRING_DATE_AUGUST,    input, ISO8601d::minimumMatchLength)) return 8;
  else if (closeEnough (STRING_DATE_SEPTEMBER, input, ISO8601d::minimumMatchLength)) return 9;
  else if (closeEnough (STRING_DATE_OCTOBER,   input, ISO8601d::minimumMatchLength)) return 10;
  else if (closeEnough (STRING_DATE_NOVEMBER,  input, ISO8601d::minimumMatchLength)) return 11;
  else if (closeEnough (STRING_DATE_DECEMBER,  input, ISO8601d::minimumMatchLength)) return 12;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Static
int ISO8601d::length (const std::string& format)
{
  int len = 0;
  for (auto& i : format)
  {
    switch (i)
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
    case 'S': len += 2;  break;
    case 'b':
    case 'j':
    case 'J':
    case 'a': len += 3;  break;
    case 'Y': len += 4;  break;
    case 'A':
    case 'B': len += 10; break;

    // Calculate the width, don't assume a single character width.
    default:  len += mk_wcwidth (i); break;
    }
  }

  return len;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::month () const
{
  struct tm* t = localtime (&_date);
  return t->tm_mon + 1;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::week () const
{
  return ISO8601d::weekOfYear (ISO8601d::dayOfWeek (ISO8601d::weekstart));
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::day () const
{
  struct tm* t = localtime (&_date);
  return t->tm_mday;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::year () const
{
  struct tm* t = localtime (&_date);
  return t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::weekOfYear (int weekStart) const
{
  struct tm* t = localtime (&_date);
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
int ISO8601d::dayOfWeek () const
{
  struct tm* t = localtime (&_date);
  return t->tm_wday;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::dayOfYear () const
{
  struct tm* t = localtime (&_date);
  return t->tm_yday + 1;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::hour () const
{
  struct tm* t = localtime (&_date);
  return t->tm_hour;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::minute () const
{
  struct tm* t = localtime (&_date);
  return t->tm_min;
}

////////////////////////////////////////////////////////////////////////////////
int ISO8601d::second () const
{
  struct tm* t = localtime (&_date);
  return t->tm_sec;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::operator== (const ISO8601d& rhs) const
{
  return rhs._date == _date;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::operator!= (const ISO8601d& rhs) const
{
  return rhs._date != _date;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::operator<  (const ISO8601d& rhs) const
{
  return _date < rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::operator> (const ISO8601d& rhs) const
{
  return _date > rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::operator<= (const ISO8601d& rhs) const
{
  return _date <= rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::operator>= (const ISO8601d& rhs) const
{
  return _date >= rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::sameHour (const ISO8601d& rhs) const
{
  return this->year ()  == rhs.year ()  &&
         this->month () == rhs.month () &&
         this->day ()   == rhs.day ()   &&
         this->hour ()  == rhs.hour ();
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::sameDay (const ISO8601d& rhs) const
{
  return this->year ()  == rhs.year ()  &&
         this->month () == rhs.month () &&
         this->day ()   == rhs.day ();
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::sameWeek (const ISO8601d& rhs) const
{
  return this->year () == rhs.year () &&
         this->week () == rhs.week ();
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::sameMonth (const ISO8601d& rhs) const
{
  return this->year ()  == rhs.year () &&
         this->month () == rhs.month ();
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601d::sameYear (const ISO8601d& rhs) const
{
  return this->year () == rhs.year ();
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d ISO8601d::operator+ (const int delta)
{
  return ISO8601d (_date + delta);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d ISO8601d::operator- (const int delta)
{
  return ISO8601d (_date - delta);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d& ISO8601d::operator+= (const int delta)
{
  _date += (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d& ISO8601d::operator-= (const int delta)
{
  _date -= (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
time_t ISO8601d::operator- (const ISO8601d& rhs)
{
  return _date - rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
// Prefix decrement by one day.
void ISO8601d::operator-- ()
{
  ISO8601d yesterday = startOfDay () - 1;
  yesterday = ISO8601d (yesterday.month (),
                        yesterday.day (),
                        yesterday.year (),
                        hour (),
                        minute (),
                        second ());
  _date = yesterday._date;
}

////////////////////////////////////////////////////////////////////////////////
// Postfix decrement by one day.
void ISO8601d::operator-- (int)
{
  ISO8601d yesterday = startOfDay () - 1;
  yesterday = ISO8601d (yesterday.month (),
                        yesterday.day (),
                        yesterday.year (),
                        hour (),
                        minute (),
                        second ());
  _date = yesterday._date;
}

////////////////////////////////////////////////////////////////////////////////
// Prefix increment by one day.
void ISO8601d::operator++ ()
{
  ISO8601d tomorrow = (startOfDay () + 90001).startOfDay ();
  tomorrow = ISO8601d (tomorrow.month (),
                       tomorrow.day (),
                       tomorrow.year (),
                       hour (),
                       minute (),
                       second ());
  _date = tomorrow._date;
}

////////////////////////////////////////////////////////////////////////////////
// Postfix increment by one day.
void ISO8601d::operator++ (int)
{
  ISO8601d tomorrow = (startOfDay () + 90001).startOfDay ();
  tomorrow = ISO8601d (tomorrow.month (),
                       tomorrow.day (),
                       tomorrow.year (),
                       hour (),
                       minute (),
                       second ());
  _date = tomorrow._date;
}

////////////////////////////////////////////////////////////////////////////////
/*
std::string ISO8601d::dump () const
{
  std::stringstream s;
  s << "ISO8601d"
    << " y"   << _year
    << " m"   << _month
    << " w"   << _week
    << " wd"  << _weekday
    << " j"   << _julian
    << " d"   << _day
    << " s"   << _seconds
    << " off" << _offset
    << " utc" << _utc
    << " ="   << _date
    << "  "   << (_date ? toISO () : "");

  return s.str ();
}
*/

////////////////////////////////////////////////////////////////////////////////
ISO8601p::ISO8601p ()
{
  clear ();
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::ISO8601p (time_t input)
{
  clear ();
  _period = input;
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::ISO8601p (const std::string& input)
{
  clear ();

  if (Lexer::isAllDigits (input))
  {
    time_t value = (time_t) strtol (input.c_str (), NULL, 10);
    if (value == 0 || value > 60)
    {
      _period = value;
      return;
    }
  }

  std::string::size_type idx = 0;
  parse (input, idx);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::~ISO8601p ()
{
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p& ISO8601p::operator= (const ISO8601p& other)
{
  if (this != &other)
  {
    _year    = other._year;
    _month   = other._month;
    _day     = other._day;
    _hours   = other._hours;
    _minutes = other._minutes;
    _seconds = other._seconds;
    _period  = other._period;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601p::operator< (const ISO8601p& other)
{
  return _period < other._period;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601p::operator> (const ISO8601p& other)
{
  return _period > other._period;
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::operator std::string () const
{
  std::stringstream s;
  s << _period;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::operator time_t () const
{
  return _period;
}

////////////////////////////////////////////////////////////////////////////////
// Supported:
//
//    duration   ::= designated                                     # duration
//
//    designated ::= 'P' [nn 'Y'] [nn 'M'] [nn 'D'] ['T' [nn 'H'] [nn 'M'] [nn 'S']]
//
// Not supported:
//
//    duration   ::= designated '/' datetime-ext                    # duration end
//                 | degignated '/' datetime                        # duration end
//                 | designated                                     # duration
//                 | 'P' datetime-ext '/' datetime-ext              # start end
//                 | 'P' datetime '/' datetime                      # start end
//                 | 'P' datetime-ext                               # start
//                 | 'P' datetime                                   # start
//                 | datetime-ext '/' designated                    # start duration
//                 | datetime-ext '/' 'P' datetime-ext              # start end
//                 | datetime-ext '/' datetime-ext                  # start end
//                 | datetime '/' designated                        # start duration
//                 | datetime '/' 'P' datetime                      # start end
//                 | datetime '/' datetime                          # start end
//                 ;
//
bool ISO8601p::parse (const std::string& input, std::string::size_type& start)
{
  // Attempt and ISO parse first.
  auto original_start = start;
  Nibbler n (input.substr (original_start));
  n.save ();

  if (parse_designated (n))
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      // Record cursor position.
      start = n.cursor ();

      resolve ();
      return true;
    }
  }

  // Attempt a legacy format parse next.
  n.restore ();

  // Static and so preserved between calls.
  static std::vector <std::string> units;
  if (units.size () == 0)
    for (unsigned int i = 0; i < NUM_DURATIONS; i++)
      units.push_back (durations[i].unit);

  std::string number;
  std::string unit;

  if (n.getOneOf (units, unit))
  {
    if (n.depleted ()                           ||
        Lexer::isWhitespace         (n.next ()) ||
        Lexer::isSingleCharOperator (n.next ()))
    {
      start = original_start + n.cursor ();

      // Linear lookup - should instead be logarithmic.
      for (unsigned int i = 0; i < NUM_DURATIONS; i++)
      {
        if (durations[i].unit == unit &&
            durations[i].standalone == true)
        {
          _period = static_cast <int> (durations[i].seconds);
          return true;
        }
      }
    }
  }

  else if (n.getNumber (number) &&
           number.find ('e') == std::string::npos &&
           number.find ('E') == std::string::npos &&
           (number.find ('+') == std::string::npos || number.find ('+') == 0) &&
           (number.find ('-') == std::string::npos || number.find ('-') == 0))
  {
    n.skipWS ();
    if (n.getOneOf (units, unit))
    {
      // The "d" unit is a special case, because it is the only one that can
      // legitimately occur at the beginning of a UUID, and be followed by an
      // operator:
      //
      //   1111111d-0000-0000-0000-000000000000
      //
      // Because Lexer::isDuration is higher precedence than Lexer::isUUID,
      // the above UUID looks like:
      //
      //   <1111111d> <-> ...
      //   duration   op  ...
      //
      // So as a special case, durations, with units of "d" are rejected if the
      // quantity exceeds 10000.
      //
      if (unit == "d" &&
          strtol (number.c_str (), NULL, 10) > 10000)
        return false;

      if (n.depleted ()                           ||
          Lexer::isWhitespace         (n.next ()) ||
          Lexer::isSingleCharOperator (n.next ()))
      {
        start = original_start + n.cursor ();
        double quantity = strtod (number.c_str (), NULL);

        // Linear lookup - should instead be logarithmic.
        double seconds = 1;
        for (unsigned int i = 0; i < NUM_DURATIONS; i++)
        {
          if (durations[i].unit == unit)
          {
            seconds = durations[i].seconds;
            _period = static_cast <int> (quantity * static_cast <double> (seconds));
            return true;
          }
        }
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void ISO8601p::clear ()
{
  _year    = 0;
  _month   = 0;
  _day     = 0;
  _hours   = 0;
  _minutes = 0;
  _seconds = 0;
  _period  = 0;
}

////////////////////////////////////////////////////////////////////////////////
const std::string ISO8601p::format () const
{
  if (_period)
  {
    time_t t = _period;
    int seconds = t % 60; t /= 60;
    int minutes = t % 60; t /= 60;
    int hours   = t % 24; t /= 24;
    int days    = t;

    std::stringstream s;
    s << 'P';
    if (days)   s << days   << 'D';

    if (hours || minutes || seconds)
    {
      s << 'T';
      if (hours)   s << hours   << 'H';
      if (minutes) s << minutes << 'M';
      if (seconds) s << seconds << 'S';
    }

    return s.str ();
  }
  else
  {
    return "PT0S";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Range      Representation
// ---------  ---------------------
// >= 365d    {n.n}y
// >= 90d     {n}mo
// >= 14d     {n}w
// >= 1d      {n}d
// >= 1h      {n}h
// >= 1min    {n}min
//            {n}s
//
const std::string ISO8601p::formatVague () const
{
  float days = (float) _period / 86400.0;

  std::stringstream formatted;
       if (_period >= 86400 * 365) formatted << std::fixed << std::setprecision (1) << (days / 365) << "y";
  else if (_period >= 86400 * 90)  formatted << static_cast <int> (days / 30)       << "mo";
  else if (_period >= 86400 * 14)  formatted << static_cast <int> (days / 7)        << "w";
  else if (_period >= 86400)       formatted << static_cast <int> (days)            << "d";
  else if (_period >= 3600)        formatted << static_cast <int> (_period / 3600)  << "h";
  else if (_period >= 60)          formatted << static_cast <int> (_period / 60)    << "min";
  else if (_period >= 1)           formatted << static_cast <int> (_period)         << "s";

  return formatted.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 'P' [nn 'Y'] [nn 'M'] [nn 'D'] ['T' [nn 'H'] [nn 'M'] [nn 'S']]
bool ISO8601p::parse_designated (Nibbler& n)
{
  Nibbler backup (n);

  if (n.skip ('P'))
  {
    int value;
    n.save ();
    if (n.getUnsignedInt (value) && n.skip ('Y'))
      _year = value;
    else
      n.restore ();

    n.save ();
    if (n.getUnsignedInt (value) && n.skip ('M'))
      _month = value;
    else
      n.restore ();

    n.save ();
    if (n.getUnsignedInt (value) && n.skip ('D'))
      _day = value;
    else
      n.restore ();

    if (n.skip ('T'))
    {
      n.save ();
      if (n.getUnsignedInt (value) && n.skip ('H'))
        _hours = value;
      else
        n.restore ();

      n.save ();
      if (n.getUnsignedInt (value) && n.skip ('M'))
        _minutes = value;
      else
        n.restore ();

      n.save ();
      if (n.getUnsignedInt (value) && n.skip ('S'))
        _seconds = value;
      else
        n.restore ();
    }

    return true;
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool ISO8601p::validate ()
{
  return _year    ||
         _month   ||
         _day     ||
         _hours   ||
         _minutes ||
         _seconds;
}

////////////////////////////////////////////////////////////////////////////////
// Allow un-normalized values.
void ISO8601p::resolve ()
{
  _period = (_year  * 365 * 86400) +
            (_month  * 30 * 86400) +
            (_day         * 86400) +
            (_hours       *  3600) +
            (_minutes     *    60) +
            _seconds;
}

////////////////////////////////////////////////////////////////////////////////
/*
std::string ISO8601p::dump () const
{
  std::stringstream s;
  s << "ISO8601p"
    << " y"  << _year
    << " mo" << _month
    << " d"  << _day
    << " h"  << _hours
    << " mi" << _minutes
    << " s"  << _seconds
    << " ="  << _period
    << "  "         << (_period ? format () : "");

  return s.str ();
}
*/

////////////////////////////////////////////////////////////////////////////////
