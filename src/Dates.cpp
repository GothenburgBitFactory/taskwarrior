////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <Dates.h>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <text.h>
#include <ISO8601.h>
#include <Lexer.h>
#include <CLI2.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
static bool isMonth (const std::string& name, int& i)
{
  i = ISO8601d::monthOfYear (name) - 1;
  return i != -2 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
static bool isDay (const std::string& name, int& i)
{
  i = ISO8601d::dayOfWeek (name);
  return i != -1 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
static void easter (struct tm* t)
{
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
  t->tm_isdst = -1;
  t->tm_hour = t->tm_min = t->tm_sec = 0;
}

////////////////////////////////////////////////////////////////////////////////
static void midsommar (struct tm* t)
{
  t->tm_mon = 5;                          // June.
  t->tm_mday = 20;                        // Saturday after 20th.
  t->tm_hour = t->tm_min = t->tm_sec = 0; // Midnight.
  t->tm_isdst = -1;                       // Probably DST, but check.

  time_t then = mktime (t);               // Obtain the weekday of June 20th.
  struct tm* mid = localtime (&then);
  t->tm_mday += 6 - mid->tm_wday;         // How many days after 20th.
}

////////////////////////////////////////////////////////////////////////////////
static void midsommarafton (struct tm* t)
{
  t->tm_mon = 5;                          // June.
  t->tm_mday = 19;                        // Saturday after 20th.
  t->tm_hour = t->tm_min = t->tm_sec = 0; // Midnight.
  t->tm_isdst = -1;                       // Probably DST, but check.

  time_t then = mktime (t);               // Obtain the weekday of June 19th.
  struct tm* mid = localtime (&then);
  t->tm_mday += 5 - mid->tm_wday;         // How many days after 19th.
}

////////////////////////////////////////////////////////////////////////////////
// Note how these are all single words:
//   <day>
//   <month>
//   Nth
//   socy, eocy
//   socq, eocq
//   socm, eocm
//   som, eom
//   soq, eoq
//   soy, eoy
//   socw, eocw
//   sow, eow
//   soww, eoww
//   sod, eod
//   yesterday
//   today
//   now
//   tomorrow
//   later          = midnight, Jan 18th, 2038.
//   someday        = midnight, Jan 18th, 2038.
//   easter
//   eastermonday
//   ascension
//   pentecost
//   goodfriday
//   midsommar      = midnight, 1st Saturday after 20th June
//   midsommarafton = midnight, 1st Friday after 19th June
//
bool namedDates (const std::string& name, Variant& value)
{
  time_t now = time (NULL);
  struct tm* t = localtime (&now);
  int i;

  int minimum = CLI2::minimumMatchLength;
  if (minimum == 0)
    minimum = 3;

  // Dynamics.
  if (closeEnough ("now", name, minimum))
  {
    value = Variant (now, Variant::type_date);
  }

  else if (closeEnough ("today", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("sod", name, minimum))
  {
    t->tm_mday++;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("eod", name, minimum))
  {
    t->tm_mday++;
    t->tm_hour = t->tm_min = 0;
    t->tm_sec = -1;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("tomorrow", name, minimum))
  {
    t->tm_mday++;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("yesterday", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    value = Variant (mktime (t) - 86400, Variant::type_date);
  }

  else if (isDay (name, i))
  {
    if (t->tm_wday >= i)
      t->tm_mday += i - t->tm_wday + 7;
    else
      t->tm_mday += i - t->tm_wday;

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (isMonth (name, i))
  {
    if (t->tm_mon >= i)
      t->tm_year++;

    t->tm_mon = i;
    t->tm_mday = 1;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("later",   name, minimum) ||
           closeEnough ("someday", name, std::max (minimum, 4)))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_year = 138;
    t->tm_mon = 0;
    t->tm_mday = 18;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("eoy", name, minimum))
  {
    t->tm_hour = t->tm_min = 0;
    t->tm_sec = -1;
    t->tm_mon = 0;
    t->tm_mday = 1;
    t->tm_year++;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("soy", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon = 0;
    t->tm_mday = 1;
    t->tm_year++;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("eoq", name, minimum))
  {
    t->tm_hour = t->tm_min = 0;
    t->tm_sec = -1;
    t->tm_mon += 3 - (t->tm_mon % 3);
    if (t->tm_mon > 11)
    {
      t->tm_mon -= 12;
      ++t->tm_year;
    }

    t->tm_mday = 1;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("soq", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon += 3 - (t->tm_mon % 3);
    if (t->tm_mon > 11)
    {
      t->tm_mon -= 12;
      ++t->tm_year;
    }

    t->tm_mday = 1;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("socm", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mday = 1;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("som", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;

    t->tm_mon++;
    if (t->tm_mon == 12)
    {
      t->tm_year++;
      t->tm_mon = 0;
    }

    t->tm_mday = 1;
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("eom",  name, minimum) ||
           closeEnough ("eocm", name, minimum))
  {
    t->tm_hour = 24;
    t->tm_min = 0;
    t->tm_sec = -1;
    t->tm_mday = ISO8601d::daysInMonth (t->tm_mon + 1, t->tm_year + 1900);
    t->tm_isdst = -1;
    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("socw", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    int extra = t->tm_wday * 86400;
    t->tm_isdst = -1;
    value = Variant (mktime (t) - extra, Variant::type_date);
  }

  else if (closeEnough ("eow",  name, minimum) ||
           closeEnough ("eocw", name, minimum))
  {
    t->tm_hour = t->tm_min = 0;
    t->tm_sec = -1;
    int extra = (7 - t->tm_wday) * 86400;
    t->tm_isdst = -1;
    value = Variant (mktime (t) + extra, Variant::type_date);
  }

  else if (closeEnough ("sow", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    int extra = (7 - t->tm_wday) * 86400;
    t->tm_isdst = -1;
    value = Variant (mktime (t) + extra, Variant::type_date);
  }

  else if (closeEnough ("soww", name, minimum))
  {
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    int days = (8 - t->tm_wday) % 7;
    int extra = days * 86400;
    t->tm_isdst = -1;
    value = Variant (mktime (t) + extra, Variant::type_date);
  }

  else if (closeEnough ("eoww", name, minimum))
  {
    t->tm_hour = 24;
    t->tm_min = 0;
    t->tm_sec = -1;
    int extra = (5 - t->tm_wday) * 86400;
    if (extra < 0)
      extra += 7 * 86400;

    t->tm_isdst = -1;
    value = Variant (mktime (t) + extra, Variant::type_date);
  }

  // Support "21st" to indicate the next date that is the 21st day.
  // 1st
  // 2nd
  // 3rd
  // 4th
  else if ((
            name.length () == 3 &&
            Lexer::isDigit (name[0])   &&
            ((name[1] == 's' && name[2] == 't') ||
             (name[1] == 'n' && name[2] == 'd') ||
             (name[1] == 'r' && name[2] == 'd') ||
             (name[1] == 't' && name[2] == 'h'))
           )
           ||
           (
            name.length () == 4 &&
            Lexer::isDigit (name[0])   &&
            Lexer::isDigit (name[1])   &&
            ((name[2] == 's' && name[3] == 't') ||
             (name[2] == 'n' && name[3] == 'd') ||
             (name[2] == 'r' && name[3] == 'd') ||
             (name[2] == 't' && name[3] == 'h'))
           )
          )
  {
    int number;
    std::string ordinal;

    if (Lexer::isDigit (name[1]))
    {
      number = strtol (name.substr (0, 2).c_str (), NULL, 10);
      ordinal = Lexer::lowerCase (name.substr (2));
    }
    else
    {
      number = strtol (name.substr (0, 1).c_str (), NULL, 10);
      ordinal = Lexer::lowerCase (name.substr (1));
    }

    // Sanity check.
    if (number <= 31)
    {
      int remainder1 = number % 10;
      int remainder2 = number % 100;
      if ((remainder2 != 11 && remainder1 == 1 && ordinal == "st") ||
          (remainder2 != 12 && remainder1 == 2 && ordinal == "nd") ||
          (remainder2 != 13 && remainder1 == 3 && ordinal == "rd") ||
          ((remainder2 == 11 ||
            remainder2 == 12 ||
            remainder2 == 13 ||
            remainder1 == 0 ||
            remainder1 > 3) && ordinal == "th"))
      {
        int y = t->tm_year + 1900;
        int m = t->tm_mon + 1;
        int d = t->tm_mday;

        // If it is this month.
        if (d < number &&
            number <= ISO8601d::daysInMonth (m, y))
        {
          t->tm_hour = t->tm_min = t->tm_sec = 0;
          t->tm_mon  = m - 1;
          t->tm_mday = number;
          t->tm_year = y - 1900;
          t->tm_isdst = -1;
          value = Variant (mktime (t), Variant::type_date);
        }
        else
        {
          if (++m > 12)
          {
            m = 1;
            y++;
          }

          t->tm_hour = t->tm_min = t->tm_sec = 0;
          t->tm_mon  = m - 1;
          t->tm_mday = number;
          t->tm_year = y - 1900;
          t->tm_isdst = -1;
          value = Variant (mktime (t), Variant::type_date);
        }
      }
    }
  }

  else if (closeEnough ("easter",       name, minimum) ||
           closeEnough ("eastermonday", name, minimum) ||
           closeEnough ("ascension",    name, minimum) ||
           closeEnough ("pentecost",    name, minimum) ||
           closeEnough ("goodfriday",   name, minimum))
  {
    Variant valueNow = Variant (mktime (t), Variant::type_date);
    easter (t);
    value = Variant (mktime (t), Variant::type_date);

    // If the result is earlier this year, then recalc for next year.
    if (value < valueNow)
    {
      t = localtime (&now);
      t->tm_year++;
      easter (t);
    }

         if (closeEnough ("goodfriday",   name, minimum)) t->tm_mday -= 2;

    // DO NOT REMOVE THIS USELESS-LOOKING LINE.
    // It is here to capture an exact match for 'easter', to prevent 'easter'
    // being a partial match for 'eastermonday'.
    else if (closeEnough ("easter",       name, minimum)) ;
    else if (closeEnough ("eastermonday", name, minimum)) t->tm_mday += 1;
    else if (closeEnough ("ascension",    name, minimum)) t->tm_mday += 39;
    else if (closeEnough ("pentecost",    name, minimum)) t->tm_mday += 49;

    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("midsommar", name, minimum))
  {
    Variant valueNow = Variant (mktime (t), Variant::type_date);
    midsommar (t);
    value = Variant (mktime (t), Variant::type_date);

    // If the result is earlier this year, then recalc for next year.
    if (value < valueNow)
    {
      t = localtime (&now);
      t->tm_year++;
      midsommar (t);
    }

    value = Variant (mktime (t), Variant::type_date);
  }

  else if (closeEnough ("midsommarafton", name, minimum))
  {
    Variant valueNow = Variant (mktime (t), Variant::type_date);
    midsommarafton (t);
    value = Variant (mktime (t), Variant::type_date);

    // If the result is earlier this year, then recalc for next year.
    if (value < valueNow)
    {
      t = localtime (&now);
      t->tm_year++;
      midsommarafton (t);
    }

    value = Variant (mktime (t), Variant::type_date);
  }

  else
    return false;

  value.source (name);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

