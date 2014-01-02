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

#include <iostream>
#include <time.h>
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

         if (name == "goodfriday")   t->tm_mday -= 2;
    else if (name == "eastermonday") t->tm_mday += 1;
    else if (name == "ascension")    t->tm_mday += 39;
    else if (name == "pentecost")    t->tm_mday += 49;

    value = Variant (mktime (t), Variant::type_date);
  }

  // TODO
/*
  {s,e}o{w,m,q,ww,cw,cm}

  midsommar
  midsommarafton
  23rd
*/

  // Constants.
  else if (name == "pi")    { value = Variant (3.14159165); }
  else if (name == "true")  { value = Variant (true);       }
  else if (name == "false") { value = Variant (false);      }
  else
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

