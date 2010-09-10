////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#ifndef INCLUDED_ELEMENT
#define INCLUDED_ELEMENT

#include <string>
#include <ncurses.h>
class Layout;

class Element
{
public:
  static Element* factory (const std::string&);

  Element ();
  Element (const Element&);            // Unimplemented
  Element& operator= (const Element&); // Unimplemented
  virtual ~Element ();

  virtual void initialize ();
  virtual void deinitialize ();

  virtual void recalc (int, int, int, int);
  virtual void relocate ();
  virtual bool event (int);
  virtual void redraw ();

public:
  // The actual dimensions of the window.  These change when stdscr resizes.
  int left;
  int top;
  int width;
  int height;
  std::string type;
  WINDOW* window;
};

#endif
////////////////////////////////////////////////////////////////////////////////

