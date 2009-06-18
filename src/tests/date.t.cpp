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
#include <Context.h>
#include <Date.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (102);

  try
  {
    Date now;
    Date yesterday;
    yesterday -= 1;

    t.ok    (yesterday <= now,       "yesterday <= now");
    t.ok    (yesterday <  now,       "yesterday < now");
    t.notok (yesterday == now,       "!(yesterday == now)");
    t.ok    (yesterday != now,       "yesterday != now");
    t.ok    (now       >= yesterday, "now >= yesterday");
    t.ok    (now       >  yesterday, "now > yesterday");

    // Loose comparisons.
    Date left ("7/4/2008");
    Date comp1 ("7/4/2008");
    t.ok (left.sameDay   (comp1), "7/4/2008 is on the same day as 7/4/2008");
    t.ok (left.sameMonth (comp1), "7/4/2008 is in the same month as 7/4/2008");
    t.ok (left.sameYear  (comp1), "7/4/2008 is in the same year as 7/4/2008");

    Date comp2 ("7/5/2008");
    t.notok (left.sameDay   (comp2), "7/4/2008 is not on the same day as 7/5/2008");
    t.ok    (left.sameMonth (comp2), "7/4/2008 is in the same month as 7/5/2008");
    t.ok    (left.sameYear  (comp2), "7/4/2008 is in the same year as 7/5/2008");

    Date comp3 ("8/4/2008");
    t.notok (left.sameDay   (comp3), "7/4/2008 is not on the same day as 8/4/2008");
    t.notok (left.sameMonth (comp3), "7/4/2008 is not in the same month as 8/4/2008");
    t.ok    (left.sameYear  (comp3), "7/4/2008 is in the same year as 8/4/2008");

    Date comp4 ("7/4/2009");
    t.notok (left.sameDay   (comp4), "7/4/2008 is not on the same day as 7/4/2009");
    t.notok (left.sameMonth (comp4), "7/4/2008 is not in the same month as 7/4/2009");
    t.notok (left.sameYear  (comp4), "7/4/2008 is not in the same year as 7/4/2009");

    // Validity.
    t.ok    (Date::valid (2, 29, 2008), "valid: 2/29/2008");
    t.notok (Date::valid (2, 29, 2007), "invalid: 2/29/2007");

    t.ok    (Date::valid ("2/29/2008"), "valid: 2/29/2008");
    t.notok (Date::valid ("2/29/2007"), "invalid: 2/29/2007");

    // Leap year.
    t.ok    (Date::leapYear (2008), "2008 is a leap year");
    t.notok (Date::leapYear (2007), "2007 is not a leap year");
    t.ok    (Date::leapYear (2000), "2000 is a leap year");
    t.notok (Date::leapYear (1900), "1900 is not a leap year");

    // Days in month.
    t.is (Date::daysInMonth (2, 2008), 29, "29 days in February 2008");
    t.is (Date::daysInMonth (2, 2007), 28, "28 days in February 2007");

    // Names.
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

    t.is (Date::dayOfWeek ("SUNDAY"),    0, "SUNDAY == 0");
    t.is (Date::dayOfWeek ("sunday"),    0, "sunday == 0");
    t.is (Date::dayOfWeek ("Sunday"),    0, "Sunday == 0");
    t.is (Date::dayOfWeek ("Monday"),    1, "Monday == 1");
    t.is (Date::dayOfWeek ("Tuesday"),   2, "Tuesday == 2");
    t.is (Date::dayOfWeek ("Wednesday"), 3, "Wednesday == 3");
    t.is (Date::dayOfWeek ("Thursday"),  4, "Thursday == 4");
    t.is (Date::dayOfWeek ("Friday"),    5, "Friday == 5");
    t.is (Date::dayOfWeek ("Saturday"),  6, "Saturday == 6");

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

    // Date parsing.
    Date fromString1 ("1/1/2008");
    t.is (fromString1.month (),   1, "ctor (std::string) -> m");
    t.is (fromString1.day (),     1, "ctor (std::string) -> d");
    t.is (fromString1.year (), 2008, "ctor (std::string) -> y");

    Date fromString2 ("1/1/2008", "m/d/Y");
    t.is (fromString2.month (),   1, "ctor (std::string) -> m");
    t.is (fromString2.day (),     1, "ctor (std::string) -> d");
    t.is (fromString2.year (), 2008, "ctor (std::string) -> y");

    Date fromString3 ("20080101", "YMD");
    t.is (fromString3.month (),   1, "ctor (std::string) -> m");
    t.is (fromString3.day (),     1, "ctor (std::string) -> d");
    t.is (fromString3.year (), 2008, "ctor (std::string) -> y");

    Date fromString4 ("12/31/2007");
    t.is (fromString4.month (),  12, "ctor (std::string) -> m");
    t.is (fromString4.day (),    31, "ctor (std::string) -> d");
    t.is (fromString4.year (), 2007, "ctor (std::string) -> y");

    Date fromString5 ("12/31/2007", "m/d/Y");
    t.is (fromString5.month (),  12, "ctor (std::string) -> m");
    t.is (fromString5.day (),    31, "ctor (std::string) -> d");
    t.is (fromString5.year (), 2007, "ctor (std::string) -> y");

    Date fromString6 ("20071231", "YMD");
    t.is (fromString6.month (),  12, "ctor (std::string) -> m");
    t.is (fromString6.day (),    31, "ctor (std::string) -> d");
    t.is (fromString6.year (), 2007, "ctor (std::string) -> y");

    Date fromString7 ("01/01/2008", "m/d/Y");
    t.is (fromString7.month (),   1, "ctor (std::string) -> m");
    t.is (fromString7.day (),     1, "ctor (std::string) -> d");
    t.is (fromString7.year (), 2008, "ctor (std::string) -> y");

    // Relative dates.
    Date r1 ("today");
    t.ok (r1.sameDay (now), "today = now");

    Date r2 ("tomorrow");
    t.ok (r2.sameDay (now + 86400), "tomorrow = now + 1d");

    Date r3 ("yesterday");
    t.ok (r3.sameDay (now - 86400), "yesterday = now - 1d");

    Date r4 ("sunday");
    if (now.dayOfWeek () >= 0)
      t.ok (r4.sameDay (now + (0 - now.dayOfWeek () + 7) * 86400), "next sunday");
    else
      t.ok (r4.sameDay (now + (0 - now.dayOfWeek ()) * 86400), "next sunday");;

    Date r5 ("monday");
    if (now.dayOfWeek () >= 1)
      t.ok (r5.sameDay (now + (1 - now.dayOfWeek () + 7) * 86400), "next monday");
    else
      t.ok (r5.sameDay (now + (1 - now.dayOfWeek ()) * 86400), "next monday");;

    Date r6 ("tuesday");
    if (now.dayOfWeek () >= 2)
      t.ok (r6.sameDay (now + (2 - now.dayOfWeek () + 7) * 86400), "next tuesday");
    else
      t.ok (r6.sameDay (now + (2 - now.dayOfWeek ()) * 86400), "next tuesday");;

    Date r7 ("wednesday");
    if (now.dayOfWeek () >= 3)
      t.ok (r7.sameDay (now + (3 - now.dayOfWeek () + 7) * 86400), "next wednesday");
    else
      t.ok (r7.sameDay (now + (3 - now.dayOfWeek ()) * 86400), "next wednesday");;

    Date r8 ("thursday");
    if (now.dayOfWeek () >= 4)
      t.ok (r8.sameDay (now + (4 - now.dayOfWeek () + 7) * 86400), "next thursday");
    else
      t.ok (r8.sameDay (now + (4 - now.dayOfWeek ()) * 86400), "next thursday");;

    Date r9 ("friday");
    if (now.dayOfWeek () >= 5)
      t.ok (r9.sameDay (now + (5 - now.dayOfWeek () + 7) * 86400), "next friday");
    else
      t.ok (r9.sameDay (now + (5 - now.dayOfWeek ()) * 86400), "next friday");;

    Date r10 ("saturday");
    if (now.dayOfWeek () >= 6)
      t.ok (r10.sameDay (now + (6 - now.dayOfWeek () + 7) * 86400), "next saturday");
    else
      t.ok (r10.sameDay (now + (6 - now.dayOfWeek ()) * 86400), "next saturday");;

    Date r11 ("eow");
    t.ok (r11 < now + (8 * 86400), "eow < 7 days away");

    Date r12 ("eom");
    t.ok (r12.sameMonth (now), "eom in same month as now");

    Date r13 ("eoy");
    t.ok (r13.sameYear (now), "eoy in same year as now");
  }

  catch (std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
