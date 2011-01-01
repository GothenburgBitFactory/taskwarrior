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

#include <ncurses.h>
#include <ctype.h>
#include <pthread.h>
#include "log.h"
#include "UI.h"

// Constriction point for ncurses calls.
pthread_mutex_t conch = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////////////////////////////
UI::UI ()
: current (NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
UI::~UI ()
{
  std::map <std::string, Layout*>::iterator it;
  for (it = layouts.begin (); it != layouts.end (); ++it)
    delete it->second;

  layouts.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void UI::add (const std::string& name, Layout* layout)
{
  logWrite ("UI::add %s", name.c_str ());
  layouts[name] = layout;

  // First layout added automatically becomes the current layout.  Subsequent
  // layouts accumulate.
  if (current == NULL)
    current = layout;
}

////////////////////////////////////////////////////////////////////////////////
void UI::switchLayout (const std::string& name)
{
  logWrite ("UI::switchLayout %s", name.c_str ());

  if (layouts.find (name) == layouts.end ())
    throw std::string ("Cannot switch to non-existent layout '") + name + "'";

  // Only switch if the proposed layout is not the current layout.
  if (layouts[name] != current)
  {
    // Close the old windows.
    current->deinitialize ();

    // Set the new current layout.
    current = layouts[name];

    // Create the new windows.
    current->initialize ();

    // Need a size recalc, because the new current layout may have been used
    // before at a different size.
    this->recalc (COLS, LINES);
  }
}

////////////////////////////////////////////////////////////////////////////////
void UI::initialize ()
{
  logWrite ("UI::initialize");

  // Rainbow.
  init_pair (1,  COLOR_WHITE,  COLOR_BLUE);
  init_pair (2,  COLOR_WHITE,  COLOR_RED);
  init_pair (3,  COLOR_BLACK,  COLOR_GREEN);
  init_pair (4,  COLOR_WHITE,  COLOR_MAGENTA);
  init_pair (5,  COLOR_BLACK,  COLOR_CYAN);
  init_pair (6,  COLOR_BLACK,  COLOR_YELLOW);
  init_pair (7,  COLOR_BLACK,  COLOR_WHITE);
  init_pair (8,  COLOR_CYAN,   COLOR_BLUE);
  init_pair (9,  COLOR_BLUE,   COLOR_CYAN);
  init_pair (10, COLOR_YELLOW, COLOR_BLUE);

//  init_pair (11, COLOR_GREEN,  COLOR_BLACK);

  // Plain.
  init_pair (20, COLOR_WHITE,  COLOR_BLACK);

  // Propagate to current layout.
  current->initialize ();
}

////////////////////////////////////////////////////////////////////////////////
void UI::deinitialize ()
{
  logWrite ("UI::deinitialize");

  current->deinitialize ();
}

////////////////////////////////////////////////////////////////////////////////
void UI::interactive ()
{
  logWrite ("UI::interactive");

  if (!current)
    throw std::string ("Cannot start interactive mode without an initial layout.");

  initscr ();
  logWrite ("UI::interactive ncurses started");
  refresh ();  // Blank screen.
  curs_set (0);

  if (has_colors ())
    start_color ();

  this->recalc (COLS, LINES);
  this->initialize ();
  current->redraw ();

  keypad (stdscr, TRUE);
  noecho ();
  nl ();
  raw ();
  cbreak ();

  mousemask (ALL_MOUSE_EVENTS, NULL);

  while (this->event (getch ()))
    ;

  this->deinitialize ();
  endwin ();
  logWrite ("UI::interactive ncurses stopped");
}

////////////////////////////////////////////////////////////////////////////////
bool UI::event (int e)
{
  switch (e)
  {
  case 'd':        // Default layout
    switchLayout ("default");
    this->recalc (COLS, LINES);
    current->redraw (true);
    break;

  case 's':        // report.stats layout
    switchLayout ("report.stats");
    this->recalc (COLS, LINES);
    current->redraw (true);
    break;

  case 'Q':        // Quit.
  case 'q':        // quit.
    logWrite ("UI::event %c", (char)e);
    return false;
    break;

  case ERR:        // No need to propagate this.
    logWrite ("UI::event ERR ignored");
    break;

  case KEY_RESIZE: // This gets propagated through UI::recalc.
    logWrite ("UI::event KEY_RESIZE");
    this->recalc (COLS, LINES);
    current->redraw (true); // TODO has no apparent effect.
    break;

  default:         // Unhandled events are propagated.
                   // Ctrl-L is handled by the layout.
    logWrite ("UI::event %d delegated", e);
    current->event (e);
    break;
  }

  return true;     // Continue;
}

////////////////////////////////////////////////////////////////////////////////
// This is called at the beginning, to calculate element sizes, and on every
// window resize.
void UI::recalc (int w, int h)
{
  logWrite ("UI::recalc %d,%d", w, h);

  // The current layout needs to know.
  current->recalc (w, h);

  // Park the cursor.
  // TODO Evaluate whether this is needed.
  pthread_mutex_lock (&conch);
  move (h - 1, w - 1);
  pthread_mutex_unlock (&conch);
}

////////////////////////////////////////////////////////////////////////////////
