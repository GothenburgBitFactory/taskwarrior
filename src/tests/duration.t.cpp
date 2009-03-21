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
#include <Date.h>
#include <test.h>
#include <../task.h>

////////////////////////////////////////////////////////////////////////////////
//   daily, day, Nd
//   weekly, Nw, sennight, biweekly, fortnight
//   monthly, bimonthly, Nm, semimonthly
//   1st 2nd 3rd 4th .. 31st
//   quarterly, Nq
//   biannual, biyearly, annual, semiannual, yearly, Ny
int main (int argc, char** argv)
{
  UnitTest t (17);

  std::string d;
  d = "daily";     t.is (convertDuration (d),   1, "duration daily = 1");
  d = "weekdays";  t.is (convertDuration (d),   1, "duration weekdays = 1");
  d = "day";       t.is (convertDuration (d),   1, "duration day = 1");
  d = "0d";        t.is (convertDuration (d),   0, "duration 0d = 0");
  d = "1d";        t.is (convertDuration (d),   1, "duration 1d = 1");
  d = "7d";        t.is (convertDuration (d),   7, "duration 7d = 7");
  d = "10d";       t.is (convertDuration (d),  10, "duration 10d = 10");
  d = "100d";      t.is (convertDuration (d), 100, "duration 100d = 100");

  d = "weekly";    t.is (convertDuration (d),   7, "duration weekly = 7");
  d = "sennight";  t.is (convertDuration (d),   7, "duration sennight = 7");
  d = "biweekly";  t.is (convertDuration (d),  14, "duration biweekly = 14");
  d = "fortnight"; t.is (convertDuration (d),  14, "duration fortnight = 14");
  d = "0w";        t.is (convertDuration (d),   0, "duration 0w = 0");
  d = "1w";        t.is (convertDuration (d),   7, "duration 1w = 7");
  d = "7w";        t.is (convertDuration (d),  49, "duration 7w = 49");
  d = "10w";       t.is (convertDuration (d),  70, "duration 10w = 70");
  d = "100w";      t.is (convertDuration (d), 700, "duration 100w = 700");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
