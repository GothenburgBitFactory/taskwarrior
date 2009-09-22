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
#ifndef INCLUDED_COLOR
#define INCLUDED_COLOR

#include <string>


////////////////////////////////////////////////////////////////////////////////
// TODO Define how these bits are used.
#define COLOR_256       0x00200000
#define COLOR_NOBG      0x00100000
#define COLOR_NOFG      0x00080000
#define COLOR_UNDERLINE 0x00040000
#define COLOR_BOLD      0x00020000
#define COLOR_BRIGHT    0x00010000
#define COLOR_BG        0x0000FF00
#define COLOR_FG        0x000000FF

class Color
{
public:
  enum color_id {nocolor = 0, black, red, blue, green, magenta, cyan, yellow, white};

  Color ();
  Color (const Color&);
  Color (unsigned int);                         // 256 | UNDERLINE | BOLD | BRIGHT | (BG << 8) | FG
  Color (const std::string&);                   // "red on bright black"
  Color (color_id, color_id, bool, bool, bool); // fg, bg, underline, bold, bright
  ~Color ();
  Color& operator= (const Color&);
  operator std::string ();
  operator int ();

  void blend (const Color&);

  std::string colorize (const std::string&);
  std::string colorize (const std::string&, const std::string&);

private:
  int find (const std::string&);
  std::string fg ();
  std::string bg ();

private:
  unsigned int value;
};

#endif

////////////////////////////////////////////////////////////////////////////////
