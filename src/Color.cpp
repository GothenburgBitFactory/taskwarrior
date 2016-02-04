////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <Color.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <main.h>
#include <Lexer.h>
#include <text.h>
#include <i18n.h>
#include <string>

// uint to string lookup table for Color::_colorize()
// _colorize() gets called _a lot_, having this lookup table is a cheap
// performance optimization.
const char *colorstring[] = {
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9",
   "10",  "11",  "12",  "13",  "14",  "15",  "16",  "17",  "18",  "19",
   "20",  "21",  "22",  "23",  "24",  "25",  "26",  "27",  "28",  "29",
   "30",  "31",  "32",  "33",  "34",  "35",  "36",  "37",  "38",  "39",
   "40",  "41",  "42",  "43",  "44",  "45",  "46",  "47",  "48",  "49",
   "50",  "51",  "52",  "53",  "54",  "55",  "56",  "57",  "58",  "59",
   "60",  "61",  "62",  "63",  "64",  "65",  "66",  "67",  "68",  "69",
   "70",  "71",  "72",  "73",  "74",  "75",  "76",  "77",  "78",  "79",
   "80",  "81",  "82",  "83",  "84",  "85",  "86",  "87",  "88",  "89",
   "90",  "91",  "92",  "93",  "94",  "95",  "96",  "97",  "98",  "99",
  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
  "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
  "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
  "130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
  "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
  "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
  "160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
  "170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
  "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
  "190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
  "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
  "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
  "220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
  "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
  "240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
  "250", "251", "252", "253", "254", "255"
};

////////////////////////////////////////////////////////////////////////////////
static struct
{
  Color::color_id id;
  std::string english_name;
  int index;                    // offset red=3 (therefore fg=33, bg=43)
} allColors[] =
{
  // Color.h enum    English     Index
  { Color::nocolor,  "none",     0},
  { Color::black,    "black",    1}, // fg 29+0  bg 39+0
  { Color::red,      "red",      2},
  { Color::green,    "green",    3},
  { Color::yellow,   "yellow",   4},
  { Color::blue,     "blue",     5},
  { Color::magenta,  "magenta",  6},
  { Color::cyan,     "cyan",     7},
  { Color::white,    "white",    8},

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
  if (!(c & _COLOR_HASFG)) _value &= ~_COLOR_FG;
  if (!(c & _COLOR_HASBG)) _value &= ~_COLOR_BG;

  _value = c & (_COLOR_256 | _COLOR_HASBG | _COLOR_HASFG |_COLOR_UNDERLINE |
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
: _value (0)
{
  // Split spec into words.
  std::vector <std::string> words;
  split (words, spec, ' ');

  // Construct the color as two separate colors, then blend them later.  This
  // make it possible to declare a color such as "color1 on black", and have
  // the upgrade work properly.
  unsigned int fg_value = 0;
  unsigned int bg_value = 0;

  bool bg = false;
  int index;
  for (auto& word : words)
  {
    word = Lexer::lowerCase (Lexer::trim (word));

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
    else if (! word.compare (0, 4, "grey", 4) ||
             ! word.compare (0, 4, "gray", 4))
    {
      index = strtol (word.substr (4).c_str (), nullptr, 10);
      if (index < 0 || index > 23)
        throw format (STRING_COLOR_UNRECOGNIZED, word);

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
    else if (! word.compare (0, 3, "rgb", 3))
    {
      index = strtol (word.substr (3).c_str (), nullptr, 10);
      if (word.length () != 6 ||
          index < 0 || index > 555)
        throw format (STRING_COLOR_UNRECOGNIZED, word);

      int r = strtol (word.substr (3, 1).c_str (), nullptr, 10);
      int g = strtol (word.substr (4, 1).c_str (), nullptr, 10);
      int b = strtol (word.substr (5, 1).c_str (), nullptr, 10);
      if (r < 0 || r > 5 ||
          g < 0 || g > 5 ||
          b < 0 || b > 5)
        throw format (STRING_COLOR_UNRECOGNIZED, word);

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
    else if (! word.compare (0, 5, "color", 5))
    {
      index = strtol (word.substr (5).c_str (), nullptr, 10);
      if (index < 0 || index > 255)
        throw format (STRING_COLOR_UNRECOGNIZED, word);

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
      throw format (STRING_COLOR_UNRECOGNIZED, word);
  }

  // Now combine the fg and bg into a single color.
  _value = fg_value;
  blend (Color (bg_value));
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg)
: _value (0)
{
  if (fg != Color::nocolor)
  {
    _value |= _COLOR_HASFG;
    _value |= fg;
  }
}

////////////////////////////////////////////////////////////////////////////////
Color::Color (color_id fg, color_id bg, bool underline, bool bold, bool bright)
: _value (0)
{
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
}

////////////////////////////////////////////////////////////////////////////////
Color::operator std::string () const
{
  std::string description;
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
}

////////////////////////////////////////////////////////////////////////////////
void Color::upgrade ()
{
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
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::colorize (const std::string& input) const
{
  std::string result;
  _colorize (result, input);
  return result;
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
void Color::_colorize (std::string &result, const std::string& input) const
{
  if (!nontrivial ())
  {
    result += input;
    return;
  }

  int count = 0;

  // 256 color
  if (_value & _COLOR_256)
  {
    if (_value & _COLOR_UNDERLINE)
      result += "\033[4m";

    if (_value & _COLOR_INVERSE)
      result += "\033[7m";

    if (_value & _COLOR_HASFG)
    {
      result += "\033[38;5;";
      result += colorstring[(_value & _COLOR_FG)];
      result += "m";
    }

    if (_value & _COLOR_HASBG)
    {
      result += "\033[48;5;";
      result += colorstring[((_value & _COLOR_BG) >> 8)];
      result += "m";
    }

    result += input;
    result += "\033[0m";
  }

  // 16 color
  else
  {
    result += "\033[";

    if (_value & _COLOR_BOLD)
    {
      if (count++) result += ";";
      result += "1";
    }

    if (_value & _COLOR_UNDERLINE)
    {
      if (count++) result += ";";
      result += "4";
    }

    if (_value & _COLOR_INVERSE)
    {
      if (count++) result += ";";
      result += "7";
    }

    if (_value & _COLOR_HASFG)
    {
      if (count++) result += ";";
      result += colorstring[(29 + (_value & _COLOR_FG))];
    }

    if (_value & _COLOR_HASBG)
    {
      if (count++) result += ";";
      result += colorstring[((_value & _COLOR_BRIGHT ? 99 : 39) + ((_value & _COLOR_BG) >> 8))];
    }

    result += "m";
    result += input;
    result += "\033[0m";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Remove color codes from a string.
std::string Color::strip (const std::string& input)
{
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
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::colorize (const std::string& input, const std::string& spec)
{
  Color c (spec);
  return c.colorize (input);
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

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string Color::bg () const
{
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

  return "";
}

////////////////////////////////////////////////////////////////////////////////
