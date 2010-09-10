////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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

#include <pthread.h>
#include "log.h"
#include "Title.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
Title::Title ()
{
//  logWrite ("Title::Title");
}

////////////////////////////////////////////////////////////////////////////////
Title::~Title ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Title::redraw ()
{
  logWrite ("Title::redraw [%d,%d]", width, height);
  pthread_mutex_lock (&conch);

  wbkgd (window, COLOR_PAIR (1) | ' ');
       if (width >= 21) mvwaddstr (window, 0, 1, "task 2.0.0 pre-alpha");
  else if (width >= 11) mvwaddstr (window, 0, 1, "task 2.0.0");
  else if (width >=  9) mvwaddstr (window, 0, 1, "task 2.0");
  else if (width >=  5) mvwaddstr (window, 0, 1, "task");

  if (height >= 2)
  {
    wbkgdset (window, COLOR_PAIR(8));
         if (width >= 23) mvwaddstr (window, 1, 1, "http://taskwarrior.org");
    else if (width >= 16) mvwaddstr (window, 1, 1, "taskwarrior.org");
  }

  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
