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
#ifndef INCLUDED_RECTANGLE
#define INCLUDED_RECTANGLE

class Rectangle
{
public:
  Rectangle ();
  Rectangle (int, int, int, int);
  Rectangle (const Rectangle&);
  Rectangle& operator= (const Rectangle&);

  bool operator== (const Rectangle&) const;
  bool operator!= (const Rectangle&) const;
  bool intersects (const Rectangle&) const;
  bool contains   (const Rectangle&) const;
  bool adjacentTo (const Rectangle&) const;

public:
  int left;
  int top;
  int width;
  int height;
};

#endif

////////////////////////////////////////////////////////////////////////////////

