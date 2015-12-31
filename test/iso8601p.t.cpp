////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
#include <time.h>
#include <test.h>
#include <ISO8601.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input,
  int in_start,
  int in_year,
  int in_month,
  int in_day,
  int in_hours,
  int in_minutes,
  int in_seconds,
  time_t in_period,
  const std::string& output,
  const std::string& vague)
{
  std::string label = std::string ("parse (\"") + input + "\") --> ";

  ISO8601p iso;
  std::string::size_type start = 0;

  t.ok (iso.parse (input, start),                 label + "true");
  t.is ((int) start,          in_start,           label + "[]");
  t.is (iso._year,            in_year,            label + "_year");
  t.is (iso._month,           in_month,           label + "_month");
  t.is (iso._day,             in_day,             label + "_day");
  t.is (iso._hours,           in_hours,           label + "_hours");
  t.is (iso._minutes,         in_minutes,         label + "_minutes");
  t.is (iso._seconds,         in_seconds,         label + "_seconds");
  t.is ((size_t) iso._period, (size_t) in_period, label + "_period");
  t.is (iso.format (),        output,             label + " format");
  t.is (iso.formatVague (),   vague,              label + " formatVague");
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (1487);

  ISO8601p iso;
  std::string::size_type start = 0;
  t.notok (iso.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  t.notok (iso.parse ("P", start),   "P --> false");
  t.is ((int)start, 0,               "P[0]");

  t.notok (iso.parse ("PT", start),  "PT --> false");
  t.is ((int)start, 0,               "PT[0]");

  t.notok (iso.parse ("P1", start),  "P1 --> false");
  t.is ((int)start, 0,               "P1[0]");

  t.notok (iso.parse ("P1T", start), "P1T --> false");
  t.is ((int)start, 0,               "P1T[0]");

  t.notok (iso.parse ("PT1", start), "PT1 --> false");
  t.is ((int)start, 0,               "PT1[0]");

  int year  = 365 * 86400;
  int month =  30 * 86400;
  int day   =       86400;
  int h     =        3600;
  int m     =          60;

  // Designated.
  //            input              i  Year  Mo   Da  Ho  Mi         Se                           time_t           format     vague
  testParse (t, "P1Y",             3,    1,  0,   0,  0,  0,         0,                            year,          "P365D",   "1.0y");
  testParse (t, "P1M",             3,    0,  1,   0,  0,  0,         0,                           month,           "P30D",     "4w");
  testParse (t, "P1D",             3,    0,  0,   1,  0,  0,         0,                             day,            "P1D",     "1d");
  testParse (t, "P1Y1M",           5,    1,  1,   0,  0,  0,         0,                    year + month,          "P395D",   "1.1y");
  testParse (t, "P1Y1D",           5,    1,  0,   1,  0,  0,         0,                      year + day,          "P366D",   "1.0y");
  testParse (t, "P1M1D",           5,    0,  1,   1,  0,  0,         0,                     month + day,           "P31D",     "4w");
  testParse (t, "P1Y1M1D",         7,    1,  1,   1,  0,  0,         0,              year + month + day,          "P396D",   "1.1y");
  testParse (t, "PT1H",            4,    0,  0,   0,  1,  0,         0,                               h,           "PT1H",     "1h");
  testParse (t, "PT1M",            4,    0,  0,   0,  0,  1,         0,                               m,           "PT1M",   "1min");
  testParse (t, "PT1S",            4,    0,  0,   0,  0,  0,         1,                               1,           "PT1S",     "1s");
  testParse (t, "PT1H1M",          6,    0,  0,   0,  1,  1,         0,                           h + m,         "PT1H1M",     "1h");
  testParse (t, "PT1H1S",          6,    0,  0,   0,  1,  0,         1,                           h + 1,         "PT1H1S",     "1h");
  testParse (t, "PT1M1S",          6,    0,  0,   0,  0,  1,         1,                           m + 1,         "PT1M1S",   "1min");
  testParse (t, "PT1H1M1S",        8,    0,  0,   0,  1,  1,         1,                       h + m + 1,       "PT1H1M1S",     "1h");
  testParse (t, "P1Y1M1DT1H1M1S", 14,    1,  1,   1,  1,  1,         1,  year + month + day + h + m + 1,   "P396DT1H1M1S",   "1.1y");
  testParse (t, "PT24H",           5,    0,  0,   0, 24,  0,         0,                             day,            "P1D",     "1d");
  testParse (t, "PT40000000S",    11,    0,  0,   0,  0,  0,  40000000,                        40000000, "P462DT23H6M40S",   "1.3y");
  testParse (t, "PT3600S",         7,    0,  0,   0,  0,  0,      3600,                               h,           "PT1H",     "1h");
  testParse (t, "PT60M",           5,    0,  0,   0,  0, 60,         0,                               h,           "PT1H",     "1h");

  testParse (t, "0seconds",        8,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 seconds",       9,    0,  0,   0,  0,  0,         0,                               2,           "PT2S",     "2s");
  testParse (t, "10seconds",       9,    0,  0,   0,  0,  0,         0,                              10,          "PT10S",    "10s");
  testParse (t, "1.5seconds",     10,    0,  0,   0,  0,  0,         0,                               1,           "PT1S",     "1s");

  testParse (t, "0second",         7,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 second",        8,    0,  0,   0,  0,  0,         0,                               2,           "PT2S",     "2s");
  testParse (t, "10second",        8,    0,  0,   0,  0,  0,         0,                              10,          "PT10S",    "10s");
  testParse (t, "1.5second",       9,    0,  0,   0,  0,  0,         0,                               1,           "PT1S",     "1s");

  testParse (t, "0s",              2,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 s",             3,    0,  0,   0,  0,  0,         0,                               2,           "PT2S",     "2s");
  testParse (t, "10s",             3,    0,  0,   0,  0,  0,         0,                              10,          "PT10S",    "10s");
  testParse (t, "1.5s",            4,    0,  0,   0,  0,  0,         0,                               1,           "PT1S",     "1s");

  testParse (t, "0minutes",        8,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 minutes",       9,    0,  0,   0,  0,  0,         0,                           2 * m,           "PT2M",   "2min");
  testParse (t, "10minutes",       9,    0,  0,   0,  0,  0,         0,                          10 * m,          "PT10M",  "10min");
  testParse (t, "1.5minutes",     10,    0,  0,   0,  0,  0,         0,                          m + 30,        "PT1M30S",   "1min");

  testParse (t, "0minute",         7,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 minute",        8,    0,  0,   0,  0,  0,         0,                           2 * m,           "PT2M",   "2min");
  testParse (t, "10minute",        8,    0,  0,   0,  0,  0,         0,                          10 * m,          "PT10M",  "10min");
  testParse (t, "1.5minute",       9,    0,  0,   0,  0,  0,         0,                          m + 30,        "PT1M30S",   "1min");

  testParse (t, "0min",            4,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 min",           5,    0,  0,   0,  0,  0,         0,                           2 * m,           "PT2M",   "2min");
  testParse (t, "10min",           5,    0,  0,   0,  0,  0,         0,                          10 * m,          "PT10M",  "10min");
  testParse (t, "1.5min",          6,    0,  0,   0,  0,  0,         0,                          m + 30,        "PT1M30S",   "1min");

  testParse (t, "0hours",          6,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 hours",         7,    0,  0,   0,  0,  0,         0,                           2 * h,           "PT2H",     "2h");
  testParse (t, "10hours",         7,    0,  0,   0,  0,  0,         0,                          10 * h,          "PT10H",    "10h");
  testParse (t, "1.5hours",        8,    0,  0,   0,  0,  0,         0,                      h + 30 * m,        "PT1H30M",     "1h");

  testParse (t, "0hour",           5,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 hour",          6,    0,  0,   0,  0,  0,         0,                           2 * h,           "PT2H",     "2h");
  testParse (t, "10hour",          6,    0,  0,   0,  0,  0,         0,                          10 * h,          "PT10H",    "10h");
  testParse (t, "1.5hour",         7,    0,  0,   0,  0,  0,         0,                      h + 30 * m,        "PT1H30M",     "1h");

  testParse (t, "0h",              2,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 h",             3,    0,  0,   0,  0,  0,         0,                           2 * h,           "PT2H",     "2h");
  testParse (t, "10h",             3,    0,  0,   0,  0,  0,         0,                          10 * h,          "PT10H",    "10h");
  testParse (t, "1.5h",            4,    0,  0,   0,  0,  0,         0,                      h + 30 * m,        "PT1H30M",     "1h");

  testParse (t, "weekdays",        8,    0,  0,   0,  0,  0,         0,                             day,            "P1D",     "1d");

  testParse (t, "daily",           5,    0,  0,   0,  0,  0,         0,                             day,            "P1D",     "1d");

  testParse (t, "0days",           5,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 days",          6,    0,  0,   0,  0,  0,         0,                         2 * day,            "P2D",     "2d");
  testParse (t, "10days",          6,    0,  0,   0,  0,  0,         0,                        10 * day,           "P10D",    "10d");
  testParse (t, "1.5days",         7,    0,  0,   0,  0,  0,         0,                    day + 12 * h,        "P1DT12H",     "1d");

  testParse (t, "0day",            4,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 day",           5,    0,  0,   0,  0,  0,         0,                         2 * day,            "P2D",     "2d");
  testParse (t, "10day",           5,    0,  0,   0,  0,  0,         0,                        10 * day,           "P10D",    "10d");
  testParse (t, "1.5day",          6,    0,  0,   0,  0,  0,         0,                    day + 12 * h,        "P1DT12H",     "1d");

  testParse (t, "0d",              2,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 d",             3,    0,  0,   0,  0,  0,         0,                         2 * day,            "P2D",     "2d");
  testParse (t, "10d",             3,    0,  0,   0,  0,  0,         0,                        10 * day,           "P10D",    "10d");
  testParse (t, "1.5d",            4,    0,  0,   0,  0,  0,         0,                    day + 12 * h,        "P1DT12H",     "1d");

  testParse (t, "weekly",          6,    0,  0,   0,  0,  0,         0,                         7 * day,            "P7D",     "7d");

  testParse (t, "0weeks",          6,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 weeks",         7,    0,  0,   0,  0,  0,         0,                        14 * day,           "P14D",     "2w");
  testParse (t, "10weeks",         7,    0,  0,   0,  0,  0,         0,                        70 * day,           "P70D",    "10w");
  testParse (t, "1.5weeks",        8,    0,  0,   0,  0,  0,         0,               10 * day + 12 * h,       "P10DT12H",    "10d");

  testParse (t, "0week",           5,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 week",          6,    0,  0,   0,  0,  0,         0,                        14 * day,           "P14D",     "2w");
  testParse (t, "10week",          6,    0,  0,   0,  0,  0,         0,                        70 * day,           "P70D",    "10w");
  testParse (t, "1.5week",         7,    0,  0,   0,  0,  0,         0,               10 * day + 12 * h,       "P10DT12H",    "10d");

  testParse (t, "0w",              2,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 w",             3,    0,  0,   0,  0,  0,         0,                        14 * day,           "P14D",     "2w");
  testParse (t, "10w",             3,    0,  0,   0,  0,  0,         0,                        70 * day,           "P70D",    "10w");
  testParse (t, "1.5w",            4,    0,  0,   0,  0,  0,         0,               10 * day + 12 * h,       "P10DT12H",    "10d");

  testParse (t, "monthly",         7,    0,  0,   0,  0,  0,         0,                        30 * day,           "P30D",     "4w");

  testParse (t, "0months",         7,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 months",        8,    0,  0,   0,  0,  0,         0,                        60 * day,           "P60D",     "8w");
  testParse (t, "10months",        8,    0,  0,   0,  0,  0,         0,                       300 * day,          "P300D",   "10mo");
  testParse (t, "1.5months",       9,    0,  0,   0,  0,  0,         0,                        45 * day,           "P45D",     "6w");

  testParse (t, "0month",          6,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 month",         7,    0,  0,   0,  0,  0,         0,                        60 * day,           "P60D",     "8w");
  testParse (t, "10month",         7,    0,  0,   0,  0,  0,         0,                       300 * day,          "P300D",   "10mo");
  testParse (t, "1.5month",        8,    0,  0,   0,  0,  0,         0,                        45 * day,           "P45D",     "6w");

  testParse (t, "0mo",             3,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 mo",            4,    0,  0,   0,  0,  0,         0,                        60 * day,           "P60D",     "8w");
  testParse (t, "10mo",            4,    0,  0,   0,  0,  0,         0,                       300 * day,          "P300D",   "10mo");
  testParse (t, "1.5mo",           5,    0,  0,   0,  0,  0,         0,                        45 * day,           "P45D",     "6w");

  testParse (t, "quarterly",       9,    0,  0,   0,  0,  0,         0,                        91 * day,           "P91D",    "3mo");

  testParse (t, "0quarters",       9,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 quarters",     10,    0,  0,   0,  0,  0,         0,                       182 * day,          "P182D",    "6mo");
  testParse (t, "10quarters",     10,    0,  0,   0,  0,  0,         0,                       910 * day,          "P910D",   "2.5y");
  testParse (t, "1.5quarters",    11,    0,  0,   0,  0,  0,         0,              136 * day + 12 * h,      "P136DT12H",    "4mo");

  testParse (t, "0quarter",        8,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 quarter",       9,    0,  0,   0,  0,  0,         0,                       182 * day,          "P182D",    "6mo");
  testParse (t, "10quarter",       9,    0,  0,   0,  0,  0,         0,                       910 * day,          "P910D",   "2.5y");
  testParse (t, "1.5quarter",     10,    0,  0,   0,  0,  0,         0,              136 * day + 12 * h,      "P136DT12H",    "4mo");

  testParse (t, "0q",              2,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 q",             3,    0,  0,   0,  0,  0,         0,                       182 * day,          "P182D",    "6mo");
  testParse (t, "10q",             3,    0,  0,   0,  0,  0,         0,                       910 * day,          "P910D",   "2.5y");
  testParse (t, "1.5q",            4,    0,  0,   0,  0,  0,         0,              136 * day + 12 * h,      "P136DT12H",    "4mo");

  testParse (t, "yearly",          6,    0,  0,   0,  0,  0,         0,                            year,          "P365D",   "1.0y");

  testParse (t, "0years",          6,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 years",         7,    0,  0,   0,  0,  0,         0,                        2 * year,          "P730D",   "2.0y");
  testParse (t, "10years",         7,    0,  0,   0,  0,  0,         0,                       10 * year,         "P3650D",  "10.0y");
  testParse (t, "1.5years",        8,    0,  0,   0,  0,  0,         0,              547 * day + 12 * h,      "P547DT12H",   "1.5y");

  testParse (t, "0year",           5,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 year",          6,    0,  0,   0,  0,  0,         0,                        2 * year,          "P730D",   "2.0y");
  testParse (t, "10year",          6,    0,  0,   0,  0,  0,         0,                       10 * year,         "P3650D",  "10.0y");
  testParse (t, "1.5year",         7,    0,  0,   0,  0,  0,         0,              547 * day + 12 * h,      "P547DT12H",   "1.5y");

  testParse (t, "0y",              2,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 y",             3,    0,  0,   0,  0,  0,         0,                        2 * year,          "P730D",   "2.0y");
  testParse (t, "10y",             3,    0,  0,   0,  0,  0,         0,                       10 * year,         "P3650D",  "10.0y");
  testParse (t, "1.5y",            4,    0,  0,   0,  0,  0,         0,              547 * day + 12 * h,      "P547DT12H",   "1.5y");

  testParse (t, "annual",          6,    0,  0,   0,  0,  0,         0,                            year,          "P365D",   "1.0y");
  testParse (t, "biannual",        8,    0,  0,   0,  0,  0,         0,                        2 * year,          "P730D",   "2.0y");
  testParse (t, "bimonthly",       9,    0,  0,   0,  0,  0,         0,                        61 * day,           "P61D",     "8w");
  testParse (t, "biweekly",        8,    0,  0,   0,  0,  0,         0,                        14 * day,           "P14D",     "2w");
  testParse (t, "biyearly",        8,    0,  0,   0,  0,  0,         0,                        2 * year,          "P730D",   "2.0y");
  testParse (t, "fortnight",       9,    0,  0,   0,  0,  0,         0,                        14 * day,           "P14D",     "2w");
  testParse (t, "semiannual",     10,    0,  0,   0,  0,  0,         0,                       183 * day,          "P183D",    "6mo");

  testParse (t, "0sennight",       9,    0,  0,   0,  0,  0,         0,                               0,           "PT0S",       "");
  testParse (t, "2 sennight",     10,    0,  0,   0,  0,  0,         0,                        28 * day,           "P28D",     "4w");
  testParse (t, "10sennight",     10,    0,  0,   0,  0,  0,         0,                       140 * day,          "P140D",    "4mo");
  testParse (t, "1.5sennight",    11,    0,  0,   0,  0,  0,         0,                        21 * day,           "P21D",     "3w");

  ISO8601p left, right;

  // operator<
  left = ISO8601p ("1s");     right = ISO8601p ("2s");     t.ok (left < right, "iso8601p 1s < 2s");
  left = ISO8601p ("-2s");    right = ISO8601p ("-1s");    t.ok (left < right, "iso8601p -2s < -1s");
  left = ISO8601p ("1s");     right = ISO8601p ("1min");   t.ok (left < right, "iso8601p 1s < 1min");
  left = ISO8601p ("1min");   right = ISO8601p ("1h");     t.ok (left < right, "iso8601p 1min < 1h");
  left = ISO8601p ("1h");     right = ISO8601p ("1d");     t.ok (left < right, "iso8601p 1h < 1d");
  left = ISO8601p ("1d");     right = ISO8601p ("1w");     t.ok (left < right, "iso8601p 1d < 1w");
  left = ISO8601p ("1w");     right = ISO8601p ("1mo");    t.ok (left < right, "iso8601p 1w < 1mo");
  left = ISO8601p ("1mo");    right = ISO8601p ("1q");     t.ok (left < right, "iso8601p 1mo < 1q");
  left = ISO8601p ("1q");     right = ISO8601p ("1y");     t.ok (left < right, "iso8601p 1q < 1y");
  left = ISO8601p ("-3s");    right = ISO8601p ("-6s");    t.ok (right < left, "iso8601p -6s < -3s");

  // operator>
  left = ISO8601p ("2s");     right = ISO8601p ("1s");     t.ok (left > right, "iso8601p 2s > 1s");
  left = ISO8601p ("-1s");    right = ISO8601p ("-2s");    t.ok (left > right, "iso8601p -1s > -2s");
  left = ISO8601p ("1min");   right = ISO8601p ("1s");     t.ok (left > right, "iso8601p 1min > 1s");
  left = ISO8601p ("1h");     right = ISO8601p ("1min");   t.ok (left > right, "iso8601p 1h > 1min");
  left = ISO8601p ("1d");     right = ISO8601p ("1h");     t.ok (left > right, "iso8601p 1d > 1h");
  left = ISO8601p ("1w");     right = ISO8601p ("1d");     t.ok (left > right, "iso8601p 1w > 1d");
  left = ISO8601p ("1mo");    right = ISO8601p ("1w");     t.ok (left > right, "iso8601p 1mo > 1w");
  left = ISO8601p ("1q");     right = ISO8601p ("1mo");    t.ok (left > right, "iso8601p 1q > 1mo");
  left = ISO8601p ("1y");     right = ISO8601p ("1q");     t.ok (left > right, "iso8601p 1y > 1q");
  left = ISO8601p ("-3s");    right = ISO8601p ("-6s");    t.ok (left > right, "iso8601p -3s > -6s");

  // operator<=
  left = ISO8601p ("1s");     right = ISO8601p ("2s");     t.ok (left <= right, "iso8601p 1s <= 2s");
  left = ISO8601p ("2s");     right = ISO8601p ("2s");     t.ok (left <= right, "iso8601p 1s <= 2s");
  left = ISO8601p ("2s");     right = ISO8601p ("1s");     t.notok (left <= right, "iso8601p NOT 1s <= 2s");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
