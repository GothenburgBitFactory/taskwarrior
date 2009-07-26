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
#ifndef INCLUDED_DURATION
#define INCLUDED_DURATION

#include <string>
#include <time.h>

class Duration
{
public:
  Duration ();                           // Default constructor
  Duration (time_t);                     // Default constructor
  Duration (const std::string&);         // Parse
  bool operator< (const Duration&);
  bool operator> (const Duration&);
  ~Duration ();                          // Destructor

  operator time_t ();
  operator std::string ();

  bool valid (const std::string&) const;
  void parse (const std::string&);

private:
  time_t mDays;
};

#endif
////////////////////////////////////////////////////////////////////////////////
