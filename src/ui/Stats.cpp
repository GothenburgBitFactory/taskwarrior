////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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

#include <pthread.h>
#include "log.h"
#include "Stats.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
Stats::Stats ()
{
//  logWrite ("Stats::Stats");
}

////////////////////////////////////////////////////////////////////////////////
Stats::~Stats ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Stats::redraw ()
{
  logWrite ("Stats::redraw");

  pthread_mutex_lock (&conch);
  wbkgd (window, COLOR_PAIR(1) | A_DIM);

  // Allowing 5-digits for tasks.
  if (height >= 3)
  {
         if (width >= 19) mvwaddstr (window, 2, 6, "Waiting tasks");
    else if (width >= 13) mvwaddstr (window, 2, 6, "Waiting");
    else if (width >= 10) mvwaddstr (window, 2, 6, "Wait");
    else if (width >=  7) mvwaddstr (window, 2, 6, "W");
  }

  if (height >= 2)
  {
         if (width >= 21) mvwaddstr (window, 1, 6, "Completed tasks");
    else if (width >= 15) mvwaddstr (window, 1, 6, "Completed");
    else if (width >= 10) mvwaddstr (window, 1, 6, "Comp");
    else if (width >=  7) mvwaddstr (window, 1, 6, "C");
  }

       if (width >= 19) mvwaddstr (window, 0, 6, "Pending tasks");
  else if (width >= 13) mvwaddstr (window, 0, 6, "Pending");
  else if (width >= 10) mvwaddstr (window, 0, 6, "Pend");
  else if (width >=  7) mvwaddstr (window, 0, 6, "P");

  if (width >= 7)
  {
    wbkgdset (window, COLOR_PAIR(1) | A_BOLD);
                     mvwaddstr (window, 0, 0, "_____");
    if (height >= 2) mvwaddstr (window, 1, 0, "_____");
    if (height >= 3) mvwaddstr (window, 2, 0, "_____");
  }

  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
