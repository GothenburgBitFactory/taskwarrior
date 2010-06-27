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
  try { Duration d (input); return (int) d; }
  catch (...) {}
  return 0;
}

int main (int argc, char** argv)
{
  UnitTest t (148);

  Duration d;

  t.ok (d.valid ("daily"),      "valid duration daily = 1");
  t.ok (d.valid ("day"),        "valid duration day = 1");
  t.ok (d.valid ("weekly"),     "valid duration weekly = 7");
  t.ok (d.valid ("weekdays"),   "valid duration weekdays = 1");
  t.ok (d.valid ("sennight"),   "valid duration sennight = 7");
  t.ok (d.valid ("biweekly"),   "valid duration biweekly = 14");
  t.ok (d.valid ("fortnight"),  "valid duration fortnight = 14");
  t.ok (d.valid ("monthly"),    "valid duration monthly = 30");
  t.ok (d.valid ("bimonthly"),  "valid duration bimonthly = 61");
  t.ok (d.valid ("quarterly"),  "valid duration quarterly = 91");
  t.ok (d.valid ("annual"),     "valid duration annual = 365");
  t.ok (d.valid ("yearly"),     "valid duration yearly = 365");
  t.ok (d.valid ("semiannual"), "valid duration semiannual = 183");
  t.ok (d.valid ("biannual"),   "valid duration biannual = 730");
  t.ok (d.valid ("biyearly"),   "valid duration biyearly = 730");

  t.ok (d.valid ("0 yrs"),      "valid duration 0 yrs = 0");
  t.ok (d.valid ("0 yr"),       "valid duration 0 yr = 0");
  t.ok (d.valid ("0y"),         "valid duration 0y = 0");
  t.ok (d.valid ("1 yrs"),      "valid duration 1 yrs = 365");
  t.ok (d.valid ("1 yr"),       "valid duration 1 yr = 365");
  t.ok (d.valid ("1y"),         "valid duration 1y = 365");
  t.ok (d.valid ("10 yrs"),     "valid duration 10 yrs = 3650");
  t.ok (d.valid ("10 yr"),      "valid duration 10 yr = 3650");
  t.ok (d.valid ("10y"),        "valid duration 10y = 3650");

  t.ok (d.valid ("0 qtrs"),     "valid duration 0 qtrs = 0");
  t.ok (d.valid ("0 qtr"),      "valid duration 0 qtr = 0");
  t.ok (d.valid ("0q"),         "valid duration 0q = 0");
  t.ok (d.valid ("1 qtrs"),     "valid duration 1 qtrs = 91");
  t.ok (d.valid ("1 qtr"),      "valid duration 1 qtr = 91");
  t.ok (d.valid ("1q"),         "valid duration 1q = 91");
  t.ok (d.valid ("10 qtrs"),    "valid duration 10 qtrs = 910");
  t.ok (d.valid ("10 qtr"),     "valid duration 10 qtr = 910");
  t.ok (d.valid ("10q"),        "valid duration 10q = 910");

  t.ok (d.valid ("0 mths"),     "valid duration 0 mths = 0");
  t.ok (d.valid ("0 mth"),      "valid duration 0 mth = 0");
  t.ok (d.valid ("0mo"),        "valid duration 0mo = 0");
  t.ok (d.valid ("1 mths"),     "valid duration 1 mths = 30");
  t.ok (d.valid ("1 mth"),      "valid duration 1 mth = 30");
  t.ok (d.valid ("1mo"),        "valid duration 1mo = 30");
  t.ok (d.valid ("10 mths"),    "valid duration 10 mths = 300");
  t.ok (d.valid ("10 mth"),     "valid duration 10 mth = 300");
  t.ok (d.valid ("10mo"),       "valid duration 10mo = 300");

  t.ok (d.valid ("0 wks"),      "valid duration 0 wks = 0");
  t.ok (d.valid ("0 wk"),       "valid duration 0 wk = 0");
  t.ok (d.valid ("0w"),         "valid duration 0w = 0");
  t.ok (d.valid ("1 wks"),      "valid duration 1 wks = 7");
  t.ok (d.valid ("1 wk"),       "valid duration 1 wk = 7");
  t.ok (d.valid ("1w"),         "valid duration 1w = 7");
  t.ok (d.valid ("10 wks"),     "valid duration 10 wks = 70");
  t.ok (d.valid ("10 wk"),      "valid duration 10 wk = 70");
  t.ok (d.valid ("10w"),        "valid duration 10w = 70");

  t.ok (d.valid ("0 days"),     "valid duration 0 days = 0");
  t.ok (d.valid ("0 day"),      "valid duration 0 day = 0");
  t.ok (d.valid ("0d"),         "valid duration 0d = 0");
  t.ok (d.valid ("1 days"),     "valid duration 1 days = 1");
  t.ok (d.valid ("1 day"),      "valid duration 1 day = 1");
  t.ok (d.valid ("1d"),         "valid duration 1d = 1");
  t.ok (d.valid ("10 days"),    "valid duration 10 days = 10");
  t.ok (d.valid ("10 day"),     "valid duration 10 day = 10");
  t.ok (d.valid ("10d"),        "valid duration 10d = 10");

  t.ok (d.valid ("0 hrs"),      "valid duration 0 hrs = 0");
  t.ok (d.valid ("0 hr"),       "valid duration 0 hr = 0");
  t.ok (d.valid ("0h"),         "valid duration 0h = 0");
  t.ok (d.valid ("1 hrs"),      "valid duration 1 hrs = 0");
  t.ok (d.valid ("1 hr"),       "valid duration 1 hr = 0");
  t.ok (d.valid ("1h"),         "valid duration 1h = 0");
  t.ok (d.valid ("10 hrs"),     "valid duration 10 hrs = 0");
  t.ok (d.valid ("10 hr"),      "valid duration 10 hr = 0");
  t.ok (d.valid ("10h"),        "valid duration 10h = 0");

  t.ok (d.valid ("0 mins"),     "valid duration 0 mins = 0");
  t.ok (d.valid ("0 min"),      "valid duration 0 min");
  t.ok (d.valid ("0m"),         "valid duration 0m = 0");
  t.ok (d.valid ("1 mins"),     "valid duration 1 mins = 0");
  t.ok (d.valid ("1 min"),      "valid duration 1 min = 0");
  t.ok (d.valid ("1m"),         "valid duration 1m = 0");
  t.ok (d.valid ("10 mins"),    "valid duration 10 mins = 0");
  t.ok (d.valid ("10 min"),     "valid duration 10 min = 0");
  t.ok (d.valid ("10m"),        "valid duration 10m = 0");

  t.ok (d.valid ("0 secs"),     "valid duration 0 secs = 0");
  t.ok (d.valid ("0 sec"),      "valid duration 0 sec = 0");
  t.ok (d.valid ("0s"),         "valid duration 0s = 0");
  t.ok (d.valid ("1 secs"),     "valid duration 1 secs = 0");
  t.ok (d.valid ("1 sec"),      "valid duration 1 sec = 0");
  t.ok (d.valid ("1s"),         "valid duration 1s = 0");
  t.ok (d.valid ("10 secs"),    "valid duration 10 secs = 0");
  t.ok (d.valid ("10 sec"),     "valid duration 10 sec = 0");
  t.ok (d.valid ("10s"),        "valid duration 10s = 0");

  t.notok (d.valid ("woof"),   "valid duration woof = fail");

  t.is (convertDuration ("daily"),         1, "valid duration daily = 1");
  t.is (convertDuration ("day"),           1, "valid duration day = 1");
  t.is (convertDuration ("weekly"),        7, "valid duration weekly = 7");
  t.is (convertDuration ("weekdays"),      1, "valid duration weekdays = 1");
  t.is (convertDuration ("sennight"),      7, "valid duration sennight = 7");
  t.is (convertDuration ("biweekly"),     14, "valid duration biweekly = 14");
  t.is (convertDuration ("fortnight"),    14, "valid duration fortnight = 14");
  t.is (convertDuration ("monthly"),      30, "valid duration monthly = 30");
  t.is (convertDuration ("bimonthly"),    61, "valid duration bimonthly = 61");
  t.is (convertDuration ("quarterly"),    91, "valid duration quarterly = 91");
  t.is (convertDuration ("annual"),      365, "valid duration annual = 365");
  t.is (convertDuration ("yearly"),      365, "valid duration yearly = 365");
  t.is (convertDuration ("semiannual"),  183, "valid duration semiannual = 183");
  t.is (convertDuration ("biannual"),    730, "valid duration biannual = 730");
  t.is (convertDuration ("biyearly"),    730, "valid duration biyearly = 730");

  t.is (convertDuration ("0 yrs"),         0, "valid duration 0 yrs = 0");
  t.is (convertDuration ("0 yr"),          0, "valid duration 0 yr = 0");
  t.is (convertDuration ("0y"),            0, "valid duration 0y = 0");
  t.is (convertDuration ("1 yrs"),       365, "valid duration 1 yrs = 365");
  t.is (convertDuration ("1 yr"),        365, "valid duration 1 yr = 365");
  t.is (convertDuration ("1y"),          365, "valid duration 1y = 365");
  t.is (convertDuration ("10 yrs"),     3650, "valid duration 10 yrs = 3650");
  t.is (convertDuration ("10 yr"),      3650, "valid duration 10 yr = 3650");
  t.is (convertDuration ("10y"),        3650, "valid duration 10y = 3650");

  t.is (convertDuration ("0 qtrs"),        0, "valid duration 0 qtrs = 0");
  t.is (convertDuration ("0 qtr"),         0, "valid duration 0 qtr = 0");
  t.is (convertDuration ("0q"),            0, "valid duration 0q = 0");
  t.is (convertDuration ("1 qtrs"),       91, "valid duration 1 qtrs = 91");
  t.is (convertDuration ("1 qtr"),        91, "valid duration 1 qtr = 91");
  t.is (convertDuration ("1q"),           91, "valid duration 1q = 91");
  t.is (convertDuration ("10 qtrs"),     910, "valid duration 10 qtrs = 910");
  t.is (convertDuration ("10 qtr"),      910, "valid duration 10 qtr = 910");
  t.is (convertDuration ("10q"),         910, "valid duration 10q = 910");

  t.is (convertDuration ("0 mths"),        0, "valid duration 0 mths = 0");
  t.is (convertDuration ("0 mth"),         0, "valid duration 0 mth = 0");
  t.is (convertDuration ("0mo"),           0, "valid duration 0mo = 0");
  t.is (convertDuration ("1 mths"),       30, "valid duration 1 mths = 30");
  t.is (convertDuration ("1 mth"),        30, "valid duration 1 mth = 30");
  t.is (convertDuration ("1mo"),          30, "valid duration 1mo = 30");
  t.is (convertDuration ("10 mths"),     300, "valid duration 10 mths = 300");
  t.is (convertDuration ("10 mth"),      300, "valid duration 10 mth = 300");
  t.is (convertDuration ("10mo"),        300, "valid duration 10mo = 300");

  t.is (convertDuration ("0 wks"),         0, "valid duration 0 wks = 0");
  t.is (convertDuration ("0 wk"),          0, "valid duration 0 wk = 0");
  t.is (convertDuration ("0w"),            0, "valid duration 0w = 0");
  t.is (convertDuration ("1 wks"),         7, "valid duration 1 wks = 7");
  t.is (convertDuration ("1 wk"),          7, "valid duration 1 wk = 7");
  t.is (convertDuration ("1w"),            7, "valid duration 1w = 7");
  t.is (convertDuration ("10 wks"),       70, "valid duration 10 wks = 70");
  t.is (convertDuration ("10 wk"),        70, "valid duration 10 wk = 70");
  t.is (convertDuration ("10w"),          70, "valid duration 10w = 70");

  t.is (convertDuration ("0 days"),        0, "valid duration 0 days = 0");
  t.is (convertDuration ("0 day"),         0, "valid duration 0 day = 0");
  t.is (convertDuration ("0d"),            0, "valid duration 0d = 0");
  t.is (convertDuration ("1 days"),        1, "valid duration 1 days = 1");
  t.is (convertDuration ("1 day"),         1, "valid duration 1 day = 1");
  t.is (convertDuration ("1d"),            1, "valid duration 1d = 1");
  t.is (convertDuration ("10 days"),      10, "valid duration 10 days = 10");
  t.is (convertDuration ("10 day"),       10, "valid duration 10 day = 10");
  t.is (convertDuration ("10d"),          10, "valid duration 10d = 10");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
