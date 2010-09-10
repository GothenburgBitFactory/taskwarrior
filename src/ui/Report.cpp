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
#include "Report.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
Report::Report ()
{
//  logWrite ("Report::Report");
}

////////////////////////////////////////////////////////////////////////////////
Report::~Report ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Report::event (int e)
{
  switch (e)
  {
  case KEY_MOUSE:
    {
      MEVENT m;
      getmouse (&m);
      logWrite ("Report::event KEY_MOUSE [%d,%d] %x (%s)",
                m.x,
                m.y,
                m.bstate,
                ((m.x >= left && m.x < left + width && m.y >= top && m.y < top + height) ? "hit" : "miss"));
    }
    return true;
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Report::redraw ()
{
  logWrite ("Report::redraw");

  pthread_mutex_lock (&conch);
  wbkgd (window, COLOR_PAIR (20));
  mvwaddstr (window, 0, 0, "default report");
  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
