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
#include <ISO8601.h>

////////////////////////////////////////////////////////////////////////////////
ISO8601d::ISO8601d ()
{
  clear ();
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::~ISO8601d ()
{
}

////////////////////////////////////////////////////////////////////////////////
ISO8601d::operator time_t () const
{
  return _value;
}

////////////////////////////////////////////////////////////////////////////////
// By default, ISO8601d allows ambiguous dates, such as YYYY, YYYYMMDD, HHMMSS.
// These are also valid numbers.  Setting ambiguity to false inhibites the
// parsing of these as dates.
void ISO8601d::ambiguity (bool value)
{
  _ambiguity = value;
}

////////////////////////////////////////////////////////////////////////////////
// Supported:
//
//    result       ::= date-ext 'T' time-ext 'Z'              # UTC
//                   | date-ext 'T' time-ext offset-ext       # Specified TZ
//                   | date-ext 'T' time-ext                  # Local
//                   | date-ext                               # Local
//                   | date 'T' time 'Z'
//                   | date 'T' time offset-ext
//                   | date 'T' time
//                   | date
//                   | time-ext 'Z'
//                   | time-ext offset-ext
//                   | time-ext
//                   | time 'Z'
//                   | time offset
//                   | time
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
//    date         ::= ±YYYYYMMDD                     Νot needed
//                   | ±YYYYYWwwD                     Νot needed
//                   | ±YYYYYWww                      Νot needed
//                   | ±YYYYYDDD                      Νot needed
//                   | ±YYYYYMM                       Νot needed
//                   | ±YYYYY                         Νot needed
//                   | ±YYY                           Νot needed
//                   | YYYYMMDD                       Ambiguous (number)
//                   | YYYYWwwD
//                   | YYYYWww
//                   | YYYYDDD                        Ambiguous (number)
//                   | YYYY-MM
//                   | YYYY                           Ambiguous (number)
//                   | YY                             Ambiguous (number)
//                   ;
//
//    time-ext     ::= hh:mm:ss[,ss]
//                   | hh:mm[,mm]
//                   | hh[,hh]                        Ambiguous (number)
//                   ;
//
//    time         ::= hhmmss[,ss]                    Ambiguous (number)
//                   | hhmm[,mm]                      Ambiguous (number)
//                   | hh[,hh]                        Ambiguous (number)
//                   ;
//
//    time-utc-ext ::= hh:mm[:ss] 'Z' ;
//    time-utc     ::= hh[mm[ss]] 'Z' ;
//
//    offset-ext   ::= ±hh[:mm] ;
//    offset       ::= ±hh[mm] ;
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
bool ISO8601d::parse (const std::string& input, std::string::size_type& start)
{
  std::string::size_type i = start;
  Nibbler n (input.substr (i));

  if (parse_date_time_ext (n)             ||   // Most complex first.
      parse_date_ext      (n)             ||
      parse_time_utc_ext  (n)             ||
      parse_time_off_ext  (n)             ||
      parse_date_time     (n)             ||
      parse_date          (n, _ambiguity) ||
      parse_time_utc      (n)             ||
      parse_time_off      (n)             ||
      parse_time_ext      (n)             ||   // Time last, as it is the most permissive.
      parse_time          (n, _ambiguity))
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

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void ISO8601d::clear ()
{
  _ambiguity       = true;
  _year            = 0;
  _month           = 0;
  _week            = 0;
  _weekday         = 0;
  _julian          = 0;
  _day             = 0;
  _seconds         = 0;
  _offset          = 0;
  _utc             = false;
  _value           = 0;
  _default_seconds = 0;
}

////////////////////////////////////////////////////////////////////////////////
void ISO8601d::set_default_time (int hours, int minutes, int seconds)
{
  _default_seconds = (hours * 3600) + (minutes * 60) + seconds;
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

      if (! isdigit (n.next ()))
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
// date 'T' time 'Z'
// date 'T' time offset
// date 'T' time
bool ISO8601d::parse_date_time (Nibbler& n)
{
  Nibbler backup (n);
  if (parse_date (n, true))
  {
    if (n.skip ('T') &&
        parse_time (n, true))
    {
      if (n.skip ('Z'))
      {
        _utc = true;
        if (!isdigit (n.next ()))
          return true;
      }
      else if (parse_off (n))
      {
        if (!isdigit (n.next ()))
          return true;
      }

      if (!isdigit (n.next ()))
        return true;
    }

    // Restore date
    _year    = 0;
    _month   = 0;
    _week    = 0;
    _weekday = 0;
    _julian  = 0;
    _day     = 0;
  }

  n = backup;
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
        n.getDigit2 (_week))
    {
      if (n.skip ('-') &&
          n.getDigit (_weekday))
      {
      }

      _year = year;
      if (!isdigit (n.next ()))
        return true;
    }
    else if (n.getDigit3 (_julian))
    {
      _year = year;
      if (!isdigit (n.next ()))
        return true;
    }
    else if (n.getDigit2 (month) &&
             n.skip ('-')        &&
             n.getDigit2 (day))
    {
      _year = year;
      _month = month;
      _day = day;
      if (!isdigit (n.next ()))
        return true;
    }
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// YYYYMMDD              Ambiguous (number)
// YYYYWwwD
// YYYYWww
// YYYYDDD               Ambiguous (number)
// YYYY-MM
bool ISO8601d::parse_date (Nibbler& n, bool ambiguous)
{
  Nibbler backup (n);
  int year;
  if (n.getDigit4 (year))
  {
    int month;
    if (n.skip ('W'))
    {
      int week;
      if (n.getDigit2 (week))
      {
        _week = week;

        int day;
        if (n.getDigit (day))
          _weekday = day;

        _year = year;
        if (!isdigit (n.next ()))
          return true;
      }
    }
    else if (n.skip ('-'))
    {
      if (n.getDigit2 (_month))
      {
        _year = year;
        if (!isdigit (n.next ()))
          return true;
      }
    }
    else if (n.getDigit4 (month))
    {
      _year = year;
      _month = month / 100;
      _day = month % 100;
      if (!isdigit (n.next ()))
        return true;
    }
    else if (ambiguous && n.getDigit3 (_julian))
    {
      _year = year;
      if (!isdigit (n.next ()))
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
  if (n.getN (1, sign))
  {
    if (sign == "+" || sign == "-")
    {
      int offset;
      int hh;
      int mm;
      if (n.getDigit2 (hh) &&
          !n.getDigit (mm))
      {
        offset = hh * 3600;
        if (n.skip (':') &&
            n.getDigit2 (mm))
          offset += mm * 60;

        _offset = (sign == "-") ? -offset : offset;
        if (!isdigit (n.next ()))
          return true;
      }
    }
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// ±hh[mm]
bool ISO8601d::parse_off (Nibbler& n)
{
  Nibbler backup (n);
  std::string sign;
  if (n.getN (1, sign))
  {
    if (sign == "+" || sign == "-")
    {
      int offset;
      int hh;
      if (n.getDigit2 (hh))
      {
        offset = hh * 3600;
        int mm;
        if (n.getDigit2 (mm))
          offset += mm * 60;

        _offset = (sign == "-") ? -offset : offset;
        if (!isdigit (n.next ()))
          return true;
      }
    }
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// hh[:mm[:ss]]
bool ISO8601d::parse_time_ext (Nibbler& n)
{
  Nibbler backup (n);
  int seconds = 0;
  int hh;
  int mm;
  int ss;
  if (n.getDigit2 (hh) &&
      !n.getDigit (mm))
  {
    seconds = hh * 3600;

    if (n.skip (':') &&
        n.getDigit2 (mm) &&
        !n.getDigit (ss))
    {
      seconds += mm * 60;

      if (n.skip (':') &&
          n.getDigit2 (ss))
        seconds += ss;

      _seconds = seconds;
      return true;
    }

    if (_ambiguity)
    {
      _seconds = seconds;
      if (!isdigit (n.next ()))
        return true;
    }
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// hh[mm[ss]]
bool ISO8601d::parse_time (Nibbler& n, bool ambiguous)
{
  if (!ambiguous)
    return false;

  Nibbler backup (n);
  int seconds = 0;
  int hh;
  if (n.getDigit2 (hh))
  {
    seconds = hh * 3600;

    int mm;
    if (n.getDigit2 (mm))
    {
      seconds += mm * 60;

      int ss;
      if (n.getDigit2 (ss))
        seconds += ss;
    }

    _seconds = seconds;
    if (!isdigit (n.next ()))
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
    if (!isdigit (n.next ()))
      return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time 'Z'
bool ISO8601d::parse_time_utc (Nibbler& n)
{
  n.save ();
  if (parse_time (n, true) &&
      n.skip ('Z'))
  {
    _utc = true;
    if (!isdigit (n.next ()))
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
    if (!isdigit (n.next ()))
      return true;
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time offset
bool ISO8601d::parse_time_off  (Nibbler& n)
{
  Nibbler backup (n);
  if (parse_time (n, true) &&
      parse_off (n))
  {
    if (!isdigit (n.next ()))
      return true;
  }

  n = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Using Zeller's Congruence.
int ISO8601d::dayOfWeek (int year, int month, int day)
{
  int adj = (14 - month) / 12;
  int m = month + 12 * adj - 2;
  int y = year - adj;
  return (day + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
}

////////////////////////////////////////////////////////////////////////////////
// Validation via simple range checking.
bool ISO8601d::validate ()
{
  // _year;
  if ((_year    && (_year    <   1900 || _year    >  2100)) ||
      (_month   && (_month   <      1 || _month   >    12)) ||
      (_week    && (_week    <      1 || _week    >    53)) ||
      (_weekday && (_weekday <      0 || _weekday >     6)) ||
      (_julian  && (_julian  <      0 || _julian  >   366)) ||
      (_day     && (_day     <      1 || _day     >    31)) ||
      (_seconds && (_seconds <      1 || _seconds > 86400)) ||
      (_offset  && (_offset  < -86400 || _offset  > 86400)))
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

  struct tm t = {0};
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

  _value = utc ? timegm (&t) : timelocal (&t);
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::ISO8601p ()
{
  clear ();
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::~ISO8601p ()
{
}

////////////////////////////////////////////////////////////////////////////////
ISO8601p::operator time_t () const
{
  return _value;
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
  std::string::size_type i = start;
  Nibbler n (input.substr (i));

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
  _value   = 0;
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
  _value = (_year  * 365 * 86400) +
           (_month  * 30 * 86400) +
           (_day         * 86400) +
           (_hours       *  3600) +
           (_minutes     *    60) +
           _seconds;
}

////////////////////////////////////////////////////////////////////////////////
