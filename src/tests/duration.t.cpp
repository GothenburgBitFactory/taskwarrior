////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <Duration.h>
#include <test.h>

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
  UnitTest t (17);

  std::string d;
  t.is (convertDuration ("daily"),       1, "duration daily = 1");
  t.is (convertDuration ("weekdays"),    1, "duration weekdays = 1");
  t.is (convertDuration ("day"),         1, "duration day = 1");
  t.is (convertDuration ("0d"),          0, "duration 0d = 0");
  t.is (convertDuration ("1d"),          1, "duration 1d = 1");
  t.is (convertDuration ("7d"),          7, "duration 7d = 7");
  t.is (convertDuration ("10d"),        10, "duration 10d = 10");
  t.is (convertDuration ("100d"),      100, "duration 100d = 100");

  t.is (convertDuration ("weekly"),      7, "duration weekly = 7");
  t.is (convertDuration ("sennight"),    7, "duration sennight = 7");
  t.is (convertDuration ("biweekly"),   14, "duration biweekly = 14");
  t.is (convertDuration ("fortnight"),  14, "duration fortnight = 14");
  t.is (convertDuration ("0w"),          0, "duration 0w = 0");
  t.is (convertDuration ("1w"),          7, "duration 1w = 7");
  t.is (convertDuration ("7w"),         49, "duration 7w = 49");
  t.is (convertDuration ("10w"),        70, "duration 10w = 70");
  t.is (convertDuration ("100w"),      700, "duration 100w = 700");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
