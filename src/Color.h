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

#ifndef INCLUDED_COLOR
#define INCLUDED_COLOR

#include <string>

#define FEATURE_COLOR 1

////////////////////////////////////////////////////////////////////////////////
#define _COLOR_INVERSE   0x00400000  // Inverse attribute.
#define _COLOR_256       0x00200000  // 256-color mode.
#define _COLOR_HASBG     0x00100000  // Has background color (all values taken).
#define _COLOR_HASFG     0x00080000  // Has foreground color (all values taken).
#define _COLOR_UNDERLINE 0x00040000  // General underline attribute.
#define _COLOR_BOLD      0x00020000  // 16-color bold attribute.
#define _COLOR_BRIGHT    0x00010000  // 16-color bright background attribute.
#define _COLOR_BG        0x0000FF00  // 8-bit background color index.
#define _COLOR_FG        0x000000FF  // 8-bit foreground color index.

class Color
{
public:
  enum color_id {nocolor = 0, black, red, green, yellow, blue, magenta, cyan, white};

  Color ();
  Color (const Color&);
  Color (unsigned int);                         // 256 | INVERSE | UNDERLINE | BOLD | BRIGHT | (BG << 8) | FG
  Color (const std::string&);                   // "red on bright black"
  Color (color_id);                             // fg.
  Color (color_id, color_id);                   // fg, bg.
  Color (color_id, color_id, bool, bool, bool); // fg, bg, underline, bold, bright
  ~Color ();
  Color& operator= (const Color&);
  operator std::string () const;
  operator int () const;

  void upgrade ();
  void blend (const Color&);

  std::string colorize (const std::string&);
  static std::string colorize (const std::string&, const std::string&);
  static std::string strip (const std::string&);

  bool nontrivial () const;

private:
  int find (const std::string&);
  std::string fg () const;
  std::string bg () const;

private:
  unsigned int _value;
};

#endif

////////////////////////////////////////////////////////////////////////////////
