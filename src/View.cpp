////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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

#include <iostream> // TODO Remove
#include <sstream>
#include <View.h>
#include <text.h>

////////////////////////////////////////////////////////////////////////////////
View::View ()
: _width (0)
, _left_margin (0)
, _odd (0)
, _even (0)
, _intra_padding (0)
, _intra_odd (0)
, _intra_even (0)
, _extra_padding (0)
, _extra_odd (0)
, _extra_even (0)
, _truncate (0)
, _lines (0)
{
}

////////////////////////////////////////////////////////////////////////////////
View::~View ()
{
}

////////////////////////////////////////////////////////////////////////////////
//       +-------+  +-------+  +-------+
//       |header |  |header |  |header |
// +--+--+-------+--+-------+--+-------+--+
// |ma|ex|cell   |in|cell   |in|cell   |ex|
// +--+--+-------+--+-------+--+-------+--+
// |ma|ex|cell   |in|cell   |in|cell   |ex|
// +--+--+-------+--+-------+--+-------+--+
// 
std::string View::render (std::vector <Task>& data, std::vector <int>& sequence)
{
  // Determine minimal, ideal column widths.
  std::vector <int> minimal;
  std::vector <int> ideal;

  std::vector <Column*>::iterator i;
  for (i = _columns.begin (); i != _columns.end (); ++i)
  {
    int global_min = characters ((*i)->getLabel ());
    int global_ideal = 0;

    std::vector <Task>::iterator d;
    for (d = data.begin (); d != data.end (); ++d)
    {
      int min;
      int ideal;
      (*i)->measure (*d, min, ideal);

      if (min > global_min)     global_min = min;
      if (ideal > global_ideal) global_ideal = ideal;
    }

    minimal.push_back (global_min);
    ideal.push_back (global_ideal);
  }

  std::string combined;
  join (combined, ",", minimal);
  std::cout << "# minimal " << combined << "\n";
  join (combined, ",", ideal);
  std::cout << "# ideal " << combined << "\n";

  // TODO Calculate final column widths.

  // TODO Compose column headers.

  // TODO Render column headers.

  // TODO Compose, render columns, in sequence.
  std::stringstream output;
  std::vector <int>::iterator s;
  for (s = sequence.begin (); s != sequence.end (); ++s)
  {
  }

  return output.str ();
}

////////////////////////////////////////////////////////////////////////////////

