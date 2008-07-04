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
  plan (97);

  try
  {
    Date now;
    Date yesterday;
    yesterday -= 1;

    ok    (yesterday <= now,       "yesterday <= now");
    ok    (yesterday <  now,       "yesterday < now");
    notok (yesterday == now,       "!(yesterday == now)");
    ok    (yesterday != now,       "yesterday != now");
    ok    (now       >= yesterday, "now >= yesterday");
    ok    (now       >  yesterday, "now > yesterday");

    // Loose comparisons.
    Date left ("7/4/2008");
    Date comp1 ("7/4/2008");
    ok (left.sameDay   (comp1), "7/4/2008 is on the same day as 7/4/2008");
    ok (left.sameMonth (comp1), "7/4/2008 is in the same month as 7/4/2008");
    ok (left.sameYear  (comp1), "7/4/2008 is in the same year as 7/4/2008");

    Date comp2 ("7/5/2008");
    notok (left.sameDay   (comp2), "7/4/2008 is not on the same day as 7/5/2008");
    ok    (left.sameMonth (comp2), "7/4/2008 is in the same month as 7/5/2008");
    ok    (left.sameYear  (comp2), "7/4/2008 is in the same year as 7/5/2008");

    Date comp3 ("8/4/2008");
    notok (left.sameDay   (comp3), "7/4/2008 is not on the same day as 8/4/2008");
    notok (left.sameMonth (comp3), "7/4/2008 is not in the same month as 8/4/2008");
    ok    (left.sameYear  (comp3), "7/4/2008 is in the same year as 8/4/2008");

    Date comp4 ("7/4/2009");
    notok (left.sameDay   (comp4), "7/4/2008 is not on the same day as 7/4/2009");
    notok (left.sameMonth (comp4), "7/4/2008 is not in the same month as 7/4/2009");
    notok (left.sameYear  (comp4), "7/4/2008 is not in the same year as 7/4/2009");

    // Validity.
    ok    (Date::valid (2, 29, 2008), "valid: 2/29/2008");
    notok (Date::valid (2, 29, 2007), "invalid: 2/29/2007");

    // Leap year.
    ok    (Date::leapYear (2008), "2008 is a leap year");
    notok (Date::leapYear (2007), "2007 is not a leap year");
    ok    (Date::leapYear (2000), "2000 is a leap year");
    ok    (Date::leapYear (1900), "1900 is a leap year");

    // Days in month.
    is (Date::daysInMonth (2, 2008), 29, "29 days in February 2008");
    is (Date::daysInMonth (2, 2007), 28, "28 days in February 2007");

    // Names.
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

    is (Date::dayOfWeek ("SUNDAY"),    0, "SUNDAY == 0");
    is (Date::dayOfWeek ("sunday"),    0, "sunday == 0");
    is (Date::dayOfWeek ("Sunday"),    0, "Sunday == 0");
    is (Date::dayOfWeek ("Monday"),    1, "Monday == 1");
    is (Date::dayOfWeek ("Tuesday"),   2, "Tuesday == 2");
    is (Date::dayOfWeek ("Wednesday"), 3, "Wednesday == 3");
    is (Date::dayOfWeek ("Thursday"),  4, "Thursday == 4");
    is (Date::dayOfWeek ("Friday"),    5, "Friday == 5");
    is (Date::dayOfWeek ("Saturday"),  6, "Saturday == 6");

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

    // Date parsing.
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

    // Relative dates.
    Date r1 ("today");
    ok (r1.sameDay (now), "today = now");

    Date r2 ("tomorrow");
    ok (r2.sameDay (now + 86400), "tomorrow = now + 1d");

    Date r3 ("yesterday");
    ok (r3.sameDay (now - 86400), "yesterday = now - 1d");

    Date r4 ("sunday");
    if (now.dayOfWeek () <= 0)
      ok (r4.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next sunday");
    else
      ok (r4.sameDay (now + (r4.dayOfWeek () - now.dayOfWeek ()) * 86400), "next sunday");

    Date r5 ("monday");
    if (now.dayOfWeek () <= 1)
      ok (r5.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next monday");
    else
      ok (r5.sameDay (now + (r5.dayOfWeek () - now.dayOfWeek ()) * 86400), "next monday");

    Date r6 ("tuesday");
    if (now.dayOfWeek () <= 2)
      ok (r6.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next tuesday");
    else
      ok (r6.sameDay (now + (r6.dayOfWeek () - now.dayOfWeek ()) * 86400), "next tuesday");

    Date r7 ("wednesday");
    if (now.dayOfWeek () <= 3)
      ok (r7.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next wednesday");
    else
      ok (r7.sameDay (now + (r7.dayOfWeek () - now.dayOfWeek ()) * 86400), "next wednesday");

    Date r8 ("thursday");
    if (now.dayOfWeek () <= 4)
      ok (r8.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next thursday");
    else
      ok (r8.sameDay (now + (r8.dayOfWeek () - now.dayOfWeek ()) * 86400), "next thursday");

    Date r9 ("friday");
    if (now.dayOfWeek () <= 5)
      ok (r9.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next friday");
    else
      ok (r9.sameDay (now + (r9.dayOfWeek () - now.dayOfWeek ()) * 86400), "next friday");

    Date r10 ("saturday");
    if (now.dayOfWeek () <= 6)
      ok (r10.sameDay (now + (7 - now.dayOfWeek ()) * 86499), "next saturday");
    else
      ok (r10.sameDay (now + (r10.dayOfWeek () - now.dayOfWeek ()) * 86400), "next saturday");

    Date r11 ("eow");
    ok (r11 < now + (8 * 86400), "eow < 7 days away");

    Date r12 ("eom");
    ok (r12.sameMonth (now), "eom in same month as now");

    Date r13 ("eoy");
    ok (r13.sameYear (now), "eoy in same year as now");
  }

  catch (std::string& e)
  {
    std::cout << e << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
