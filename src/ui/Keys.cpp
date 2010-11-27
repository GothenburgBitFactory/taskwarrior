////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

#include <pthread.h>
#include "log.h"
#include "Keys.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
Keys::Keys ()
{
//  logWrite ("Keys::Keys");
}

////////////////////////////////////////////////////////////////////////////////
Keys::~Keys ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Keys::redraw ()
{
  logWrite ("Keys::redraw");

  pthread_mutex_lock (&conch);
  wbkgd (window, COLOR_PAIR (1));

  if (width >= 5)
  {
    wbkgdset (window, COLOR_PAIR(1) | A_BOLD);
    mvwaddstr (window, 0, 0, " q");

    wbkgdset (window, COLOR_PAIR(1));
    mvwaddstr (window, 0, 2, "uit");

    if (width >= 13)
    {
      wbkgdset (window, COLOR_PAIR(1) | A_BOLD);
      mvwaddstr (window, 0, 5, " d");

      wbkgdset (window, COLOR_PAIR(1));
      mvwaddstr (window, 0, 7, "efault");

      if (width >= 20)
      {
        wbkgdset (window, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr (window, 0, 13, " s");

        wbkgdset (window, COLOR_PAIR(1));
        mvwaddstr (window, 0, 15, "tats");
      }
    }
  }

  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
