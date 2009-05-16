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

#include <string>
#include "TDB.h"

////////////////////////////////////////////////////////////////////////////////
TDB::TDB ()
{
}

////////////////////////////////////////////////////////////////////////////////
TDB::TDB (const TDB& other)
{
  throw std::string ("unimplemented TDB::TDB");
  mLocations = other.mLocations;
}

////////////////////////////////////////////////////////////////////////////////
TDB& TDB::operator= (const TDB& other)
{
  throw std::string ("unimplemented TDB::operator=");
  if (this != &other)
  {
    mLocations = other.mLocations;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
TDB::~TDB ()
{
}

////////////////////////////////////////////////////////////////////////////////
void TDB::location (const std::string& path)
{
  throw std::string ("unimplemented TDB::location");
  mLocations.push_back (path);
}

////////////////////////////////////////////////////////////////////////////////
// TODO Returns number of filtered tasks.
int TDB::load (std::vector <T>& tasks, Filter& filter)
{
  throw std::string ("unimplemented TDB::load");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Write to transaction log.
void TDB::update (T& before, T& after)
{
  throw std::string ("unimplemented TDB::update");
}

////////////////////////////////////////////////////////////////////////////////
// TODO writes all, including comments
int TDB::commit ()
{
  throw std::string ("unimplemented TDB::commit");
}

////////////////////////////////////////////////////////////////////////////////
// TODO -> FF4
void TDB::upgrade ()
{
  throw std::string ("unimplemented TDB::upgrade");
}

////////////////////////////////////////////////////////////////////////////////
