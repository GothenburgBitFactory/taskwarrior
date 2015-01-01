////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2015, Göteborg Bit Factory.
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
  UnitTest t (450);

  Duration dur;
  std::string::size_type start = 0;
  t.notok (dur.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  const int day    = 86400;
  const int hour   =  3600;
  const int minute =    60;
  const int second =     1;

  // Parsing.

  testParse (t, "0seconds",       8,     0 * second);
  testParse (t, "2 seconds",      9,     2 * second);
  testParse (t, "10seconds",      9,    10 * second);
  testParse (t, "1.5seconds",    10,     1 * second);

  testParse (t, "0second",        7,     0 * second);
  testParse (t, "2 second",       8,     2 * second);
  testParse (t, "10second",       8,    10 * second);
  testParse (t, "1.5second",      9,     1 * second);

  testParse (t, "0s",             2,     0 * second);
  testParse (t, "2 s",            3,     2 * second);
  testParse (t, "10s",            3,    10 * second);
  testParse (t, "1.5s",           4,     1 * second);

  testParse (t, "0minutes",       8,     0 * minute);
  testParse (t, "2 minutes",      9,     2 * minute);
  testParse (t, "10minutes",      9,    10 * minute);
  testParse (t, "1.5minutes",    10,    90 * second);

  testParse (t, "0minute",        7,     0 * minute);
  testParse (t, "2 minute",       8,     2 * minute);
  testParse (t, "10minute",       8,    10 * minute);
  testParse (t, "1.5minute",      9,    90 * second);

  testParse (t, "0min",           4,     0 * minute);
  testParse (t, "2 min",          5,     2 * minute);
  testParse (t, "10min",          5,    10 * minute);
  testParse (t, "1.5min",         6,    90 * second);

  testParse (t, "0hours",         6,     0 * hour);
  testParse (t, "2 hours",        7,     2 * hour);
  testParse (t, "10hours",        7,    10 * hour);
  testParse (t, "1.5hours",       8,    90 * minute);

  testParse (t, "0hour",          5,     0 * hour);
  testParse (t, "2 hour",         6,     2 * hour);
  testParse (t, "10hour",         6,    10 * hour);
  testParse (t, "1.5hour",        7,    90 * minute);

  testParse (t, "0h",             2,     0 * hour);
  testParse (t, "2 h",            3,     2 * hour);
  testParse (t, "10h",            3,    10 * hour);
  testParse (t, "1.5h",           4,    90 * minute);

  testParse (t, "weekdays",       8,     1 * day);

  testParse (t, "daily",          5,     1 * day);

  testParse (t, "0days",          5,     0 * day);
  testParse (t, "2 days",         6,     2 * day);
  testParse (t, "10days",         6,    10 * day);
  testParse (t, "1.5days",        7,    36 * hour);

  testParse (t, "0day",           4,     0 * day);
  testParse (t, "2 day",          5,     2 * day);
  testParse (t, "10day",          5,    10 * day);
  testParse (t, "1.5day",         6,    36 * hour);

  testParse (t, "0d",             2,     0 * day);
  testParse (t, "2 d",            3,     2 * day);
  testParse (t, "10d",            3,    10 * day);
  testParse (t, "1.5d",           4,    36 * hour);

  testParse (t, "weekly",         6,     7 * day);

  testParse (t, "0weeks",         6,     0 * day);
  testParse (t, "2 weeks",        7,    14 * day);
  testParse (t, "10weeks",        7,    70 * day);
  testParse (t, "1.5weeks",       8,   252 * hour);

  testParse (t, "0week",          5,     0 * day);
  testParse (t, "2 week",         6,    14 * day);
  testParse (t, "10week",         6,    70 * day);
  testParse (t, "1.5week",        7,   252 * hour);

  testParse (t, "0w",             2,     0 * day);
  testParse (t, "2 w",            3,    14 * day);
  testParse (t, "10w",            3,    70 * day);
  testParse (t, "1.5w",           4,   252 * hour);

  testParse (t, "monthly",        7,    30 * day);

  testParse (t, "0months",        7,     0 * day);
  testParse (t, "2 months",       8,    60 * day);
  testParse (t, "10months",       8,   300 * day);
  testParse (t, "1.5months",      9,    45 * day);

  testParse (t, "0month",         6,     0 * day);
  testParse (t, "2 month",        7,    60 * day);
  testParse (t, "10month",        7,   300 * day);
  testParse (t, "1.5month",       8,    45 * day);

  testParse (t, "0mo",            3,     0 * day);
  testParse (t, "2 mo",           4,    60 * day);
  testParse (t, "10mo",           4,   300 * day);
  testParse (t, "1.5mo",          5,    45 * day);

  testParse (t, "quarterly",      9,    91 * day);

  testParse (t, "0quarters",      9,     0 * day);
  testParse (t, "2 quarters",    10,   182 * day);
  testParse (t, "10quarters",    10,   910 * day);
  testParse (t, "1.5quarters",   11,  3276 * hour);

  testParse (t, "0quarter",       8,     0 * day);
  testParse (t, "2 quarter",      9,   182 * day);
  testParse (t, "10quarter",      9,   910 * day);
  testParse (t, "1.5quarter",    10,  3276 * hour);

  testParse (t, "0q",             2,     0 * day);
  testParse (t, "2 q",            3,   182 * day);
  testParse (t, "10q",            3,   910 * day);
  testParse (t, "1.5q",           4,  3276 * hour);

  testParse (t, "yearly",         6,   365 * day);

  testParse (t, "0years",         6,     0 * day);
  testParse (t, "2 years",        7,   730 * day);
  testParse (t, "10years",        7,  3650 * day);
  testParse (t, "1.5years",       8, 13140 * hour);

  testParse (t, "0year",          5,     0 * day);
  testParse (t, "2 year",         6,   730 * day);
  testParse (t, "10year",         6,  3650 * day);
  testParse (t, "1.5year",        7, 13140 * hour);

  testParse (t, "0y",             2,     0 * day);
  testParse (t, "2 y",            3,   730 * day);
  testParse (t, "10y",            3,  3650 * day);
  testParse (t, "1.5y",           4, 13140 * hour);

  testParse (t, "annual",         6,   365 * day);

  testParse (t, "biannual",       8,   730 * day);

  testParse (t, "bimonthly",      9,    61 * day);

  testParse (t, "biweekly",       8,    14 * day);

  testParse (t, "biyearly",       8,   730 * day);

  testParse (t, "fortnight",      9,    14 * day);

  testParse (t, "semiannual",    10,   183 * day);

  testParse (t, "0sennight",      9,     0 * day);
  testParse (t, "2 sennight",    10,    28 * day);
  testParse (t, "10sennight",    10,   140 * day);
  testParse (t, "1.5sennight",   11,    21 * day);

  Duration d;

  // std::string format ();
  d = Duration (0);               t.is (d.format (), "-",          "0 -> -");
  d = Duration (1);               t.is (d.format (), "1 second",   "1 -> 1 second");
  d = Duration (2);               t.is (d.format (), "2 seconds",  "2 -> 2 seconds");
  d = Duration (59);              t.is (d.format (), "59 seconds", "59 -> 59 seconds");
  d = Duration (60);              t.is (d.format (), "1 minute",   "60 -> 1 minute");
  d = Duration (119);             t.is (d.format (), "1 minute",   "119 -> 1 minute");
  d = Duration (120);             t.is (d.format (), "2 minutes",  "120 -> 2 minutes");
  d = Duration (121);             t.is (d.format (), "2 minutes",  "121 -> 2 minutes");
  d = Duration (3599);            t.is (d.format (), "59 minutes", "3599 -> 59 minutes");
  d = Duration (3600);            t.is (d.format (), "1 hour",     "3600 -> 1 hour");
  d = Duration (3601);            t.is (d.format (), "1 hour",     "3601 -> 1 hour");
  d = Duration (86399);           t.is (d.format (), "23 hours",   "86399 -> 23 hours");
  d = Duration (86400);           t.is (d.format (), "1 day",      "86400 -> 1 day");
  d = Duration (86401);           t.is (d.format (), "1 day",      "86401 -> 1 day");
  d = Duration (14 * 86400 - 1);  t.is (d.format (), "1 week",     "14 days - 1 s -> 1 week");
  d = Duration (14 * 86400);      t.is (d.format (), "2 weeks",    "14 days -> 2 weeks");
  d = Duration (14 * 86400 + 1);  t.is (d.format (), "2 weeks",    "14 days + 1 s -> 2 weeks");
  d = Duration (85 * 86400 - 1);  t.is (d.format (), "2 months",   "85 days - 1 s -> 2 months");
  d = Duration (85 * 86400);      t.is (d.format (), "2 months",   "85 days -> 2 months");
  d = Duration (85 * 86400 + 1);  t.is (d.format (), "2 months",   "85 days + 1 s -> 2 months");
  d = Duration (365 * 86400 - 1); t.is (d.format (), "12 months",  "365 days - 1 s -> 12 months");
  d = Duration (365 * 86400);     t.is (d.format (), "1.0 year",   "365 days -> 1.0 year");
  d = Duration (365 * 86400 + 1); t.is (d.format (), "1.0 year",   "365 days + 1 s -> 1.0 year");

  // std::string formatCompact ();
  d = Duration (0);               t.is (d.formatCompact (), "",      "0 ->");
  d = Duration (1),               t.is (d.formatCompact (), "1s",    "1 -> 1s");
  d = Duration (2),               t.is (d.formatCompact (), "2s",    "2 -> 2s");
  d = Duration (59),              t.is (d.formatCompact (), "59s",   "59 -> 59s");
  d = Duration (60),              t.is (d.formatCompact (), "1min",  "60 -> 1min");
  d = Duration (119),             t.is (d.formatCompact (), "1min",  "119 -> 1min");
  d = Duration (120),             t.is (d.formatCompact (), "2min",  "120 -> 2min");
  d = Duration (121),             t.is (d.formatCompact (), "2min",  "121 -> 2min");
  d = Duration (3599),            t.is (d.formatCompact (), "59min", "3599 -> 59min");
  d = Duration (3600),            t.is (d.formatCompact (), "1h",    "3600 -> 1h");
  d = Duration (3601),            t.is (d.formatCompact (), "1h",    "3601 -> 1h");
  d = Duration (86399),           t.is (d.formatCompact (), "23h",   "86399 -> 23h");
  d = Duration (86400),           t.is (d.formatCompact (), "1d",    "86400 -> 1d");
  d = Duration (86401),           t.is (d.formatCompact (), "1d",    "86401 -> 1d");
  d = Duration (14 * 86400 - 1),  t.is (d.formatCompact (), "1w",    "14 days - 1 s -> 1w");
  d = Duration (14 * 86400),      t.is (d.formatCompact (), "2w",    "14 days -> 2w");
  d = Duration (14 * 86400 + 1),  t.is (d.formatCompact (), "2w",    "14 days + 1 s -> 2w");
  d = Duration (85 * 86400 - 1),  t.is (d.formatCompact (), "2mo",   "85 days - 1 s -> 2mo");
  d = Duration (85 * 86400),      t.is (d.formatCompact (), "2mo",   "85 days -> 2mo");
  d = Duration (85 * 86400 + 1),  t.is (d.formatCompact (), "2mo",   "85 days + 1 s -> 2mo");
  d = Duration (365 * 86400 - 1), t.is (d.formatCompact (), "12mo",  "365 days - 1 s -> 12mo");
  d = Duration (365 * 86400),     t.is (d.formatCompact (), "1.0y",  "365 days -> 1.0y");
  d = Duration (365 * 86400 + 1), t.is (d.formatCompact (), "1.0y",  "365 days + 1 s -> 1.0y");

  // std::string formatPrecise ();
  d = Duration (0);               t.is (d.formatPrecise (), "0:00:00",       "0 -> 0:00:00");
  d = Duration (1);               t.is (d.formatPrecise (), "0:00:01",       "1 -> 0:00:01");
  d = Duration (2);               t.is (d.formatPrecise (), "0:00:02",       "2 -> 0:00:02");
  d = Duration (59);              t.is (d.formatPrecise (), "0:00:59",       "59 -> 0:00:59");
  d = Duration (60);              t.is (d.formatPrecise (), "0:01:00",       "60 -> 0:01;00");
  d = Duration (119);             t.is (d.formatPrecise (), "0:01:59",       "119 -> 0:01:59");
  d = Duration (120);             t.is (d.formatPrecise (), "0:02:00",       "120 -> 0:02:00");
  d = Duration (121);             t.is (d.formatPrecise (), "0:02:01",       "121 -> 0:02:01");
  d = Duration (3599);            t.is (d.formatPrecise (), "0:59:59",       "3599 -> 0:59:59");
  d = Duration (3600);            t.is (d.formatPrecise (), "1:00:00",       "3600 -> 1:00:00");
  d = Duration (3601);            t.is (d.formatPrecise (), "1:00:01",       "3601 -> 1:00:01");
  d = Duration (86399);           t.is (d.formatPrecise (), "23:59:59",      "86399 -> 23:59:59");
  d = Duration (86400);           t.is (d.formatPrecise (), "1d 0:00:00",    "86400 -> 1d 0:00:00");
  d = Duration (86401);           t.is (d.formatPrecise (), "1d 0:00:01",    "86401 -> 1d 0:00:01");
  d = Duration (14 * 86400 - 1);  t.is (d.formatPrecise (), "13d 23:59:59",  "(14 x 86400) - 1 s -> 13d 23:59:59");
  d = Duration (14 * 86400);      t.is (d.formatPrecise (), "14d 0:00:00",   "(14 x 86400) -> 14d 0:00:00");
  d = Duration (14 * 86400 + 1);  t.is (d.formatPrecise (), "14d 0:00:01",   "(14 x 86400) + 1 -> 14d 0:00:01");
  d = Duration (365 * 86400 - 1); t.is (d.formatPrecise (), "364d 23:59:59", "365 days - 1 s -> 364d 23:59:59");
  d = Duration (365 * 86400);     t.is (d.formatPrecise (), "365d 0:00:00",  "365 days -> 365d 0:00:00");
  d = Duration (365 * 86400 + 1); t.is (d.formatPrecise (), "365d 0:00:01",  "365 days + 1 s -> 365d 0:00:01");

  // std::string formatISO ();
  d = Duration (0);               t.is (d.formatISO (), "P0S",             "0 -> P0S");
  d = Duration (1);               t.is (d.formatISO (), "PT1S",            "1 -> PT1S");
  d = Duration (2);               t.is (d.formatISO (), "PT2S",            "2 -> PT2S");
  d = Duration (59);              t.is (d.formatISO (), "PT59S",           "59 -> PT59S");
  d = Duration (60);              t.is (d.formatISO (), "PT1M",            "60 -> PT1TM");
  d = Duration (119);             t.is (d.formatISO (), "PT1M59S",         "119 -> PT1M59S");
  d = Duration (120);             t.is (d.formatISO (), "PT2M",            "120 -> PT2M");
  d = Duration (121);             t.is (d.formatISO (), "PT2M1S",          "121 -> PT2M1S");
  d = Duration (3599);            t.is (d.formatISO (), "PT59M59S",        "3599 -> PT59M59S");
  d = Duration (3600);            t.is (d.formatISO (), "PT1H",            "3600 -> PT1H");
  d = Duration (3601);            t.is (d.formatISO (), "PT1H1S",          "3601 -> PT1H1S");
  d = Duration (86399);           t.is (d.formatISO (), "PT23H59M59S",     "86399 -> PT23H59M59S");
  d = Duration (86400);           t.is (d.formatISO (), "P1D",             "86400 -> P1D");
  d = Duration (86401);           t.is (d.formatISO (), "P1DT1S",          "86401 -> P1DT1S");
  d = Duration (14 * 86400 - 1);  t.is (d.formatISO (), "P13DT23H59M59S",  "(14 x 86400) - 1 s -> P13DT23H59M59S");
  d = Duration (14 * 86400);      t.is (d.formatISO (), "P14D",            "(14 x 86400) -> P14D");
  d = Duration (14 * 86400 + 1);  t.is (d.formatISO (), "P14DT1S",         "(14 x 86400) + 1 -> P14DT1S");
  d = Duration (365 * 86400 - 1); t.is (d.formatISO (), "P1Y4DT23H59M59S", "365 days - 1 s -> P1Y4DT23H59M59S");
  d = Duration (365 * 86400);     t.is (d.formatISO (), "P1Y5D",           "365 days -> P1Y5D");
  d = Duration (365 * 86400 + 1); t.is (d.formatISO (), "P1Y5DT1S",        "365 days + 1 s -> P1Y5DT1S");

  Duration left, right;

  // operator<
  left = Duration ("1s");     right = Duration ("2s");     t.ok (left < right, "duration 1s < 2s");
  left = Duration ("-2s");    right = Duration ("-1s");    t.ok (left < right, "duration -2s < -1s");
  left = Duration ("1s");     right = Duration ("1min");   t.ok (left < right, "duration 1s < 1min");
  left = Duration ("1min");   right = Duration ("1h");     t.ok (left < right, "duration 1min < 1h");
  left = Duration ("1h");     right = Duration ("1d");     t.ok (left < right, "duration 1h < 1d");
  left = Duration ("1d");     right = Duration ("1w");     t.ok (left < right, "duration 1d < 1w");
  left = Duration ("1w");     right = Duration ("1mo");    t.ok (left < right, "duration 1w < 1mo");
  left = Duration ("1mo");    right = Duration ("1q");     t.ok (left < right, "duration 1mo < 1q");
  left = Duration ("1q");     right = Duration ("1y");     t.ok (left < right, "duration 1q < 1y");
  left = Duration ("-3s");    right = Duration ("-6s");    t.ok (right < left, "duration -6s < -3s");

  // operator>
  left = Duration ("2s");     right = Duration ("1s");     t.ok (left > right, "2s > 1s");
  left = Duration ("-1s");    right = Duration ("-2s");    t.ok (left > right, "-1s > -2s");
  left = Duration ("1min");   right = Duration ("1s");     t.ok (left > right, "1min > 1s");
  left = Duration ("1h");     right = Duration ("1min");   t.ok (left > right, "1h > 1min");
  left = Duration ("1d");     right = Duration ("1h");     t.ok (left > right, "1d > 1h");
  left = Duration ("1w");     right = Duration ("1d");     t.ok (left > right, "1w > 1d");
  left = Duration ("1mo");    right = Duration ("1w");     t.ok (left > right, "1mo > 1w");
  left = Duration ("1q");     right = Duration ("1mo");    t.ok (left > right, "1q > 1mo");
  left = Duration ("1y");     right = Duration ("1q");     t.ok (left > right, "1y > 1q");
  left = Duration ("-3s");    right = Duration ("-6s");    t.ok (left > right, "duration -3s > -6s");

  // operator<=
  left = Duration ("1s");     right = Duration ("2s");     t.ok (left <= right, "duration 1s <= 2s");
  left = Duration ("2s");     right = Duration ("2s");     t.ok (left <= right, "duration 1s <= 2s");
  left = Duration ("2s");     right = Duration ("1s");     t.notok (left <= right, "duration NOT 1s <= 2s");

  // TODO Formatting.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
