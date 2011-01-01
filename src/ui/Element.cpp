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

#include <stdlib.h>
#include <pthread.h>
#include "log.h"
#include "Element.h"
#include "Report.h"
#include "ReportStats.h"
#include "Title.h"
#include "Clock.h"
#include "Keys.h"
#include "Message.h"
#include "Stats.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
// Factory for Element construction, based on spec.
Element* Element::factory (const std::string& name)
{
//  logWrite ("Element::factory %s", name.c_str ());

  Element* e;
       if (name == "report")       e = new Report      ();  // TODO Remove.
  else if (name == "report.stats") e = new ReportStats ();
  else if (name == "title")        e = new Title       ();
  else if (name == "clock")        e = new Clock       ();
  else if (name == "keys")         e = new Keys        ();
  else if (name == "message")      e = new Message     ();
  else if (name == "stats")        e = new Stats       ();
  else
    throw std::string ("Unrecognized element type: '") + name + "'";

  e->type = name;
  return e;
}

////////////////////////////////////////////////////////////////////////////////
Element::Element ()
: left (0)
, top (0)
, width (0)
, height (0)
, type ("")
, window (NULL)
{
//  logWrite ("Element::Element");
}

////////////////////////////////////////////////////////////////////////////////
Element::~Element ()
{
  deinitialize ();
}

////////////////////////////////////////////////////////////////////////////////
// When a layout is selected, all the elements are initialized, which means the
// window is created.
void Element::initialize ()
{
//  logWrite ("Element::initialize %s", type.c_str ());

  window = newwin (height, width, top, left);
}

////////////////////////////////////////////////////////////////////////////////
// When a layout is switched out, the elements are deinitialized, which means
// the window is deleted.  The original specs are preserved, and ready for a
// possible later initialize if the layout is switched in.
void Element::deinitialize ()
{
//  logWrite ("Element::deinitialize %s", type.c_str ());

  if (window)
  {
    pthread_mutex_lock (&conch);
    delwin (window);
    pthread_mutex_unlock (&conch);

    window = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Notification of new size for stdscr, triggering a size recalc.
void Element::recalc (int l, int t, int w, int h)
{
  left   = l;
  top    = t;
  width  = w;
  height = h;
  logWrite ("Element::recalc %-7s [l%d,t%d,w%d,h%d]", type.c_str (), left, top, width, height);
}

////////////////////////////////////////////////////////////////////////////////
void Element::relocate ()
{
//  logWrite ("Element::relocate %s", type.c_str ());

  if (window)
  {
    pthread_mutex_lock (&conch);
    mvwin (window, top, left);
    wresize (window, height, width);
    pthread_mutex_unlock (&conch);

    logWrite ("Element::relocate %-7s [l%d,t%d,w%d,h%d]", type.c_str (), left, top, height, width);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Default event handler for Element.
bool Element::event (int e)
{
//  logWrite ("Element::event %s %d", type.c_str (), e);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Default repaint handler for Element.
void Element::redraw ()
{
  logWrite ("Element::redraw %s", type.c_str ());
}

////////////////////////////////////////////////////////////////////////////////
