////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_DATE
#define INCLUDED_DATE

#include <stdio.h>
#include <vector>
#include <string>

class Date;

class Date
{
public:
           Date ();
           Date (time_t);
           Date (const int, const int, const int);
           Date (const int, const int, const int, const int, const int, const int);
           Date (const std::string&,
                 const std::string& format = "m/d/Y",
                 const bool iso = true,
                 const bool epoch = true);
           Date (const std::string&,
                 std::string::size_type&,
                 const std::string& format = "m/d/Y",
                 const bool iso = true,
                 const bool epoch = true);
           Date (const Date&);
  virtual ~Date ();

  void toEpoch (time_t&);
  time_t toEpoch ();
  std::string toEpochString ();
  std::string toISO ();
  double toJulian ();
  void toMDY (int&, int&, int&);
  const std::string toString (const std::string& format = "m/d/Y") const;

  Date startOfDay () const;
  Date startOfWeek () const;
  Date startOfMonth () const;
  Date startOfYear () const;

  static bool valid (const std::string&, const std::string& format = "m/d/Y");
  static bool valid (const int, const int, const int, const int, const int, const int);
  static bool valid (const int, const int, const int);
  static bool valid (const int, const int);

  static time_t easter (int year);
  static bool leapYear (int);
  static int daysInMonth (int, int);
  static int daysInYear (int);
  static std::string monthName (int);
  static void dayName (int, std::string&);
  static std::string dayName (int);
  static int dayOfWeek (const std::string&);
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

  bool operator== (const Date&) const;
  bool operator!= (const Date&) const;
  bool operator<  (const Date&) const;
  bool operator>  (const Date&) const;
  bool operator<= (const Date&) const;
  bool operator>= (const Date&) const;
  bool sameHour   (const Date&) const;
  bool sameDay    (const Date&) const;
  bool sameWeek   (const Date&) const;
  bool sameMonth  (const Date&) const;
  bool sameYear   (const Date&) const;

  Date operator+  (const int);
  Date operator-  (const int);
  Date& operator+= (const int);
  Date& operator-= (const int);

  time_t operator- (const Date&);

  void operator-- ();    // Prefix
  void operator-- (int); // Postfix
  void operator++ ();    // Prefix
  void operator++ (int); // Postfix

private:
  bool isEpoch (const std::string&);

protected:
  time_t _t;
};

#endif

////////////////////////////////////////////////////////////////////////////////
