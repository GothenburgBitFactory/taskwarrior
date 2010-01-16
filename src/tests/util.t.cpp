////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "main.h"
#include "util.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (430);

  // TODO bool confirm (const std::string&);
  // TODO int confirm3 (const std::string&);
  // TODO int confirm4 (const std::string&);
  // TODO void delay (float);

  // std::string formatSeconds (time_t);
  t.is (formatSeconds (0),               "-",       "0 -> -");
  t.is (formatSeconds (1),               "1 sec",   "1 -> 1 sec");
  t.is (formatSeconds (2),               "2 secs",  "2 -> 2 secs");
  t.is (formatSeconds (59),              "59 secs", "59 -> 59 secs");
  t.is (formatSeconds (60),              "1 min",   "60 -> 1 min");
  t.is (formatSeconds (119),             "1 min",   "119 -> 1 min");
  t.is (formatSeconds (120),             "2 mins",  "120 -> 2 mins");
  t.is (formatSeconds (121),             "2 mins",  "121 -> 2 mins");
  t.is (formatSeconds (3599),            "59 mins", "3599 -> 59 mins");
  t.is (formatSeconds (3600),            "1 hr",    "3600 -> 1 hr");
  t.is (formatSeconds (3601),            "1 hr",    "3601 -> 1 hr");
  t.is (formatSeconds (86399),           "23 hrs",  "86399 -> 23 hrs");
  t.is (formatSeconds (86400),           "1 day",   "86400 -> 1 day");
  t.is (formatSeconds (86401),           "1 day",   "86401 -> 1 day");
  t.is (formatSeconds (14 * 86400 - 1),  "1 wk",    "14 days - 1 sec -> 1 wk");
  t.is (formatSeconds (14 * 86400),      "2 wks",   "14 days -> 2 wks");
  t.is (formatSeconds (14 * 86400 + 1),  "2 wks",   "14 days + 1 sec -> 2 wks");
  t.is (formatSeconds (85 * 86400 - 1),  "2 mths",  "85 days - 1 sec -> 2 mths");
  t.is (formatSeconds (85 * 86400),      "2 mths",  "85 days -> 2 mths");
  t.is (formatSeconds (85 * 86400 + 1),  "2 mths",  "85 days + 1 sec -> 2 mths");
  t.is (formatSeconds (365 * 86400 - 1), "11 mths", "365 days - 1 sec -> 11 mths");
  t.is (formatSeconds (365 * 86400),     "1.0 yrs", "365 days -> 1.0 yrs");
  t.is (formatSeconds (365 * 86400 + 1), "1.0 yrs", "365 days + 1 sec -> 1.0 yrs");

  // std::string formatSecondsCompact (time_t);
  t.is (formatSecondsCompact (0),               "-",    "0 -> -");
  t.is (formatSecondsCompact (1),               "1s",   "1 -> 1s");
  t.is (formatSecondsCompact (2),               "2s",   "2 -> 2s");
  t.is (formatSecondsCompact (59),              "59s",  "59 -> 59s");
  t.is (formatSecondsCompact (60),              "1m",   "60 -> 1m");
  t.is (formatSecondsCompact (119),             "1m",   "119 -> 1m");
  t.is (formatSecondsCompact (120),             "2m",   "120 -> 2m");
  t.is (formatSecondsCompact (121),             "2m",   "121 -> 2m");
  t.is (formatSecondsCompact (3599),            "59m",  "3599 -> 59m");
  t.is (formatSecondsCompact (3600),            "1h",   "3600 -> 1h");
  t.is (formatSecondsCompact (3601),            "1h",   "3601 -> 1h");
  t.is (formatSecondsCompact (86399),           "23h",  "86399 -> 23h");
  t.is (formatSecondsCompact (86400),           "1d",   "86400 -> 1d");
  t.is (formatSecondsCompact (86401),           "1d",   "86401 -> 1d");
  t.is (formatSecondsCompact (14 * 86400 - 1),  "1wk",  "14 days - 1 sec -> 1wk");
  t.is (formatSecondsCompact (14 * 86400),      "2wk",  "14 days -> 2wk");
  t.is (formatSecondsCompact (14 * 86400 + 1),  "2wk",  "14 days + 1 sec -> 2wk");
  t.is (formatSecondsCompact (85 * 86400 - 1),  "2mo",  "85 days - 1 sec -> 2mo");
  t.is (formatSecondsCompact (85 * 86400),      "2mo",  "85 days -> 2mo");
  t.is (formatSecondsCompact (85 * 86400 + 1),  "2mo",  "85 days + 1 sec -> 2mo");
  t.is (formatSecondsCompact (365 * 86400 - 1), "11mo", "365 days - 1 sec -> 11mo");
  t.is (formatSecondsCompact (365 * 86400),     "1.0y", "365 days -> 1.0y");
  t.is (formatSecondsCompact (365 * 86400 + 1), "1.0y", "365 days + 1 sec -> 1.0y");

  // Iterate for a whole year.  Why?  Just to see where the boundaries are,
  // so that changes can be made with some reference point.
  t.is (formatSecondsCompact (  1*86400), "1d", "1*86400 -> 1d");
  t.is (formatSecondsCompact (  2*86400), "2d", "2*86400 -> 2d");
  t.is (formatSecondsCompact (  3*86400), "3d", "3*86400 -> 3d");
  t.is (formatSecondsCompact (  4*86400), "4d", "4*86400 -> 4d");
  t.is (formatSecondsCompact (  5*86400), "5d", "5*86400 -> 5d");
  t.is (formatSecondsCompact (  6*86400), "6d", "6*86400 -> 6d");
  t.is (formatSecondsCompact (  7*86400), "7d", "7*86400 -> 7d");
  t.is (formatSecondsCompact (  8*86400), "8d", "8*86400 -> 8d");
  t.is (formatSecondsCompact (  9*86400), "9d", "9*86400 -> 9d");
  t.is (formatSecondsCompact ( 10*86400), "10d", "10*86400 -> 10d");

  t.is (formatSecondsCompact ( 11*86400), "11d", "11*86400 -> 11d");
  t.is (formatSecondsCompact ( 12*86400), "12d", "12*86400 -> 12d");
  t.is (formatSecondsCompact ( 13*86400), "1wk", "13*86400 -> 1wk");
  t.is (formatSecondsCompact ( 14*86400), "2wk", "14*86400 -> 2wk");
  t.is (formatSecondsCompact ( 15*86400), "2wk", "15*86400 -> 2wk");
  t.is (formatSecondsCompact ( 16*86400), "2wk", "16*86400 -> 2wk");
  t.is (formatSecondsCompact ( 17*86400), "2wk", "17*86400 -> 2wk");
  t.is (formatSecondsCompact ( 18*86400), "2wk", "18*86400 -> 2wk");
  t.is (formatSecondsCompact ( 19*86400), "2wk", "19*86400 -> 2wk");
  t.is (formatSecondsCompact ( 20*86400), "2wk", "20*86400 -> 2wk");

  t.is (formatSecondsCompact ( 21*86400), "3wk", "21*86400 -> 3wk");
  t.is (formatSecondsCompact ( 22*86400), "3wk", "22*86400 -> 3wk");
  t.is (formatSecondsCompact ( 23*86400), "3wk", "23*86400 -> 3wk");
  t.is (formatSecondsCompact ( 24*86400), "3wk", "24*86400 -> 3wk");
  t.is (formatSecondsCompact ( 25*86400), "3wk", "25*86400 -> 3wk");
  t.is (formatSecondsCompact ( 26*86400), "3wk", "26*86400 -> 3wk");
  t.is (formatSecondsCompact ( 27*86400), "3wk", "27*86400 -> 3wk");
  t.is (formatSecondsCompact ( 28*86400), "4wk", "28*86400 -> 4wk");
  t.is (formatSecondsCompact ( 29*86400), "4wk", "29*86400 -> 4wk");
  t.is (formatSecondsCompact ( 30*86400), "4wk", "30*86400 -> 4wk");

  t.is (formatSecondsCompact ( 31*86400), "4wk", "31*86400 -> 4wk");
  t.is (formatSecondsCompact ( 32*86400), "4wk", "32*86400 -> 4wk");
  t.is (formatSecondsCompact ( 33*86400), "4wk", "33*86400 -> 4wk");
  t.is (formatSecondsCompact ( 34*86400), "4wk", "34*86400 -> 4wk");
  t.is (formatSecondsCompact ( 35*86400), "5wk", "35*86400 -> 5wk");
  t.is (formatSecondsCompact ( 36*86400), "5wk", "36*86400 -> 5wk");
  t.is (formatSecondsCompact ( 37*86400), "5wk", "37*86400 -> 5wk");
  t.is (formatSecondsCompact ( 38*86400), "5wk", "38*86400 -> 5wk");
  t.is (formatSecondsCompact ( 39*86400), "5wk", "39*86400 -> 5wk");
  t.is (formatSecondsCompact ( 40*86400), "5wk", "40*86400 -> 5wk");

  t.is (formatSecondsCompact ( 41*86400), "5wk", "41*86400 -> 5wk");
  t.is (formatSecondsCompact ( 42*86400), "6wk", "42*86400 -> 6wk");
  t.is (formatSecondsCompact ( 43*86400), "6wk", "43*86400 -> 6wk");
  t.is (formatSecondsCompact ( 44*86400), "6wk", "44*86400 -> 6wk");
  t.is (formatSecondsCompact ( 45*86400), "6wk", "45*86400 -> 6wk");
  t.is (formatSecondsCompact ( 46*86400), "6wk", "46*86400 -> 6wk");
  t.is (formatSecondsCompact ( 47*86400), "6wk", "47*86400 -> 6wk");
  t.is (formatSecondsCompact ( 48*86400), "6wk", "48*86400 -> 6wk");
  t.is (formatSecondsCompact ( 49*86400), "7wk", "49*86400 -> 7wk");
  t.is (formatSecondsCompact ( 50*86400), "7wk", "50*86400 -> 7wk");

  t.is (formatSecondsCompact ( 51*86400), "7wk", "51*86400 -> 7wk");
  t.is (formatSecondsCompact ( 52*86400), "7wk", "52*86400 -> 7wk");
  t.is (formatSecondsCompact ( 53*86400), "7wk", "53*86400 -> 7wk");
  t.is (formatSecondsCompact ( 54*86400), "7wk", "54*86400 -> 7wk");
  t.is (formatSecondsCompact ( 55*86400), "7wk", "55*86400 -> 7wk");
  t.is (formatSecondsCompact ( 56*86400), "8wk", "56*86400 -> 8wk");
  t.is (formatSecondsCompact ( 57*86400), "8wk", "57*86400 -> 8wk");
  t.is (formatSecondsCompact ( 58*86400), "8wk", "58*86400 -> 8wk");
  t.is (formatSecondsCompact ( 59*86400), "8wk", "59*86400 -> 8wk");
  t.is (formatSecondsCompact ( 60*86400), "8wk", "60*86400 -> 8wk");

  t.is (formatSecondsCompact ( 61*86400), "8wk", "61*86400 -> 8wk");
  t.is (formatSecondsCompact ( 62*86400), "8wk", "62*86400 -> 8wk");
  t.is (formatSecondsCompact ( 63*86400), "9wk", "63*86400 -> 9wk");
  t.is (formatSecondsCompact ( 64*86400), "9wk", "64*86400 -> 9wk");
  t.is (formatSecondsCompact ( 65*86400), "9wk", "65*86400 -> 9wk");
  t.is (formatSecondsCompact ( 66*86400), "9wk", "66*86400 -> 9wk");
  t.is (formatSecondsCompact ( 67*86400), "9wk", "67*86400 -> 9wk");
  t.is (formatSecondsCompact ( 68*86400), "9wk", "68*86400 -> 9wk");
  t.is (formatSecondsCompact ( 69*86400), "9wk", "69*86400 -> 9wk");
  t.is (formatSecondsCompact ( 70*86400), "10wk", "70*86400 -> 10wk");

  t.is (formatSecondsCompact ( 71*86400), "10wk", "71*86400 -> 10wk");
  t.is (formatSecondsCompact ( 72*86400), "10wk", "72*86400 -> 10wk");
  t.is (formatSecondsCompact ( 73*86400), "10wk", "73*86400 -> 10wk");
  t.is (formatSecondsCompact ( 74*86400), "10wk", "74*86400 -> 10wk");
  t.is (formatSecondsCompact ( 75*86400), "10wk", "75*86400 -> 10wk");
  t.is (formatSecondsCompact ( 76*86400), "10wk", "76*86400 -> 10wk");
  t.is (formatSecondsCompact ( 77*86400), "11wk", "77*86400 -> 11wk");
  t.is (formatSecondsCompact ( 78*86400), "11wk", "78*86400 -> 11wk");
  t.is (formatSecondsCompact ( 79*86400), "11wk", "79*86400 -> 11wk");
  t.is (formatSecondsCompact ( 80*86400), "11wk", "80*86400 -> 11wk");

  t.is (formatSecondsCompact ( 81*86400), "11wk", "81*86400 -> 11wk");
  t.is (formatSecondsCompact ( 82*86400), "11wk", "82*86400 -> 11wk");
  t.is (formatSecondsCompact ( 83*86400), "11wk", "83*86400 -> 11wk");
  t.is (formatSecondsCompact ( 84*86400), "2mo", "84*86400 -> 2mo");
  t.is (formatSecondsCompact ( 85*86400), "2mo", "85*86400 -> 2mo");
  t.is (formatSecondsCompact ( 86*86400), "2mo", "86*86400 -> 2mo");
  t.is (formatSecondsCompact ( 87*86400), "2mo", "87*86400 -> 2mo");
  t.is (formatSecondsCompact ( 88*86400), "2mo", "88*86400 -> 2mo");
  t.is (formatSecondsCompact ( 89*86400), "2mo", "89*86400 -> 2mo");
  t.is (formatSecondsCompact ( 90*86400), "2mo", "90*86400 -> 2mo");

  t.is (formatSecondsCompact ( 91*86400), "2mo", "91*86400 -> 2mo");
  t.is (formatSecondsCompact ( 92*86400), "3mo", "92*86400 -> 3mo");
  t.is (formatSecondsCompact ( 93*86400), "3mo", "93*86400 -> 3mo");
  t.is (formatSecondsCompact ( 94*86400), "3mo", "94*86400 -> 3mo");
  t.is (formatSecondsCompact ( 95*86400), "3mo", "95*86400 -> 3mo");
  t.is (formatSecondsCompact ( 96*86400), "3mo", "96*86400 -> 3mo");
  t.is (formatSecondsCompact ( 97*86400), "3mo", "97*86400 -> 3mo");
  t.is (formatSecondsCompact ( 98*86400), "3mo", "98*86400 -> 3mo");
  t.is (formatSecondsCompact ( 99*86400), "3mo", "99*86400 -> 3mo");
  t.is (formatSecondsCompact (100*86400), "3mo", "100*86400 -> 3mo");

  t.is (formatSecondsCompact (101*86400), "3mo", "101*86400 -> 3mo");
  t.is (formatSecondsCompact (102*86400), "3mo", "102*86400 -> 3mo");
  t.is (formatSecondsCompact (103*86400), "3mo", "103*86400 -> 3mo");
  t.is (formatSecondsCompact (104*86400), "3mo", "104*86400 -> 3mo");
  t.is (formatSecondsCompact (105*86400), "3mo", "105*86400 -> 3mo");
  t.is (formatSecondsCompact (106*86400), "3mo", "106*86400 -> 3mo");
  t.is (formatSecondsCompact (107*86400), "3mo", "107*86400 -> 3mo");
  t.is (formatSecondsCompact (108*86400), "3mo", "108*86400 -> 3mo");
  t.is (formatSecondsCompact (109*86400), "3mo", "109*86400 -> 3mo");
  t.is (formatSecondsCompact (110*86400), "3mo", "110*86400 -> 3mo");

  t.is (formatSecondsCompact (111*86400), "3mo", "111*86400 -> 3mo");
  t.is (formatSecondsCompact (112*86400), "3mo", "112*86400 -> 3mo");
  t.is (formatSecondsCompact (113*86400), "3mo", "113*86400 -> 3mo");
  t.is (formatSecondsCompact (114*86400), "3mo", "114*86400 -> 3mo");
  t.is (formatSecondsCompact (115*86400), "3mo", "115*86400 -> 3mo");
  t.is (formatSecondsCompact (116*86400), "3mo", "116*86400 -> 3mo");
  t.is (formatSecondsCompact (117*86400), "3mo", "117*86400 -> 3mo");
  t.is (formatSecondsCompact (118*86400), "3mo", "118*86400 -> 3mo");
  t.is (formatSecondsCompact (119*86400), "3mo", "119*86400 -> 3mo");
  t.is (formatSecondsCompact (120*86400), "3mo", "120*86400 -> 3mo");

  t.is (formatSecondsCompact (121*86400), "3mo", "121*86400 -> 3mo");
  t.is (formatSecondsCompact (122*86400), "3mo", "122*86400 -> 3mo");
  t.is (formatSecondsCompact (123*86400), "4mo", "123*86400 -> 4mo");
  t.is (formatSecondsCompact (124*86400), "4mo", "124*86400 -> 4mo");
  t.is (formatSecondsCompact (125*86400), "4mo", "125*86400 -> 4mo");
  t.is (formatSecondsCompact (126*86400), "4mo", "126*86400 -> 4mo");
  t.is (formatSecondsCompact (127*86400), "4mo", "127*86400 -> 4mo");
  t.is (formatSecondsCompact (128*86400), "4mo", "128*86400 -> 4mo");
  t.is (formatSecondsCompact (129*86400), "4mo", "129*86400 -> 4mo");
  t.is (formatSecondsCompact (130*86400), "4mo", "130*86400 -> 4mo");

  t.is (formatSecondsCompact (131*86400), "4mo", "131*86400 -> 4mo");
  t.is (formatSecondsCompact (132*86400), "4mo", "132*86400 -> 4mo");
  t.is (formatSecondsCompact (133*86400), "4mo", "133*86400 -> 4mo");
  t.is (formatSecondsCompact (134*86400), "4mo", "134*86400 -> 4mo");
  t.is (formatSecondsCompact (135*86400), "4mo", "135*86400 -> 4mo");
  t.is (formatSecondsCompact (136*86400), "4mo", "136*86400 -> 4mo");
  t.is (formatSecondsCompact (137*86400), "4mo", "137*86400 -> 4mo");
  t.is (formatSecondsCompact (138*86400), "4mo", "138*86400 -> 4mo");
  t.is (formatSecondsCompact (139*86400), "4mo", "139*86400 -> 4mo");
  t.is (formatSecondsCompact (140*86400), "4mo", "140*86400 -> 4mo");

  t.is (formatSecondsCompact (141*86400), "4mo", "141*86400 -> 4mo");
  t.is (formatSecondsCompact (142*86400), "4mo", "142*86400 -> 4mo");
  t.is (formatSecondsCompact (143*86400), "4mo", "143*86400 -> 4mo");
  t.is (formatSecondsCompact (144*86400), "4mo", "144*86400 -> 4mo");
  t.is (formatSecondsCompact (145*86400), "4mo", "145*86400 -> 4mo");
  t.is (formatSecondsCompact (146*86400), "4mo", "146*86400 -> 4mo");
  t.is (formatSecondsCompact (147*86400), "4mo", "147*86400 -> 4mo");
  t.is (formatSecondsCompact (148*86400), "4mo", "148*86400 -> 4mo");
  t.is (formatSecondsCompact (149*86400), "4mo", "149*86400 -> 4mo");
  t.is (formatSecondsCompact (150*86400), "4mo", "150*86400 -> 4mo");

  t.is (formatSecondsCompact (151*86400), "4mo", "151*86400 -> 4mo");
  t.is (formatSecondsCompact (152*86400), "4mo", "152*86400 -> 4mo");
  t.is (formatSecondsCompact (153*86400), "5mo", "153*86400 -> 5mo");
  t.is (formatSecondsCompact (154*86400), "5mo", "154*86400 -> 5mo");
  t.is (formatSecondsCompact (155*86400), "5mo", "155*86400 -> 5mo");
  t.is (formatSecondsCompact (156*86400), "5mo", "156*86400 -> 5mo");
  t.is (formatSecondsCompact (157*86400), "5mo", "157*86400 -> 5mo");
  t.is (formatSecondsCompact (158*86400), "5mo", "158*86400 -> 5mo");
  t.is (formatSecondsCompact (159*86400), "5mo", "159*86400 -> 5mo");
  t.is (formatSecondsCompact (160*86400), "5mo", "160*86400 -> 5mo");

  t.is (formatSecondsCompact (161*86400), "5mo", "161*86400 -> 5mo");
  t.is (formatSecondsCompact (162*86400), "5mo", "162*86400 -> 5mo");
  t.is (formatSecondsCompact (163*86400), "5mo", "163*86400 -> 5mo");
  t.is (formatSecondsCompact (164*86400), "5mo", "164*86400 -> 5mo");
  t.is (formatSecondsCompact (165*86400), "5mo", "165*86400 -> 5mo");
  t.is (formatSecondsCompact (166*86400), "5mo", "166*86400 -> 5mo");
  t.is (formatSecondsCompact (167*86400), "5mo", "167*86400 -> 5mo");
  t.is (formatSecondsCompact (168*86400), "5mo", "168*86400 -> 5mo");
  t.is (formatSecondsCompact (169*86400), "5mo", "169*86400 -> 5mo");
  t.is (formatSecondsCompact (170*86400), "5mo", "170*86400 -> 5mo");

  t.is (formatSecondsCompact (171*86400), "5mo", "171*86400 -> 5mo");
  t.is (formatSecondsCompact (172*86400), "5mo", "172*86400 -> 5mo");
  t.is (formatSecondsCompact (173*86400), "5mo", "173*86400 -> 5mo");
  t.is (formatSecondsCompact (174*86400), "5mo", "174*86400 -> 5mo");
  t.is (formatSecondsCompact (175*86400), "5mo", "175*86400 -> 5mo");
  t.is (formatSecondsCompact (176*86400), "5mo", "176*86400 -> 5mo");
  t.is (formatSecondsCompact (177*86400), "5mo", "177*86400 -> 5mo");
  t.is (formatSecondsCompact (178*86400), "5mo", "178*86400 -> 5mo");
  t.is (formatSecondsCompact (179*86400), "5mo", "179*86400 -> 5mo");
  t.is (formatSecondsCompact (180*86400), "5mo", "180*86400 -> 5mo");

  t.is (formatSecondsCompact (181*86400), "5mo", "181*86400 -> 5mo");
  t.is (formatSecondsCompact (182*86400), "5mo", "182*86400 -> 5mo");
  t.is (formatSecondsCompact (183*86400), "5mo", "183*86400 -> 5mo");
  t.is (formatSecondsCompact (184*86400), "6mo", "184*86400 -> 6mo");
  t.is (formatSecondsCompact (185*86400), "6mo", "185*86400 -> 6mo");
  t.is (formatSecondsCompact (186*86400), "6mo", "186*86400 -> 6mo");
  t.is (formatSecondsCompact (187*86400), "6mo", "187*86400 -> 6mo");
  t.is (formatSecondsCompact (188*86400), "6mo", "188*86400 -> 6mo");
  t.is (formatSecondsCompact (189*86400), "6mo", "189*86400 -> 6mo");
  t.is (formatSecondsCompact (190*86400), "6mo", "190*86400 -> 6mo");

  t.is (formatSecondsCompact (191*86400), "6mo", "191*86400 -> 6mo");
  t.is (formatSecondsCompact (192*86400), "6mo", "192*86400 -> 6mo");
  t.is (formatSecondsCompact (193*86400), "6mo", "193*86400 -> 6mo");
  t.is (formatSecondsCompact (194*86400), "6mo", "194*86400 -> 6mo");
  t.is (formatSecondsCompact (195*86400), "6mo", "195*86400 -> 6mo");
  t.is (formatSecondsCompact (196*86400), "6mo", "196*86400 -> 6mo");
  t.is (formatSecondsCompact (197*86400), "6mo", "197*86400 -> 6mo");
  t.is (formatSecondsCompact (198*86400), "6mo", "198*86400 -> 6mo");
  t.is (formatSecondsCompact (199*86400), "6mo", "199*86400 -> 6mo");
  t.is (formatSecondsCompact (200*86400), "6mo", "200*86400 -> 6mo");

  t.is (formatSecondsCompact (201*86400), "6mo", "201*86400 -> 6mo");
  t.is (formatSecondsCompact (202*86400), "6mo", "202*86400 -> 6mo");
  t.is (formatSecondsCompact (203*86400), "6mo", "203*86400 -> 6mo");
  t.is (formatSecondsCompact (204*86400), "6mo", "204*86400 -> 6mo");
  t.is (formatSecondsCompact (205*86400), "6mo", "205*86400 -> 6mo");
  t.is (formatSecondsCompact (206*86400), "6mo", "206*86400 -> 6mo");
  t.is (formatSecondsCompact (207*86400), "6mo", "207*86400 -> 6mo");
  t.is (formatSecondsCompact (208*86400), "6mo", "208*86400 -> 6mo");
  t.is (formatSecondsCompact (209*86400), "6mo", "209*86400 -> 6mo");
  t.is (formatSecondsCompact (210*86400), "6mo", "210*86400 -> 6mo");

  t.is (formatSecondsCompact (211*86400), "6mo", "211*86400 -> 6mo");
  t.is (formatSecondsCompact (212*86400), "6mo", "212*86400 -> 6mo");
  t.is (formatSecondsCompact (213*86400), "6mo", "213*86400 -> 6mo");
  t.is (formatSecondsCompact (214*86400), "6mo", "214*86400 -> 6mo");
  t.is (formatSecondsCompact (215*86400), "7mo", "215*86400 -> 7mo");
  t.is (formatSecondsCompact (216*86400), "7mo", "216*86400 -> 7mo");
  t.is (formatSecondsCompact (217*86400), "7mo", "217*86400 -> 7mo");
  t.is (formatSecondsCompact (218*86400), "7mo", "218*86400 -> 7mo");
  t.is (formatSecondsCompact (219*86400), "7mo", "219*86400 -> 7mo");
  t.is (formatSecondsCompact (220*86400), "7mo", "220*86400 -> 7mo");

  t.is (formatSecondsCompact (221*86400), "7mo", "221*86400 -> 7mo");
  t.is (formatSecondsCompact (222*86400), "7mo", "222*86400 -> 7mo");
  t.is (formatSecondsCompact (223*86400), "7mo", "223*86400 -> 7mo");
  t.is (formatSecondsCompact (224*86400), "7mo", "224*86400 -> 7mo");
  t.is (formatSecondsCompact (225*86400), "7mo", "225*86400 -> 7mo");
  t.is (formatSecondsCompact (226*86400), "7mo", "226*86400 -> 7mo");
  t.is (formatSecondsCompact (227*86400), "7mo", "227*86400 -> 7mo");
  t.is (formatSecondsCompact (228*86400), "7mo", "228*86400 -> 7mo");
  t.is (formatSecondsCompact (229*86400), "7mo", "229*86400 -> 7mo");
  t.is (formatSecondsCompact (230*86400), "7mo", "230*86400 -> 7mo");

  t.is (formatSecondsCompact (231*86400), "7mo", "231*86400 -> 7mo");
  t.is (formatSecondsCompact (232*86400), "7mo", "232*86400 -> 7mo");
  t.is (formatSecondsCompact (233*86400), "7mo", "233*86400 -> 7mo");
  t.is (formatSecondsCompact (234*86400), "7mo", "234*86400 -> 7mo");
  t.is (formatSecondsCompact (235*86400), "7mo", "235*86400 -> 7mo");
  t.is (formatSecondsCompact (236*86400), "7mo", "236*86400 -> 7mo");
  t.is (formatSecondsCompact (237*86400), "7mo", "237*86400 -> 7mo");
  t.is (formatSecondsCompact (238*86400), "7mo", "238*86400 -> 7mo");
  t.is (formatSecondsCompact (239*86400), "7mo", "239*86400 -> 7mo");
  t.is (formatSecondsCompact (240*86400), "7mo", "240*86400 -> 7mo");

  t.is (formatSecondsCompact (241*86400), "7mo", "241*86400 -> 7mo");
  t.is (formatSecondsCompact (242*86400), "7mo", "242*86400 -> 7mo");
  t.is (formatSecondsCompact (243*86400), "7mo", "243*86400 -> 7mo");
  t.is (formatSecondsCompact (244*86400), "7mo", "244*86400 -> 7mo");
  t.is (formatSecondsCompact (245*86400), "8mo", "245*86400 -> 8mo");
  t.is (formatSecondsCompact (246*86400), "8mo", "246*86400 -> 8mo");
  t.is (formatSecondsCompact (247*86400), "8mo", "247*86400 -> 8mo");
  t.is (formatSecondsCompact (248*86400), "8mo", "248*86400 -> 8mo");
  t.is (formatSecondsCompact (249*86400), "8mo", "249*86400 -> 8mo");
  t.is (formatSecondsCompact (250*86400), "8mo", "250*86400 -> 8mo");

  t.is (formatSecondsCompact (251*86400), "8mo", "251*86400 -> 8mo");
  t.is (formatSecondsCompact (252*86400), "8mo", "252*86400 -> 8mo");
  t.is (formatSecondsCompact (253*86400), "8mo", "253*86400 -> 8mo");
  t.is (formatSecondsCompact (254*86400), "8mo", "254*86400 -> 8mo");
  t.is (formatSecondsCompact (255*86400), "8mo", "255*86400 -> 8mo");
  t.is (formatSecondsCompact (256*86400), "8mo", "256*86400 -> 8mo");
  t.is (formatSecondsCompact (257*86400), "8mo", "257*86400 -> 8mo");
  t.is (formatSecondsCompact (258*86400), "8mo", "258*86400 -> 8mo");
  t.is (formatSecondsCompact (259*86400), "8mo", "259*86400 -> 8mo");
  t.is (formatSecondsCompact (260*86400), "8mo", "260*86400 -> 8mo");

  t.is (formatSecondsCompact (261*86400), "8mo", "261*86400 -> 8mo");
  t.is (formatSecondsCompact (262*86400), "8mo", "262*86400 -> 8mo");
  t.is (formatSecondsCompact (263*86400), "8mo", "263*86400 -> 8mo");
  t.is (formatSecondsCompact (264*86400), "8mo", "264*86400 -> 8mo");
  t.is (formatSecondsCompact (265*86400), "8mo", "265*86400 -> 8mo");
  t.is (formatSecondsCompact (266*86400), "8mo", "266*86400 -> 8mo");
  t.is (formatSecondsCompact (267*86400), "8mo", "267*86400 -> 8mo");
  t.is (formatSecondsCompact (268*86400), "8mo", "268*86400 -> 8mo");
  t.is (formatSecondsCompact (269*86400), "8mo", "269*86400 -> 8mo");
  t.is (formatSecondsCompact (270*86400), "8mo", "270*86400 -> 8mo");

  t.is (formatSecondsCompact (271*86400), "8mo", "271*86400 -> 8mo");
  t.is (formatSecondsCompact (272*86400), "8mo", "272*86400 -> 8mo");
  t.is (formatSecondsCompact (273*86400), "8mo", "273*86400 -> 8mo");
  t.is (formatSecondsCompact (274*86400), "8mo", "274*86400 -> 8mo");
  t.is (formatSecondsCompact (275*86400), "8mo", "275*86400 -> 8mo");
  t.is (formatSecondsCompact (276*86400), "9mo", "276*86400 -> 9mo");
  t.is (formatSecondsCompact (277*86400), "9mo", "277*86400 -> 9mo");
  t.is (formatSecondsCompact (278*86400), "9mo", "278*86400 -> 9mo");
  t.is (formatSecondsCompact (279*86400), "9mo", "279*86400 -> 9mo");
  t.is (formatSecondsCompact (280*86400), "9mo", "280*86400 -> 9mo");

  t.is (formatSecondsCompact (281*86400), "9mo", "281*86400 -> 9mo");
  t.is (formatSecondsCompact (282*86400), "9mo", "282*86400 -> 9mo");
  t.is (formatSecondsCompact (283*86400), "9mo", "283*86400 -> 9mo");
  t.is (formatSecondsCompact (284*86400), "9mo", "284*86400 -> 9mo");
  t.is (formatSecondsCompact (285*86400), "9mo", "285*86400 -> 9mo");
  t.is (formatSecondsCompact (286*86400), "9mo", "286*86400 -> 9mo");
  t.is (formatSecondsCompact (287*86400), "9mo", "287*86400 -> 9mo");
  t.is (formatSecondsCompact (288*86400), "9mo", "288*86400 -> 9mo");
  t.is (formatSecondsCompact (289*86400), "9mo", "289*86400 -> 9mo");
  t.is (formatSecondsCompact (290*86400), "9mo", "290*86400 -> 9mo");

  t.is (formatSecondsCompact (291*86400), "9mo", "291*86400 -> 9mo");
  t.is (formatSecondsCompact (292*86400), "9mo", "292*86400 -> 9mo");
  t.is (formatSecondsCompact (293*86400), "9mo", "293*86400 -> 9mo");
  t.is (formatSecondsCompact (294*86400), "9mo", "294*86400 -> 9mo");
  t.is (formatSecondsCompact (295*86400), "9mo", "295*86400 -> 9mo");
  t.is (formatSecondsCompact (296*86400), "9mo", "296*86400 -> 9mo");
  t.is (formatSecondsCompact (297*86400), "9mo", "297*86400 -> 9mo");
  t.is (formatSecondsCompact (298*86400), "9mo", "298*86400 -> 9mo");
  t.is (formatSecondsCompact (299*86400), "9mo", "299*86400 -> 9mo");
  t.is (formatSecondsCompact (300*86400), "9mo", "300*86400 -> 9mo");

  t.is (formatSecondsCompact (301*86400), "9mo", "301*86400 -> 9mo");
  t.is (formatSecondsCompact (302*86400), "9mo", "302*86400 -> 9mo");
  t.is (formatSecondsCompact (303*86400), "9mo", "303*86400 -> 9mo");
  t.is (formatSecondsCompact (304*86400), "9mo", "304*86400 -> 9mo");
  t.is (formatSecondsCompact (305*86400), "9mo", "305*86400 -> 9mo");
  t.is (formatSecondsCompact (306*86400), "10mo", "306*86400 -> 10mo");
  t.is (formatSecondsCompact (307*86400), "10mo", "307*86400 -> 10mo");
  t.is (formatSecondsCompact (308*86400), "10mo", "308*86400 -> 10mo");
  t.is (formatSecondsCompact (309*86400), "10mo", "309*86400 -> 10mo");
  t.is (formatSecondsCompact (310*86400), "10mo", "310*86400 -> 10mo");

  t.is (formatSecondsCompact (311*86400), "10mo", "311*86400 -> 10mo");
  t.is (formatSecondsCompact (312*86400), "10mo", "312*86400 -> 10mo");
  t.is (formatSecondsCompact (313*86400), "10mo", "313*86400 -> 10mo");
  t.is (formatSecondsCompact (314*86400), "10mo", "314*86400 -> 10mo");
  t.is (formatSecondsCompact (315*86400), "10mo", "315*86400 -> 10mo");
  t.is (formatSecondsCompact (316*86400), "10mo", "316*86400 -> 10mo");
  t.is (formatSecondsCompact (317*86400), "10mo", "317*86400 -> 10mo");
  t.is (formatSecondsCompact (318*86400), "10mo", "318*86400 -> 10mo");
  t.is (formatSecondsCompact (319*86400), "10mo", "319*86400 -> 10mo");
  t.is (formatSecondsCompact (320*86400), "10mo", "320*86400 -> 10mo");

  t.is (formatSecondsCompact (321*86400), "10mo", "321*86400 -> 10mo");
  t.is (formatSecondsCompact (322*86400), "10mo", "322*86400 -> 10mo");
  t.is (formatSecondsCompact (323*86400), "10mo", "323*86400 -> 10mo");
  t.is (formatSecondsCompact (324*86400), "10mo", "324*86400 -> 10mo");
  t.is (formatSecondsCompact (325*86400), "10mo", "325*86400 -> 10mo");
  t.is (formatSecondsCompact (326*86400), "10mo", "326*86400 -> 10mo");
  t.is (formatSecondsCompact (327*86400), "10mo", "327*86400 -> 10mo");
  t.is (formatSecondsCompact (328*86400), "10mo", "328*86400 -> 10mo");
  t.is (formatSecondsCompact (329*86400), "10mo", "329*86400 -> 10mo");
  t.is (formatSecondsCompact (330*86400), "10mo", "330*86400 -> 10mo");

  t.is (formatSecondsCompact (331*86400), "10mo", "331*86400 -> 10mo");
  t.is (formatSecondsCompact (332*86400), "10mo", "332*86400 -> 10mo");
  t.is (formatSecondsCompact (333*86400), "10mo", "333*86400 -> 10mo");
  t.is (formatSecondsCompact (334*86400), "10mo", "334*86400 -> 10mo");
  t.is (formatSecondsCompact (335*86400), "10mo", "335*86400 -> 10mo");
  t.is (formatSecondsCompact (336*86400), "10mo", "336*86400 -> 10mo");
  t.is (formatSecondsCompact (337*86400), "11mo", "337*86400 -> 11mo");
  t.is (formatSecondsCompact (338*86400), "11mo", "338*86400 -> 11mo");
  t.is (formatSecondsCompact (339*86400), "11mo", "339*86400 -> 11mo");
  t.is (formatSecondsCompact (340*86400), "11mo", "340*86400 -> 11mo");

  t.is (formatSecondsCompact (341*86400), "11mo", "341*86400 -> 11mo");
  t.is (formatSecondsCompact (342*86400), "11mo", "342*86400 -> 11mo");
  t.is (formatSecondsCompact (343*86400), "11mo", "343*86400 -> 11mo");
  t.is (formatSecondsCompact (344*86400), "11mo", "344*86400 -> 11mo");
  t.is (formatSecondsCompact (345*86400), "11mo", "345*86400 -> 11mo");
  t.is (formatSecondsCompact (346*86400), "11mo", "346*86400 -> 11mo");
  t.is (formatSecondsCompact (347*86400), "11mo", "347*86400 -> 11mo");
  t.is (formatSecondsCompact (348*86400), "11mo", "348*86400 -> 11mo");
  t.is (formatSecondsCompact (349*86400), "11mo", "349*86400 -> 11mo");
  t.is (formatSecondsCompact (350*86400), "11mo", "350*86400 -> 11mo");

  t.is (formatSecondsCompact (351*86400), "11mo", "351*86400 -> 11mo");
  t.is (formatSecondsCompact (352*86400), "11mo", "352*86400 -> 11mo");
  t.is (formatSecondsCompact (353*86400), "11mo", "353*86400 -> 11mo");
  t.is (formatSecondsCompact (354*86400), "11mo", "354*86400 -> 11mo");
  t.is (formatSecondsCompact (355*86400), "11mo", "355*86400 -> 11mo");
  t.is (formatSecondsCompact (356*86400), "11mo", "356*86400 -> 11mo");
  t.is (formatSecondsCompact (357*86400), "11mo", "357*86400 -> 11mo");
  t.is (formatSecondsCompact (358*86400), "11mo", "358*86400 -> 11mo");
  t.is (formatSecondsCompact (359*86400), "11mo", "359*86400 -> 11mo");
  t.is (formatSecondsCompact (360*86400), "11mo", "360*86400 -> 11mo");

  t.is (formatSecondsCompact (361*86400), "11mo", "361*86400 -> 11mo");
  t.is (formatSecondsCompact (362*86400), "11mo", "362*86400 -> 11mo");
  t.is (formatSecondsCompact (363*86400), "11mo", "363*86400 -> 11mo");
  t.is (formatSecondsCompact (364*86400), "11mo", "364*86400 -> 11mo");
  t.is (formatSecondsCompact (365*86400), "1.0y", "365*86400 -> 1.0y");

  // std::string formatBytes (size_t);
  t.is (formatBytes (0), "0 B", "0 -> 0 B");

  t.is (formatBytes (994),  "994 B", "994 -> 994 B");
  t.is (formatBytes (995),  "1.0 KiB", "995 -> 1.0 KiB");
  t.is (formatBytes (999),  "1.0 KiB", "999 -> 1.0 KiB");
  t.is (formatBytes (1000), "1.0 KiB", "1000 -> 1.0 KiB");
  t.is (formatBytes (1001), "1.0 KiB", "1001 -> 1.0 KiB");

  t.is (formatBytes (999999),  "1.0 MiB", "999999 -> 1.0 MiB");
  t.is (formatBytes (1000000), "1.0 MiB", "1000000 -> 1.0 MiB");
  t.is (formatBytes (1000001), "1.0 MiB", "1000001 -> 1.0 MiB");

  t.is (formatBytes (999999999),  "1.0 GiB", "999999999 -> 1.0 GiB");
  t.is (formatBytes (1000000000), "1.0 GiB", "1000000000 -> 1.0 GiB");
  t.is (formatBytes (1000000001), "1.0 GiB", "1000000001 -> 1.0 GiB");

  // TODO const std::string uuid ();

  // std::string taskDiff (const Task&, const Task&);
  Task left;
  left.set ("zero", "0");
  left.set ("one",  1);
  left.set ("two",  2);

  Task right;
  right.set ("zero",  "00");
  right.set ("two",   2);
  right.set ("three", 3);

  Task rightAgain (right);

  std::string output = taskDifferences (left, right);
  t.ok (taskDiff (left, right),                                                     "Detected changes");
  t.ok (output.find ("zero will be changed from '0' to '00'") != std::string::npos, "Detected change zero:0 -> zero:00");
  t.ok (output.find ("one will be deleted")                   != std::string::npos, "Detected deletion one:1 ->");
  t.ok (output.find ("two")                                   == std::string::npos, "Detected no change two:2 -> two:2");
  t.ok (output.find ("three will be set to '3'")              != std::string::npos, "Detected addition -> three:3");

  t.notok (taskDiff (right, rightAgain),                                            "No changes detected");
  output = taskDifferences (right, rightAgain);
  t.ok (output.find ("No changes will be made")               != std::string::npos, "No changes detected");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

