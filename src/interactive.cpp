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

//#include <iostream>
//#include <pwd.h>
//#include <stdlib.h>
//#include <string.h>
#include "Context.h"
//#include "text.h"
//#include "util.h"
//#include "task.h"
//#include "i18n.h"
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
  WINDOW* w = initscr ();
  int width  = w->_maxx + 1;
  int height = w->_maxy + 1;

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
    std::string title = "task 2.0.0";
    while ((int) title.length () < width)
      title += " ";

    bkgdset (COLOR_PAIR (1));
    mvprintw (0, 0, title.c_str ());

    bkgdset (COLOR_PAIR (2));
    int line = height / 2;
    mvprintw (line,     width / 2 - 14, " I n t e r a c t i v e   t a s k ");
    mvprintw (line + 1, width / 2 - 14, "     Coming in version 2.0.0     ");

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

  throw stringtable (INTERACTIVE_NO_NCURSES,
                     "Interactive task is only available when built with ncurses "
                     "support.");

#endif
}

////////////////////////////////////////////////////////////////////////////////
