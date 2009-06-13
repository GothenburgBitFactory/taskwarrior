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
#include "text.h"
#include "util.h"
#include "i18n.h"
#include "color.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
namespace Text
{

static struct
{
  color id;
  int string_id;
  std::string english_name;
  std::string escape_sequence;
} allColors[] =
{
//  Text::color             i18n.h                   English                   vt220?  xterm?
  { nocolor,                0,                       "",                       ""             },
  { off,                    COLOR_OFF,               "off",                    "[0m"        },

  { bold,                   COLOR_BOLD,              "bold",                   "\033[1m"      },
  { underline,              COLOR_UL,                "underline",              "\033[4m"      },
  { bold_underline,         COLOR_B_UL,              "bold_underline",         "\033[1;4m"    },

  { black,                  COLOR_BLACK,             "black",                  "\033[30m"     },
  { red,                    COLOR_RED,               "red",                    "\033[31m"     },
  { green,                  COLOR_GREEN,             "green",                  "\033[32m"     },
  { yellow,                 COLOR_YELLOW,            "yellow",                 "\033[33m"     },
  { blue,                   COLOR_BLUE,              "blue",                   "\033[34m"     },
  { magenta,                COLOR_MAGENTA,           "magenta",                "\033[35m"     },
  { cyan,                   COLOR_CYAN,              "cyan",                   "\033[36m"     },
  { white,                  COLOR_WHITE,             "white",                  "\033[37m"     },

  { bold_black,             COLOR_B_BLACK,           "bold_black",             "\033[90m"     },
  { bold_red,               COLOR_B_RED,             "bold_red",               "\033[91m"     },
  { bold_green,             COLOR_B_GREEN,           "bold_green",             "\033[92m"     },
  { bold_yellow,            COLOR_B_YELLOW,          "bold_yellow",            "\033[93m"     },
  { bold_blue,              COLOR_B_BLUE,            "bold_blue",              "\033[94m"     },
  { bold_magenta,           COLOR_B_MAGENTA,         "bold_magenta",           "\033[95m"     },
  { bold_cyan,              COLOR_B_CYAN,            "bold_cyan",              "\033[96m"     },
  { bold_white,             COLOR_B_WHITE,           "bold_white",             "\033[97m"     },

  { underline_black,        COLOR_UL_BLACK,          "underline_black",        "\033[4;30m"   },
  { underline_red,          COLOR_UL_RED,            "underline_red",          "\033[4;31m"   },
  { underline_green,        COLOR_UL_GREEN,          "underline_green",        "\033[4;32m"   },
  { underline_yellow,       COLOR_UL_YELLOW,         "underline_yellow",       "\033[4;33m"   },
  { underline_blue,         COLOR_UL_BLUE,           "underline_blue",         "\033[4;34m"   },
  { underline_magenta,      COLOR_UL_MAGENTA,        "underline_magenta",      "\033[4;35m"   },
  { underline_cyan,         COLOR_UL_CYAN,           "underline_cyan",         "\033[4;36m"   },
  { underline_white,        COLOR_UL_WHITE,          "underline_white",        "\033[4;37m"   },

  { bold_underline_black,   COLOR_B_UL_BLACK,        "bold_underline_black",   "\033[1;4;30m" },
  { bold_underline_red,     COLOR_B_UL_RED,          "bold_underline_red",     "\033[1;4;31m" },
  { bold_underline_green,   COLOR_B_UL_GREEN,        "bold_underline_green",   "\033[1;4;32m" },
  { bold_underline_yellow,  COLOR_B_UL_YELLOW,       "bold_underline_yellow",  "\033[1;4;33m" },
  { bold_underline_blue,    COLOR_B_UL_BLUE,         "bold_underline_blue",    "\033[1;4;34m" },
  { bold_underline_magenta, COLOR_B_UL_MAGENTA,      "bold_underline_magenta", "\033[1;4;35m" },
  { bold_underline_cyan,    COLOR_B_UL_CYAN,         "bold_underline_cyan",    "\033[1;4;36m" },
  { bold_underline_white,   COLOR_B_UL_WHITE,        "bold_underline_white",   "\033[1;4;37m" },

  { on_black,               COLOR_ON_BLACK,          "on_black",               "\033[40m"     },
  { on_red,                 COLOR_ON_RED,            "on_red",                 "\033[41m"     },
  { on_green,               COLOR_ON_GREEN,          "on_green",               "\033[42m"     },
  { on_yellow,              COLOR_ON_YELLOW,         "on_yellow",              "\033[43m"     },
  { on_blue,                COLOR_ON_BLUE,           "on_blue",                "\033[44m"     },
  { on_magenta,             COLOR_ON_MAGENTA,        "on_magenta",             "\033[45m"     },
  { on_cyan,                COLOR_ON_CYAN,           "on_cyan",                "\033[46m"     },
  { on_white,               COLOR_ON_WHITE,          "on_white",               "\033[47m"     },

  { on_bright_black,        COLOR_ON_BRIGHT_BLACK,   "on_bright_black",        "\033[100m"    },
  { on_bright_red,          COLOR_ON_BRIGHT_RED,     "on_bright_red",          "\033[101m"    },
  { on_bright_green,        COLOR_ON_BRIGHT_GREEN,   "on_bright_green",        "\033[102m"    },
  { on_bright_yellow,       COLOR_ON_BRIGHT_YELLOW,  "on_bright_yellow",       "\033[103m"    },
  { on_bright_blue,         COLOR_ON_BRIGHT_BLUE,    "on_bright_blue",         "\033[104m"    },
  { on_bright_magenta,      COLOR_ON_BRIGHT_MAGENTA, "on_bright_magenta",      "\033[105m"    },
  { on_bright_cyan,         COLOR_ON_BRIGHT_CYAN,    "on_bright_cyan",         "\033[106m"    },
  { on_bright_white,        COLOR_ON_BRIGHT_WHITE,   "on_bright_white",        "\033[107m"    },
};

#define NUM_COLORS (sizeof (allColors) / sizeof (allColors[0]))

////////////////////////////////////////////////////////////////////////////////
std::string colorName (color c)
{
  for (unsigned int i = 0; i < NUM_COLORS; ++i)
    if (allColors[i].id == c)
      return allColors[i].english_name;

  throw context.stringtable.get (COLOR_UNKNOWN, "Unknown color value");
  return "";
}

////////////////////////////////////////////////////////////////////////////////
color colorCode (const std::string& c)
{
  for (unsigned int i = 0; i < NUM_COLORS; ++i)
    if (context.stringtable.get (allColors[i].string_id, allColors[i].english_name) == c)
      return allColors[i].id;

  return nocolor;
}

////////////////////////////////////////////////////////////////////////////////
std::string decode (color c)
{
  for (unsigned int i = 0; i < NUM_COLORS; ++i)
    if (allColors[i].id == c)
      return allColors[i].escape_sequence;

  throw context.stringtable.get (COLOR_UNKNOWN, "Unknown color value");
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
std::string guessColor (const std::string& name)
{
  std::vector <std::string> all;
  for (unsigned int i = 0; i < NUM_COLORS; ++i)
    all.push_back (context.stringtable.get (
                     allColors[i].string_id,
                     allColors[i].english_name));

  std::vector <std::string> matches;
  autoComplete (name, all, matches);

  if (matches.size () == 0)
    throw std::string ("Unrecognized color '") + name + "'";

  else if (matches.size () != 1)
  {
    std::string error = "Ambiguous color '" + name + "' - could be either of "; // TODO i18n

    std::string combined;
    join (combined, ", ", matches);
    error += combined;

    throw error + combined;
  }

  return matches[0];
}

////////////////////////////////////////////////////////////////////////////////
}
