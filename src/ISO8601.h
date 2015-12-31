////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_ISO8601
#define INCLUDED_ISO8601

#include <Nibbler.h>
#include <time.h>

// Date
class ISO8601d
{
public:
  static std::string weekstart;
  static int minimumMatchLength;
  static bool isoEnabled;

  ISO8601d ();
  ISO8601d (const std::string&, const std::string& format = "");
  ISO8601d (time_t);
  ISO8601d (const int, const int, const int);
  ISO8601d (const int, const int, const int, const int, const int, const int);
  ~ISO8601d ();
  bool parse (const std::string&, std::string::size_type&, const std::string& format = "");

  time_t toEpoch () const;
  std::string toEpochString () const;
  std::string toISO () const;
  std::string toISOLocalExtended () const;
  double toJulian () const;
  void toMDY (int&, int&, int&) const;
  const std::string toString (const std::string& format = "m/d/Y") const;

  ISO8601d startOfDay () const;
  ISO8601d startOfWeek () const;
  ISO8601d startOfMonth () const;
  ISO8601d startOfYear () const;

  static bool valid (const std::string&, const std::string& format = "");
  static bool valid (const int, const int, const int, const int, const int, const int);
  static bool valid (const int, const int, const int);
  static bool valid (const int, const int);
  static bool leapYear (int);
  static int daysInMonth (int, int);
  static int daysInYear (int);
  static std::string monthName (int);
  static std::string monthNameShort (int);
  static std::string dayName (int);
  static std::string dayNameShort (int);
  static int dayOfWeek (const std::string&);
  static int dayOfWeek (int, int, int);
  static int monthOfYear (const std::string&);
  static int length (const std::string&);

  int month () const;
  int week () const;
  int day () const;
  int year () const;
  int weekOfYear (int) const;
  int dayOfWeek () const;
  int dayOfYear () const;
  int hour () const;
  int minute () const;
  int second () const;

  bool operator== (const ISO8601d&) const;
  bool operator!= (const ISO8601d&) const;
  bool operator<  (const ISO8601d&) const;
  bool operator>  (const ISO8601d&) const;
  bool operator<= (const ISO8601d&) const;
  bool operator>= (const ISO8601d&) const;
  bool sameHour   (const ISO8601d&) const;
  bool sameDay    (const ISO8601d&) const;
  bool sameWeek   (const ISO8601d&) const;
  bool sameMonth  (const ISO8601d&) const;
  bool sameYear   (const ISO8601d&) const;
  ISO8601d operator+  (const int);
  ISO8601d operator-  (const int);
  ISO8601d& operator+= (const int);
  ISO8601d& operator-= (const int);
  time_t operator- (const ISO8601d&);
  void operator-- ();    // Prefix
  void operator-- (int); // Postfix
  void operator++ ();    // Prefix
  void operator++ (int); // Postfix

private:
  void clear ();
  bool parse_formatted     (Nibbler&, const std::string&);
  bool parse_named         (Nibbler&);
  bool parse_epoch         (Nibbler&);
  bool parse_date_time     (Nibbler&);
  bool parse_date_time_ext (Nibbler&);
  bool parse_date_ext      (Nibbler&);
  bool parse_off_ext       (Nibbler&);
  bool parse_time_ext      (Nibbler&);
  bool parse_time_utc_ext  (Nibbler&);
  bool parse_time_off_ext  (Nibbler&);
  bool validate ();
  void resolve ();
  std::string dump () const;

public:
  int _year;
  int _month;
  int _week;
  int _weekday;
  int _julian;
  int _day;
  int _seconds;
  int _offset;
  bool _utc;
  time_t _date;
};

// Period
class ISO8601p
{
public:
  static bool isoEnabled;

  ISO8601p ();
  ISO8601p (time_t);
  ISO8601p (const std::string&);
  ~ISO8601p ();
  ISO8601p (const ISO8601p&);            // Unimplemented
  ISO8601p& operator= (const ISO8601p&);
  bool operator< (const ISO8601p&);
  bool operator> (const ISO8601p&);
  operator std::string () const;
  operator time_t () const;
  bool parse (const std::string&, std::string::size_type&);
  const std::string format () const;
  const std::string formatVague () const;

private:
  void clear ();
  bool parse_designated (Nibbler&);
  bool validate ();
  void resolve ();
  std::string dump () const;

public:
  int _year;
  int _month;
  int _day;
  int _hours;
  int _minutes;
  int _seconds;
  time_t _period;
};

#endif

////////////////////////////////////////////////////////////////////////////////
