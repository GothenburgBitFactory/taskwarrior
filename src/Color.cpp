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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Color.h"
#include "text.h"
#include "i18n.h"

////////////////////////////////////////////////////////////////////////////////
static struct
{
  Color::color_id id;
  int string_id;
  std::string english_name;
  int index;                    // offset red=3 (therefore fg=33, bg=43)
} allColors[] =
{
  // Color.h enum    i18n.h          English     Index
  { Color::nocolor,  0,              "none",     0},
  { Color::black,    CCOLOR_BLACK,   "black",    1}, // fg 29+0  bg 39+0
  { Color::red,      CCOLOR_RED,     "red",      2},
  { Color::green,    CCOLOR_GREEN,   "green",    3},
  { Color::yellow,   CCOLOR_YELLOW,  "yellow",   4},
  { Color::blue,     CCOLOR_BLUE,    "blue",     5},
  { Color::magenta,  CCOLOR_MAGENTA, "magenta",  6},
  { Color::cyan,     CCOLOR_CYAN,    "cyan",     7},
  { Color::white,    CCOLOR_WHITE,   "white",    8},

};

#define NUM_COLORS (sizeof (allColors) / sizeof (allColors[0]))

////////////////////////////////////////////////////////////////////////////////
Color::Color ()
: value (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (const Color& other)
{
  value = other.value;
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (unsigned int c)
: value (0)
{
  if (!(c & _COLOR_HASFG)) value &= ~_COLOR_FG;
  if (!(c & _COLOR_HASBG)) value &= ~_COLOR_BG;

  value = c & (_COLOR_256 | _COLOR_HASBG | _COLOR_HASFG |_COLOR_UNDERLINE |
               _COLOR_INVERSE | _COLOR_BOLD | _COLOR_BRIGHT | _COLOR_BG |
               _COLOR_FG);
}

////////////////////////////////////////////////////////////////////////////////
// Supports the following constructs:
//   [bright] [color] [on color] [bright] [underline]
//
// Where [color] is one of:
//   black
//   red
//   ...
//   grayN  0 <= N <= 23       fg 38;5;232 + N              bg 48;5;232 + N
//   greyN  0 <= N <= 23       fg 38;5;232 + N              bg 48;5;232 + N
//   colorN 0 <= N <= 255      fg 38;5;N                    bg 48;5;N
//   rgbRGB 0 <= R,G,B <= 5    fg 38;5;16 + R*36 + G*6 + B  bg 48;5;16 + R*36 + G*6 + B
Color::Color (const std::string& spec)
: value (0)
{
  // By converting underscores to spaces, we inherently support the old "on_red"
  // style of specifying background colors.  We consider underscores to be
  // deprecated.
  std::string modifiable_spec = spec;
  std::replace (modifiable_spec.begin (), modifiable_spec.end (), '_', ' ');

  // Split spec into words.
  std::vector <std::string> words;
  split (words, modifiable_spec, ' ');

  // Construct the color as two separate colors, then blend them later.  This
  // make it possible to declare a color such as "color1 on black", and have
  // the upgrade work properly.
  unsigned int fg_value = 0;
  unsigned int bg_value = 0;

  bool bg = false;
  int index;
  std::string word;
  std::vector <std::string>::iterator it;
  for (it = words.begin (); it != words.end (); ++it)
  {
    word = lowerCase (trim (*it));

         if (word == "bold")      fg_value |= _COLOR_BOLD;
    else if (word == "bright")    bg_value |= _COLOR_BRIGHT;
    else if (word == "underline") fg_value |= _COLOR_UNDERLINE;
    else if (word == "inverse")   fg_value |= _COLOR_INVERSE;
    else if (word == "on")        bg = true;

    // X where X is one of black, red, blue ...
    else if ((index = find (word)) != -1)
    {
      if (index)
      {
        if (bg)
        {
          bg_value |= _COLOR_HASBG;
          bg_value |= index << 8;
        }
        else
        {
          fg_value |= _COLOR_HASFG;
          fg_value |= index;
        }
      }
    }

    // greyN/grayN, where 0 <= N <= 23.
    else if (word.substr (0, 4) == "grey" ||
             word.substr (0, 4) == "gray")
    {
      index = atoi (word.substr (4).c_str ());
      if (index < 0 || index > 23)
        throw std::string ("The color '") + *it + "' is not recognized.";

      if (bg)
      {
        bg_value |= _COLOR_HASBG;
        bg_value |= (index + 232) << 8;
        bg_value |= _COLOR_256;
      }
      else
      {
        fg_value |= _COLOR_HASFG;
        fg_value |= index + 232;
        fg_value |= _COLOR_256;
      }
    }

    // rgbRGB, where 0 <= R,G,B <= 5.
    else if (word.substr (0, 3) == "rgb")
    {
      index = atoi (word.substr (3).c_str ());
      if (word.length () != 6 ||
          index < 0 || index > 555)
        throw std::string ("The color '") + *it + "' is not recognized.";

      int r = atoi (word.substr (3, 1).c_str ());
      int g = atoi (word.substr (4, 1).c_str ());
      int b = atoi (word.substr (5, 1).c_str ());
      if (r < 0 || r > 5 ||
          g < 0 || g > 5 ||
          b < 0 || b > 5)
        throw std::string ("The color '") + *it + "' is not recognized.";

      index = 16 + r*36 + g*6 + b;

      if (bg)
      {
        bg_value |= _COLOR_HASBG;
        bg_value |= index << 8;
        bg_value |= _COLOR_256;
      }
      else
      {
        fg_value |= _COLOR_HASFG;
        fg_value |= index;
        fg_value |= _COLOR_256;
      }
    }

    // colorN, where 0 <= N <= 255.
    else if (word.substr (0, 5) == "color")
    {
      index = atoi (word.substr (5).c_str ());
      if (index < 0 || index > 255)
        throw std::string ("The color '") + *it + "' is not recognized.";

      upgrade ();

      if (bg)
      {
        bg_value |= _COLOR_HASBG;
        bg_value |= index << 8;
        bg_value |= _COLOR_256;
      }
      else
      {
        fg_value |= _COLOR_HASFG;
        fg_value |= index;
        fg_value |= _COLOR_256;
      }
    }
    else if (word != "")
      throw std::string ("The color '") + *it + "' is not recognized.";
  }

  // Now combine the fg and bg into a single color.
  value = fg_value;
  blend (Color (bg_value));
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg)
: value (0)
{
  if (fg != Color::nocolor)
  {
    value |= _COLOR_HASFG;
    value |= fg;
  }
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg, color_id bg)
: value (0)
{
  if (bg != Color::nocolor)
  {
    value |= _COLOR_HASBG;
    value |= (bg << 8);
  }

  if (fg != Color::nocolor)
  {
    value |= _COLOR_HASFG;
    value |= fg;
  }
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg, color_id bg, bool underline, bool bold, bool bright)
: value (0)
{
  value |= ((underline ? 1 : 0) << 18)
        |  ((bold      ? 1 : 0) << 17)
        |  ((bright    ? 1 : 0) << 16);

  if (bg != Color::nocolor)
  {
    value |= _COLOR_HASBG;
    value |= (bg << 8);
  }

  if (fg != Color::nocolor)
  {
    value |= _COLOR_HASFG;
    value |= fg;
  }
}

////////////////////////////////////////////////////////////////////////////////
Color::~Color ()
{
}

////////////////////////////////////////////////////////////////////////////////
Color& Color::operator= (const Color& other)
{
  if (this != &other)
    value = other.value;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Color::operator std::string () const
{
  std::string description;
  if (value & _COLOR_BOLD) description += "bold";

  if (value & _COLOR_UNDERLINE)
    description += std::string (description.length () ? " " : "") + "underline";

  if (value & _COLOR_INVERSE)
    description += std::string (description.length () ? " " : "") + "inverse";

  if (value & _COLOR_HASFG)
    description += std::string (description.length () ? " " : "") + fg ();

  if (value & _COLOR_HASBG)
  {
    description += std::string (description.length () ? " " : "") + "on";

    if (value & _COLOR_BRIGHT)
      description += std::string (description.length () ? " " : "") + "bright";

    description += " " + bg ();
  }

  return description;
}

////////////////////////////////////////////////////////////////////////////////
Color::operator int () const
{
  return (int) value;
}

////////////////////////////////////////////////////////////////////////////////
// If 'other' has styles that are compatible, merge them into this.  Colors in
// other take precedence.
void Color::blend (const Color& other)
{
  Color c (other);
  value |= (c.value & _COLOR_UNDERLINE);    // Always inherit underline.
  value |= (c.value & _COLOR_INVERSE);      // Always inherit inverse.

  // 16 <-- 16.
  if (!(value   & _COLOR_256) &&
      !(c.value & _COLOR_256))
  {
    value |= (c.value & _COLOR_BOLD);       // Inherit bold.
    value |= (c.value & _COLOR_BRIGHT);     // Inherit bright.

    if (c.value & _COLOR_HASFG)
    {
      value |= _COLOR_HASFG;                // There is now a color.
      value &= ~_COLOR_FG;                  // Remove previous color.
      value |= (c.value & _COLOR_FG);       // Apply other color.
    }

    if (c.value & _COLOR_HASBG)
    {
      value |= _COLOR_HASBG;                // There is now a color.
      value &= ~_COLOR_BG;                  // Remove previous color.
      value |= (c.value & _COLOR_BG);       // Apply other color.
    }

    return;
  }
  else
  {
    // Upgrade either color, if necessary.
    if (!(value   & _COLOR_256)) upgrade ();
    if (!(c.value & _COLOR_256)) c.upgrade ();

    // 256 <-- 256.
    if (c.value & _COLOR_HASFG)
    {
      value |= _COLOR_HASFG;                  // There is now a color.
      value &= ~_COLOR_FG;                    // Remove previous color.
      value |= (c.value & _COLOR_FG);         // Apply other color.
    }

    if (c.value & _COLOR_HASBG)
    {
      value |= _COLOR_HASBG;                  // There is now a color.
      value &= ~_COLOR_BG;                    // Remove previous color.
      value |= (c.value & _COLOR_BG);         // Apply other color.
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Color::upgrade ()
{
  if (!(value & _COLOR_256))
  {
    if (value & _COLOR_HASFG)
    {
      bool bold = value & _COLOR_BOLD;
      unsigned int fg = value & _COLOR_FG;
      value &= ~_COLOR_FG;
      value &= ~_COLOR_BOLD;
      value |= (bold ? fg + 7 : fg - 1);
    }

    if (value & _COLOR_HASBG)
    {
      bool bright = value & _COLOR_BRIGHT;
      unsigned int bg = (value & _COLOR_BG) >> 8;
      value &= ~_COLOR_BG;
      value &= ~_COLOR_BRIGHT;
      value |= (bright ? bg + 7 : bg - 1) << 8;
    }

    value |= _COLOR_256;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Sample color codes:
//   red                  \033[31m
//   bold red             \033[91m
//   underline red        \033[4;31m
//   bold underline red   \033[1;4;31m
//
//   on red               \033[41m
//   on bright red        \033[101m
//
//   256 fg               \033[38;5;Nm
//   256 bg               \033[48;5;Nm
std::string Color::colorize (const std::string& input)
{
  if (value == 0)
    return input;

  int count = 0;
  std::stringstream result;

  // 256 color
  if (value & _COLOR_256)
  {
    bool needTerminator = false;

    if (value & _COLOR_UNDERLINE)
    {
      result << "\033[4m";
      needTerminator = true;
    }

    if (value & _COLOR_INVERSE)
    {
      result << "\033[7m";
      needTerminator = true;
    }

    if (value & _COLOR_HASFG)
    {
      result << "\033[38;5;" << (value & _COLOR_FG) << "m";
      needTerminator = true;
    }

    if (value & _COLOR_HASBG)
    {
      result << "\033[48;5;" << ((value & _COLOR_BG) >> 8) << "m";
      needTerminator = true;
    }

    result << input;
    if (needTerminator)
      result << "\033[0m";

    return result.str ();
  }

  // 16 color
  if (value != 0)
  {
    result << "\033[";

    if (value & _COLOR_BOLD)
    {
      if (count++) result << ";";
      result << "1";
    }

    if (value & _COLOR_UNDERLINE)
    {
      if (count++) result << ";";
      result << "4";
    }

    if (value & _COLOR_INVERSE)
    {
      if (count++) result << ";";
      result << "7";
    }

    if (value & _COLOR_HASFG)
    {
      if (count++) result << ";";
      result << (29 + (value & _COLOR_FG));
    }

    if (value & _COLOR_HASBG)
    {
      if (count++) result << ";";
      result << ((value & _COLOR_BRIGHT ? 99 : 39) + ((value & _COLOR_BG) >> 8));
    }

    result << "m" << input << "\033[0m";
    return result.str ();
  }

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::colorize (const std::string& input, const std::string& spec)
{
  Color c (spec);
  return c.colorize (input);
}

////////////////////////////////////////////////////////////////////////////////
bool Color::nontrivial ()
{
  return value != 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
int Color::find (const std::string& input)
{
  for (unsigned int i = 0; i < NUM_COLORS; ++i)
    if (allColors[i].english_name == input)
      return (int) i;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::fg () const
{
  int index = value & _COLOR_FG;

  if (value & _COLOR_256)
  {
    if (value & _COLOR_HASFG)
    {
      std::stringstream s;
      s << "color" << (value & _COLOR_FG);
      return s.str ();
    }
  }
  else
  {
    for (unsigned int i = 0; i < NUM_COLORS; ++i)
      if (allColors[i].index == index)
        return allColors[i].english_name;
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::bg () const
{
  int index = (value & _COLOR_BG) >> 8;

  if (value & _COLOR_256)
  {
    if (value & _COLOR_HASBG)
    {
      std::stringstream s;
      s << "color" << ((value & _COLOR_BG) >> 8);
      return s.str ();
    }
  }
  else
  {
    for (unsigned int i = 0; i < NUM_COLORS; ++i)
      if (allColors[i].index == index)
        return allColors[i].english_name;
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
