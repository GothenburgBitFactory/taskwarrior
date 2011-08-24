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

#define L10N                                           // Localization complete.

#include <sstream>
#include <stdio.h>
#include <sys/ioctl.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <cmake.h>

#ifdef SOLARIS
#include <sys/termios.h>
#endif

////////////////////////////////////////////////////////////////////////////////
int Context::getWidth ()
{
  // Determine window size.
  int width = config.getInteger ("defaultwidth");

  // A zero width value means 'infinity', which is approximated here by 2^16.
  if (width == 0)
    return 65536;

  if (config.getBoolean ("detection"))
  {
    if (terminal_width == 0 &&
        terminal_height == 0)
    {
      unsigned short buff[4];
      if (ioctl (fileno(stdout), TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];

        debug (format (STRING_INTERACTIVE_WIDTH, terminal_width));
      }
    }

    width = terminal_width;

    // Ncurses does this, and perhaps we need to as well, to avoid a problem on
    // Cygwin where the display goes right up to the terminal width, and causes
    // and odd color wrapping problem.
    if (config.getBoolean ("avoidlastcolumn"))
      --width;
  }

  return width;
}

////////////////////////////////////////////////////////////////////////////////
int Context::getHeight ()
{
  // Determine window size.
  int height = config.getInteger ("defaultheight");

  // A zero height value means 'infinity', which is approximated here by 2^16.
  if (height == 0)
    return 65536;

  if (config.getBoolean ("detection"))
  {
    if (terminal_width == 0 &&
        terminal_height == 0)
    {
      unsigned short buff[4];
      if (ioctl (fileno(stdout), TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];

        debug (format (STRING_INTERACTIVE_HEIGHT, terminal_height));
      }
    }

    height = terminal_height;
  }

  return height;
}

////////////////////////////////////////////////////////////////////////////////
