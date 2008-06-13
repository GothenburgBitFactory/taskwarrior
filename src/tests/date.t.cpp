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
  plan (63);

  Date now;
  Date yesterday;
  yesterday -= 1;

  ok    (yesterday <= now,       "yesterday <= now");
  ok    (yesterday <  now,       "yesterday < now");
  notok (yesterday == now,       "!(yesterday == now)");
  ok    (yesterday != now,       "yesterday != now");
  ok    (now       >= yesterday, "now >= yesterday");
  ok    (now       >  yesterday, "now > yesterday");

  ok    (Date::valid (2, 29, 2008), "valid: 2/29/2008");
  notok (Date::valid (2, 29, 2007), "invalid: 2/29/2007");

  ok    (Date::leapYear (2008), "2008 is a leap year");
  notok (Date::leapYear (2007), "2007 is not a leap year");
  ok    (Date::leapYear (2000), "2000 is a leap year");
  ok    (Date::leapYear (1900), "1900 is a leap year");

  is (Date::daysInMonth (2, 2008), 29, "29 days in February 2008");
  is (Date::daysInMonth (2, 2007), 28, "28 days in February 2007");

  is (Date::monthName (1), "January",   "1 = January");
  is (Date::monthName (2), "February",  "2 = February");
  is (Date::monthName (3), "March",     "3 = March");
  is (Date::monthName (4), "April",     "4 = April");
  is (Date::monthName (5), "May",       "5 = May");
  is (Date::monthName (6), "June",      "6 = June");
  is (Date::monthName (7), "July",      "7 = July");
  is (Date::monthName (8), "August",    "8 = August");
  is (Date::monthName (9), "September", "9 = September");
  is (Date::monthName (10), "October",  "10 = October");
  is (Date::monthName (11), "November", "11 = November");
  is (Date::monthName (12), "December", "12 = December");

  is (Date::dayName (0), "Sunday",    "0 == Sunday");
  is (Date::dayName (1), "Monday",    "1 == Monday");
  is (Date::dayName (2), "Tuesday",   "2 == Tuesday");
  is (Date::dayName (3), "Wednesday", "3 == Wednesday");
  is (Date::dayName (4), "Thursday",  "4 == Thursday");
  is (Date::dayName (5), "Friday",    "5 == Friday");
  is (Date::dayName (6), "Saturday",  "6 == Saturday");

  Date happyNewYear (1, 1, 2008);
  is (happyNewYear.dayOfWeek (), 2, "1/1/2008 == Tuesday");
  is (happyNewYear.month (),     1, "1/1/2008 == January");
  is (happyNewYear.day (),       1, "1/1/2008 == 1");
  is (happyNewYear.year (),   2008, "1/1/2008 == 2008");

  is (now - yesterday, 1, "today - yesterday == 1");

  is (happyNewYear.toString (), "1/1/2008", "toString 1/1/2008");

  int m, d, y;
  happyNewYear.toMDY (m, d, y);
  is (m, 1, "1/1/2008 == January");
  is (d, 1, "1/1/2008 == 1");
  is (y, 2008, "1/1/2008 == 2008");

  Date epoch (9, 8, 2001);
  ok ((int)epoch.toEpoch () < 1000000000, "9/8/2001 < 1,000,000,000");
  epoch += 86400;
  ok ((int)epoch.toEpoch () > 1000000000, "9/9/2001 > 1,000,000,000");

  Date fromEpoch (epoch.toEpoch ());
  is (fromEpoch.toString (), epoch.toString (), "ctor (time_t)");

  Date fromString1 ("1/1/2008");
  is (fromString1.month (),   1, "ctor (std::string) -> m");
  is (fromString1.day (),     1, "ctor (std::string) -> d");
  is (fromString1.year (), 2008, "ctor (std::string) -> y");

  Date fromString2 ("1/1/2008", "m/d/Y");
  is (fromString2.month (),   1, "ctor (std::string) -> m");
  is (fromString2.day (),     1, "ctor (std::string) -> d");
  is (fromString2.year (), 2008, "ctor (std::string) -> y");

  Date fromString3 ("20080101", "YMD");
  is (fromString3.month (),   1, "ctor (std::string) -> m");
  is (fromString3.day (),     1, "ctor (std::string) -> d");
  is (fromString3.year (), 2008, "ctor (std::string) -> y");

  Date fromString4 ("12/31/2007");
  is (fromString4.month (),  12, "ctor (std::string) -> m");
  is (fromString4.day (),    31, "ctor (std::string) -> d");
  is (fromString4.year (), 2007, "ctor (std::string) -> y");

  Date fromString5 ("12/31/2007", "m/d/Y");
  is (fromString5.month (),  12, "ctor (std::string) -> m");
  is (fromString5.day (),    31, "ctor (std::string) -> d");
  is (fromString5.year (), 2007, "ctor (std::string) -> y");

  Date fromString6 ("20071231", "YMD");
  is (fromString6.month (),  12, "ctor (std::string) -> m");
  is (fromString6.day (),    31, "ctor (std::string) -> d");
  is (fromString6.year (), 2007, "ctor (std::string) -> y");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
