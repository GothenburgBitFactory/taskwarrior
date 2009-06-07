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
#include <string>
#include "Context.h"
#include "i18n.h"
#include "color.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
namespace Text
{

std::string colorName (color c)
{
  switch (c)
  {
  case nocolor:                 return "";
  case off:                     return context.stringtable.get (COLOR_OFF,               "off");

  case bold:                    return context.stringtable.get (COLOR_BOLD,              "bold");
  case underline:               return context.stringtable.get (COLOR_UL,                "underline");
  case bold_underline:          return context.stringtable.get (COLOR_B_UL,              "bold_underline");

  case black:                   return context.stringtable.get (COLOR_BLACK,             "black");
  case red:                     return context.stringtable.get (COLOR_RED,               "red");
  case green:                   return context.stringtable.get (COLOR_GREEN,             "green");
  case yellow:                  return context.stringtable.get (COLOR_YELLOW,            "yellow");
  case blue:                    return context.stringtable.get (COLOR_BLUE,              "blue");
  case magenta:                 return context.stringtable.get (COLOR_MAGENTA,           "magenta");
  case cyan:                    return context.stringtable.get (COLOR_CYAN,              "cyan");
  case white:                   return context.stringtable.get (COLOR_WHITE,             "white");

  case bold_black:              return context.stringtable.get (COLOR_B_BLACK,           "bold_black");
  case bold_red:                return context.stringtable.get (COLOR_B_RED,             "bold_red");
  case bold_green:              return context.stringtable.get (COLOR_B_GREEN,           "bold_green");
  case bold_yellow:             return context.stringtable.get (COLOR_B_YELLOW,          "bold_yellow");
  case bold_blue:               return context.stringtable.get (COLOR_B_BLUE,            "bold_blue");
  case bold_magenta:            return context.stringtable.get (COLOR_B_MAGENTA,         "bold_magenta");
  case bold_cyan:               return context.stringtable.get (COLOR_B_CYAN,            "bold_cyan");
  case bold_white:              return context.stringtable.get (COLOR_B_WHITE,           "bold_white");

  case underline_black:         return context.stringtable.get (COLOR_UL_BLACK,          "underline_black");
  case underline_red:           return context.stringtable.get (COLOR_UL_RED,            "underline_red");
  case underline_green:         return context.stringtable.get (COLOR_UL_GREEN,          "underline_green");
  case underline_yellow:        return context.stringtable.get (COLOR_UL_YELLOW,         "underline_yellow");
  case underline_blue:          return context.stringtable.get (COLOR_UL_BLUE,           "underline_blue");
  case underline_magenta:       return context.stringtable.get (COLOR_UL_MAGENTA,        "underline_magenta");
  case underline_cyan:          return context.stringtable.get (COLOR_UL_CYAN,           "underline_cyan");
  case underline_white:         return context.stringtable.get (COLOR_UL_WHITE,          "underline_white");

  case bold_underline_black:    return context.stringtable.get (COLOR_B_UL_BLACK,        "bold_underline_black");
  case bold_underline_red:      return context.stringtable.get (COLOR_B_UL_RED,          "bold_underline_red");
  case bold_underline_green:    return context.stringtable.get (COLOR_B_UL_GREEN,        "bold_underline_green");
  case bold_underline_yellow:   return context.stringtable.get (COLOR_B_UL_YELLOW,       "bold_underline_yellow");
  case bold_underline_blue:     return context.stringtable.get (COLOR_B_UL_BLUE,         "bold_underline_blue");
  case bold_underline_magenta:  return context.stringtable.get (COLOR_B_UL_MAGENTA,      "bold_underline_magenta");
  case bold_underline_cyan:     return context.stringtable.get (COLOR_B_UL_CYAN,         "bold_underline_cyan");
  case bold_underline_white:    return context.stringtable.get (COLOR_B_UL_WHITE,        "bold_underline_white");

  case on_black:                return context.stringtable.get (COLOR_ON_BLACK,          "on_black");
  case on_red:                  return context.stringtable.get (COLOR_ON_RED,            "on_red");
  case on_green:                return context.stringtable.get (COLOR_ON_GREEN,          "on_green");
  case on_yellow:               return context.stringtable.get (COLOR_ON_YELLOW,         "on_yellow");
  case on_blue:                 return context.stringtable.get (COLOR_ON_BLUE,           "on_blue");
  case on_magenta:              return context.stringtable.get (COLOR_ON_MAGENTA,        "on_magenta");
  case on_cyan:                 return context.stringtable.get (COLOR_ON_CYAN,           "on_cyan");
  case on_white:                return context.stringtable.get (COLOR_ON_WHITE,          "on_white");

  case on_bright_black:         return context.stringtable.get (COLOR_ON_BRIGHT_BLACK,   "on_bright_black");
  case on_bright_red:           return context.stringtable.get (COLOR_ON_BRIGHT_RED,     "on_bright_red");
  case on_bright_green:         return context.stringtable.get (COLOR_ON_BRIGHT_GREEN,   "on_bright_green");
  case on_bright_yellow:        return context.stringtable.get (COLOR_ON_BRIGHT_YELLOW,  "on_bright_yellow");
  case on_bright_blue:          return context.stringtable.get (COLOR_ON_BRIGHT_BLUE,    "on_bright_blue");
  case on_bright_magenta:       return context.stringtable.get (COLOR_ON_BRIGHT_MAGENTA, "on_bright_magenta");
  case on_bright_cyan:          return context.stringtable.get (COLOR_ON_BRIGHT_CYAN,    "on_bright_cyan");
  case on_bright_white:         return context.stringtable.get (COLOR_ON_BRIGHT_WHITE,   "on_bright_white");

  default: throw context.stringtable.get (COLOR_UNKNOWN, "Unknown color value");
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
color colorCode (const std::string& c)
{
       if (c == context.stringtable.get (COLOR_OFF,               "off"))                     return off;
  else if (c == context.stringtable.get (COLOR_BOLD,              "bold"))                    return bold;
  else if (c == context.stringtable.get (COLOR_UL,                "underline"))               return underline;
  else if (c == context.stringtable.get (COLOR_B_UL,              "bold_underline"))          return bold_underline;

  else if (c == context.stringtable.get (COLOR_BLACK,             "black"))                   return black;
  else if (c == context.stringtable.get (COLOR_RED,               "red"))                     return red;
  else if (c == context.stringtable.get (COLOR_GREEN,             "green"))                   return green;
  else if (c == context.stringtable.get (COLOR_YELLOW,            "yellow"))                  return yellow;
  else if (c == context.stringtable.get (COLOR_BLUE,              "blue"))                    return blue;
  else if (c == context.stringtable.get (COLOR_MAGENTA,           "magenta"))                 return magenta;
  else if (c == context.stringtable.get (COLOR_CYAN,              "cyan"))                    return cyan;
  else if (c == context.stringtable.get (COLOR_WHITE,             "white"))                   return white;

  else if (c == context.stringtable.get (COLOR_B_BLACK,           "bold_black"))              return bold_black;
  else if (c == context.stringtable.get (COLOR_B_RED,             "bold_red"))                return bold_red;
  else if (c == context.stringtable.get (COLOR_B_GREEN,           "bold_green"))              return bold_green;
  else if (c == context.stringtable.get (COLOR_B_YELLOW,          "bold_yellow"))             return bold_yellow;
  else if (c == context.stringtable.get (COLOR_B_BLUE,            "bold_blue"))               return bold_blue;
  else if (c == context.stringtable.get (COLOR_B_MAGENTA,         "bold_magenta"))            return bold_magenta;
  else if (c == context.stringtable.get (COLOR_B_CYAN,            "bold_cyan"))               return bold_cyan;
  else if (c == context.stringtable.get (COLOR_B_WHITE,           "bold_white"))              return bold_white;

  else if (c == context.stringtable.get (COLOR_UL_BLACK,          "underline_black"))         return underline_black;
  else if (c == context.stringtable.get (COLOR_UL_RED,            "underline_red"))           return underline_red;
  else if (c == context.stringtable.get (COLOR_UL_GREEN,          "underline_green"))         return underline_green;
  else if (c == context.stringtable.get (COLOR_UL_YELLOW,         "underline_yellow"))        return underline_yellow;
  else if (c == context.stringtable.get (COLOR_UL_BLUE,           "underline_blue"))          return underline_blue;
  else if (c == context.stringtable.get (COLOR_UL_MAGENTA,        "underline_magenta"))       return underline_magenta;
  else if (c == context.stringtable.get (COLOR_UL_CYAN,           "underline_cyan"))          return underline_cyan;
  else if (c == context.stringtable.get (COLOR_UL_WHITE,          "underline_white"))         return underline_white;

  else if (c == context.stringtable.get (COLOR_B_UL_BLACK,        "bold_underline_black"))    return bold_underline_black;
  else if (c == context.stringtable.get (COLOR_B_UL_RED,          "bold_underline_red"))      return bold_underline_red;
  else if (c == context.stringtable.get (COLOR_B_UL_GREEN,        "bold_underline_green"))    return bold_underline_green;
  else if (c == context.stringtable.get (COLOR_B_UL_YELLOW,       "bold_underline_yellow"))   return bold_underline_yellow;
  else if (c == context.stringtable.get (COLOR_B_UL_BLUE,         "bold_underline_blue"))     return bold_underline_blue;
  else if (c == context.stringtable.get (COLOR_B_UL_MAGENTA,      "bold_underline_magenta"))  return bold_underline_magenta;
  else if (c == context.stringtable.get (COLOR_B_UL_CYAN,         "bold_underline_cyan"))     return bold_underline_cyan;
  else if (c == context.stringtable.get (COLOR_B_UL_WHITE,        "bold_underline_white"))    return bold_underline_white;

  else if (c == context.stringtable.get (COLOR_ON_BLACK,          "on_black"))                return on_black;
  else if (c == context.stringtable.get (COLOR_ON_RED,            "on_red"))                  return on_red;
  else if (c == context.stringtable.get (COLOR_ON_GREEN,          "on_green"))                return on_green;
  else if (c == context.stringtable.get (COLOR_ON_YELLOW,         "on_yellow"))               return on_yellow;
  else if (c == context.stringtable.get (COLOR_ON_BLUE,           "on_blue"))                 return on_blue;
  else if (c == context.stringtable.get (COLOR_ON_MAGENTA,        "on_magenta"))              return on_magenta;
  else if (c == context.stringtable.get (COLOR_ON_CYAN,           "on_cyan"))                 return on_cyan;
  else if (c == context.stringtable.get (COLOR_ON_WHITE,          "on_white"))                return on_white;

  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_BLACK,   "on_bright_black"))         return on_bright_black;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_RED,     "on_bright_red"))           return on_bright_red;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_GREEN,   "on_bright_green"))         return on_bright_green;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_YELLOW,  "on_bright_yellow"))        return on_bright_yellow;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_BLUE,    "on_bright_blue"))          return on_bright_blue;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_MAGENTA, "on_bright_magenta"))       return on_bright_magenta;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_CYAN,    "on_bright_cyan"))          return on_bright_cyan;
  else if (c == context.stringtable.get (COLOR_ON_BRIGHT_WHITE,   "on_bright_white"))         return on_bright_white;

  return nocolor;
}

////////////////////////////////////////////////////////////////////////////////
std::string decode (color c)
{
  switch (c)
  {
  case nocolor:                 return "";
  case off:                     return "\033[0m";

  case bold:                    return "\033[1m";
  case underline:               return "\033[4m";
  case bold_underline:          return "\033[1;4m";

  case black:                   return "\033[30m";
  case red:                     return "\033[31m";
  case green:                   return "\033[32m";
  case yellow:                  return "\033[33m";
  case blue:                    return "\033[34m";
  case magenta:                 return "\033[35m";
  case cyan:                    return "\033[36m";
  case white:                   return "\033[37m";

  case bold_black:              return "\033[90m";
  case bold_red:                return "\033[91m";
  case bold_green:              return "\033[92m";
  case bold_yellow:             return "\033[93m";
  case bold_blue:               return "\033[94m";
  case bold_magenta:            return "\033[95m";
  case bold_cyan:               return "\033[96m";
  case bold_white:              return "\033[97m";

  case underline_black:         return "\033[4;30m";
  case underline_red:           return "\033[4;31m";
  case underline_green:         return "\033[4;32m";
  case underline_yellow:        return "\033[4;33m";
  case underline_blue:          return "\033[4;34m";
  case underline_magenta:       return "\033[4;35m";
  case underline_cyan:          return "\033[4;36m";
  case underline_white:         return "\033[4;37m";

  case bold_underline_black:    return "\033[1;4;30m";
  case bold_underline_red:      return "\033[1;4;31m";
  case bold_underline_green:    return "\033[1;4;32m";
  case bold_underline_yellow:   return "\033[1;4;33m";
  case bold_underline_blue:     return "\033[1;4;34m";
  case bold_underline_magenta:  return "\033[1;4;35m";
  case bold_underline_cyan:     return "\033[1;4;36m";
  case bold_underline_white:    return "\033[1;4;37m";

  case on_black:                return "\033[40m";
  case on_red:                  return "\033[41m";
  case on_green:                return "\033[42m";
  case on_yellow:               return "\033[43m";
  case on_blue:                 return "\033[44m";
  case on_magenta:              return "\033[45m";
  case on_cyan:                 return "\033[46m";
  case on_white:                return "\033[47m";

  case on_bright_black:         return "\033[100m";
  case on_bright_red:           return "\033[101m";
  case on_bright_green:         return "\033[102m";
  case on_bright_yellow:        return "\033[103m";
  case on_bright_blue:          return "\033[104m";
  case on_bright_magenta:       return "\033[105m";
  case on_bright_cyan:          return "\033[106m";
  case on_bright_white:         return "\033[107m";

  default: throw context.stringtable.get (COLOR_UNKNOWN, "Unknown color value");
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string colorize (color fg, color bg, const std::string& input)
{
  if (input.length ())
    if (fg != nocolor || bg != nocolor)
      return decode (fg) + decode (bg) + input + decode (off);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorize (color fg, color bg)
{
  return decode (fg) + decode (bg);
}

////////////////////////////////////////////////////////////////////////////////
std::string colorize ()
{
  return decode (off);
}

////////////////////////////////////////////////////////////////////////////////
}
