////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <text.h>
#include <Dates.h>

static const char* days[] =
{
  "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday",
};

static const char* days_short[] =
{
  "sun", "mon", "tue", "wed", "thu", "fri", "sat",
};

static const char* months[] =
{
  "january", "february", "march", "april", "may", "june",
  "july", "august", "september", "october", "november", "december",
};

static const char* months_short[] =
{
  "jan", "feb", "mar", "apr", "may", "jun",
  "jul", "aug", "sep", "oct", "nov", "dec",
};

static int month_days[12] =
{
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

////////////////////////////////////////////////////////////////////////////////
static bool isMonth (const std::string& name, int& i)
{
  for (i = 0; i < 12; i++)
    if (name == months[i] || name == months_short[i])
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool isDay (const std::string& name, int& i)
{
  for (i = 0; i < 7; i++)
    if (name == days[i] || name == days_short[i])
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool leapYear (int year)
{
  return ((!(year % 4)) && (year % 100)) ||
         (!(year % 400));
}

////////////////////////////////////////////////////////////////////////////////
static int daysInMonth (int year, int month)
{
  if (month == 2 && leapYear (year))
    return 29;

  return month_days[month - 1];
}

////////////////////////////////////////////////////////////////////////////////
// now            = current date/time.
// today          = previous midnight.
// sod            = previous midnight.
// yesterday      = 2nd previous midnight.
// tomorrow       = next midnight.
// eod            = next midnight.
// <day>          = midnight at start of next <day>.
// <month>        = midnight on the 1st of <month>.
// soy            = midnight on January 1st, <year>.
// eoy            = midnight on December 31st, <year>.
// socm           = midnight on the 1st of current month.
// som            = midnight on the 1st of next month.
// eom            = midnight on the 1st of the next month.
// eocm           = midnight on the 1st of the next month.
// sow            =
// eow            =
// eocw           =
// socw           =
// soww           =
// eoww           =
// soq            =
// eoq            =
// later          = midnight, Jan 18th, 2038.
// someday        = midnight, Jan 18th, 2038.
// easter         =
// eastermonday   =
// ascension      =
// pentecost      =
// goodfriday     =
// midsommar      =
// midsommarafton =
// Nth            =

bool namedDates (const std::string& name, Variant& value)
{
  time_t now = time (NULL);
  int i;

  // TODO Extract helper functions from this code.

  // Dynamics.
  if (name == "now")
  {
    value = Variant (now, Variant::type_date);
  }

  else if (name == "today" || name == "sod")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "yesterday")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    value = Variant (mktime (t) - 86400, Variant::type_date);
  }

  else if (name == "tomorrow" || name == "eod")
  {
    struct tm* t = localtime (&now);
    t->tm_mday++;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (isDay (name, i))
  {
    struct tm* t = localtime (&now);

    if (t->tm_wday >= i)
      t->tm_mday += i - t->tm_wday + 7;
    else
      t->tm_mday += i - t->tm_wday;

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (isMonth (name, i))
  {
    struct tm* t = localtime (&now);
    if (t->tm_mon >= i)
      t->tm_year++;

    t->tm_mon = i;
    t->tm_mday = 1;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "soy")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon = 0;
    t->tm_mday = 1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "eoy")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon = 11;
    t->tm_mday = 31;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "socm")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mday = 1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "som")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;

    t->tm_mon++;
    if (t->tm_mon == 12)
    {
      t->tm_year++;
      t->tm_mon = 0;
    }

    t->tm_mday = 1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "eom" || name == "eocm")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mday = daysInMonth (t->tm_year + 1900, t->tm_mon + 1);
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "sow")
  {
/*
    Date sow (_t);
    sow -= (dayOfWeek () * 86400);
    return Date (sow.month (), sow.day (), sow.year (), 0, 0, 0);
*/
  }

  else if (name == "eow" || name == "eocw")
  {
/*
    if (found == "eow" || found == "eoww")
      dow = 5;
*/
  }

  else if (name == "socw")
  {
/*
    Date sow (_t);
    sow -= (dayOfWeek () * 86400);
    return Date (sow.month (), sow.day (), sow.year (), 0, 0, 0);
*/
  }

  else if (name == "soww")
  {
/*
    Date sow (_t);
    sow -= (dayOfWeek () * 86400);
    return Date (sow.month (), sow.day (), sow.year (), 0, 0, 0);
*/
  }

  else if (name == "eoww")
  {
/*
    if (found == "eow" || found == "eoww")
      dow = 5;
*/
  }

  else if (name == "soq" || name == "eoq")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;

    t->tm_mon += 3 - (t->tm_mon % 3);
    if (t->tm_mon > 11)
    {
      t->tm_mon -= 12;
      ++t->tm_year;
    }

    // TODO eoq: should be 24:00:00
    // t->tm_mday = daysInMonth (t->tm_year + 1900, t->tm_mon + 1);

    t->tm_mday = 1;

    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "later" || name == "someday")
  {
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_year = 138;
    t->tm_mon = 0;
    t->tm_mday = 18;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "easter"       ||
           name == "eastermonday" ||
           name == "ascension"    ||
           name == "pentecost"    ||
           name == "goodfriday")
  {
    struct tm* t = localtime (&now);

    int Y = t->tm_year + 1900;
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

    t->tm_isdst = -1;   // Requests that mktime determine summer time effect.
    t->tm_mday  = day;
    t->tm_mon   = month - 1;
    t->tm_year  = Y - 1900;
    t->tm_hour = t->tm_min = t->tm_sec = 0;

         if (name == "goodfriday")   t->tm_mday -= 2;
    else if (name == "eastermonday") t->tm_mday += 1;
    else if (name == "ascension")    t->tm_mday += 39;
    else if (name == "pentecost")    t->tm_mday += 49;

    value = Variant (mktime (t), Variant::type_date);
  }

  else if (name == "midsommar")
  {
    struct tm* t = localtime (&now);
    t->tm_mon = 5;                          // June.
    t->tm_mday = 20;                        // Saturday after 20th.
    t->tm_hour = t->tm_min = t->tm_sec = 0; // Midnight.

    time_t then = mktime (t);
    struct tm* mid = localtime (&then);
    int offset = 6 - mid->tm_wday;          // How many days after 20th.

    mid->tm_mday += offset;
    value = Variant (mktime (mid), Variant::type_date);
  }

  else if (name == "midsommarafton")
  {
    struct tm* t = localtime (&now);
    t->tm_mon = 5;                          // June.
    t->tm_mday = 19;                        // Friday after 19th.
    t->tm_hour = t->tm_min = t->tm_sec = 0; // Midnight.

    time_t then = mktime (t);
    struct tm* mid = localtime (&then);
    int offset = 5 - mid->tm_wday;          // How many days after 19th.

    mid->tm_mday += offset;
    value = Variant (mktime (mid), Variant::type_date);
  }

  // Support "21st" to indicate the next date that is the 21st day.
  // 1st
  // 2nd
  // 3rd
  else if (name.length () >= 3 &&
           name.length () <= 4 &&
           isdigit (name[0]))
  {
    int number;
    std::string ordinal;

    if (isdigit (name[1]))
    {
      number = strtol (name.substr (0, 2).c_str (), NULL, 10);
      ordinal = lowerCase (name.substr (2));
    }
    else
    {
      number = strtol (name.substr (0, 2).c_str (), NULL, 10);
      ordinal = lowerCase (name.substr (1));
    }

    // Sanity check.
    if (number <= 31)
    {
      if (ordinal == "st" ||
          ordinal == "nd" ||
          ordinal == "rd" ||
          ordinal == "th")
      {
        struct tm* t = localtime (&now);
        int y = t->tm_year + 1900;
        int m = t->tm_mon + 1;
        int d = t->tm_mday;

        // If it is this month.
        if (d < number &&
            number <= daysInMonth (m, y))
        {
          t->tm_hour = t->tm_min = t->tm_sec = 0;
          t->tm_mon  = m - 1;
          t->tm_mday = number;
          t->tm_year = y - 1900;
          value = Variant (mktime (t), Variant::type_date);
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
        while (number > daysInMonth (m, y));

        t->tm_hour = t->tm_min = t->tm_sec = 0;
        t->tm_mon  = m - 1;
        t->tm_mday = number;
        t->tm_year = y - 1900;
        value = Variant (mktime (t), Variant::type_date);
        return true;
      }
    }
  }

  else
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

