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

//#include <iostream>
#include <sstream>
//#include <pwd.h>
#include <stdio.h>
#include <sys/ioctl.h>
//#include <stdlib.h>
//#include <string.h>
#include "Context.h"
//#include "text.h"
//#include "util.h"
#include "main.h"
#include "i18n.h"
#include "../cmake.h"

////////////////////////////////////////////////////////////////////////////////
int Context::interactive ()
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int Context::getWidth ()
{
  // Determine window size.
  int width = config.getInteger ("defaultwidth");

  // A zero width value means 'infinity', which is approximated here by 2^16.
  if (width == 0)
    return 65536;

  if (config.getBoolean ("curses"))
  {
    if (terminal_width == 0 &&
        terminal_height == 0)
    {
      unsigned short buff[4];
      if (ioctl (fileno(stdout), TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];

        std::stringstream out;
        out << "Context::getWidth: determined width of " << width << " characters";
        debug (out.str ());
      }
    }

    width = terminal_width;
  }

  return width;
}

////////////////////////////////////////////////////////////////////////////////
int Context::getHeight ()
{
  int height = 24;

  if (config.getBoolean ("curses"))
  {
    if (terminal_width == 0 &&
        terminal_height == 0)
    {
      unsigned short buff[4];
      if (ioctl (fileno(stdout), TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];

        std::stringstream out;
        out << "Context::getWidth: determined height of " << terminal_height << " characters";
        debug (out.str ());
      }
    }

    height = terminal_height;
  }

  return height;
}

////////////////////////////////////////////////////////////////////////////////
