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

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "log.h"
#include "Clock.h"
#include "Context.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

extern Context context;

static int refreshDelay;  // Override with ui.clock.refresh

////////////////////////////////////////////////////////////////////////////////
// This thread sleeps and updates the time every second.
static void tick (int* arg)
{
  Clock* c = (Clock*) arg;
  while (c)
  {
    sleep (refreshDelay);

    // If the element is deinitialized the window goes away, but the thread does
    // not.  Protect against an attempted redraw of a dead window.
    if (c->window != NULL)
      c->redraw ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Create thread to track time.
Clock::Clock ()
{
  refreshDelay = context.config.getInteger ("ui.clock.refresh");
  logWrite ("Clock::Clock refreshDelay=%d", refreshDelay);
  pthread_create (&ticker, NULL, (void*(*)(void*)) tick, (void*) this);
}

////////////////////////////////////////////////////////////////////////////////
Clock::~Clock ()
{
  pthread_kill (ticker, SIGQUIT);
}

////////////////////////////////////////////////////////////////////////////////
void Clock::redraw ()
{
  logWrite ("Clock::redraw");

  time_t now;
  time (&now);
  struct tm* t = localtime (&now);

  // TODO Load these from config.
  const char* formats[] =
  {
    //                                  These should remain sorted from longest
    //                                  to shortest, in their expanded form,
    //                                  shown below.
    " %A %e %B %Y, wk%U, %H:%M:%S %z ", // " Sunday 18 October 2009, wk42, 22:42:39 -0400 "
    " %A %e %B %Y, wk%U, %H:%M:%S %Z ", // " Sunday 18 October 2009, wk42, 22:42:39 EDT "
     " %A %e %B %Y wk%U, %H:%M:%S %Z ", // " Sunday 18 October 2009 wk42, 22:42:39 EDT "
      " %A %e %B %Y wk%U %H:%M:%S %Z ", // " Sunday 18 October 2009 wk42 22:42:39 EDT "
    " %A %e %B %Y, wk%U, %H:%M:%S ",    // " Sunday 18 October 2009, wk42, 22:42:39 "
    " %A %e %B %Y, wk%U, %H:%M:%S ",    // " Sunday 18 October 2009, wk42, 22:42:39 "
     " %A %e %B %Y wk%U, %H:%M:%S ",    // " Sunday 18 October 2009 wk42, 22:42:39 "
      " %A %e %B %Y wk%U %H:%M:%S ",    // " Sunday 18 October 2009 wk42 22:42:39 "
    " %A %e %b %Y,wk%U,  %H:%M:%S ",    // " Sunday 18 Oct 2009, wk42, 22:42:39 "
     " %A %e %b %Y wk%U, %H:%M:%S ",    // " Sunday 18 Oct 2009 wk42, 22:42:39 "
      " %A %e %b %Y wk%U %H:%M:%S ",    // " Sunday 18 Oct 2009 wk42 22:42:39 "
    " %a %e %b %Y, wk%U, %H:%M:%S ",    // " Sun 18 Oct 2009, wk42, 22:42:39 "
     " %a %e %b %Y wk%U, %H:%M:%S ",    // " Sun 18 Oct 2009 wk42, 22:42:39 "
          " %a %e %b %Y, %H:%M:%S ",    // " Sun 18 Oct 2009, 22:42:39 "
           " %a %e %b %Y %H:%M:%S ",    // " Sun 18 Oct 2009 22:42:39 "
             " %a %e %b, %H:%M:%S ",    // " Sun 18 Oct, 22:42:39 "
              " %a %e %b %H:%M:%S ",    // " Sun 18 Oct 22:42:39 "
                " %e %b, %H:%M:%S ",    // " 18 Oct, 22:42:39 "
                 " %e %b %H:%M:%S ",    // " 18 Oct 22:42:39 "
                       " %H:%M:%S ",    // " 22:42:39 "
                        "%H:%M:%S",     // "22:42:39"
                       " %H:%M ",       // " 22:42 "
                        "%H:%M",        // "22:42"
                        "",
  };

  // TODO %U (Sun), $W (Mon)
  // TODO i18n "wk"

  // Render them all, but only keep the longest one that fits.
  std::string keeper = "";

  char buffer[128];
  for (int i = 0; formats[i][0]; ++i)
  {
    strftime (buffer, 128, formats[i], t);
    size_t len = strlen (buffer);
    if (len <= (size_t) width && len > keeper.length ())
      keeper = buffer;
  }

  pthread_mutex_lock (&conch);
  wbkgd (window, COLOR_PAIR (5));
  wbkgdset (window, COLOR_PAIR(5) | A_DIM);
  mvwaddstr (window, 0, width - keeper.length (), keeper.c_str ());
  wrefresh (window);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
