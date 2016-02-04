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

#include <cmake.h>
#include <Timer.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Timer::Timer ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Timer starts when the object is constructed with a description.
Timer::Timer (const std::string& description)
: _description (description)
{
  start ();
}

////////////////////////////////////////////////////////////////////////////////
// Timer stops when the object is destructed.
Timer::~Timer ()
{
  stop ();

  std::stringstream s;
  s << "Timer "
    << _description
    << " "
    << std::setprecision (6)
    << std::fixed
    << _total / 1000000.0
    << " sec";

  context.debug (s.str ());
}

////////////////////////////////////////////////////////////////////////////////
void Timer::start ()
{
  if (!_running)
  {
    gettimeofday (&_start, nullptr);
    _running = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Timer::stop ()
{
  if (_running)
  {
    struct timeval end;
    gettimeofday (&end, nullptr);
    _running = false;
    _total += (end.tv_sec - _start.tv_sec) * 1000000
            + (end.tv_usec - _start.tv_usec);
  }
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Timer::total () const
{
  return _total;
}

////////////////////////////////////////////////////////////////////////////////
void Timer::subtract (unsigned long value)
{
  if (value > _total)
    _total = 0;
  else
    _total -= value;
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Timer::now ()
{
  struct timeval now;
  gettimeofday (&now, nullptr);
  return now.tv_sec * 1000000 + now.tv_usec;
}

////////////////////////////////////////////////////////////////////////////////
