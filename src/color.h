////////////////////////////////////////////////////////////////////////////////
// Copyright 2008, Paul Beckingham.
// All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED_COLOR
#define INCLUDED_COLOR

namespace Text
{
  enum color
  {
    nocolor = 0,
    off,
             bold,         underline,         bold_underline,
    black,   bold_black,   underline_black,   bold_underline_black,   on_black,   on_bright_black,
    red,     bold_red,     underline_red,     bold_underline_red,     on_red,     on_bright_red,
    green,   bold_green,   underline_green,   bold_underline_green,   on_green,   on_bright_green,
    yellow,  bold_yellow,  underline_yellow,  bold_underline_yellow,  on_yellow,  on_bright_yellow,
    blue,    bold_blue,    underline_blue,    bold_underline_blue,    on_blue,    on_bright_blue,
    magenta, bold_magenta, underline_magenta, bold_underline_magenta, on_magenta, on_bright_magenta,
    cyan,    bold_cyan,    underline_cyan,    bold_underline_cyan,    on_cyan,    on_bright_cyan,
    white,   bold_white,   underline_white,   bold_underline_white,   on_white,   on_bright_white
  };

  std::string colorName (color);
  color colorCode (const std::string&);

  std::string colorize (color, color, const std::string& string);
  std::string colorize (color, color);
  std::string colorize ();
}

#endif

////////////////////////////////////////////////////////////////////////////////
