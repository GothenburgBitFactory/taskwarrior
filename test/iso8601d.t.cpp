////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2015, Göteborg Bit Factory.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <iostream>
#include <time.h>
#include <test.h>
#include <ISO8601.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input,
  int in_start,
  int in_year,
  int in_month,
  int in_week,
  int in_weekday,
  int in_julian,
  int in_day,
  int in_seconds,
  int in_offset,
  bool in_utc,
  time_t in_date)
{
  std::string label = std::string ("parse (\"") + input + "\") --> ";

  ISO8601d iso;
  std::string::size_type start = 0;

  t.ok (iso.parse (input, start),             label + "true");
  t.is ((int) start,        in_start,         label + "[]");
  t.is (iso._year,          in_year,          label + "_year");
  t.is (iso._month,         in_month,         label + "_month");
  t.is (iso._week,          in_week,          label + "_week");
  t.is (iso._weekday,       in_weekday,       label + "_weekday");
  t.is (iso._julian,        in_julian,        label + "_julian");
  t.is (iso._day,           in_day,           label + "_day");
  t.is (iso._seconds,       in_seconds,       label + "_seconds");
  t.is (iso._offset,        in_offset,        label + "_offset");
  t.is (iso._utc,           in_utc,           label + "_utc");
  t.is ((size_t) iso._date, (size_t) in_date, label + "_date");
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (850);

  ISO8601d iso;
  std::string::size_type start = 0;
  t.notok (iso.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  // Determine local and UTC time.
  time_t now = time (NULL);
  struct tm* local_now = localtime (&now);
  int local_s = (local_now->tm_hour * 3600) +
                (local_now->tm_min  * 60)   +
                local_now->tm_sec;
  local_now->tm_hour  = 0;
  local_now->tm_min   = 0;
  local_now->tm_sec   = 0;
  local_now->tm_isdst = -1;
  time_t local = mktime (local_now);
  std::cout << "# local midnight today " << local << "\n";

  local_now->tm_year  = 2013 - 1900;
  local_now->tm_mon   = 12 - 1;
  local_now->tm_mday  = 6;
  local_now->tm_isdst = 0;
  time_t local6 = mktime (local_now);
  std::cout << "# local midnight 2013-12-06 " << local6 << "\n";

  local_now->tm_year  = 2013 - 1900;
  local_now->tm_mon   = 12 - 1;
  local_now->tm_mday  = 1;
  local_now->tm_isdst = 0;
  time_t local1 = mktime (local_now);
  std::cout << "# local midnight 2013-12-01 " << local1 << "\n";

  struct tm* utc_now = gmtime (&now);
  int utc_s = (utc_now->tm_hour * 3600) +
              (utc_now->tm_min  * 60)   +
              utc_now->tm_sec;
  utc_now->tm_hour  = 0;
  utc_now->tm_min   = 0;
  utc_now->tm_sec   = 0;
  utc_now->tm_isdst = -1;
  time_t utc = timegm (utc_now);
  std::cout << "# utc midnight today " << utc << "\n";

  utc_now->tm_year  = 2013 - 1900;
  utc_now->tm_mon   = 12 - 1;
  utc_now->tm_mday  = 6;
  utc_now->tm_isdst = 0;
  time_t utc6 = timegm (utc_now);
  std::cout << "# utc midnight 2013-12-06 " << utc6 << "\n";

  utc_now->tm_year  = 2013 - 1900;
  utc_now->tm_mon   = 12 - 1;
  utc_now->tm_mday  = 1;
  utc_now->tm_isdst = 0;
  time_t utc1 = timegm (utc_now);
  std::cout << "# utc midnight 2013-12-01 " << utc1 << "\n";

  int hms = (12 * 3600) + (34 * 60) + 56; // The time 12:34:56 in seconds.
  int hm  = (12 * 3600) + (34 * 60);      // The time 12:34:00 in seconds.
  int z   = 3600;                         // TZ offset.

  int ld = local_s > hms ? 86400 : 0;     // Local extra day if now > hms.
  int ud = utc_s   > hms ? 86400 : 0;     // UTC extra day if now > hms.
  std::cout << "# ld " << ld << "\n";
  std::cout << "# ud " << ud << "\n";

  // Aggregated.
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "12:34:56  ",                 8,    0,  0,  0, 0,   0,  0,   hms,     0, false, local+hms+ld );

  // time-ext
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "12:34:56Z",                  9,    0,  0,  0, 0,   0,  0,   hms,     0,  true, utc+hms+ud   );
  testParse (t, "12:34Z",                     6,    0,  0,  0, 0,   0,  0,    hm,     0,  true, utc+hm+ud    );
  testParse (t, "12:34:56+01:00",            14,    0,  0,  0, 0,   0,  0,   hms,  3600, false, utc+hms-z+ud );
  testParse (t, "12:34:56+01",               11,    0,  0,  0, 0,   0,  0,   hms,  3600, false, utc+hms-z+ud );
  testParse (t, "12:34+01:00",               11,    0,  0,  0, 0,   0,  0,    hm,  3600, false, utc+hm-z+ud  );
  testParse (t, "12:34+01",                   8,    0,  0,  0, 0,   0,  0,    hm,  3600, false, utc+hm-z+ud  );
  testParse (t, "12:34:56",                   8,    0,  0,  0, 0,   0,  0,   hms,     0, false, local+hms+ld );
  testParse (t, "12:34",                      5,    0,  0,  0, 0,   0,  0,    hm,     0, false, local+hm+ld  );

  // datetime-ext
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "2013-12-06",                10, 2013, 12,  0, 0,   0,  6,     0,     0, false, local6    );
  testParse (t, "2013-340",                   8, 2013,  0,  0, 0, 340,  0,     0,     0, false, local6    );
  testParse (t, "2013-W49-5",                10, 2013,  0, 49, 5,   0,  0,     0,     0, false, local6    );
  testParse (t, "2013-W49",                   8, 2013,  0, 49, 0,   0,  0,     0,     0, false, local1    );

  testParse (t, "2013-12-06T12:34:56",       19, 2013, 12,  0, 0,   0,  6,   hms,     0, false, local6+hms);
  testParse (t, "2013-12-06T12:34",          16, 2013, 12,  0, 0,   0,  6,    hm,     0, false, local6+hm );
  testParse (t, "2013-340T12:34:56",         17, 2013,  0,  0, 0, 340,  0,   hms,     0, false, local6+hms);
  testParse (t, "2013-340T12:34",            14, 2013,  0,  0, 0, 340,  0,    hm,     0, false, local6+hm );
  testParse (t, "2013-W49-5T12:34:56",       19, 2013,  0, 49, 5,   0,  0,   hms,     0, false, local6+hms);
  testParse (t, "2013-W49-5T12:34",          16, 2013,  0, 49, 5,   0,  0,    hm,     0, false, local6+hm );
  testParse (t, "2013-W49T12:34:56",         17, 2013,  0, 49, 0,   0,  0,   hms,     0, false, local1+hms);
  testParse (t, "2013-W49T12:34",            14, 2013,  0, 49, 0,   0,  0,    hm,     0, false, local1+hm );

  testParse (t, "2013-12-06T12:34:56Z",      20, 2013, 12,  0, 0,   0,  6,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013-12-06T12:34Z",         17, 2013, 12,  0, 0,   0,  6,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013-340T12:34:56Z",        18, 2013,  0,  0, 0, 340,  0,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013-340T12:34Z",           15, 2013,  0,  0, 0, 340,  0,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013-W49-5T12:34:56Z",      20, 2013,  0, 49, 5,   0,  0,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013-W49-5T12:34Z",         17, 2013,  0, 49, 5,   0,  0,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013-W49T12:34:56Z",        18, 2013,  0, 49, 0,   0,  0,   hms,     0,  true, utc1+hms  );
  testParse (t, "2013-W49T12:34Z",           15, 2013,  0, 49, 0,   0,  0,    hm,     0,  true, utc1+hm   );

  testParse (t, "2013-12-06T12:34:56+01:00", 25, 2013, 12,  0, 0,   0,  6,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-12-06T12:34:56+01",    22, 2013, 12,  0, 0,   0,  6,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-12-06T12:34:56-01:00", 25, 2013, 12,  0, 0,   0,  6,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-12-06T12:34:56-01",    22, 2013, 12,  0, 0,   0,  6,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-12-06T12:34+01:00",    22, 2013, 12,  0, 0,   0,  6,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-12-06T12:34+01",       19, 2013, 12,  0, 0,   0,  6,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-12-06T12:34-01:00",    22, 2013, 12,  0, 0,   0,  6,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-12-06T12:34-01",       19, 2013, 12,  0, 0,   0,  6,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-340T12:34:56+01:00",   23, 2013,  0,  0, 0, 340,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-340T12:34:56+01",      20, 2013,  0,  0, 0, 340,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-340T12:34:56-01:00",   23, 2013,  0,  0, 0, 340,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-340T12:34:56-01",      20, 2013,  0,  0, 0, 340,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-340T12:34+01:00",      20, 2013,  0,  0, 0, 340,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-340T12:34+01",         17, 2013,  0,  0, 0, 340,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-340T12:34-01:00",      20, 2013,  0,  0, 0, 340,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-340T12:34-01",         17, 2013,  0,  0, 0, 340,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-W49-5T12:34:56+01:00", 25, 2013,  0, 49, 5,   0,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-W49-5T12:34:56+01",    22, 2013,  0, 49, 5,   0,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-W49-5T12:34:56-01:00", 25, 2013,  0, 49, 5,   0,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-W49-5T12:34:56-01",    22, 2013,  0, 49, 5,   0,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-W49-5T12:34+01:00",    22, 2013,  0, 49, 5,   0,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-W49-5T12:34+01",       19, 2013,  0, 49, 5,   0,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-W49-5T12:34-01:00",    22, 2013,  0, 49, 5,   0,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-W49-5T12:34-01",       19, 2013,  0, 49, 5,   0,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-W49T12:34:56+01:00",   23, 2013,  0, 49, 0,   0,  0,   hms,  3600, false, utc1+hms-z);
  testParse (t, "2013-W49T12:34:56+01",      20, 2013,  0, 49, 0,   0,  0,   hms,  3600, false, utc1+hms-z);
  testParse (t, "2013-W49T12:34:56-01:00",   23, 2013,  0, 49, 0,   0,  0,   hms, -3600, false, utc1+hms+z);
  testParse (t, "2013-W49T12:34:56-01",      20, 2013,  0, 49, 0,   0,  0,   hms, -3600, false, utc1+hms+z);
  testParse (t, "2013-W49T12:34+01:00",      20, 2013,  0, 49, 0,   0,  0,    hm,  3600, false, utc1+hm-z );
  testParse (t, "2013-W49T12:34+01",         17, 2013,  0, 49, 0,   0,  0,    hm,  3600, false, utc1+hm-z );
  testParse (t, "2013-W49T12:34-01:00",      20, 2013,  0, 49, 0,   0,  0,    hm, -3600, false, utc1+hm+z );
  testParse (t, "2013-W49T12:34-01",         17, 2013,  0, 49, 0,   0,  0,    hm, -3600, false, utc1+hm+z );

  // The only non-extended forms.
  testParse (t, "20131206T123456Z",          16, 2013, 12,  0, 0,   0,  6,   hms,     0,  true, utc6+hms  );
  testParse (t, "20131206T123456",           15, 2013, 12,  0, 0,   0,  6,   hms,     0, false, local6+hms);

  try
  {
    ISO8601d now;
    t.ok (now.toISO ().find ("1969") == std::string::npos, "'now' != 1969");

    ISO8601d yesterday;
    yesterday -= 86400;
    ISO8601d tomorrow;
    tomorrow += 86400;

    t.ok    (yesterday <= now,       "yesterday <= now");
    t.ok    (yesterday <  now,       "yesterday < now");
    t.notok (yesterday == now,       "!(yesterday == now)");
    t.ok    (yesterday != now,       "yesterday != now");
    t.ok    (now       >= yesterday, "now >= yesterday");
    t.ok    (now       >  yesterday, "now > yesterday");

    t.ok    (tomorrow >= now,        "tomorrow >= now");
    t.ok    (tomorrow >  now,        "tomorrow > now");
    t.notok (tomorrow == now,        "!(tomorrow == now)");
    t.ok    (tomorrow != now,        "tomorrow != now");
    t.ok    (now      <= tomorrow,   "now <= tomorrow");
    t.ok    (now      <  tomorrow,   "now < tomorrow");

    // Leap year.
    t.ok    (ISO8601d::leapYear (2008), "2008 is a leap year");
    t.notok (ISO8601d::leapYear (2007), "2007 is not a leap year");
    t.ok    (ISO8601d::leapYear (2000), "2000 is a leap year");
    t.notok (ISO8601d::leapYear (1900), "1900 is not a leap year");

    // Days in year.
    t.is (ISO8601d::daysInYear (2016), 366, "366 days in 2016");
    t.is (ISO8601d::daysInYear (2015), 365, "365 days in 2015");

    // Days in month.
    t.is (ISO8601d::daysInMonth (2, 2008), 29, "29 days in February 2008");
    t.is (ISO8601d::daysInMonth (2, 2007), 28, "28 days in February 2007");

    // Names.
    t.is (ISO8601d::monthName (1), "January",   "1 = January");
    t.is (ISO8601d::monthName (2), "February",  "2 = February");
    t.is (ISO8601d::monthName (3), "March",     "3 = March");
    t.is (ISO8601d::monthName (4), "April",     "4 = April");
    t.is (ISO8601d::monthName (5), "May",       "5 = May");
    t.is (ISO8601d::monthName (6), "June",      "6 = June");
    t.is (ISO8601d::monthName (7), "July",      "7 = July");
    t.is (ISO8601d::monthName (8), "August",    "8 = August");
    t.is (ISO8601d::monthName (9), "September", "9 = September");
    t.is (ISO8601d::monthName (10), "October",  "10 = October");
    t.is (ISO8601d::monthName (11), "November", "11 = November");
    t.is (ISO8601d::monthName (12), "December", "12 = December");

    // Names.
    t.is (ISO8601d::monthOfYear ("January"),   1,  "January   =  1");
    t.is (ISO8601d::monthOfYear ("February"),  2,  "February  =  2");
    t.is (ISO8601d::monthOfYear ("March"),     3,  "March     =  3");
    t.is (ISO8601d::monthOfYear ("April"),     4,  "April     =  4");
    t.is (ISO8601d::monthOfYear ("May"),       5,  "May       =  5");
    t.is (ISO8601d::monthOfYear ("June"),      6,  "June      =  6");
    t.is (ISO8601d::monthOfYear ("July"),      7,  "July      =  7");
    t.is (ISO8601d::monthOfYear ("August"),    8,  "August    =  8");
    t.is (ISO8601d::monthOfYear ("September"), 9,  "September =  9");
    t.is (ISO8601d::monthOfYear ("October"),   10, "October   = 10");
    t.is (ISO8601d::monthOfYear ("November"),  11, "November  = 11");
    t.is (ISO8601d::monthOfYear ("December"),  12, "December  = 12");

    t.is (ISO8601d::dayName (0), "Sunday",    "0 == Sunday");
    t.is (ISO8601d::dayName (1), "Monday",    "1 == Monday");
    t.is (ISO8601d::dayName (2), "Tuesday",   "2 == Tuesday");
    t.is (ISO8601d::dayName (3), "Wednesday", "3 == Wednesday");
    t.is (ISO8601d::dayName (4), "Thursday",  "4 == Thursday");
    t.is (ISO8601d::dayName (5), "Friday",    "5 == Friday");
    t.is (ISO8601d::dayName (6), "Saturday",  "6 == Saturday");

    t.is (ISO8601d::dayOfWeek ("SUNDAY"),    0, "SUNDAY == 0");
    t.is (ISO8601d::dayOfWeek ("sunday"),    0, "sunday == 0");
    t.is (ISO8601d::dayOfWeek ("Sunday"),    0, "Sunday == 0");
    t.is (ISO8601d::dayOfWeek ("Monday"),    1, "Monday == 1");
    t.is (ISO8601d::dayOfWeek ("Tuesday"),   2, "Tuesday == 2");
    t.is (ISO8601d::dayOfWeek ("Wednesday"), 3, "Wednesday == 3");
    t.is (ISO8601d::dayOfWeek ("Thursday"),  4, "Thursday == 4");
    t.is (ISO8601d::dayOfWeek ("Friday"),    5, "Friday == 5");
    t.is (ISO8601d::dayOfWeek ("Saturday"),  6, "Saturday == 6");

    ISO8601d happyNewYear (1, 1, 2008);
    t.is (happyNewYear.dayOfWeek (), 2, "1/1/2008 == Tuesday");
    t.is (happyNewYear.month (),     1, "1/1/2008 == January");
    t.is (happyNewYear.day (),       1, "1/1/2008 == 1");
    t.is (happyNewYear.year (),   2008, "1/1/2008 == 2008");

    t.is (happyNewYear.toString (), "1/1/2008", "toString 1/1/2008");

    int m, d, y;
    happyNewYear.toMDY (m, d, y);
    t.is (m, 1, "1/1/2008 == January");
    t.is (d, 1, "1/1/2008 == 1");
    t.is (y, 2008, "1/1/2008 == 2008");

    ISO8601d epoch (9, 8, 2001);
    t.ok ((int)epoch.toEpoch () < 1000000000, "9/8/2001 < 1,000,000,000");
    epoch += 172800;
    t.ok ((int)epoch.toEpoch () > 1000000000, "9/10/2001 > 1,000,000,000");

    // int ISO8601d::length (const std::string&);
    t.is (ISO8601d::length ("m"), 2,  "length 'm' --> 2");
    t.is (ISO8601d::length ("M"), 2,  "length 'M' --> 2");
    t.is (ISO8601d::length ("d"), 2,  "length 'd' --> 2");
    t.is (ISO8601d::length ("D"), 2,  "length 'D' --> 2");
    t.is (ISO8601d::length ("y"), 2,  "length 'y' --> 2");
    t.is (ISO8601d::length ("Y"), 4,  "length 'Y' --> 4");
    t.is (ISO8601d::length ("a"), 3,  "length 'a' --> 3");
    t.is (ISO8601d::length ("A"), 10, "length 'A' --> 10");
    t.is (ISO8601d::length ("b"), 3,  "length 'b' --> 3");
    t.is (ISO8601d::length ("B"), 10, "length 'B' --> 10");
    t.is (ISO8601d::length ("v"), 2,  "length 'v' --> 2");
    t.is (ISO8601d::length ("V"), 2,  "length 'V' --> 2");
    t.is (ISO8601d::length ("h"), 2,  "length 'h' --> 2");
    t.is (ISO8601d::length ("H"), 2,  "length 'H' --> 2");
    t.is (ISO8601d::length ("n"), 2,  "length 'n' --> 2");
    t.is (ISO8601d::length ("N"), 2,  "length 'N' --> 2");
    t.is (ISO8601d::length ("s"), 2,  "length 's' --> 2");
    t.is (ISO8601d::length ("S"), 2,  "length 'S' --> 2");
    t.is (ISO8601d::length ("j"), 3,  "length 'j' --> 3");
    t.is (ISO8601d::length ("J"), 3,  "length 'J' --> 3");

    t.is (ISO8601d::length (" "), 1, "length ' ' --> 1");
  }

  catch (const std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
