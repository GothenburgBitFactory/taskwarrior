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
#include <iostream>
#include <Context.h>
#include <color.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  const char* colors[] =
  {
    "off",
    "bold", "underline", "bold_underline",
    "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white",
    "bold_black", "bold_red", "bold_green", "bold_yellow", "bold_blue",
      "bold_magenta", "bold_cyan", "bold_white",
    "underline_black", "underline_red", "underline_green", "underline_yellow",
      "underline_blue", "underline_magenta", "underline_cyan", "underline_white",
    "bold_underline_black", "bold_underline_red", "bold_underline_green",
      "bold_underline_yellow", "bold_underline_blue", "bold_underline_magenta",
      "bold_underline_cyan", "bold_underline_white",
    "on_black", "on_red", "on_green", "on_yellow", "on_blue", "on_magenta",
      "on_cyan", "on_white",
    "on_bright_black", "on_bright_red", "on_bright_green", "on_bright_yellow",
      "on_bright_blue", "on_bright_magenta", "on_bright_cyan", "on_bright_white",
  };

  #define NUM_COLORS (sizeof (colors) / sizeof (colors[0]))

  UnitTest t (NUM_COLORS + 2);

  for (unsigned int i = 0; i < NUM_COLORS; ++i)
    t.is (std::string (colors[i]),
          Text::colorName (Text::colorCode (colors[i])),
          std::string ("round-trip ") + colors[i]);

  t.is (Text::colorName (Text::nocolor), "", "nocolor == \'\'");
  t.is (Text::colorCode (""), Text::nocolor, "\'\' == nocolor");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
