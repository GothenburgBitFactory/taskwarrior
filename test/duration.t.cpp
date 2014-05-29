////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2014, Göteborg Bit Factory.
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
#include <test.h>
#include <Duration.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input,
  int in_start,
  time_t in_value)
{
  std::string label = std::string ("parse (\"") + input + "\") --> ";

  Duration dur;
  std::string::size_type start = 0;

  t.ok (dur.parse (input, start),                 label + "true");
  t.is ((int) start,                    in_start, label + "[]");
  t.is ((size_t) (time_t) dur, (size_t) in_value, label + "_secs");
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (812);

  Duration dur;
  std::string::size_type start = 0;
  t.notok (dur.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  const int day    = 86400;
  const int hour   =  3600;
  const int minute =    60;
  const int second =     1;

  // Parsing.

  testParse (t, "seconds",        7,     1 * second);
  testParse (t, "0seconds",       8,     0 * second);
  testParse (t, "2 seconds",      9,     2 * second);
  testParse (t, "10seconds",      9,    10 * second);
  testParse (t, "1.5seconds",    10,     1 * second);

  testParse (t, "second",         6,     1 * second);
  testParse (t, "0second",        7,     0 * second);
  testParse (t, "2 second",       8,     2 * second);
  testParse (t, "10second",       8,    10 * second);
  testParse (t, "1.5second",      9,     1 * second);

  testParse (t, "secs",           4,     1 * second);
  testParse (t, "0secs",          5,     0 * second);
  testParse (t, "2 secs",         6,     2 * second);
  testParse (t, "10secs",         6,    10 * second);
  testParse (t, "1.5secs",        7,     1 * second);

  testParse (t, "sec",            3,     1 * second);
  testParse (t, "0sec",           4,     0 * second);
  testParse (t, "2 sec",          5,     2 * second);
  testParse (t, "10sec",          5,    10 * second);
  testParse (t, "1.5sec",         6,     1 * second);

  testParse (t, "s",              1,     1 * second);
  testParse (t, "0s",             2,     0 * second);
  testParse (t, "2 s",            3,     2 * second);
  testParse (t, "10s",            3,    10 * second);
  testParse (t, "1.5s",           4,     1 * second);

  testParse (t, "minutes",        7,     1 * minute);
  testParse (t, "0minutes",       8,     0 * minute);
  testParse (t, "2 minutes",      9,     2 * minute);
  testParse (t, "10minutes",      9,    10 * minute);
  testParse (t, "1.5minutes",    10,    90 * second);

  testParse (t, "minute",         6,     1 * minute);
  testParse (t, "0minute",        7,     0 * minute);
  testParse (t, "2 minute",       8,     2 * minute);
  testParse (t, "10minute",       8,    10 * minute);
  testParse (t, "1.5minute",      9,    90 * second);

  testParse (t, "mins",           4,     1 * minute);
  testParse (t, "0mins",          5,     0 * minute);
  testParse (t, "2 mins",         6,     2 * minute);
  testParse (t, "10mins",         6,    10 * minute);
  testParse (t, "1.5mins",        7,    90 * second);

  testParse (t, "min",            3,     1 * minute);
  testParse (t, "0min",           4,     0 * minute);
  testParse (t, "2 min",          5,     2 * minute);
  testParse (t, "10min",          5,    10 * minute);
  testParse (t, "1.5min",         6,    90 * second);

  testParse (t, "hours",          5,     1 * hour);
  testParse (t, "0hours",         6,     0 * hour);
  testParse (t, "2 hours",        7,     2 * hour);
  testParse (t, "10hours",        7,    10 * hour);
  testParse (t, "1.5hours",       8,    90 * minute);

  testParse (t, "hour",           4,     1 * hour);
  testParse (t, "0hour",          5,     0 * hour);
  testParse (t, "2 hour",         6,     2 * hour);
  testParse (t, "10hour",         6,    10 * hour);
  testParse (t, "1.5hour",        7,    90 * minute);

  testParse (t, "hrs",            3,     1 * hour);
  testParse (t, "0hrs",           4,     0 * hour);
  testParse (t, "2 hrs",          5,     2 * hour);
  testParse (t, "10hrs",          5,    10 * hour);
  testParse (t, "1.5hrs",         6,    90 * minute);

  testParse (t, "hr",             2,     1 * hour);
  testParse (t, "0hr",            3,     0 * hour);
  testParse (t, "2 hr",           4,     2 * hour);
  testParse (t, "10hr",           4,    10 * hour);
  testParse (t, "1.5hr",          5,    90 * minute);

  testParse (t, "h",              1,     1 * hour);
  testParse (t, "0h",             2,     0 * hour);
  testParse (t, "2 h",            3,     2 * hour);
  testParse (t, "10h",            3,    10 * hour);
  testParse (t, "1.5h",           4,    90 * minute);

  testParse (t, "weekdays",       8,     1 * day);
  testParse (t, "0weekdays",      9,     0 * day);
  testParse (t, "2 weekdays",    10,     2 * day);
  testParse (t, "10weekdays",    10,    10 * day);
  testParse (t, "1.5weekdays",   11,    36 * hour);

  testParse (t, "daily",          5,     1 * day);
  testParse (t, "0daily",         6,     0 * day);
  testParse (t, "2 daily",        7,     2 * day);
  testParse (t, "10daily",        7,    10 * day);
  testParse (t, "1.5daily",       8,    36 * hour);

  testParse (t, "days",           4,     1 * day);
  testParse (t, "0days",          5,     0 * day);
  testParse (t, "2 days",         6,     2 * day);
  testParse (t, "10days",         6,    10 * day);
  testParse (t, "1.5days",        7,    36 * hour);

  testParse (t, "day",            3,     1 * day);
  testParse (t, "0day",           4,     0 * day);
  testParse (t, "2 day",          5,     2 * day);
  testParse (t, "10day",          5,    10 * day);
  testParse (t, "1.5day",         6,    36 * hour);

  testParse (t, "d",              1,     1 * day);
  testParse (t, "0d",             2,     0 * day);
  testParse (t, "2 d",            3,     2 * day);
  testParse (t, "10d",            3,    10 * day);
  testParse (t, "1.5d",           4,    36 * hour);

  testParse (t, "weekly",         6,     7 * day);
  testParse (t, "0weekly",        7,     0 * day);
  testParse (t, "2 weekly",       8,    14 * day);
  testParse (t, "10weekly",       8,    70 * day);
  testParse (t, "1.5weekly",      9,   252 * hour);

  testParse (t, "weeks",          5,     7 * day);
  testParse (t, "0weeks",         6,     0 * day);
  testParse (t, "2 weeks",        7,    14 * day);
  testParse (t, "10weeks",        7,    70 * day);
  testParse (t, "1.5weeks",       8,   252 * hour);

  testParse (t, "week",           4,     7 * day);
  testParse (t, "0week",          5,     0 * day);
  testParse (t, "2 week",         6,    14 * day);
  testParse (t, "10week",         6,    70 * day);
  testParse (t, "1.5week",        7,   252 * hour);

  testParse (t, "wks",            3,     7 * day);
  testParse (t, "0wks",           4,     0 * day);
  testParse (t, "2 wks",          5,    14 * day);
  testParse (t, "10wks",          5,    70 * day);
  testParse (t, "1.5wks",         6,   252 * hour);

  testParse (t, "wk",             2,     7 * day);
  testParse (t, "0wk",            3,     0 * day);
  testParse (t, "2 wk",           4,    14 * day);
  testParse (t, "10wk",           4,    70 * day);
  testParse (t, "1.5wk",          5,   252 * hour);

  testParse (t, "w",              1,     7 * day);
  testParse (t, "0w",             2,     0 * day);
  testParse (t, "2 w",            3,    14 * day);
  testParse (t, "10w",            3,    70 * day);
  testParse (t, "1.5w",           4,   252 * hour);

  testParse (t, "monthly",        7,    30 * day);
  testParse (t, "0monthly",       8,     0 * day);
  testParse (t, "2 monthly",      9,    60 * day);
  testParse (t, "10monthly",      9,   300 * day);
  testParse (t, "1.5monthly",    10,    45 * day);

  testParse (t, "months",         6,    30 * day);
  testParse (t, "0months",        7,     0 * day);
  testParse (t, "2 months",       8,    60 * day);
  testParse (t, "10months",       8,   300 * day);
  testParse (t, "1.5months",      9,    45 * day);

  testParse (t, "month",          5,    30 * day);
  testParse (t, "0month",         6,     0 * day);
  testParse (t, "2 month",        7,    60 * day);
  testParse (t, "10month",        7,   300 * day);
  testParse (t, "1.5month",       8,    45 * day);

  testParse (t, "mnths",          5,    30 * day);
  testParse (t, "0mnths",         6,     0 * day);
  testParse (t, "2 mnths",        7,    60 * day);
  testParse (t, "10mnths",        7,   300 * day);
  testParse (t, "1.5mnths",       8,    45 * day);

  testParse (t, "mths",           4,    30 * day);
  testParse (t, "0mths",          5,     0 * day);
  testParse (t, "2 mths",         6,    60 * day);
  testParse (t, "10mths",         6,   300 * day);
  testParse (t, "1.5mths",        7,    45 * day);

  testParse (t, "mth",            3,    30 * day);
  testParse (t, "0mth",           4,     0 * day);
  testParse (t, "2 mth",          5,    60 * day);
  testParse (t, "10mth",          5,   300 * day);
  testParse (t, "1.5mth",         6,    45 * day);

  testParse (t, "mos",            3,    30 * day);
  testParse (t, "0mos",           4,     0 * day);
  testParse (t, "2 mos",          5,    60 * day);
  testParse (t, "10mos",          5,   300 * day);
  testParse (t, "1.5mos",         6,    45 * day);

  testParse (t, "mo",             2,    30 * day);
  testParse (t, "0mo",            3,     0 * day);
  testParse (t, "2 mo",           4,    60 * day);
  testParse (t, "10mo",           4,   300 * day);
  testParse (t, "1.5mo",          5,    45 * day);

  testParse (t, "quarterly",      9,    91 * day);
  testParse (t, "0quarterly",    10,     0 * day);
  testParse (t, "2 quarterly",   11,   182 * day);
  testParse (t, "10quarterly",   11,   910 * day);
  testParse (t, "1.5quarterly",  12,  3276 * hour);

  testParse (t, "quarters",       8,    91 * day);
  testParse (t, "0quarters",      9,     0 * day);
  testParse (t, "2 quarters",    10,   182 * day);
  testParse (t, "10quarters",    10,   910 * day);
  testParse (t, "1.5quarters",   11,  3276 * hour);

  testParse (t, "quarter",        7,    91 * day);
  testParse (t, "0quarter",       8,     0 * day);
  testParse (t, "2 quarter",      9,   182 * day);
  testParse (t, "10quarter",      9,   910 * day);
  testParse (t, "1.5quarter",    10,  3276 * hour);

  testParse (t, "qrtrs",          5,    91 * day);
  testParse (t, "0qrtrs",         6,     0 * day);
  testParse (t, "2 qrtrs",        7,   182 * day);
  testParse (t, "10qrtrs",        7,   910 * day);
  testParse (t, "1.5qrtrs",       8,  3276 * hour);

  testParse (t, "qtrs",           4,    91 * day);
  testParse (t, "0qtrs",          5,     0 * day);
  testParse (t, "2 qtrs",         6,   182 * day);
  testParse (t, "10qtrs",         6,   910 * day);
  testParse (t, "1.5qtrs",        7,  3276 * hour);

  testParse (t, "qtr",            3,    91 * day);
  testParse (t, "0qtr",           4,     0 * day);
  testParse (t, "2 qtr",          5,   182 * day);
  testParse (t, "10qtr",          5,   910 * day);
  testParse (t, "1.5qtr",         6,  3276 * hour);

  testParse (t, "q",              1,    91 * day);
  testParse (t, "0q",             2,     0 * day);
  testParse (t, "2 q",            3,   182 * day);
  testParse (t, "10q",            3,   910 * day);
  testParse (t, "1.5q",           4,  3276 * hour);

  testParse (t, "yearly",         6,   365 * day);
  testParse (t, "0yearly",        7,     0 * day);
  testParse (t, "2 yearly",       8,   730 * day);
  testParse (t, "10yearly",       8,  3650 * day);
  testParse (t, "1.5yearly",      9, 13140 * hour);

  testParse (t, "years",          5,   365 * day);
  testParse (t, "0years",         6,     0 * day);
  testParse (t, "2 years",        7,   730 * day);
  testParse (t, "10years",        7,  3650 * day);
  testParse (t, "1.5years",       8, 13140 * hour);

  testParse (t, "year",           4,   365 * day);
  testParse (t, "0year",          5,     0 * day);
  testParse (t, "2 year",         6,   730 * day);
  testParse (t, "10year",         6,  3650 * day);
  testParse (t, "1.5year",        7, 13140 * hour);

  testParse (t, "yrs",            3,   365 * day);
  testParse (t, "0yrs",           4,     0 * day);
  testParse (t, "2 yrs",          5,   730 * day);
  testParse (t, "10yrs",          5,  3650 * day);
  testParse (t, "1.5yrs",         6, 13140 * hour);

  testParse (t, "yr",             2,   365 * day);
  testParse (t, "0yr",            3,     0 * day);
  testParse (t, "2 yr",           4,   730 * day);
  testParse (t, "10yr",           4,  3650 * day);
  testParse (t, "1.5yr",          5, 13140 * hour);

  testParse (t, "y",              1,   365 * day);
  testParse (t, "0y",             2,     0 * day);
  testParse (t, "2 y",            3,   730 * day);
  testParse (t, "10y",            3,  3650 * day);
  testParse (t, "1.5y",           4, 13140 * hour);

  testParse (t, "annual",         6,   365 * day);
  testParse (t, "0annual",        7,     0 * day);
  testParse (t, "2 annual",       8,   730 * day);
  testParse (t, "10annual",       8,  3650 * day);
  testParse (t, "1.5annual",      9, 13140 * hour);

  testParse (t, "biannual",       8,   730 * day);
  testParse (t, "0biannual",      9,     0 * day);
  testParse (t, "2 biannual",    10,  1460 * day);
  testParse (t, "10biannual",    10,  7300 * day);
  testParse (t, "1.5biannual",   11,  1095 * day);

  testParse (t, "bimonthly",      9,    61 * day);
  testParse (t, "0bimonthly",    10,     0 * day);
  testParse (t, "2 bimonthly",   11,   122 * day);
  testParse (t, "10bimonthly",   11,   610 * day);
  testParse (t, "1.5bimonthly",  12,  2196 * hour);

  testParse (t, "biweekly",       8,    14 * day);
  testParse (t, "0biweekly",      9,     0 * day);
  testParse (t, "2 biweekly",    10,    28 * day);
  testParse (t, "10biweekly",    10,   140 * day);
  testParse (t, "1.5biweekly",   11,    21 * day);

  testParse (t, "biyearly",       8,   730 * day);
  testParse (t, "0biyearly",      9,     0 * day);
  testParse (t, "2 biyearly",    10,  1460 * day);
  testParse (t, "10biyearly",    10,  7300 * day);
  testParse (t, "1.5biyearly",   11,  1095 * day);

  testParse (t, "fortnight",      9,    14 * day);
  testParse (t, "0fortnight",    10,     0 * day);
  testParse (t, "2 fortnight",   11,    28 * day);
  testParse (t, "10fortnight",   11,   140 * day);
  testParse (t, "1.5fortnight",  12,    21 * day);

  testParse (t, "semiannual",    10,   183 * day);
  testParse (t, "0semiannual",   11,     0 * day);
  testParse (t, "2 semiannual",  12,   366 * day);
  testParse (t, "10semiannual",  12,  1830 * day);
  testParse (t, "1.5semiannual", 13,  6588 * hour);

  testParse (t, "sennight",       8,    14 * day);
  testParse (t, "0sennight",      9,     0 * day);
  testParse (t, "2 sennight",    10,    28 * day);
  testParse (t, "10sennight",    10,   140 * day);
  testParse (t, "1.5sennight",   11,    21 * day);

  // TODO Formatting.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
