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
#include <iostream>
#include <iomanip>
#include <sstream>
#include "Timer.h"
#include "Context.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Timer starts when the object is constructed.
Timer::Timer (const std::string& description)
: mDescription (description)
{
  ::gettimeofday (&mStart, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// Timer stops when the object is destructed.
Timer::~Timer ()
{
  struct timeval end;
  ::gettimeofday (&end, NULL);

  std::stringstream s;
  s << "Timer " // No i18n
    << mDescription
    << " "
    << std::setprecision (6)
//    << std::fixed
    << ((end.tv_sec - mStart.tv_sec) + ((end.tv_usec - mStart.tv_usec )
       / 1000000.0))
    << " sec";

  context.debug (s.str ());
}

////////////////////////////////////////////////////////////////////////////////

