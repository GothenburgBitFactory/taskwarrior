////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
  UnitTest t (168);

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

    // Date::Date ("now")
    Date relative_now ("now");
    t.ok (relative_now.sameHour (now),  "Date ().sameHour (Date (now))");
    t.ok (relative_now.sameDay (now),   "Date ().sameDay (Date (now))");
    t.ok (relative_now.sameMonth (now), "Date ().sameMonth (Date (now))");
    t.ok (relative_now.sameYear (now),  "Date ().sameYear (Date (now))");

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

    // Time validity.
    t.ok    (Date::valid (2, 28, 2010,  0,  0,  0), "valid 2/28/2010 0:00:00");
    t.ok    (Date::valid (2, 28, 2010, 23, 59, 59), "valid 2/28/2010 23:59:59");
    t.notok (Date::valid (2, 28, 2010, 24, 59, 59), "valid 2/28/2010 24:59:59");
    t.notok (Date::valid (2, 28, 2010, -1,  0,  0), "valid 2/28/2010 -1:00:00");

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
    epoch += 172800;
    t.ok ((int)epoch.toEpoch () > 1000000000, "9/10/2001 > 1,000,000,000");

    Date fromEpoch (epoch.toEpoch ());
    t.is (fromEpoch.toString (), epoch.toString (), "ctor (time_t)");

    Date iso (1000000000);
    t.is (iso.toISO (), "20010909T014640Z", "1,000,000,000 -> 20010909T014640Z");

    // Quantization.
    Date quant (1234526400);
    t.is (quant.startOfDay ().toString ("YMDHNS"),   "20090213000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 2/13/2009 0:00:00");
    t.is (quant.startOfWeek ().toString ("YMDHNS"),  "20090208000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 2/8/2009 0:00:00");
    t.is (quant.startOfMonth ().toString ("YMDHNS"), "20090201000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 2/1/2009 0:00:00");
    t.is (quant.startOfYear ().toString ("YMDHNS"),  "20090101000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 1/1/2009 0:00:00");

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

    Date fromString8 ("Tue 01 Jan 2008 (01)", "a D b Y (V)");
    t.is (fromString8.month (),   1, "ctor (std::string) -> m");
    t.is (fromString8.day (),     1, "ctor (std::string) -> d");
    t.is (fromString8.year (), 2008, "ctor (std::string) -> y");

    Date fromString9 ("Tuesday, January 1, 2008", "A, B d, Y");
    t.is (fromString9.month (),   1, "ctor (std::string) -> m");
    t.is (fromString9.day (),     1, "ctor (std::string) -> d");
    t.is (fromString9.year (), 2008, "ctor (std::string) -> y");

    Date fromString10 ("v01 Tue 2008-01-01", "vV a Y-M-D");
    t.is (fromString10.month (),   1, "ctor (std::string) -> m");
    t.is (fromString10.day (),     1, "ctor (std::string) -> d");
    t.is (fromString10.year (), 2008, "ctor (std::string) -> y");

    Date fromString11 ("6/7/2010 1:23:45",  "m/d/Y h:N:S");
    t.is (fromString11.month (),     6, "ctor (std::string) -> m");
    t.is (fromString11.day (),       7, "ctor (std::string) -> d");
    t.is (fromString11.year (),   2010, "ctor (std::string) -> Y");
    t.is (fromString11.hour (),      1, "ctor (std::string) -> h");
    t.is (fromString11.minute (),   23, "ctor (std::string) -> N");
    t.is (fromString11.second (),   45, "ctor (std::string) -> S");

    Date fromString12 ("6/7/2010 01:23:45", "m/d/Y H:N:S");
    t.is (fromString12.month (),     6, "ctor (std::string) -> m");
    t.is (fromString12.day (),       7, "ctor (std::string) -> d");
    t.is (fromString12.year (),   2010, "ctor (std::string) -> Y");
    t.is (fromString12.hour (),      1, "ctor (std::string) -> h");
    t.is (fromString12.minute (),   23, "ctor (std::string) -> N");
    t.is (fromString12.second (),   45, "ctor (std::string) -> S");

    Date fromString13 ("6/7/2010 12:34:56", "m/d/Y H:N:S");
    t.is (fromString13.month (),     6, "ctor (std::string) -> m");
    t.is (fromString13.day (),       7, "ctor (std::string) -> d");
    t.is (fromString13.year (),   2010, "ctor (std::string) -> Y");
    t.is (fromString13.hour (),     12, "ctor (std::string) -> h");
    t.is (fromString13.minute (),   34, "ctor (std::string) -> N");
    t.is (fromString13.second (),   56, "ctor (std::string) -> S");

    // Day of year
    t.is (Date ("1/1/2011",   "m/d/Y").dayOfYear (),   1, "dayOfYear (1/1/2011)   ->   1");
    t.is (Date ("5/1/2011",   "m/d/Y").dayOfYear (), 121, "dayOfYear (5/1/2011)   -> 121");
    t.is (Date ("12/31/2011", "m/d/Y").dayOfYear (), 365, "dayOfYear (12/31/2011) -> 365");

    // Easter
    Date e1 (Date::easter(1980));
    t.is (e1.toString (), "4/6/1980", "Easter 4/6/1980");
    Date e2 (Date::easter(1995));
    t.is (e2.toString (), "4/16/1995", "Easter 4/16/1995");
    Date e3 (Date::easter(2000));
    t.is (e3.toString (), "4/23/2000", "Easter 4/23/2000");
    Date e4 (Date::easter(2009));
    t.is (e4.toString (), "4/12/2009", "Easter 4/12/2009");
    Date e5 (Date::easter(2010));
    t.is (e5.toString (), "4/4/2010", "Easter 4/4/2010");
    Date e6 (Date::easter(2011));
    t.is (e6.toString (), "4/24/2011", "Easter 4/24/2011");
    Date e7 (Date::easter(2012));
    t.is (e7.toString (), "4/8/2012", "Easter 4/8/2012");
    Date e8 (Date::easter(2020));
    t.is (e8.toString (), "4/12/2020", "Easter 4/12/2020");

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

    Date r20 ("eocw");
    t.ok (r20 < now + (8 * 86400), "eocw < 7 days away");

    Date r12 ("eom");
    t.ok (r12.sameMonth (now), "eom in same month as now");

    Date r13 ("eoy");
    t.ok (r13.sameYear (now), "eoy in same year as now");

    Date r14 ("sow");
    t.ok (r14 < now + (8 * 86400), "sow < 7 days away");

    Date r21 ("socw");
    t.ok (r21 < now + (8 * 86400), "sow < 7 days away");

    Date r15 ("som");
    t.notok (r15.sameMonth (now), "som not in same month as now");

    Date r16 ("soy");
    t.notok (r16.sameYear (now), "soy not in same year as now");

    Date later ("later");
    t.is (later.month (),   1, "later -> m = 1");
    t.is (later.day (),    18, "later -> d = 18");
    t.is (later.year (), 2038, "later -> y = 2038");

    // Date::sameHour
    Date r17 ("6/7/2010 01:00:00", "m/d/Y H:N:S");
    Date r18 ("6/7/2010 01:59:59", "m/d/Y H:N:S");
    t.ok (r17.sameHour (r18), "two dates within the same hour");

    Date r19 ("6/7/2010 00:59:59", "m/d/Y H:N:S");
    t.notok (r17.sameHour (r19), "two dates not within the same hour");

    // Date::operator-
    Date r22 (1234567890);
    t.is ((r22 - 1).toEpoch (), 1234567889, "1234567890 - 1 = 1234567889");

    // Date::operator--
    Date r23 (11, 7, 2010, 23, 59, 59);
    r23--;
    t.is (r23.toString ("YMDHNS"), "20101106235959", "decrement across fall DST boundary");

    Date r24 (3, 14, 2010, 23, 59, 59);
    r24--;
    t.is (r24.toString ("YMDHNS"), "20100313235959", "decrement across spring DST boundary");

    // Date::operator++
    Date r25 (11, 6, 2010, 23, 59, 59);
    r25++;
    t.is (r25.toString ("YMDHNS"), "20101107235959", "increment across fall DST boundary");

    Date r26 (3, 13, 2010, 23, 59, 59);
    r26++;
    t.is (r26.toString ("YMDHNS"), "20100314235959", "increment across spring DST boundary");
  }

  catch (std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
