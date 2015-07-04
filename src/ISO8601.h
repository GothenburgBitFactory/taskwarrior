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
#ifndef INCLUDED_ISO8601
#define INCLUDED_ISO8601

#include <Nibbler.h>
#include <time.h>

// Date
class ISO8601d
{
public:
  ISO8601d ();
  ~ISO8601d ();
  ISO8601d (const ISO8601d&);                 // Unimplemented
  ISO8601d& operator= (const ISO8601d&);      // Unimplemented
  operator time_t () const;
  bool parse (const std::string&, std::string::size_type&);
  void clear ();
  void set_default_time (int, int, int);

private:
  bool parse_date_time     (Nibbler&);
  bool parse_date_time_ext (Nibbler&);
  bool parse_date_ext      (Nibbler&);
  bool parse_off_ext       (Nibbler&);
  bool parse_time_ext      (Nibbler&);
  bool parse_time_utc_ext  (Nibbler&);
  bool parse_time_off_ext  (Nibbler&);
  int dayOfWeek (int, int, int);
  bool validate ();
  void resolve ();

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
  time_t _value;

  int _default_seconds;
};

// Period
class ISO8601p
{
public:
  ISO8601p ();
  ~ISO8601p ();
  ISO8601p (const ISO8601p&);                 // Unimplemented
  ISO8601p& operator= (const ISO8601p&);      // Unimplemented
  operator time_t () const;
  bool parse (const std::string&, std::string::size_type&);
  void clear ();

private:
  bool parse_designated (Nibbler&);
  bool validate ();
  void resolve ();

public:
  int _year;
  int _month;
  int _day;
  int _hours;
  int _minutes;
  int _seconds;
  time_t _value;
};

// TODO Recurrence

#endif

////////////////////////////////////////////////////////////////////////////////
