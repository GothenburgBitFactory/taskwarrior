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
#include <ctype.h>
#include "util.h"
#include "text.h"
#include "Sequence.h"

////////////////////////////////////////////////////////////////////////////////
Sequence::Sequence ()
{
}

////////////////////////////////////////////////////////////////////////////////
Sequence::Sequence (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Sequence::~Sequence ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Sequence::parse (const std::string& input)
{
  std::vector <std::string> ranges;
  split (ranges, input, ',');

  std::vector <std::string>::iterator it;
  for (it = ranges.begin (); it != ranges.end (); ++it)
  {
    std::vector <std::string> range;
    split (range, *it, '-');

    switch (range.size ())
    {
    case 1:
      {
        if (! validId (range[0]))
          throw std::string ("Invalid ID in sequence");

        int id = ::atoi (range[0].c_str ());
        this->push_back (id);
      }
      break;

    case 2:
      {
        if (! validId (range[0]) ||
            ! validId (range[1]))
          throw std::string ("Invalid ID in range");

        int low  = ::atoi (range[0].c_str ());
        int high = ::atoi (range[1].c_str ());
        if (low > high)
          throw std::string ("Inverted sequence range high-low");

        if (high - low >= SEQUENCE_MAX)
          throw std::string ("ID Range too large");

        for (int i = low; i <= high; ++i)
          this->push_back (i);
      }
      break;

    default:
      throw std::string ("Not a sequence.");
      break;
    }
  }
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
bool Sequence::validId (const std::string& input)
{
  if (input.length () == 0)
    return false;

  for (size_t i = 0; i < input.length (); ++i)
    if (!::isdigit (input[i]))
      return false;

  return true;
}

//////////////////////////////////////////////////////////////////////////////// 
