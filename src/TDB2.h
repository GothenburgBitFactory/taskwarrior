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
#ifndef INCLUDED_TDB2
#define INCLUDED_TDB2

#include <map>
#include <vector>
#include <string>
#include <Location.h>
#include <Filter.h>
#include <T2.h>

// Length of longest line.
#define T_LINE_MAX 32768

class TDB2
{
public:
  TDB2 ();                      // Default constructor
  TDB2 (const TDB2&);            // Copy constructor
  TDB2& operator= (const TDB2&); // Assignment operator
  ~TDB2 ();                     // Destructor

  void  location (const std::string&);

  void  lock (bool lockFile = true);
  void  unlock ();

  int   load (std::vector <T2>&, Filter&);
  void  add (T2&);
  void  update (T2&, T2&);
  int   commit ();
  void  upgrade ();

private:
  FILE* openAndLock (const std::string&);

private:
  std::vector <Location> mLocations;
  bool mLock;
  bool mAllOpenAndLocked;

  // TODO Need cache of raw file contents to preserve comments.
};

#endif
////////////////////////////////////////////////////////////////////////////////
