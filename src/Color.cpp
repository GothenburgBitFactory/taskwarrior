////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <main.h>
#include <Color.h>
#include <text.h>
#include <i18n.h>

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
: _value (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (const Color& other)
{
  _value = other._value;
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (unsigned int c)
: _value (0)
{
#ifdef FEATURE_COLOR
  if (!(c & _COLOR_HASFG)) _value &= ~_COLOR_FG;
  if (!(c & _COLOR_HASBG)) _value &= ~_COLOR_BG;

  _value = c & (_COLOR_256 | _COLOR_HASBG | _COLOR_HASFG |_COLOR_UNDERLINE |
                _COLOR_INVERSE | _COLOR_BOLD | _COLOR_BRIGHT | _COLOR_BG |
                _COLOR_FG);
#endif
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
: _value (0)
{
#ifdef FEATURE_COLOR
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
        throw format (STRING_COLOR_UNRECOGNIZED, *it);

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
        throw format (STRING_COLOR_UNRECOGNIZED, *it);

      int r = atoi (word.substr (3, 1).c_str ());
      int g = atoi (word.substr (4, 1).c_str ());
      int b = atoi (word.substr (5, 1).c_str ());
      if (r < 0 || r > 5 ||
          g < 0 || g > 5 ||
          b < 0 || b > 5)
        throw format (STRING_COLOR_UNRECOGNIZED, *it);

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
        throw format (STRING_COLOR_UNRECOGNIZED, *it);

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
      throw format (STRING_COLOR_UNRECOGNIZED, *it);
  }

  // Now combine the fg and bg into a single color.
  _value = fg_value;
  blend (Color (bg_value));
#endif
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg)
: _value (0)
{
#ifdef FEATURE_COLOR
  if (fg != Color::nocolor)
  {
    _value |= _COLOR_HASFG;
    _value |= fg;
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg, color_id bg)
: _value (0)
{
#ifdef FEATURE_COLOR
  if (bg != Color::nocolor)
  {
    _value |= _COLOR_HASBG;
    _value |= (bg << 8);
  }

  if (fg != Color::nocolor)
  {
    _value |= _COLOR_HASFG;
    _value |= fg;
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg, color_id bg, bool underline, bool bold, bool bright)
: _value (0)
{
#ifdef FEATURE_COLOR
  _value |= ((underline ? 1 : 0) << 18)
         |  ((bold      ? 1 : 0) << 17)
         |  ((bright    ? 1 : 0) << 16);

  if (bg != Color::nocolor)
  {
    _value |= _COLOR_HASBG;
    _value |= (bg << 8);
  }

  if (fg != Color::nocolor)
  {
    _value |= _COLOR_HASFG;
    _value |= fg;
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
Color::~Color ()
{
}

////////////////////////////////////////////////////////////////////////////////
Color& Color::operator= (const Color& other)
{
  if (this != &other)
    _value = other._value;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Color::operator std::string () const
{
  std::string description;
#ifdef FEATURE_COLOR
  if (_value & _COLOR_BOLD) description += "bold";

  if (_value & _COLOR_UNDERLINE)
    description += std::string (description.length () ? " " : "") + "underline";

  if (_value & _COLOR_INVERSE)
    description += std::string (description.length () ? " " : "") + "inverse";

  if (_value & _COLOR_HASFG)
    description += std::string (description.length () ? " " : "") + fg ();

  if (_value & _COLOR_HASBG)
  {
    description += std::string (description.length () ? " " : "") + "on";

    if (_value & _COLOR_BRIGHT)
      description += std::string (description.length () ? " " : "") + "bright";

    description += " " + bg ();
  }
#endif

  return description;
}

////////////////////////////////////////////////////////////////////////////////
Color::operator int () const
{
  return (int) _value;
}

////////////////////////////////////////////////////////////////////////////////
// If 'other' has styles that are compatible, merge them into this.  Colors in
// other take precedence.
void Color::blend (const Color& other)
{
#ifdef FEATURE_COLOR
  if (!other.nontrivial ())
    return;

  Color c (other);
  _value |= (c._value & _COLOR_UNDERLINE);    // Always inherit underline.
  _value |= (c._value & _COLOR_INVERSE);      // Always inherit inverse.

  // 16 <-- 16.
  if (!(_value   & _COLOR_256) &&
      !(c._value & _COLOR_256))
  {
    _value |= (c._value & _COLOR_BOLD);       // Inherit bold.
    _value |= (c._value & _COLOR_BRIGHT);     // Inherit bright.

    if (c._value & _COLOR_HASFG)
    {
      _value |= _COLOR_HASFG;                 // There is now a color.
      _value &= ~_COLOR_FG;                   // Remove previous color.
      _value |= (c._value & _COLOR_FG);       // Apply other color.
    }

    if (c._value & _COLOR_HASBG)
    {
      _value |= _COLOR_HASBG;                 // There is now a color.
      _value &= ~_COLOR_BG;                   // Remove previous color.
      _value |= (c._value & _COLOR_BG);       // Apply other color.
    }

    return;
  }
  else
  {
    // Upgrade either color, if necessary.
    if (!(_value   & _COLOR_256)) upgrade ();
    if (!(c._value & _COLOR_256)) c.upgrade ();

    // 256 <-- 256.
    if (c._value & _COLOR_HASFG)
    {
      _value |= _COLOR_HASFG;                  // There is now a color.
      _value &= ~_COLOR_FG;                    // Remove previous color.
      _value |= (c._value & _COLOR_FG);        // Apply other color.
    }

    if (c._value & _COLOR_HASBG)
    {
      _value |= _COLOR_HASBG;                  // There is now a color.
      _value &= ~_COLOR_BG;                    // Remove previous color.
      _value |= (c._value & _COLOR_BG);        // Apply other color.
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////
void Color::upgrade ()
{
#ifdef FEATURE_COLOR
  if (!(_value & _COLOR_256))
  {
    if (_value & _COLOR_HASFG)
    {
      bool bold = _value & _COLOR_BOLD;
      unsigned int fg = _value & _COLOR_FG;
      _value &= ~_COLOR_FG;
      _value &= ~_COLOR_BOLD;
      _value |= (bold ? fg + 7 : fg - 1);
    }

    if (_value & _COLOR_HASBG)
    {
      bool bright = _value & _COLOR_BRIGHT;
      unsigned int bg = (_value & _COLOR_BG) >> 8;
      _value &= ~_COLOR_BG;
      _value &= ~_COLOR_BRIGHT;
      _value |= (bright ? bg + 7 : bg - 1) << 8;
    }

    _value |= _COLOR_256;
  }
#endif
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
#ifdef FEATURE_COLOR
  if (!nontrivial ())
    return input;

  int count = 0;
  std::stringstream result;

  // 256 color
  if (_value & _COLOR_256)
  {
    bool needTerminator = false;

    if (_value & _COLOR_UNDERLINE)
    {
      result << "\033[4m";
      needTerminator = true;
    }

    if (_value & _COLOR_INVERSE)
    {
      result << "\033[7m";
      needTerminator = true;
    }

    if (_value & _COLOR_HASFG)
    {
      result << "\033[38;5;" << (_value & _COLOR_FG) << "m";
      needTerminator = true;
    }

    if (_value & _COLOR_HASBG)
    {
      result << "\033[48;5;" << ((_value & _COLOR_BG) >> 8) << "m";
      needTerminator = true;
    }

    result << input;
    if (needTerminator)
      result << "\033[0m";

    return result.str ();
  }

  // 16 color
  else
  {
    result << "\033[";

    if (_value & _COLOR_BOLD)
    {
      if (count++) result << ";";
      result << "1";
    }

    if (_value & _COLOR_UNDERLINE)
    {
      if (count++) result << ";";
      result << "4";
    }

    if (_value & _COLOR_INVERSE)
    {
      if (count++) result << ";";
      result << "7";
    }

    if (_value & _COLOR_HASFG)
    {
      if (count++) result << ";";
      result << (29 + (_value & _COLOR_FG));
    }

    if (_value & _COLOR_HASBG)
    {
      if (count++) result << ";";
      result << ((_value & _COLOR_BRIGHT ? 99 : 39) + ((_value & _COLOR_BG) >> 8));
    }

    result << "m" << input << "\033[0m";
    return result.str ();
  }
#endif

  return input;
}

////////////////////////////////////////////////////////////////////////////////
// Remove color codes from a string.
std::string Color::strip (const std::string& input)
{
#ifdef FEATURE_COLOR
  int length = input.length ();
  bool inside = false;
  std::string output;
  for (int i = 0; i < length; ++i)
  {
    if (inside)
    {
      if (input[i] == 'm')
        inside = false;
    }
    else
    {
      if (input[i] == 033)
        inside = true;
      else
        output += input[i];
    }
  }

  return output;
#else
  return input;
#endif
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::colorize (const std::string& input, const std::string& spec)
{
#ifdef FEATURE_COLOR
  Color c (spec);
  return c.colorize (input);
#else
  return input;
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool Color::nontrivial () const
{
  return _value != 0 ? true : false;
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
#ifdef FEATURE_COLOR
  int index = _value & _COLOR_FG;

  if (_value & _COLOR_256)
  {
    if (_value & _COLOR_HASFG)
    {
      std::stringstream s;
      s << "color" << (_value & _COLOR_FG);
      return s.str ();
    }
  }
  else
  {
    for (unsigned int i = 0; i < NUM_COLORS; ++i)
      if (allColors[i].index == index)
        return allColors[i].english_name;
  }
#endif

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::bg () const
{
#ifdef FEATURE_COLOR
  int index = (_value & _COLOR_BG) >> 8;

  if (_value & _COLOR_256)
  {
    if (_value & _COLOR_HASBG)
    {
      std::stringstream s;
      s << "color" << ((_value & _COLOR_BG) >> 8);
      return s.str ();
    }
  }
  else
  {
    for (unsigned int i = 0; i < NUM_COLORS; ++i)
      if (allColors[i].index == index)
        return allColors[i].english_name;
  }
#endif

  return "";
}

////////////////////////////////////////////////////////////////////////////////
