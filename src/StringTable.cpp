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

#include <sstream>
#include "StringTable.h"

////////////////////////////////////////////////////////////////////////////////
StringTable::StringTable ()
{
}

////////////////////////////////////////////////////////////////////////////////
StringTable::StringTable (const StringTable& other)
{
  throw std::string ("unimplemented StringTable::StringTable");
  mMapping = other.mMapping;
}

////////////////////////////////////////////////////////////////////////////////
StringTable& StringTable::operator= (const StringTable& other)
{
  throw std::string ("unimplemented StringTable::operator=");
  if (this != &other)
  {
    mMapping = other.mMapping;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
StringTable::~StringTable ()
{
}

////////////////////////////////////////////////////////////////////////////////
// [data.location] / language . XX
// UTF-8 encoding
//
//  123 This is the string
//  124 This is another string
//  ...
void StringTable::load (const std::string& file)
{
  // TODO Load the specified file.
}

////////////////////////////////////////////////////////////////////////////////
std::string StringTable::get (int id)
{
  // Return the right string.
  if (mMapping.find (id) != mMapping.end ())
    return mMapping[id];

  std::stringstream error;
  error << "MISSING " << id;
  return error.str ();
}

////////////////////////////////////////////////////////////////////////////////
