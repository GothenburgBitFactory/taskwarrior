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
#ifndef INCLUDED_TDB
#define INCLUDED_TDB

class TDB
{
public:
  TDB ();                    // Default constructor
  TDB (const TDB&);            // Copy constructor
  TDB& operator= (const TDB&); // Assignment operator
  ~TDB ();                   // Destructor

/*
location (path to task dir)
std::vector <T> load (filter)
  caches all raw, including comments

update (T& old, T& new)
commit ()
  writes all, including comments

autoupgrade ()
  -> FF4
*/

private:
};

#endif
////////////////////////////////////////////////////////////////////////////////
