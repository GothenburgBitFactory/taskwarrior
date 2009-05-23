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

#include <map>
#include <string>
#include "util.h"
#include "Sequence.h"

////////////////////////////////////////////////////////////////////////////////
Sequence::Sequence ()
{
}

////////////////////////////////////////////////////////////////////////////////
Sequence::Sequence (const Sequence& other)
{
  throw std::string ("unimplemented Sequence::Sequence");
}

////////////////////////////////////////////////////////////////////////////////
Sequence& Sequence::operator= (const Sequence& other)
{
  throw std::string ("unimplemented Sequence::operator=");
  if (this != &other)
  {
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Sequence::~Sequence ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Sequence::parse (const std::string& input)
{
  throw std::string ("unimplemented Sequence::parse");
}

////////////////////////////////////////////////////////////////////////////////
void Sequence::combine (const Sequence& other)
{
  // Create a map using the sequence elements as keys.  This will create a
  // unique list, with no duplicates.
  std::map <int, int> both;
  foreach (i, *this) both[*i] = 0;
  foreach (i, other) both[*i] = 0;

  // Now make a sequence out of the keys of the map.
  this->clear ();
  foreach (i, both)
    this->push_back (i->first);

  std::sort (this->begin (), this->end ());
}

////////////////////////////////////////////////////////////////////////////////
