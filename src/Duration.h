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

#ifndef INCLUDED_DURATION
#define INCLUDED_DURATION

#include <string>
#include <time.h>

class Duration
{
public:
  Duration ();                           // Default constructor
  Duration (time_t);                     // Constructor
  Duration (const std::string&);         // Parse
  ~Duration ();                          // Destructor
  Duration (const Duration&);            // Unimplemented
  bool operator< (const Duration&);
  bool operator> (const Duration&);
  Duration& operator= (const Duration&);
  operator time_t () const;
  operator std::string () const;
  std::string format () const;
  std::string formatCompact () const;
  std::string formatPrecise () const;
  std::string formatSeconds () const;
  std::string formatISO () const;

  bool parse (const std::string&, std::string::size_type&);

protected:
  time_t _secs;
};

#endif
////////////////////////////////////////////////////////////////////////////////
