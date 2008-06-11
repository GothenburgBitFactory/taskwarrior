////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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

#include <string>

class Date;

class Date
{
public:
           Date ();
           Date (time_t);
           Date (const int, const int, const int);
           Date (const std::string&, const std::string format = "m/d/Y");
           Date (const Date&);
  virtual ~Date ();

  void toEpoch (time_t&);
  time_t toEpoch ();
  void toMDY (int&, int&, int&);
  std::string toString (const std::string& format = "m/d/Y");
  static bool valid (const int, const int, const int);

  static bool leapYear (int);
  static int daysInMonth (int, int);
  static std::string monthName (int);
  static void dayName (int, std::string&);
  static std::string dayName (int);
  int dayOfWeek () const;

  int month () const;
  int day () const;
  int year () const;

  bool operator== (const Date&);
  bool operator!= (const Date&);
  bool operator<  (const Date&);
  bool operator>  (const Date&);
  bool operator<= (const Date&);
  bool operator>= (const Date&);

  Date operator+  (const int);
  Date& operator+= (const int);
  Date& operator-= (const int);

  time_t operator- (const Date&);

protected:
  time_t mT;
};

#endif

////////////////////////////////////////////////////////////////////////////////
