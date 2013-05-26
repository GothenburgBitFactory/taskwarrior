////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <Context.h>
#include <Duration.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
//   daily, day, Nd
//   weekly, 1w, sennight, biweekly, fortnight
//   monthly, bimonthly, Nm, semimonthly
//   1st 2nd 3rd 4th .. 31st
//   quarterly, 1q
//   biannual, biyearly, annual, semiannual, yearly, 1y

int convertDuration (const std::string& input)
{
  try { Duration d (input); return ((int) d) / 86400; }
  catch (...) {}
  return 0;
}

int main (int argc, char** argv)
{
  UnitTest t (644);

  Duration d;

  // std::string format ();
  d = Duration (0);               t.is (d.format (), "-",       "0 -> -");                        // 1
  d = Duration (1);               t.is (d.format (), "1 sec",   "1 -> 1 sec");
  d = Duration (2);               t.is (d.format (), "2 secs",  "2 -> 2 secs");
  d = Duration (59);              t.is (d.format (), "59 secs", "59 -> 59 secs");
  d = Duration (60);              t.is (d.format (), "1 min",   "60 -> 1 min");
  d = Duration (119);             t.is (d.format (), "1 min",   "119 -> 1 min");
  d = Duration (120);             t.is (d.format (), "2 mins",  "120 -> 2 mins");
  d = Duration (121);             t.is (d.format (), "2 mins",  "121 -> 2 mins");
  d = Duration (3599);            t.is (d.format (), "59 mins", "3599 -> 59 mins");
  d = Duration (3600);            t.is (d.format (), "1 hr",    "3600 -> 1 hr");
  d = Duration (3601);            t.is (d.format (), "1 hr",    "3601 -> 1 hr");
  d = Duration (86399);           t.is (d.format (), "23 hrs",  "86399 -> 23 hrs");
  d = Duration (86400);           t.is (d.format (), "1 day",   "86400 -> 1 day");
  d = Duration (86401);           t.is (d.format (), "1 day",   "86401 -> 1 day");
  d = Duration (14 * 86400 - 1);  t.is (d.format (), "1 wk",    "14 days - 1 sec -> 1 wk");
  d = Duration (14 * 86400);      t.is (d.format (), "2 wks",   "14 days -> 2 wks");
  d = Duration (14 * 86400 + 1);  t.is (d.format (), "2 wks",   "14 days + 1 sec -> 2 wks");
  d = Duration (85 * 86400 - 1);  t.is (d.format (), "2 mths",  "85 days - 1 sec -> 2 mths");
  d = Duration (85 * 86400);      t.is (d.format (), "2 mths",  "85 days -> 2 mths");
  d = Duration (85 * 86400 + 1);  t.is (d.format (), "2 mths",  "85 days + 1 sec -> 2 mths");
  d = Duration (365 * 86400 - 1); t.is (d.format (), "12 mths", "365 days - 1 sec -> 12 mths");
  d = Duration (365 * 86400);     t.is (d.format (), "1.0 yrs", "365 days -> 1.0 yrs");
  d = Duration (365 * 86400 + 1); t.is (d.format (), "1.0 yrs", "365 days + 1 sec -> 1.0 yrs");

  // std::string formatCompact ();
  d = Duration (0);               t.is (d.formatCompact (), "",     "0 ->");                    // 24
  d = Duration (1),               t.is (d.formatCompact (), "1s",   "1 -> 1s");
  d = Duration (2),               t.is (d.formatCompact (), "2s",   "2 -> 2s");
  d = Duration (59),              t.is (d.formatCompact (), "59s",  "59 -> 59s");
  d = Duration (60),              t.is (d.formatCompact (), "1m",   "60 -> 1m");
  d = Duration (119),             t.is (d.formatCompact (), "1m",   "119 -> 1m");
  d = Duration (120),             t.is (d.formatCompact (), "2m",   "120 -> 2m");
  d = Duration (121),             t.is (d.formatCompact (), "2m",   "121 -> 2m");
  d = Duration (3599),            t.is (d.formatCompact (), "59m",  "3599 -> 59m");
  d = Duration (3600),            t.is (d.formatCompact (), "1h",   "3600 -> 1h");
  d = Duration (3601),            t.is (d.formatCompact (), "1h",   "3601 -> 1h");
  d = Duration (86399),           t.is (d.formatCompact (), "23h",  "86399 -> 23h");
  d = Duration (86400),           t.is (d.formatCompact (), "1d",   "86400 -> 1d");
  d = Duration (86401),           t.is (d.formatCompact (), "1d",   "86401 -> 1d");
  d = Duration (14 * 86400 - 1),  t.is (d.formatCompact (), "1wk",  "14 days - 1 sec -> 1wk");
  d = Duration (14 * 86400),      t.is (d.formatCompact (), "2wk",  "14 days -> 2wk");
  d = Duration (14 * 86400 + 1),  t.is (d.formatCompact (), "2wk",  "14 days + 1 sec -> 2wk");
  d = Duration (85 * 86400 - 1),  t.is (d.formatCompact (), "2mo",  "85 days - 1 sec -> 2mo");
  d = Duration (85 * 86400),      t.is (d.formatCompact (), "2mo",  "85 days -> 2mo");
  d = Duration (85 * 86400 + 1),  t.is (d.formatCompact (), "2mo",  "85 days + 1 sec -> 2mo");
  d = Duration (365 * 86400 - 1), t.is (d.formatCompact (), "12mo", "365 days - 1 sec -> 12mo");
  d = Duration (365 * 86400),     t.is (d.formatCompact (), "1.0y", "365 days -> 1.0y");
  d = Duration (365 * 86400 + 1), t.is (d.formatCompact (), "1.0y", "365 days + 1 sec -> 1.0y");

  // std::string formatPrecise ();
  d = Duration (0);               t.is (d.formatPrecise (), "0:00:00",       "0 -> 0:00:00");      // 47
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
  d = Duration (14 * 86400 - 1);  t.is (d.formatPrecise (), "13d 23:59:59",  "(14 x 86400) - 1 sec -> 13d 23:59:59");
  d = Duration (14 * 86400);      t.is (d.formatPrecise (), "14d 0:00:00",   "(14 x 86400) -> 14d 0:00:00");
  d = Duration (14 * 86400 + 1);  t.is (d.formatPrecise (), "14d 0:00:01",   "(14 x 86400) + 1 -> 14d 0:00:01");
  d = Duration (365 * 86400 - 1); t.is (d.formatPrecise (), "364d 23:59:59", "365 days - 1 sec -> 364d 23:59:59");
  d = Duration (365 * 86400);     t.is (d.formatPrecise (), "365d 0:00:00",  "365 days -> 365d 0:00:00");
  d = Duration (365 * 86400 + 1); t.is (d.formatPrecise (), "365d 0:00:01",  "365 days + 1 sec -> 365d 0:00:01");

  // Iterate for a whole year.  Why?  Just to see where the boundaries are,
  // so that changes can be made with some reference point.
  d = Duration (  1*86400); t.is (d.formatCompact (), "1d", "1*86400 -> 1d");                     // 67
  d = Duration (  2*86400); t.is (d.formatCompact (), "2d", "2*86400 -> 2d");
  d = Duration (  3*86400); t.is (d.formatCompact (), "3d", "3*86400 -> 3d");
  d = Duration (  4*86400); t.is (d.formatCompact (), "4d", "4*86400 -> 4d");
  d = Duration (  5*86400); t.is (d.formatCompact (), "5d", "5*86400 -> 5d");
  d = Duration (  6*86400); t.is (d.formatCompact (), "6d", "6*86400 -> 6d");
  d = Duration (  7*86400); t.is (d.formatCompact (), "7d", "7*86400 -> 7d");
  d = Duration (  8*86400); t.is (d.formatCompact (), "8d", "8*86400 -> 8d");
  d = Duration (  9*86400); t.is (d.formatCompact (), "9d", "9*86400 -> 9d");
  d = Duration ( 10*86400); t.is (d.formatCompact (), "10d", "10*86400 -> 10d");

  d = Duration ( 11*86400); t.is (d.formatCompact (), "11d", "11*86400 -> 11d");
  d = Duration ( 12*86400); t.is (d.formatCompact (), "12d", "12*86400 -> 12d");
  d = Duration ( 13*86400); t.is (d.formatCompact (), "1wk", "13*86400 -> 1wk");
  d = Duration ( 14*86400); t.is (d.formatCompact (), "2wk", "14*86400 -> 2wk");
  d = Duration ( 15*86400); t.is (d.formatCompact (), "2wk", "15*86400 -> 2wk");
  d = Duration ( 16*86400); t.is (d.formatCompact (), "2wk", "16*86400 -> 2wk");
  d = Duration ( 17*86400); t.is (d.formatCompact (), "2wk", "17*86400 -> 2wk");
  d = Duration ( 18*86400); t.is (d.formatCompact (), "2wk", "18*86400 -> 2wk");
  d = Duration ( 19*86400); t.is (d.formatCompact (), "2wk", "19*86400 -> 2wk");
  d = Duration ( 20*86400); t.is (d.formatCompact (), "2wk", "20*86400 -> 2wk");

  d = Duration ( 21*86400); t.is (d.formatCompact (), "3wk", "21*86400 -> 3wk");
  d = Duration ( 22*86400); t.is (d.formatCompact (), "3wk", "22*86400 -> 3wk");
  d = Duration ( 23*86400); t.is (d.formatCompact (), "3wk", "23*86400 -> 3wk");
  d = Duration ( 24*86400); t.is (d.formatCompact (), "3wk", "24*86400 -> 3wk");
  d = Duration ( 25*86400); t.is (d.formatCompact (), "3wk", "25*86400 -> 3wk");
  d = Duration ( 26*86400); t.is (d.formatCompact (), "3wk", "26*86400 -> 3wk");
  d = Duration ( 27*86400); t.is (d.formatCompact (), "3wk", "27*86400 -> 3wk");
  d = Duration ( 28*86400); t.is (d.formatCompact (), "4wk", "28*86400 -> 4wk");
  d = Duration ( 29*86400); t.is (d.formatCompact (), "4wk", "29*86400 -> 4wk");
  d = Duration ( 30*86400); t.is (d.formatCompact (), "4wk", "30*86400 -> 4wk");

  d = Duration ( 31*86400); t.is (d.formatCompact (), "4wk", "31*86400 -> 4wk");
  d = Duration ( 32*86400); t.is (d.formatCompact (), "4wk", "32*86400 -> 4wk");
  d = Duration ( 33*86400); t.is (d.formatCompact (), "4wk", "33*86400 -> 4wk");
  d = Duration ( 34*86400); t.is (d.formatCompact (), "4wk", "34*86400 -> 4wk");
  d = Duration ( 35*86400); t.is (d.formatCompact (), "5wk", "35*86400 -> 5wk");
  d = Duration ( 36*86400); t.is (d.formatCompact (), "5wk", "36*86400 -> 5wk");
  d = Duration ( 37*86400); t.is (d.formatCompact (), "5wk", "37*86400 -> 5wk");
  d = Duration ( 38*86400); t.is (d.formatCompact (), "5wk", "38*86400 -> 5wk");
  d = Duration ( 39*86400); t.is (d.formatCompact (), "5wk", "39*86400 -> 5wk");
  d = Duration ( 40*86400); t.is (d.formatCompact (), "5wk", "40*86400 -> 5wk");

  d = Duration ( 41*86400); t.is (d.formatCompact (), "5wk", "41*86400 -> 5wk");
  d = Duration ( 42*86400); t.is (d.formatCompact (), "6wk", "42*86400 -> 6wk");
  d = Duration ( 43*86400); t.is (d.formatCompact (), "6wk", "43*86400 -> 6wk");
  d = Duration ( 44*86400); t.is (d.formatCompact (), "6wk", "44*86400 -> 6wk");
  d = Duration ( 45*86400); t.is (d.formatCompact (), "6wk", "45*86400 -> 6wk");
  d = Duration ( 46*86400); t.is (d.formatCompact (), "6wk", "46*86400 -> 6wk");
  d = Duration ( 47*86400); t.is (d.formatCompact (), "6wk", "47*86400 -> 6wk");
  d = Duration ( 48*86400); t.is (d.formatCompact (), "6wk", "48*86400 -> 6wk");
  d = Duration ( 49*86400); t.is (d.formatCompact (), "7wk", "49*86400 -> 7wk");
  d = Duration ( 50*86400); t.is (d.formatCompact (), "7wk", "50*86400 -> 7wk");

  d = Duration ( 51*86400); t.is (d.formatCompact (), "7wk", "51*86400 -> 7wk");
  d = Duration ( 52*86400); t.is (d.formatCompact (), "7wk", "52*86400 -> 7wk");
  d = Duration ( 53*86400); t.is (d.formatCompact (), "7wk", "53*86400 -> 7wk");
  d = Duration ( 54*86400); t.is (d.formatCompact (), "7wk", "54*86400 -> 7wk");
  d = Duration ( 55*86400); t.is (d.formatCompact (), "7wk", "55*86400 -> 7wk");
  d = Duration ( 56*86400); t.is (d.formatCompact (), "8wk", "56*86400 -> 8wk");
  d = Duration ( 57*86400); t.is (d.formatCompact (), "8wk", "57*86400 -> 8wk");
  d = Duration ( 58*86400); t.is (d.formatCompact (), "8wk", "58*86400 -> 8wk");
  d = Duration ( 59*86400); t.is (d.formatCompact (), "8wk", "59*86400 -> 8wk");
  d = Duration ( 60*86400); t.is (d.formatCompact (), "8wk", "60*86400 -> 8wk");

  d = Duration ( 61*86400); t.is (d.formatCompact (), "8wk", "61*86400 -> 8wk");
  d = Duration ( 62*86400); t.is (d.formatCompact (), "8wk", "62*86400 -> 8wk");
  d = Duration ( 63*86400); t.is (d.formatCompact (), "9wk", "63*86400 -> 9wk");
  d = Duration ( 64*86400); t.is (d.formatCompact (), "9wk", "64*86400 -> 9wk");
  d = Duration ( 65*86400); t.is (d.formatCompact (), "9wk", "65*86400 -> 9wk");
  d = Duration ( 66*86400); t.is (d.formatCompact (), "9wk", "66*86400 -> 9wk");
  d = Duration ( 67*86400); t.is (d.formatCompact (), "9wk", "67*86400 -> 9wk");
  d = Duration ( 68*86400); t.is (d.formatCompact (), "9wk", "68*86400 -> 9wk");
  d = Duration ( 69*86400); t.is (d.formatCompact (), "9wk", "69*86400 -> 9wk");
  d = Duration ( 70*86400); t.is (d.formatCompact (), "10wk", "70*86400 -> 10wk");

  d = Duration ( 71*86400); t.is (d.formatCompact (), "10wk", "71*86400 -> 10wk");
  d = Duration ( 72*86400); t.is (d.formatCompact (), "10wk", "72*86400 -> 10wk");
  d = Duration ( 73*86400); t.is (d.formatCompact (), "10wk", "73*86400 -> 10wk");
  d = Duration ( 74*86400); t.is (d.formatCompact (), "10wk", "74*86400 -> 10wk");
  d = Duration ( 75*86400); t.is (d.formatCompact (), "10wk", "75*86400 -> 10wk");
  d = Duration ( 76*86400); t.is (d.formatCompact (), "10wk", "76*86400 -> 10wk");
  d = Duration ( 77*86400); t.is (d.formatCompact (), "11wk", "77*86400 -> 11wk");
  d = Duration ( 78*86400); t.is (d.formatCompact (), "11wk", "78*86400 -> 11wk");
  d = Duration ( 79*86400); t.is (d.formatCompact (), "11wk", "79*86400 -> 11wk");
  d = Duration ( 80*86400); t.is (d.formatCompact (), "11wk", "80*86400 -> 11wk");

  d = Duration ( 81*86400); t.is (d.formatCompact (), "11wk", "81*86400 -> 11wk");
  d = Duration ( 82*86400); t.is (d.formatCompact (), "11wk", "82*86400 -> 11wk");
  d = Duration ( 83*86400); t.is (d.formatCompact (), "11wk", "83*86400 -> 11wk");
  d = Duration ( 84*86400); t.is (d.formatCompact (), "2mo", "84*86400 -> 2mo");
  d = Duration ( 85*86400); t.is (d.formatCompact (), "2mo", "85*86400 -> 2mo");
  d = Duration ( 86*86400); t.is (d.formatCompact (), "2mo", "86*86400 -> 2mo");
  d = Duration ( 87*86400); t.is (d.formatCompact (), "2mo", "87*86400 -> 2mo");
  d = Duration ( 88*86400); t.is (d.formatCompact (), "2mo", "88*86400 -> 2mo");
  d = Duration ( 89*86400); t.is (d.formatCompact (), "2mo", "89*86400 -> 2mo");
  d = Duration ( 90*86400); t.is (d.formatCompact (), "3mo", "90*86400 -> 3mo");

  d = Duration ( 91*86400); t.is (d.formatCompact (), "3mo", "91*86400 -> 3mo");
  d = Duration ( 92*86400); t.is (d.formatCompact (), "3mo", "92*86400 -> 3mo");
  d = Duration ( 93*86400); t.is (d.formatCompact (), "3mo", "93*86400 -> 3mo");
  d = Duration ( 94*86400); t.is (d.formatCompact (), "3mo", "94*86400 -> 3mo");
  d = Duration ( 95*86400); t.is (d.formatCompact (), "3mo", "95*86400 -> 3mo");
  d = Duration ( 96*86400); t.is (d.formatCompact (), "3mo", "96*86400 -> 3mo");
  d = Duration ( 97*86400); t.is (d.formatCompact (), "3mo", "97*86400 -> 3mo");
  d = Duration ( 98*86400); t.is (d.formatCompact (), "3mo", "98*86400 -> 3mo");
  d = Duration ( 99*86400); t.is (d.formatCompact (), "3mo", "99*86400 -> 3mo");
  d = Duration (100*86400); t.is (d.formatCompact (), "3mo", "100*86400 -> 3mo");

  d = Duration (101*86400); t.is (d.formatCompact (), "3mo", "101*86400 -> 3mo");
  d = Duration (102*86400); t.is (d.formatCompact (), "3mo", "102*86400 -> 3mo");
  d = Duration (103*86400); t.is (d.formatCompact (), "3mo", "103*86400 -> 3mo");
  d = Duration (104*86400); t.is (d.formatCompact (), "3mo", "104*86400 -> 3mo");
  d = Duration (105*86400); t.is (d.formatCompact (), "3mo", "105*86400 -> 3mo");
  d = Duration (106*86400); t.is (d.formatCompact (), "3mo", "106*86400 -> 3mo");
  d = Duration (107*86400); t.is (d.formatCompact (), "3mo", "107*86400 -> 3mo");
  d = Duration (108*86400); t.is (d.formatCompact (), "3mo", "108*86400 -> 3mo");
  d = Duration (109*86400); t.is (d.formatCompact (), "3mo", "109*86400 -> 3mo");
  d = Duration (110*86400); t.is (d.formatCompact (), "3mo", "110*86400 -> 3mo");

  d = Duration (111*86400); t.is (d.formatCompact (), "3mo", "111*86400 -> 3mo");
  d = Duration (112*86400); t.is (d.formatCompact (), "3mo", "112*86400 -> 3mo");
  d = Duration (113*86400); t.is (d.formatCompact (), "3mo", "113*86400 -> 3mo");
  d = Duration (114*86400); t.is (d.formatCompact (), "3mo", "114*86400 -> 3mo");
  d = Duration (115*86400); t.is (d.formatCompact (), "3mo", "115*86400 -> 3mo");
  d = Duration (116*86400); t.is (d.formatCompact (), "3mo", "116*86400 -> 3mo");
  d = Duration (117*86400); t.is (d.formatCompact (), "3mo", "117*86400 -> 3mo");
  d = Duration (118*86400); t.is (d.formatCompact (), "3mo", "118*86400 -> 3mo");
  d = Duration (119*86400); t.is (d.formatCompact (), "3mo", "119*86400 -> 3mo");
  d = Duration (120*86400); t.is (d.formatCompact (), "4mo", "120*86400 -> 4mo");

  d = Duration (121*86400); t.is (d.formatCompact (), "4mo", "121*86400 -> 4mo");
  d = Duration (122*86400); t.is (d.formatCompact (), "4mo", "122*86400 -> 4mo");
  d = Duration (123*86400); t.is (d.formatCompact (), "4mo", "123*86400 -> 4mo");
  d = Duration (124*86400); t.is (d.formatCompact (), "4mo", "124*86400 -> 4mo");
  d = Duration (125*86400); t.is (d.formatCompact (), "4mo", "125*86400 -> 4mo");
  d = Duration (126*86400); t.is (d.formatCompact (), "4mo", "126*86400 -> 4mo");
  d = Duration (127*86400); t.is (d.formatCompact (), "4mo", "127*86400 -> 4mo");
  d = Duration (128*86400); t.is (d.formatCompact (), "4mo", "128*86400 -> 4mo");
  d = Duration (129*86400); t.is (d.formatCompact (), "4mo", "129*86400 -> 4mo");
  d = Duration (130*86400); t.is (d.formatCompact (), "4mo", "130*86400 -> 4mo");

  d = Duration (131*86400); t.is (d.formatCompact (), "4mo", "131*86400 -> 4mo");
  d = Duration (132*86400); t.is (d.formatCompact (), "4mo", "132*86400 -> 4mo");
  d = Duration (133*86400); t.is (d.formatCompact (), "4mo", "133*86400 -> 4mo");
  d = Duration (134*86400); t.is (d.formatCompact (), "4mo", "134*86400 -> 4mo");
  d = Duration (135*86400); t.is (d.formatCompact (), "4mo", "135*86400 -> 4mo");
  d = Duration (136*86400); t.is (d.formatCompact (), "4mo", "136*86400 -> 4mo");
  d = Duration (137*86400); t.is (d.formatCompact (), "4mo", "137*86400 -> 4mo");
  d = Duration (138*86400); t.is (d.formatCompact (), "4mo", "138*86400 -> 4mo");
  d = Duration (139*86400); t.is (d.formatCompact (), "4mo", "139*86400 -> 4mo");
  d = Duration (140*86400); t.is (d.formatCompact (), "4mo", "140*86400 -> 4mo");

  d = Duration (141*86400); t.is (d.formatCompact (), "4mo", "141*86400 -> 4mo");
  d = Duration (142*86400); t.is (d.formatCompact (), "4mo", "142*86400 -> 4mo");
  d = Duration (143*86400); t.is (d.formatCompact (), "4mo", "143*86400 -> 4mo");
  d = Duration (144*86400); t.is (d.formatCompact (), "4mo", "144*86400 -> 4mo");
  d = Duration (145*86400); t.is (d.formatCompact (), "4mo", "145*86400 -> 4mo");
  d = Duration (146*86400); t.is (d.formatCompact (), "4mo", "146*86400 -> 4mo");
  d = Duration (147*86400); t.is (d.formatCompact (), "4mo", "147*86400 -> 4mo");
  d = Duration (148*86400); t.is (d.formatCompact (), "4mo", "148*86400 -> 4mo");
  d = Duration (149*86400); t.is (d.formatCompact (), "4mo", "149*86400 -> 4mo");
  d = Duration (150*86400); t.is (d.formatCompact (), "5mo", "150*86400 -> 5mo");

  d = Duration (151*86400); t.is (d.formatCompact (), "5mo", "151*86400 -> 5mo");
  d = Duration (152*86400); t.is (d.formatCompact (), "5mo", "152*86400 -> 5mo");
  d = Duration (153*86400); t.is (d.formatCompact (), "5mo", "153*86400 -> 5mo");
  d = Duration (154*86400); t.is (d.formatCompact (), "5mo", "154*86400 -> 5mo");
  d = Duration (155*86400); t.is (d.formatCompact (), "5mo", "155*86400 -> 5mo");
  d = Duration (156*86400); t.is (d.formatCompact (), "5mo", "156*86400 -> 5mo");
  d = Duration (157*86400); t.is (d.formatCompact (), "5mo", "157*86400 -> 5mo");
  d = Duration (158*86400); t.is (d.formatCompact (), "5mo", "158*86400 -> 5mo");
  d = Duration (159*86400); t.is (d.formatCompact (), "5mo", "159*86400 -> 5mo");
  d = Duration (160*86400); t.is (d.formatCompact (), "5mo", "160*86400 -> 5mo");

  d = Duration (161*86400); t.is (d.formatCompact (), "5mo", "161*86400 -> 5mo");
  d = Duration (162*86400); t.is (d.formatCompact (), "5mo", "162*86400 -> 5mo");
  d = Duration (163*86400); t.is (d.formatCompact (), "5mo", "163*86400 -> 5mo");
  d = Duration (164*86400); t.is (d.formatCompact (), "5mo", "164*86400 -> 5mo");
  d = Duration (165*86400); t.is (d.formatCompact (), "5mo", "165*86400 -> 5mo");
  d = Duration (166*86400); t.is (d.formatCompact (), "5mo", "166*86400 -> 5mo");
  d = Duration (167*86400); t.is (d.formatCompact (), "5mo", "167*86400 -> 5mo");
  d = Duration (168*86400); t.is (d.formatCompact (), "5mo", "168*86400 -> 5mo");
  d = Duration (169*86400); t.is (d.formatCompact (), "5mo", "169*86400 -> 5mo");
  d = Duration (170*86400); t.is (d.formatCompact (), "5mo", "170*86400 -> 5mo");

  d = Duration (171*86400); t.is (d.formatCompact (), "5mo", "171*86400 -> 5mo");
  d = Duration (172*86400); t.is (d.formatCompact (), "5mo", "172*86400 -> 5mo");
  d = Duration (173*86400); t.is (d.formatCompact (), "5mo", "173*86400 -> 5mo");
  d = Duration (174*86400); t.is (d.formatCompact (), "5mo", "174*86400 -> 5mo");
  d = Duration (175*86400); t.is (d.formatCompact (), "5mo", "175*86400 -> 5mo");
  d = Duration (176*86400); t.is (d.formatCompact (), "5mo", "176*86400 -> 5mo");
  d = Duration (177*86400); t.is (d.formatCompact (), "5mo", "177*86400 -> 5mo");
  d = Duration (178*86400); t.is (d.formatCompact (), "5mo", "178*86400 -> 5mo");
  d = Duration (179*86400); t.is (d.formatCompact (), "5mo", "179*86400 -> 5mo");
  d = Duration (180*86400); t.is (d.formatCompact (), "6mo", "180*86400 -> 6mo");

  d = Duration (181*86400); t.is (d.formatCompact (), "6mo", "181*86400 -> 6mo");
  d = Duration (182*86400); t.is (d.formatCompact (), "6mo", "182*86400 -> 6mo");
  d = Duration (183*86400); t.is (d.formatCompact (), "6mo", "183*86400 -> 6mo");
  d = Duration (184*86400); t.is (d.formatCompact (), "6mo", "184*86400 -> 6mo");
  d = Duration (185*86400); t.is (d.formatCompact (), "6mo", "185*86400 -> 6mo");
  d = Duration (186*86400); t.is (d.formatCompact (), "6mo", "186*86400 -> 6mo");
  d = Duration (187*86400); t.is (d.formatCompact (), "6mo", "187*86400 -> 6mo");
  d = Duration (188*86400); t.is (d.formatCompact (), "6mo", "188*86400 -> 6mo");
  d = Duration (189*86400); t.is (d.formatCompact (), "6mo", "189*86400 -> 6mo");
  d = Duration (190*86400); t.is (d.formatCompact (), "6mo", "190*86400 -> 6mo");

  d = Duration (191*86400); t.is (d.formatCompact (), "6mo", "191*86400 -> 6mo");
  d = Duration (192*86400); t.is (d.formatCompact (), "6mo", "192*86400 -> 6mo");
  d = Duration (193*86400); t.is (d.formatCompact (), "6mo", "193*86400 -> 6mo");
  d = Duration (194*86400); t.is (d.formatCompact (), "6mo", "194*86400 -> 6mo");
  d = Duration (195*86400); t.is (d.formatCompact (), "6mo", "195*86400 -> 6mo");
  d = Duration (196*86400); t.is (d.formatCompact (), "6mo", "196*86400 -> 6mo");
  d = Duration (197*86400); t.is (d.formatCompact (), "6mo", "197*86400 -> 6mo");
  d = Duration (198*86400); t.is (d.formatCompact (), "6mo", "198*86400 -> 6mo");
  d = Duration (199*86400); t.is (d.formatCompact (), "6mo", "199*86400 -> 6mo");
  d = Duration (200*86400); t.is (d.formatCompact (), "6mo", "200*86400 -> 6mo");

  d = Duration (201*86400); t.is (d.formatCompact (), "6mo", "201*86400 -> 6mo");
  d = Duration (202*86400); t.is (d.formatCompact (), "6mo", "202*86400 -> 6mo");
  d = Duration (203*86400); t.is (d.formatCompact (), "6mo", "203*86400 -> 6mo");
  d = Duration (204*86400); t.is (d.formatCompact (), "6mo", "204*86400 -> 6mo");
  d = Duration (205*86400); t.is (d.formatCompact (), "6mo", "205*86400 -> 6mo");
  d = Duration (206*86400); t.is (d.formatCompact (), "6mo", "206*86400 -> 6mo");
  d = Duration (207*86400); t.is (d.formatCompact (), "6mo", "207*86400 -> 6mo");
  d = Duration (208*86400); t.is (d.formatCompact (), "6mo", "208*86400 -> 6mo");
  d = Duration (209*86400); t.is (d.formatCompact (), "6mo", "209*86400 -> 6mo");
  d = Duration (210*86400); t.is (d.formatCompact (), "7mo", "210*86400 -> 7mo");

  d = Duration (211*86400); t.is (d.formatCompact (), "7mo", "211*86400 -> 7mo");
  d = Duration (212*86400); t.is (d.formatCompact (), "7mo", "212*86400 -> 7mo");
  d = Duration (213*86400); t.is (d.formatCompact (), "7mo", "213*86400 -> 7mo");
  d = Duration (214*86400); t.is (d.formatCompact (), "7mo", "214*86400 -> 7mo");
  d = Duration (215*86400); t.is (d.formatCompact (), "7mo", "215*86400 -> 7mo");
  d = Duration (216*86400); t.is (d.formatCompact (), "7mo", "216*86400 -> 7mo");
  d = Duration (217*86400); t.is (d.formatCompact (), "7mo", "217*86400 -> 7mo");
  d = Duration (218*86400); t.is (d.formatCompact (), "7mo", "218*86400 -> 7mo");
  d = Duration (219*86400); t.is (d.formatCompact (), "7mo", "219*86400 -> 7mo");
  d = Duration (220*86400); t.is (d.formatCompact (), "7mo", "220*86400 -> 7mo");

  d = Duration (221*86400); t.is (d.formatCompact (), "7mo", "221*86400 -> 7mo");
  d = Duration (222*86400); t.is (d.formatCompact (), "7mo", "222*86400 -> 7mo");
  d = Duration (223*86400); t.is (d.formatCompact (), "7mo", "223*86400 -> 7mo");
  d = Duration (224*86400); t.is (d.formatCompact (), "7mo", "224*86400 -> 7mo");
  d = Duration (225*86400); t.is (d.formatCompact (), "7mo", "225*86400 -> 7mo");
  d = Duration (226*86400); t.is (d.formatCompact (), "7mo", "226*86400 -> 7mo");
  d = Duration (227*86400); t.is (d.formatCompact (), "7mo", "227*86400 -> 7mo");
  d = Duration (228*86400); t.is (d.formatCompact (), "7mo", "228*86400 -> 7mo");
  d = Duration (229*86400); t.is (d.formatCompact (), "7mo", "229*86400 -> 7mo");
  d = Duration (230*86400); t.is (d.formatCompact (), "7mo", "230*86400 -> 7mo");

  d = Duration (231*86400); t.is (d.formatCompact (), "7mo", "231*86400 -> 7mo");
  d = Duration (232*86400); t.is (d.formatCompact (), "7mo", "232*86400 -> 7mo");
  d = Duration (233*86400); t.is (d.formatCompact (), "7mo", "233*86400 -> 7mo");
  d = Duration (234*86400); t.is (d.formatCompact (), "7mo", "234*86400 -> 7mo");
  d = Duration (235*86400); t.is (d.formatCompact (), "7mo", "235*86400 -> 7mo");
  d = Duration (236*86400); t.is (d.formatCompact (), "7mo", "236*86400 -> 7mo");
  d = Duration (237*86400); t.is (d.formatCompact (), "7mo", "237*86400 -> 7mo");
  d = Duration (238*86400); t.is (d.formatCompact (), "7mo", "238*86400 -> 7mo");
  d = Duration (239*86400); t.is (d.formatCompact (), "7mo", "239*86400 -> 7mo");
  d = Duration (240*86400); t.is (d.formatCompact (), "8mo", "240*86400 -> 8mo");

  d = Duration (241*86400); t.is (d.formatCompact (), "8mo", "241*86400 -> 8mo");
  d = Duration (242*86400); t.is (d.formatCompact (), "8mo", "242*86400 -> 8mo");
  d = Duration (243*86400); t.is (d.formatCompact (), "8mo", "243*86400 -> 8mo");
  d = Duration (244*86400); t.is (d.formatCompact (), "8mo", "244*86400 -> 8mo");
  d = Duration (245*86400); t.is (d.formatCompact (), "8mo", "245*86400 -> 8mo");
  d = Duration (246*86400); t.is (d.formatCompact (), "8mo", "246*86400 -> 8mo");
  d = Duration (247*86400); t.is (d.formatCompact (), "8mo", "247*86400 -> 8mo");
  d = Duration (248*86400); t.is (d.formatCompact (), "8mo", "248*86400 -> 8mo");
  d = Duration (249*86400); t.is (d.formatCompact (), "8mo", "249*86400 -> 8mo");
  d = Duration (250*86400); t.is (d.formatCompact (), "8mo", "250*86400 -> 8mo");

  d = Duration (251*86400); t.is (d.formatCompact (), "8mo", "251*86400 -> 8mo");
  d = Duration (252*86400); t.is (d.formatCompact (), "8mo", "252*86400 -> 8mo");
  d = Duration (253*86400); t.is (d.formatCompact (), "8mo", "253*86400 -> 8mo");
  d = Duration (254*86400); t.is (d.formatCompact (), "8mo", "254*86400 -> 8mo");
  d = Duration (255*86400); t.is (d.formatCompact (), "8mo", "255*86400 -> 8mo");
  d = Duration (256*86400); t.is (d.formatCompact (), "8mo", "256*86400 -> 8mo");
  d = Duration (257*86400); t.is (d.formatCompact (), "8mo", "257*86400 -> 8mo");
  d = Duration (258*86400); t.is (d.formatCompact (), "8mo", "258*86400 -> 8mo");
  d = Duration (259*86400); t.is (d.formatCompact (), "8mo", "259*86400 -> 8mo");
  d = Duration (260*86400); t.is (d.formatCompact (), "8mo", "260*86400 -> 8mo");

  d = Duration (261*86400); t.is (d.formatCompact (), "8mo", "261*86400 -> 8mo");
  d = Duration (262*86400); t.is (d.formatCompact (), "8mo", "262*86400 -> 8mo");
  d = Duration (263*86400); t.is (d.formatCompact (), "8mo", "263*86400 -> 8mo");
  d = Duration (264*86400); t.is (d.formatCompact (), "8mo", "264*86400 -> 8mo");
  d = Duration (265*86400); t.is (d.formatCompact (), "8mo", "265*86400 -> 8mo");
  d = Duration (266*86400); t.is (d.formatCompact (), "8mo", "266*86400 -> 8mo");
  d = Duration (267*86400); t.is (d.formatCompact (), "8mo", "267*86400 -> 8mo");
  d = Duration (268*86400); t.is (d.formatCompact (), "8mo", "268*86400 -> 8mo");
  d = Duration (269*86400); t.is (d.formatCompact (), "8mo", "269*86400 -> 8mo");
  d = Duration (270*86400); t.is (d.formatCompact (), "9mo", "270*86400 -> 9mo");

  d = Duration (271*86400); t.is (d.formatCompact (), "9mo", "271*86400 -> 9mo");
  d = Duration (272*86400); t.is (d.formatCompact (), "9mo", "272*86400 -> 9mo");
  d = Duration (273*86400); t.is (d.formatCompact (), "9mo", "273*86400 -> 9mo");
  d = Duration (274*86400); t.is (d.formatCompact (), "9mo", "274*86400 -> 9mo");
  d = Duration (275*86400); t.is (d.formatCompact (), "9mo", "275*86400 -> 9mo");
  d = Duration (276*86400); t.is (d.formatCompact (), "9mo", "276*86400 -> 9mo");
  d = Duration (277*86400); t.is (d.formatCompact (), "9mo", "277*86400 -> 9mo");
  d = Duration (278*86400); t.is (d.formatCompact (), "9mo", "278*86400 -> 9mo");
  d = Duration (279*86400); t.is (d.formatCompact (), "9mo", "279*86400 -> 9mo");
  d = Duration (280*86400); t.is (d.formatCompact (), "9mo", "280*86400 -> 9mo");

  d = Duration (281*86400); t.is (d.formatCompact (), "9mo", "281*86400 -> 9mo");
  d = Duration (282*86400); t.is (d.formatCompact (), "9mo", "282*86400 -> 9mo");
  d = Duration (283*86400); t.is (d.formatCompact (), "9mo", "283*86400 -> 9mo");
  d = Duration (284*86400); t.is (d.formatCompact (), "9mo", "284*86400 -> 9mo");
  d = Duration (285*86400); t.is (d.formatCompact (), "9mo", "285*86400 -> 9mo");
  d = Duration (286*86400); t.is (d.formatCompact (), "9mo", "286*86400 -> 9mo");
  d = Duration (287*86400); t.is (d.formatCompact (), "9mo", "287*86400 -> 9mo");
  d = Duration (288*86400); t.is (d.formatCompact (), "9mo", "288*86400 -> 9mo");
  d = Duration (289*86400); t.is (d.formatCompact (), "9mo", "289*86400 -> 9mo");
  d = Duration (290*86400); t.is (d.formatCompact (), "9mo", "290*86400 -> 9mo");

  d = Duration (291*86400); t.is (d.formatCompact (), "9mo", "291*86400 -> 9mo");
  d = Duration (292*86400); t.is (d.formatCompact (), "9mo", "292*86400 -> 9mo");
  d = Duration (293*86400); t.is (d.formatCompact (), "9mo", "293*86400 -> 9mo");
  d = Duration (294*86400); t.is (d.formatCompact (), "9mo", "294*86400 -> 9mo");
  d = Duration (295*86400); t.is (d.formatCompact (), "9mo", "295*86400 -> 9mo");
  d = Duration (296*86400); t.is (d.formatCompact (), "9mo", "296*86400 -> 9mo");
  d = Duration (297*86400); t.is (d.formatCompact (), "9mo", "297*86400 -> 9mo");
  d = Duration (298*86400); t.is (d.formatCompact (), "9mo", "298*86400 -> 9mo");
  d = Duration (299*86400); t.is (d.formatCompact (), "9mo", "299*86400 -> 9mo");
  d = Duration (300*86400); t.is (d.formatCompact (), "10mo", "300*86400 -> 10mo");

  d = Duration (301*86400); t.is (d.formatCompact (), "10mo", "301*86400 -> 10mo");
  d = Duration (302*86400); t.is (d.formatCompact (), "10mo", "302*86400 -> 10mo");
  d = Duration (303*86400); t.is (d.formatCompact (), "10mo", "303*86400 -> 10mo");
  d = Duration (304*86400); t.is (d.formatCompact (), "10mo", "304*86400 -> 10mo");
  d = Duration (305*86400); t.is (d.formatCompact (), "10mo", "305*86400 -> 10mo");
  d = Duration (306*86400); t.is (d.formatCompact (), "10mo", "306*86400 -> 10mo");
  d = Duration (307*86400); t.is (d.formatCompact (), "10mo", "307*86400 -> 10mo");
  d = Duration (308*86400); t.is (d.formatCompact (), "10mo", "308*86400 -> 10mo");
  d = Duration (309*86400); t.is (d.formatCompact (), "10mo", "309*86400 -> 10mo");
  d = Duration (310*86400); t.is (d.formatCompact (), "10mo", "310*86400 -> 10mo");

  d = Duration (311*86400); t.is (d.formatCompact (), "10mo", "311*86400 -> 10mo");
  d = Duration (312*86400); t.is (d.formatCompact (), "10mo", "312*86400 -> 10mo");
  d = Duration (313*86400); t.is (d.formatCompact (), "10mo", "313*86400 -> 10mo");
  d = Duration (314*86400); t.is (d.formatCompact (), "10mo", "314*86400 -> 10mo");
  d = Duration (315*86400); t.is (d.formatCompact (), "10mo", "315*86400 -> 10mo");
  d = Duration (316*86400); t.is (d.formatCompact (), "10mo", "316*86400 -> 10mo");
  d = Duration (317*86400); t.is (d.formatCompact (), "10mo", "317*86400 -> 10mo");
  d = Duration (318*86400); t.is (d.formatCompact (), "10mo", "318*86400 -> 10mo");
  d = Duration (319*86400); t.is (d.formatCompact (), "10mo", "319*86400 -> 10mo");
  d = Duration (320*86400); t.is (d.formatCompact (), "10mo", "320*86400 -> 10mo");

  d = Duration (321*86400); t.is (d.formatCompact (), "10mo", "321*86400 -> 10mo");
  d = Duration (322*86400); t.is (d.formatCompact (), "10mo", "322*86400 -> 10mo");
  d = Duration (323*86400); t.is (d.formatCompact (), "10mo", "323*86400 -> 10mo");
  d = Duration (324*86400); t.is (d.formatCompact (), "10mo", "324*86400 -> 10mo");
  d = Duration (325*86400); t.is (d.formatCompact (), "10mo", "325*86400 -> 10mo");
  d = Duration (326*86400); t.is (d.formatCompact (), "10mo", "326*86400 -> 10mo");
  d = Duration (327*86400); t.is (d.formatCompact (), "10mo", "327*86400 -> 10mo");
  d = Duration (328*86400); t.is (d.formatCompact (), "10mo", "328*86400 -> 10mo");
  d = Duration (329*86400); t.is (d.formatCompact (), "10mo", "329*86400 -> 10mo");
  d = Duration (330*86400); t.is (d.formatCompact (), "11mo", "330*86400 -> 11mo");

  d = Duration (331*86400); t.is (d.formatCompact (), "11mo", "331*86400 -> 11mo");
  d = Duration (332*86400); t.is (d.formatCompact (), "11mo", "332*86400 -> 11mo");
  d = Duration (333*86400); t.is (d.formatCompact (), "11mo", "333*86400 -> 11mo");
  d = Duration (334*86400); t.is (d.formatCompact (), "11mo", "334*86400 -> 11mo");
  d = Duration (335*86400); t.is (d.formatCompact (), "11mo", "335*86400 -> 11mo");
  d = Duration (336*86400); t.is (d.formatCompact (), "11mo", "336*86400 -> 11mo");
  d = Duration (337*86400); t.is (d.formatCompact (), "11mo", "337*86400 -> 11mo");
  d = Duration (338*86400); t.is (d.formatCompact (), "11mo", "338*86400 -> 11mo");
  d = Duration (339*86400); t.is (d.formatCompact (), "11mo", "339*86400 -> 11mo");
  d = Duration (340*86400); t.is (d.formatCompact (), "11mo", "340*86400 -> 11mo");

  d = Duration (341*86400); t.is (d.formatCompact (), "11mo", "341*86400 -> 11mo");
  d = Duration (342*86400); t.is (d.formatCompact (), "11mo", "342*86400 -> 11mo");
  d = Duration (343*86400); t.is (d.formatCompact (), "11mo", "343*86400 -> 11mo");
  d = Duration (344*86400); t.is (d.formatCompact (), "11mo", "344*86400 -> 11mo");
  d = Duration (345*86400); t.is (d.formatCompact (), "11mo", "345*86400 -> 11mo");
  d = Duration (346*86400); t.is (d.formatCompact (), "11mo", "346*86400 -> 11mo");
  d = Duration (347*86400); t.is (d.formatCompact (), "11mo", "347*86400 -> 11mo");
  d = Duration (348*86400); t.is (d.formatCompact (), "11mo", "348*86400 -> 11mo");
  d = Duration (349*86400); t.is (d.formatCompact (), "11mo", "349*86400 -> 11mo");
  d = Duration (350*86400); t.is (d.formatCompact (), "11mo", "350*86400 -> 11mo");

  d = Duration (351*86400); t.is (d.formatCompact (), "11mo", "351*86400 -> 11mo");
  d = Duration (352*86400); t.is (d.formatCompact (), "11mo", "352*86400 -> 11mo");
  d = Duration (353*86400); t.is (d.formatCompact (), "11mo", "353*86400 -> 11mo");
  d = Duration (354*86400); t.is (d.formatCompact (), "11mo", "354*86400 -> 11mo");
  d = Duration (355*86400); t.is (d.formatCompact (), "11mo", "355*86400 -> 11mo");
  d = Duration (356*86400); t.is (d.formatCompact (), "11mo", "356*86400 -> 11mo");
  d = Duration (357*86400); t.is (d.formatCompact (), "11mo", "357*86400 -> 11mo");
  d = Duration (358*86400); t.is (d.formatCompact (), "11mo", "358*86400 -> 11mo");
  d = Duration (359*86400); t.is (d.formatCompact (), "11mo", "359*86400 -> 11mo");
  d = Duration (360*86400); t.is (d.formatCompact (), "12mo", "360*86400 -> 12mo");

  d = Duration (361*86400); t.is (d.formatCompact (), "12mo", "361*86400 -> 12mo");
  d = Duration (362*86400); t.is (d.formatCompact (), "12mo", "362*86400 -> 12mo");
  d = Duration (363*86400); t.is (d.formatCompact (), "12mo", "363*86400 -> 12mo");
  d = Duration (364*86400); t.is (d.formatCompact (), "12mo", "364*86400 -> 12mo");
  d = Duration (365*86400); t.is (d.formatCompact (), "1.0y", "365*86400 -> 1.0y");

  d = Duration ("86400");   t.is (d.formatCompact (), "1d",   "string '86400' -> 1d");

	t.ok (d.valid ("daily"),      "valid duration daily");
  t.ok (d.valid ("day"),        "valid duration day");
  t.ok (d.valid ("weekly"),     "valid duration weekly");
  t.ok (d.valid ("weekdays"),   "valid duration weekdays");
  t.ok (d.valid ("sennight"),   "valid duration sennight");
  t.ok (d.valid ("biweekly"),   "valid duration biweekly");
  t.ok (d.valid ("fortnight"),  "valid duration fortnight");
  t.ok (d.valid ("monthly"),    "valid duration monthly");
  t.ok (d.valid ("bimonthly"),  "valid duration bimonthly");
  t.ok (d.valid ("quarterly"),  "valid duration quarterly");
  t.ok (d.valid ("annual"),     "valid duration annual");
  t.ok (d.valid ("yearly"),     "valid duration yearly");
  t.ok (d.valid ("semiannual"), "valid duration semiannual");
  t.ok (d.valid ("biannual"),   "valid duration biannual");
  t.ok (d.valid ("biyearly"),   "valid duration biyearly");

  t.ok (d.valid ("0yrs"),       "valid duration 0yrs");
  t.ok (d.valid ("0yr"),        "valid duration 0yr");
  t.ok (d.valid ("0y"),         "valid duration 0y");
  t.ok (d.valid ("1yrs"),       "valid duration 1yrs");
  t.ok (d.valid ("1yr"),        "valid duration 1yr");
  t.ok (d.valid ("1y"),         "valid duration 1y");
  t.ok (d.valid ("10yrs"),      "valid duration 10yrs");
  t.ok (d.valid ("10yr"),       "valid duration 10yr");
  t.ok (d.valid ("10y"),        "valid duration 10y");
  t.ok (d.valid ("1.1yrs"),     "valid duration 1.1yrs");
  t.ok (d.valid ("-1.1yrs"),    "valid duration -1.1yrs");
  t.ok (d.valid ("1.1y"),       "valid duration 1.1y");
  t.ok (d.valid ("-1.1y"),      "valid duration -1.1y");

  t.ok (d.valid ("0qtrs"),      "valid duration 0qtrs");
  t.ok (d.valid ("0qtr"),       "valid duration 0qtr");
  t.ok (d.valid ("0q"),         "valid duration 0q");
  t.ok (d.valid ("1qtrs"),      "valid duration 1qtrs");
  t.ok (d.valid ("1qtr"),       "valid duration 1qtr");
  t.ok (d.valid ("1q"),         "valid duration 1q");
  t.ok (d.valid ("10qtrs"),     "valid duration 10qtrs");
  t.ok (d.valid ("10qtr"),      "valid duration 10qtr");
  t.ok (d.valid ("10q"),        "valid duration 10q");

  t.ok (d.valid ("0mnths"),     "valid duration 0mnths");
  t.ok (d.valid ("0mnth"),      "valid duration 0mnth");
  t.ok (d.valid ("0mo"),        "valid duration 0mo");
  t.ok (d.valid ("1mnths"),     "valid duration 1mnths");
  t.ok (d.valid ("1mnth"),      "valid duration 1mnth");
  t.ok (d.valid ("1mo"),        "valid duration 1mo");
  t.ok (d.valid ("10mnths"),    "valid duration 10mnths");
  t.ok (d.valid ("10mnth"),     "valid duration 10mnth");
  t.ok (d.valid ("10mo"),       "valid duration 10mo");
  t.ok (d.valid ("-1mnths"),    "valid duration -1mnths");
  t.ok (d.valid ("-1mnth"),     "valid duration -1mnth");
  t.ok (d.valid ("-1mths"),     "valid duration -1mths");
  t.ok (d.valid ("-1mth"),      "valid duration -1mth");
  t.ok (d.valid ("-1mo"),       "valid duration -1mo");

  t.ok (d.valid ("0wks"),       "valid duration 0wks");
  t.ok (d.valid ("0wk"),        "valid duration 0wk");
  t.ok (d.valid ("0w"),         "valid duration 0w");
  t.ok (d.valid ("1wks"),       "valid duration 1wks");
  t.ok (d.valid ("1wk"),        "valid duration 1wk");
  t.ok (d.valid ("1wk"),        "valid duration 1wk");
  t.ok (d.valid ("1w"),         "valid duration 1w");
  t.ok (d.valid ("10wks"),      "valid duration 10wks");
  t.ok (d.valid ("10wk"),       "valid duration 10wk");
  t.ok (d.valid ("10w"),        "valid duration 10w");
  t.ok (d.valid ("-1wks"),      "valid duration -1wks");
  t.ok (d.valid ("-1wk"),       "valid duration -1wk");
  t.ok (d.valid ("-1wk"),       "valid duration -1wk");
  t.ok (d.valid ("-1w"),        "valid duration -1w");

  t.ok (d.valid ("0days"),      "valid duration 0days");
  t.ok (d.valid ("0day"),       "valid duration 0day");
  t.ok (d.valid ("0d"),         "valid duration 0d");
  t.ok (d.valid ("1days"),      "valid duration 1days");
  t.ok (d.valid ("1day"),       "valid duration 1day");
  t.ok (d.valid ("1d"),         "valid duration 1d");
  t.ok (d.valid ("10days"),     "valid duration 10days");
  t.ok (d.valid ("10day"),      "valid duration 10day");
  t.ok (d.valid ("10d"),        "valid duration 10d");
  t.ok (d.valid ("-1days"),     "valid duration -1days");
  t.ok (d.valid ("-1day"),      "valid duration -1day");
  t.ok (d.valid ("-1d"),        "valid duration -1d");

  t.ok (d.valid ("0hrs"),       "valid duration 0hrs");
  t.ok (d.valid ("0hr"),        "valid duration 0hr");
  t.ok (d.valid ("0h"),         "valid duration 0h");
  t.ok (d.valid ("1hrs"),       "valid duration 1hrs");
  t.ok (d.valid ("1hr"),        "valid duration 1hr");
  t.ok (d.valid ("1h"),         "valid duration 1h");
  t.ok (d.valid ("10hrs"),      "valid duration 10hrs");
  t.ok (d.valid ("10hr"),       "valid duration 10hr");
  t.ok (d.valid ("10h"),        "valid duration 10h");
  t.ok (d.valid ("-1hrs"),      "valid duration -1hrs");
  t.ok (d.valid ("-1hr"),       "valid duration -1hr");
  t.ok (d.valid ("-1h"),        "valid duration -1h");

  t.ok (d.valid ("0mins"),      "valid duration 0mins");
  t.ok (d.valid ("0min"),       "valid duration 0min");
  t.ok (d.valid ("1mins"),      "valid duration 1mins");
  t.ok (d.valid ("1min"),       "valid duration 1min");
  t.ok (d.valid ("10mins"),     "valid duration 10mins");
  t.ok (d.valid ("10min"),      "valid duration 10min");
  t.ok (d.valid ("-1mins"),     "valid duration -1mins");
  t.ok (d.valid ("-1min"),      "valid duration -1min");

  t.ok (d.valid ("0secs"),      "valid duration 0secs");
  t.ok (d.valid ("0sec"),       "valid duration 0sec");
  t.ok (d.valid ("0s"),         "valid duration 0s");
  t.ok (d.valid ("1secs"),      "valid duration 1secs");
  t.ok (d.valid ("1sec"),       "valid duration 1sec");
  t.ok (d.valid ("1s"),         "valid duration 1s");
  t.ok (d.valid ("10secs"),     "valid duration 10secs");
  t.ok (d.valid ("10sec"),      "valid duration 10sec");
  t.ok (d.valid ("10s"),        "valid duration 10s");
  t.ok (d.valid ("-1secs"),     "valid duration -1secs");
  t.ok (d.valid ("-1sec"),      "valid duration -1sec");
  t.ok (d.valid ("-1s"),        "valid duration -1s");

  t.ok (d.valid ("-"),          "valid duration -");

  t.ok (d.valid ("86400"),      "valid duration '86400'");

  t.notok (d.valid ("woof"),    "valid duration woof = fail");

  t.is (convertDuration ("daily"),         1, "valid duration daily");
  t.is (convertDuration ("day"),           1, "valid duration day");
  t.is (convertDuration ("weekly"),        7, "valid duration weekly");
  t.is (convertDuration ("weekdays"),      1, "valid duration weekdays");
  t.is (convertDuration ("sennight"),      7, "valid duration sennight");
  t.is (convertDuration ("biweekly"),     14, "valid duration biweekly");
  t.is (convertDuration ("fortnight"),    14, "valid duration fortnight");
  t.is (convertDuration ("monthly"),      30, "valid duration monthly");
  t.is (convertDuration ("bimonthly"),    61, "valid duration bimonthly");
  t.is (convertDuration ("quarterly"),    91, "valid duration quarterly");
  t.is (convertDuration ("annual"),      365, "valid duration annual");
  t.is (convertDuration ("yearly"),      365, "valid duration yearly");
  t.is (convertDuration ("semiannual"),  183, "valid duration semiannual");
  t.is (convertDuration ("biannual"),    730, "valid duration biannual");
  t.is (convertDuration ("biyearly"),    730, "valid duration biyearly");

  t.is (convertDuration ("0yrs"),          0, "valid duration 0yrs");
  t.is (convertDuration ("0yr"),           0, "valid duration 0yr");
  t.is (convertDuration ("0y"),            0, "valid duration 0y");
  t.is (convertDuration ("1yrs"),        365, "valid duration 1yrs");
  t.is (convertDuration ("1yr"),         365, "valid duration 1yr");
  t.is (convertDuration ("1y"),          365, "valid duration 1y");
  t.is (convertDuration ("10yrs"),      3650, "valid duration 10yrs");
  t.is (convertDuration ("10yr"),       3650, "valid duration 10yr");
  t.is (convertDuration ("10y"),        3650, "valid duration 10y");

  t.is (convertDuration ("0qtrs"),         0, "valid duration 0qtrs");
  t.is (convertDuration ("0qtr"),          0, "valid duration 0qtr");
  t.is (convertDuration ("0q"),            0, "valid duration 0q");
  t.is (convertDuration ("1qtrs"),        91, "valid duration 1qtrs");
  t.is (convertDuration ("1qtr"),         91, "valid duration 1qtr");
  t.is (convertDuration ("1q"),           91, "valid duration 1q");
  t.is (convertDuration ("10qtrs"),      910, "valid duration 10qtrs");
  t.is (convertDuration ("10qtr"),       910, "valid duration 10qtr");
  t.is (convertDuration ("10q"),         910, "valid duration 10q");

  t.is (convertDuration ("0mths"),         0, "valid duration 0mths");
  t.is (convertDuration ("0mth"),          0, "valid duration 0mth");
  t.is (convertDuration ("0mo"),           0, "valid duration 0mo");
  t.is (convertDuration ("1mths"),        30, "valid duration 1mths");
  t.is (convertDuration ("1mth"),         30, "valid duration 1mth");
  t.is (convertDuration ("1mo"),          30, "valid duration 1mo");
  t.is (convertDuration ("10mths"),      300, "valid duration 10mths");
  t.is (convertDuration ("10mth"),       300, "valid duration 10mth");
  t.is (convertDuration ("10mo"),        300, "valid duration 10mo");

  t.is (convertDuration ("0wks"),          0, "valid duration 0wks");
  t.is (convertDuration ("0wk"),           0, "valid duration 0wk");
  t.is (convertDuration ("0w"),            0, "valid duration 0w");
  t.is (convertDuration ("1wks"),          7, "valid duration 1wks");
  t.is (convertDuration ("1wk"),           7, "valid duration 1wk");
  t.is (convertDuration ("1w"),            7, "valid duration 1w");
  t.is (convertDuration ("10wks"),        70, "valid duration 10wks");
  t.is (convertDuration ("10wk"),         70, "valid duration 10wk");
  t.is (convertDuration ("10w"),          70, "valid duration 10w");

  t.is (convertDuration ("0days"),         0, "valid duration 0days");
  t.is (convertDuration ("0day"),          0, "valid duration 0day");
  t.is (convertDuration ("0d"),            0, "valid duration 0d");
  t.is (convertDuration ("1days"),         1, "valid duration 1days");
  t.is (convertDuration ("1day"),          1, "valid duration 1day");
  t.is (convertDuration ("1d"),            1, "valid duration 1d");
  t.is (convertDuration ("10days"),       10, "valid duration 10days");
  t.is (convertDuration ("10day"),        10, "valid duration 10day");
  t.is (convertDuration ("10d"),          10, "valid duration 10d");

  t.is (convertDuration ("-"),             0, "valid duration -");
  try
  {
    Duration left, right;

    // operator<
    left = Duration ("1sec");   right = Duration ("2secs");  t.ok (left < right, "duration 1sec < 2secs");
    left = Duration ("-2secs"); right = Duration ("-1sec");  t.ok (left < right, "duration -2secs < -1sec");
    left = Duration ("1sec");   right = Duration ("1min");   t.ok (left < right, "duration 1sec < 1min");
    left = Duration ("1min");   right = Duration ("1hr");    t.ok (left < right, "duration 1min < 1hr");
    left = Duration ("1hr");    right = Duration ("1d");     t.ok (left < right, "duration 1hr < 1d");
    left = Duration ("1d");     right = Duration ("1w");     t.ok (left < right, "duration 1d < 1w");
    left = Duration ("1w");     right = Duration ("1mo");    t.ok (left < right, "duration 1w < 1mo");
    left = Duration ("1mo");    right = Duration ("1q");     t.ok (left < right, "duration 1mo < 1q");
    left = Duration ("1q");     right = Duration ("1y");     t.ok (left < right, "duration 1q < 1y");

    left = Duration ("-3s");    right = Duration ("-6s");    t.ok (right < left, "duration -6s < -3s");

    // operator>
    left = Duration ("2secs");  right = Duration ("1sec");   t.ok (left > right, "2sec > 1secs");
    left = Duration ("-1sec");  right = Duration ("-2secs"); t.ok (left > right, "-1secs > -2sec");
    left = Duration ("1min");   right = Duration ("1sec");   t.ok (left > right, "1min > 1sec");
    left = Duration ("1hr");    right = Duration ("1min");   t.ok (left > right, "1hr > 1min");
    left = Duration ("1d");     right = Duration ("1hr");    t.ok (left > right, "1d > 1hr");
    left = Duration ("1w");     right = Duration ("1d");     t.ok (left > right, "1w > 1d");
    left = Duration ("1mo");    right = Duration ("1w");     t.ok (left > right, "1mo > 1w");
    left = Duration ("1q");     right = Duration ("1mo");    t.ok (left > right, "1q > 1mo");
    left = Duration ("1y");     right = Duration ("1q");     t.ok (left > right, "1y > 1q");

    left = Duration ("-3s");    right = Duration ("-6s");    t.ok (left > right, "duration -3s > -6s");

    // operator<=
    left = Duration ("1sec");   right = Duration ("2secs");  t.ok (left <= right, "duration 1sec <= 2secs");
    left = Duration ("2secs");  right = Duration ("2secs");  t.ok (left <= right, "duration 1sec <= 2secs");
    left = Duration ("2secs");  right = Duration ("1secs");  t.notok (left <= right, "duration NOT 1sec <= 2secs");

    // operator>=
    left = Duration ("1sec");   right = Duration ("2secs");  t.notok (left >= right, "duration NOT 1sec >= 2secs");
    left = Duration ("2secs");  right = Duration ("2secs");  t.ok (left >= right, "duration 1sec >= 2secs");
    left = Duration ("2secs");  right = Duration ("1secs");  t.ok (left >= right, "duration 1sec >= 2secs");

    // operator+
    left  = Duration (1);
    right = Duration (2);
    Duration result = left + right;
    t.is ((int)(time_t)left,   1, "1 + 2 = 3, 1 is still 1");
    t.is ((int)(time_t)right,  2, "1 + 2 = 3, 2 is still 2");
    t.is ((int)(time_t)result, 3, "1 + 2 = 3");

    // operator+=
    left  = Duration (1);
    right = Duration (2);
    left += right;
    t.is ((int)(time_t)left,   3, "1 += 2, 1 is now 3");
    t.is ((int)(time_t)right,  2, "1 += 2, 2 is still 2");

    // operator-
    left  = Duration (3);
    right = Duration (2);
    result = left - right;
    t.is ((int)(time_t)left,   3, "3 - 2 = 1, 3 is still 3");
    t.is ((int)(time_t)right,  2, "3 - 2 = 1, 2 is still 2");
    t.is ((int)(time_t)result, 1, "3 - 2 = 1");

    // operator-=
    left  = Duration (3);
    right = Duration (2);
    left -= right;
    t.is ((int)(time_t)left,   1, "3 -= 2, 3 is now 1");
    t.is ((int)(time_t)right,  2, "3 -= 2, 2 is still 2");

    // Assorted regression tests.
    left = Duration ("-864000.00000");
    t.is ((int)(time_t)left, 864000, "-864000.00000 -> 864000");
  }

  catch (const std::string& e) { t.diag (e); }
  catch (...) { t.diag ("Unknown error"); }

  // Duration::negative
  t.ok (  Duration ("-1day").negative (), "-1day is negative");
  t.ok (! Duration ("1day").negative (),  "1day is not negative");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
