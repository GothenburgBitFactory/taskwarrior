////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
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
#ifndef INCLUDED_LOCATION
#define INCLUDED_LOCATION

#include <string>
#include <stdio.h>

class Location
{
public:
  Location ();                           // Default constructor
  Location (const std::string&);         // Default constructor
  Location (const Location&);            // Copy constructor
  Location& operator= (const Location&); // Assignment operator
  ~Location ();                          // Destructor

public:
  std::string path;
  FILE*       pending;
  FILE*       completed;
  FILE*       undo;
};

#endif
////////////////////////////////////////////////////////////////////////////////
