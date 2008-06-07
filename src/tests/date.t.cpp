////////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <Date.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (46);

  Date now;
  Date yesterday;
  yesterday -= 1;

  t.ok    (yesterday <= now,       "yesterday <= now");
  t.ok    (yesterday <  now,       "yesterday < now");
  t.notok (yesterday == now,       "!(yesterday == now)");
  t.ok    (yesterday != now,       "yesterday != now");
  t.ok    (now       >= yesterday, "now >= yesterday");
  t.ok    (now       >  yesterday, "now > yesterday");

  t.ok    (Date::valid (2, 29, 2008), "valid: 2/29/2008");
  t.notok (Date::valid (2, 29, 2007), "invalid: 2/29/2007");

  t.ok    (Date::leapYear (2008), "2008 is a leap year");
  t.notok (Date::leapYear (2007), "2007 is not a leap year");

  t.is (Date::daysInMonth (2, 2008), 29, "29 days in February 2008");
  t.is (Date::daysInMonth (2, 2007), 28, "28 days in February 2007");

  t.is (Date::monthName (1), "January",   "1 = January");
  t.is (Date::monthName (2), "February",  "2 = February");
  t.is (Date::monthName (3), "March",     "3 = March");
  t.is (Date::monthName (4), "April",     "4 = April");
  t.is (Date::monthName (5), "May",       "5 = May");
  t.is (Date::monthName (6), "June",      "6 = June");
  t.is (Date::monthName (7), "July",      "7 = July");
  t.is (Date::monthName (8), "August",    "8 = August");
  t.is (Date::monthName (9), "September", "9 = September");
  t.is (Date::monthName (10), "October",  "10 = October");
  t.is (Date::monthName (11), "November", "11 = November");
  t.is (Date::monthName (12), "December", "12 = December");

  t.is (Date::dayName (0), "Sunday",    "0 == Sunday");
  t.is (Date::dayName (1), "Monday",    "1 == Monday");
  t.is (Date::dayName (2), "Tuesday",   "2 == Tuesday");
  t.is (Date::dayName (3), "Wednesday", "3 == Wednesday");
  t.is (Date::dayName (4), "Thursday",  "4 == Thursday");
  t.is (Date::dayName (5), "Friday",    "5 == Friday");
  t.is (Date::dayName (6), "Saturday",  "6 == Saturday");

  Date happyNewYear (1, 1, 2008);
  t.is (happyNewYear.dayOfWeek (), 2, "1/1/2008 == Tuesday");
  t.is (happyNewYear.month (),     1, "1/1/2008 == January");
  t.is (happyNewYear.day (),       1, "1/1/2008 == 1");
  t.is (happyNewYear.year (),   2008, "1/1/2008 == 2008");

  t.is (now - yesterday, 1, "today - yesterday == 1");

  t.is (happyNewYear.toString (), "1/1/2008", "toString 1/1/2008");

  int m, d, y;
  happyNewYear.toMDY (m, d, y);
  t.is (m, 1, "1/1/2008 == January");
  t.is (d, 1, "1/1/2008 == 1");
  t.is (y, 2008, "1/1/2008 == 2008");

  Date epoch (9, 8, 2001);
  t.ok ((int)epoch.toEpoch () < 1000000000, "9/8/2001 < 1,000,000,000");
  epoch += 86400;
  t.ok ((int)epoch.toEpoch () > 1000000000, "9/9/2001 > 1,000,000,000");

  Date fromEpoch (epoch.toEpoch ());
  t.is (fromEpoch.toString (), epoch.toString (), "ctor (time_t)");

  Date fromString ("1/1/2008");
  t.is (fromString.month (),   1, "ctor (std::string) -> m");
  t.is (fromString.day (),     1, "ctor (std::string) -> d");
  t.is (fromString.year (), 2008, "ctor (std::string) -> y");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
