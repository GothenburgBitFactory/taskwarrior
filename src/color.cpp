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
#include "color.h"

////////////////////////////////////////////////////////////////////////////////
namespace Text
{

std::string colorName (color c)
{
  switch (c)
  {
  case nocolor:                 return "";
  case off:                     return "off";

  case bold:                    return "bold";
  case underline:               return "underline";
  case bold_underline:          return "bold_underline";

  case black:                   return "black";
  case red:                     return "red";
  case green:                   return "green";
  case yellow:                  return "yellow";
  case blue:                    return "blue";
  case magenta:                 return "magenta";
  case cyan:                    return "cyan";
  case white:                   return "white";

  case bold_black:              return "bold_black";
  case bold_red:                return "bold_red";
  case bold_green:              return "bold_green";
  case bold_yellow:             return "bold_yellow";
  case bold_blue:               return "bold_blue";
  case bold_magenta:            return "bold_magenta";
  case bold_cyan:               return "bold_cyan";
  case bold_white:              return "bold_white";

  case underline_black:         return "underline_black";
  case underline_red:           return "underline_red";
  case underline_green:         return "underline_green";
  case underline_yellow:        return "underline_yellow";
  case underline_blue:          return "underline_blue";
  case underline_magenta:       return "underline_magenta";
  case underline_cyan:          return "underline_cyan";
  case underline_white:         return "underline_white";

  case bold_underline_black:    return "bold_underline_black";
  case bold_underline_red:      return "bold_underline_red";
  case bold_underline_green:    return "bold_underline_green";
  case bold_underline_yellow:   return "bold_underline_yellow";
  case bold_underline_blue:     return "bold_underline_blue";
  case bold_underline_magenta:  return "bold_underline_magenta";
  case bold_underline_cyan:     return "bold_underline_cyan";
  case bold_underline_white:    return "bold_underline_white";

  case on_black:                return "on_black";
  case on_red:                  return "on_red";
  case on_green:                return "on_green";
  case on_yellow:               return "on_yellow";
  case on_blue:                 return "on_blue";
  case on_magenta:              return "on_magenta";
  case on_cyan:                 return "on_cyan";
  case on_white:                return "on_white";

  case on_bright_black:         return "on_bright_black";
  case on_bright_red:           return "on_bright_red";
  case on_bright_green:         return "on_bright_green";
  case on_bright_yellow:        return "on_bright_yellow";
  case on_bright_blue:          return "on_bright_blue";
  case on_bright_magenta:       return "on_bright_magenta";
  case on_bright_cyan:          return "on_bright_cyan";
  case on_bright_white:         return "on_bright_white";

  default: throw "Unknown Text::color value";
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
color colorCode (const std::string& c)
{
       if (c == "off")                     return off;
  else if (c == "bold")                    return bold;
  else if (c == "underline")               return underline;
  else if (c == "bold_underline")          return bold_underline;

  else if (c == "black")                   return black;
  else if (c == "red")                     return red;
  else if (c == "green")                   return green;
  else if (c == "yellow")                  return yellow;
  else if (c == "blue")                    return blue;
  else if (c == "magenta")                 return magenta;
  else if (c == "cyan")                    return cyan;
  else if (c == "white")                   return white;

  else if (c == "bold_black")              return bold_black;
  else if (c == "bold_red")                return bold_red;
  else if (c == "bold_green")              return bold_green;
  else if (c == "bold_yellow")             return bold_yellow;
  else if (c == "bold_blue")               return bold_blue;
  else if (c == "bold_magenta")            return bold_magenta;
  else if (c == "bold_cyan")               return bold_cyan;
  else if (c == "bold_white")              return bold_white;

  else if (c == "underline_black")         return underline_black;
  else if (c == "underline_red")           return underline_red;
  else if (c == "underline_green")         return underline_green;
  else if (c == "underline_yellow")        return underline_yellow;
  else if (c == "underline_blue")          return underline_blue;
  else if (c == "underline_magenta")       return underline_magenta;
  else if (c == "underline_cyan")          return underline_cyan;
  else if (c == "underline_white")         return underline_white;

  else if (c == "bold_underline_black")    return bold_underline_black;
  else if (c == "bold_underline_red")      return bold_underline_red;
  else if (c == "bold_underline_green")    return bold_underline_green;
  else if (c == "bold_underline_yellow")   return bold_underline_yellow;
  else if (c == "bold_underline_blue")     return bold_underline_blue;
  else if (c == "bold_underline_magenta")  return bold_underline_magenta;
  else if (c == "bold_underline_cyan")     return bold_underline_cyan;
  else if (c == "bold_underline_white")    return bold_underline_white;

  else if (c == "on_black")                return on_black;
  else if (c == "on_red")                  return on_red;
  else if (c == "on_green")                return on_green;
  else if (c == "on_yellow")               return on_yellow;
  else if (c == "on_blue")                 return on_blue;
  else if (c == "on_magenta")              return on_magenta;
  else if (c == "on_cyan")                 return on_cyan;
  else if (c == "on_white")                return on_white;

  else if (c == "on_bright_black")         return on_bright_black;
  else if (c == "on_bright_red")           return on_bright_red;
  else if (c == "on_bright_green")         return on_bright_green;
  else if (c == "on_bright_yellow")        return on_bright_yellow;
  else if (c == "on_bright_blue")          return on_bright_blue;
  else if (c == "on_bright_magenta")       return on_bright_magenta;
  else if (c == "on_bright_cyan")          return on_bright_cyan;
  else if (c == "on_bright_white")         return on_bright_white;

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

  default: throw "Unknown Text::color value";
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
