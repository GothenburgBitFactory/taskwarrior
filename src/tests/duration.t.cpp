////////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
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
  plan (17);

  std::string d;
  d = "daily";     is (convertDuration (d),   1, "duration daily = 1");
  d = "day";       is (convertDuration (d),   1, "duration day = 1");
  d = "0d";        is (convertDuration (d),   0, "duration 0d = 0");
  d = "1d";        is (convertDuration (d),   1, "duration 1d = 1");
  d = "7d";        is (convertDuration (d),   7, "duration 7d = 7");
  d = "10d";       is (convertDuration (d),  10, "duration 10d = 10");
  d = "100d";      is (convertDuration (d), 100, "duration 100d = 100");

  d = "weekly";    is (convertDuration (d),   7, "duration weekly = 7");
  d = "sennight";  is (convertDuration (d),   7, "duration sennight = 7");
  d = "biweekly";  is (convertDuration (d),  14, "duration biweekly = 14");
  d = "fortnight"; is (convertDuration (d),  14, "duration fortnight = 14");
  d = "week";      is (convertDuration (d),   7, "duration week = 7");
  d = "0w";        is (convertDuration (d),   0, "duration 0w = 0");
  d = "1w";        is (convertDuration (d),   7, "duration 1w = 7");
  d = "7w";        is (convertDuration (d),  49, "duration 7w = 49");
  d = "10w";       is (convertDuration (d),  70, "duration 10w = 70");
  d = "100w";      is (convertDuration (d), 700, "duration 100w = 700");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
