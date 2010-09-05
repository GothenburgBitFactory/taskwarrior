////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

#include "Rectangle.h"

////////////////////////////////////////////////////////////////////////////////
Rectangle::Rectangle ()
: left (0)
, top (0)
, width (0)
, height (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Rectangle::Rectangle (int l, int t, int w, int h)
{
  left   = l;
  top    = t;
  width  = w;
  height = h;
}

////////////////////////////////////////////////////////////////////////////////
Rectangle::Rectangle (const Rectangle& other)
{
  *this = other;
}

////////////////////////////////////////////////////////////////////////////////
Rectangle& Rectangle::operator= (const Rectangle& other)
{
  if (this != &other)
  {
    left   = other.left;
    top    = other.top;
    width  = other.width;
    height = other.height;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Rectangle::operator== (const Rectangle& other) const
{
  if (left   == other.left   &&
      top    == other.top    &&
      width  == other.width  &&
      height == other.height)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Rectangle::operator!= (const Rectangle& other) const
{
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////
// Algorithm:
//   Find the conditions at which the rectangles do not intersect and then
//   negate the result.
bool Rectangle::intersects (const Rectangle& other) const
{
  return ! (other.left                   > left + width - 1 ||
            other.left + other.width - 1 < left             ||
            other.top                    > top + height - 1 ||
            other.top + other.height - 1 < top);
}

///////////////////////////////////////////////////////////////////////////////
bool Rectangle::contains (const Rectangle& other) const
{
  if (other.left                >= left         &&
      other.left + other.width  <= left + width &&
      other.top                 >= top          &&
      other.top  + other.height <= top + height)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Adjacent rectangles must not intersect, but must have at top/bottom,
// bottom/top, left/right or right/left adjacency.
bool Rectangle::adjacentTo (const Rectangle& other) const
{
  if (! this->intersects (other))
    if (top + height   == other.top                 ||
        top            == other.top  + other.height ||
        left + width   == other.left                ||
        left           == other.left + other.width)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////

