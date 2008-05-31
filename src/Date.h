////////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
//
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
           Date (const std::string&);
           Date (const Date&);
  virtual ~Date ();

  void toEpoch (time_t&);
  time_t toEpoch ();
  void toMDY (int&, int&, int&);
  void toString (std::string&);
  std::string toString (void);
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
