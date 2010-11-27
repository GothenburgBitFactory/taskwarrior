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
#include "Message.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
Message::Message ()
{
//  logWrite ("Message::Message");
}

////////////////////////////////////////////////////////////////////////////////
Message::~Message ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Message::redraw ()
{
  logWrite ("Message::redraw");

  // Truncate the message, if necessary.
  std::string s = "Using database ~/.task";
  if (width - 1 < (int) s.length ())
    s = s.substr (0, width - 4) + "...";

  // Need at least space for blank + 1 char + ellipsis.
  if (width <= 5)
    s = "";

  pthread_mutex_lock (&conch);
  wbkgd (window, COLOR_PAIR (5));
  wbkgdset (window, COLOR_PAIR(5) | A_DIM);

  if (width <= 5)
    touchwin (window);

  mvwaddstr (window, 0, 1, s.c_str ());

  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
