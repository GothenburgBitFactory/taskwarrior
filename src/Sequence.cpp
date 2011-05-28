////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <text.h>
#include <Context.h>
#include <Sequence.h>

extern Context context;

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
bool Sequence::valid (const std::string& input) const
{
  std::vector <std::string> ranges;
  split (ranges, input, ',');

  std::vector <std::string>::iterator it;
  for (it = ranges.begin (); it != ranges.end (); ++it)
  {
    std::vector <std::string> range;
    split (range, *it, '-');

    if (range.size () < 1 ||
        range.size () > 2)
      return false;

    if (range.size () <= 2 && !validId (range[0]))
      return false;

    if (range.size () == 2 && !validId (range[1]))
      return false;
  }

  return true;
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
          throw std::string ("Invalid ID in sequence.");

        int id = strtol (range[0].c_str (), NULL, 10);
        this->push_back (id);
      }
      break;

    case 2:
      {
        if (! validId (range[0]) ||
            ! validId (range[1]))
          throw std::string ("Invalid ID in range.");

        int low  = strtol (range[0].c_str (), NULL, 10);
        int high = strtol (range[1].c_str (), NULL, 10);
        if (low > high)
          throw std::string ("Inverted sequence range high-low.");

        if (high - low >= SEQUENCE_MAX)
          throw std::string ("ID Range too large.");

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
  std::vector <int>::iterator i1;
  for (i1 = this->begin (); i1 != this->end (); ++i1) both[*i1] = 0;

  std::vector <int>::const_iterator i2;
  for (i2 = other.begin (); i2 != other.end (); ++i2) both[*i2] = 0;

  // Now make a sequence out of the keys of the map.
  this->clear ();
  std::map <int, int>::iterator i3;
  for (i3 = both.begin (); i3 != both.end (); ++i3)
    this->push_back (i3->first);

  std::sort (this->begin (), this->end ());
}

////////////////////////////////////////////////////////////////////////////////
bool Sequence::validId (const std::string& input) const
{
  if (input.length () == 0)
    return false;

  return digitsOnly (input);
}

//////////////////////////////////////////////////////////////////////////////// 
