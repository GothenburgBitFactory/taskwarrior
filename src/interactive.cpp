////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "Context.h"
//#include "text.h"
//#include "util.h"
#include "main.h"
#include "i18n.h"
#include "../auto.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

////////////////////////////////////////////////////////////////////////////////
int Context::interactive ()
{
#ifdef HAVE_LIBNCURSES

  // TODO init ncurses
  // TODO create worker thread
  // TODO create refresh thread

  // TODO join refresh thread
  // TODO join worker thread
  // TODO take down ncurses

//  throw std::string ("unimplemented Context::interactive");

  // Fake interactive teaser...
#ifdef FEATURE_NCURSES_COLS
  initscr ();
  int width = COLS;
  int height = LINES;
#else
  WINDOW* w = initscr ();
  int width  = w->_maxx + 1;
  int height = w->_maxy + 1;
#endif

  (void) nonl ();
  (void) cbreak ();

  start_color ();
  init_pair (1, COLOR_WHITE, COLOR_BLUE);
  init_pair (2, COLOR_WHITE, COLOR_RED);
  init_pair (3, COLOR_CYAN, COLOR_BLUE);

  // Process commands.
  std::string command = "";
  int c;
  while (command != "quit")
  {
    // Render title.
    std::string title = "taskwarrior 3.0.0";
    while ((int) title.length () < width)
      title += " ";

    bkgdset (COLOR_PAIR (1));
    mvprintw (0, 0, title.c_str ());

    bkgdset (COLOR_PAIR (2));
    int line = height / 2;
    mvprintw (line,     width / 2 - 24, " I n t e r a c t i v e   t a s k w a r r i o r ");
    mvprintw (line + 1, width / 2 - 24, "            Coming in version 3.0.0            ");

    std::string footer = "Press 'q' to quit.";
    while ((int) footer.length () < width)
      footer = " " + footer;

    bkgdset (COLOR_PAIR (3));
    mvprintw (height - 1, 0, footer.c_str ());

    move (1, 0);
    refresh ();

    if ((c = getch ()) != ERR)
    {
      // 'Esc' and 'Enter' clear the accumulated commands.
      // TODO Looks like \n is not preserved by getch.
      if (c == 033 || c == '\n')
      {
        command = "";
      }

      else if (c == 'q')
      {
        command = "quit";
        break;
      }
    }
  }

  endwin ();
  return 0;

#else

  throw stringtable.get (INTERACTIVE_NO_NCURSES,
                         "Interactive taskwarrior is only available when built "
                         "with ncurses support.");

#endif
}

////////////////////////////////////////////////////////////////////////////////
int Context::getWidth ()
{
  // Determine window size, and set table accordingly.
  int width = config.getInteger ("defaultwidth");

#ifdef HAVE_LIBNCURSES
  if (config.getBoolean ("curses"))
  {
#ifdef FEATURE_NCURSES_COLS
    initscr ();
    width = COLS;
#else
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
#endif
    endwin ();

    std::stringstream out;
    out << "Context::getWidth: ncurses determined width of " << width << " characters";
    debug (out.str ());
  }
  else
    debug ("Context::getWidth: ncurses available but disabled.");
#else
  std::stringstream out;
  out << "Context::getWidth: no ncurses, using width of " << width << " characters";
  debug (out.str ());
#endif

  return width;
}

////////////////////////////////////////////////////////////////////////////////
int Context::getHeight ()
{
  // Determine window size, and set table accordingly.
  int height = 25; // TODO Is there a better number?

#ifdef HAVE_LIBNCURSES
  if (config.getBoolean ("curses"))
  {
#ifdef FEATURE_NCURSES_COLS
    initscr ();
    height = LINES;
#else
    WINDOW* w = initscr ();
    height = w->_maxy + 1;
#endif
    endwin ();

    std::stringstream out;
    out << "Context::getHeight: ncurses determined height of " << height << " characters";
    debug (out.str ());
  }
  else
    debug ("Context::getHeight: ncurses available but disabled.");
#else
  std::stringstream out;
  out << "Context::getHeight: no ncurses, using height of " << height << " characters";
  debug (out.str ());
#endif

  return height;
}

////////////////////////////////////////////////////////////////////////////////
