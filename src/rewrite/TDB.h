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

#include <vector>
#include <string>
#include "Filter.h"
#include "T.h"

class TDB
{
public:
  TDB ();                      // Default constructor
  TDB (const TDB&);            // Copy constructor
  TDB& operator= (const TDB&); // Assignment operator
  ~TDB ();                     // Destructor

  void location (const std::string&);
  int load (std::vector <T>&, Filter&);
  void update (T&, T&);
  int commit ();
  void upgrade ();

  void noLock ();

private:
  void getPendingFiles (std::vector <std::string>);
  void getCompletedFiles (std::vector <std::string>);

private:
  std::vector <std::string> mLocations;
  bool mLock;

  // TODO Need cache of raw file contents.
};

#endif
////////////////////////////////////////////////////////////////////////////////
