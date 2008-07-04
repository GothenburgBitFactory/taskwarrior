////////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <Date.h>
#include <test.h>
#include <../task.h>

////////////////////////////////////////////////////////////////////////////////
//   daily, day, d, Nd
//   weekly, w, Nw, sennight, biweekly, fortnight
//   monthly, m, bimonthly, Nm, semimonthly
//   1st 2nd 3rd 4th .. 31st
//   quarterly, q, Nq
//   biannual, biyearly, annual, semiannual, yearly, y, Na, Ny
int main (int argc, char** argv)
{
  plan (19);

  is (convertDuration ("daily"),       1, "duration daily = 1");
  is (convertDuration ("day"),         1, "duration day = 1");
  is (convertDuration ("d"),           0, "duration d = 1");
  is (convertDuration ("0d"),          0, "duration 0d = 0");
  is (convertDuration ("1d"),          1, "duration 1d = 1");
  is (convertDuration ("7d"),          7, "duration 7d = 7");
  is (convertDuration ("10d"),        10, "duration 10d = 10");
  is (convertDuration ("100d"),      100, "duration 100d = 100");

  is (convertDuration ("weekly"),      7, "duration weekly = 7");
  is (convertDuration ("sennight"),    7, "duration sennight = 7");
  is (convertDuration ("biweekly"),   14, "duration biweekly = 14");
  is (convertDuration ("fortnight"),  14, "duration fortnight = 14");
  is (convertDuration ("week"),        7, "duration week = 7");
  is (convertDuration ("w"),           7, "duration w = 7");
  is (convertDuration ("0w"),          0, "duration 0w = 0");
  is (convertDuration ("1w"),          7, "duration 1w = 7");
  is (convertDuration ("7w"),         49, "duration 7w = 49");
  is (convertDuration ("10w"),        70, "duration 10w = 70");
  is (convertDuration ("100w"),      700, "duration 100w = 700");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
