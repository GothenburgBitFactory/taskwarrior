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

#define L10N                                           // Localization complete.

#include <iostream>
#include <iomanip>
#include <sstream>
#include <Timer.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Timer starts when the object is constructed.
Timer::Timer ()
: _description ("-")
, _running (false)
, _total (0)
{
}

////////////////////////////////////////////////////////////////////////////////
// Timer starts when the object is constructed with a description.
Timer::Timer (const std::string& description)
: _description (description)
, _running (false)
, _total (0)
{
  start ();
}

////////////////////////////////////////////////////////////////////////////////
// Timer stops when the object is destructed.
Timer::~Timer ()
{
  stop ();

  std::stringstream s;
  s << "Timer " // No i18n
    << _description
    << " "
    << std::setprecision (6)
#ifndef HAIKU
    // Haiku fails on this - don't know why.
    << std::fixed
#endif
    << _total / 1000000.0
    << " sec";

  context.debug (s.str ());
}

////////////////////////////////////////////////////////////////////////////////
void Timer::start ()
{
  if (!_running)
  {
    gettimeofday (&_start, NULL);
    _running = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Timer::stop ()
{
  if (_running)
  {
    struct timeval end;
    gettimeofday (&end, NULL);
    _running = false;
    _total += (end.tv_sec - _start.tv_sec) * 1000000.0
            + (end.tv_usec - _start.tv_usec);
  }
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Timer::total () const
{
  return _total;
}

////////////////////////////////////////////////////////////////////////////////

