////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#ifndef INCLUDED_DATE
#define INCLUDED_DATE

#include <stdio.h>
#include <string>

class Date;

class Date
{
public:
           Date ();
           Date (time_t);
           Date (const int, const int, const int);
           Date (const std::string&, const std::string& format = "m/d/Y");
           Date (const Date&);
  virtual ~Date ();

  void toEpoch (time_t&);
  time_t toEpoch ();
  std::string toEpochString ();
  void toMDY (int&, int&, int&);
  const std::string toString (const std::string& format = "m/d/Y") const;
  const std::string toStringWithTime (const std::string& format = "m/d/Y") const;
  static bool valid (const std::string&, const std::string& format = "m/d/Y");
  static bool valid (const int, const int, const int);

  static bool leapYear (int);
  static int daysInMonth (int, int);
  static std::string monthName (int);
  static void dayName (int, std::string&);
  static std::string dayName (int);
  static int weekOfYear (const std::string&);
  static int dayOfWeek (const std::string&);

  int month () const;
  int day () const;
  int year () const;
  int weekOfYear (int) const;
  int dayOfWeek () const;
  int hour () const;
  int minute () const;
  int second () const;

  bool operator== (const Date&);
  bool operator!= (const Date&);
  bool operator<  (const Date&);
  bool operator>  (const Date&);
  bool operator<= (const Date&);
  bool operator>= (const Date&);
  bool sameDay    (const Date&);
  bool sameMonth  (const Date&);
  bool sameYear   (const Date&);

  Date operator+  (const int);
  Date& operator+= (const int);
  Date& operator-= (const int);

  time_t operator- (const Date&);

private:
  bool isEpoch (const std::string&);
  bool isRelativeDate (const std::string&);

protected:
  time_t mT;
};

#endif

////////////////////////////////////////////////////////////////////////////////
