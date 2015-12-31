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
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <i18n.h>

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
      if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];
      }
    }

    width = terminal_width;

    // Ncurses does this, and perhaps we need to as well, to avoid a problem on
    // Cygwin where the display goes right up to the terminal width, and causes
    // an odd color wrapping problem.
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
      if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];
      }
    }

    height = terminal_height;
  }

  return height;
}

////////////////////////////////////////////////////////////////////////////////
