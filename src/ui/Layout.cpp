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
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include "log.h"
#include "Rectangle.h"
#include "Layout.h"

// Constriction point for ncurses calls.
extern pthread_mutex_t conch;

////////////////////////////////////////////////////////////////////////////////
Layout::Layout (const std::string& spec)
: definition (spec)
{
  logWrite ("Layout::Layout %s", spec.c_str ());

  // Invoke the resize just to extract the element names.
  void resize (const std::string&, const int, const int, std::string& name, std::map <std::string, Rectangle>&);
  std::map <std::string, Rectangle> defs;
  resize (definition, 80, 24, name, defs);
  logWrite ("Layout::Layout name=%s", name.c_str ());

  // Create an element for each panel in the definition.
  std::map <std::string, Rectangle>::iterator it;
  for (it = defs.begin (); it != defs.end (); ++it)
  {
    Element* e = Element::factory (it->first);
    elements[e->type] = e;
  }
}

////////////////////////////////////////////////////////////////////////////////
Layout::~Layout ()
{
  std::map <std::string, Element*>::iterator it;
  for (it = elements.begin (); it != elements.end (); ++it)
    delete it->second;

  elements.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void Layout::initialize ()
{
  logWrite ("Layout::initialize");

  // Iterate over elements[] (specs) and create windows.
  std::map <std::string, Element*>::iterator it;
  for (it = elements.begin (); it != elements.end (); ++it)
    it->second->initialize ();
}

////////////////////////////////////////////////////////////////////////////////
void Layout::deinitialize ()
{
  logWrite ("Layout::deinitialize");

  std::map <std::string, Element*>::iterator it;
  for (it = elements.begin (); it != elements.end (); ++it)
    it->second->deinitialize ();

  // At this point the display should be stale but not cleared.
}

////////////////////////////////////////////////////////////////////////////////
bool Layout::event (int e)
{
  switch (e)
  {
  case 12:
    logWrite ("Layout::event Ctrl-L");
    this->redraw (true);
    return true;        // handled.
    break;

  default:
    logWrite ("Layout::event %d delegated", e);
    {
      std::map <std::string, Element*>::iterator it;
      for (it = elements.begin (); it != elements.end (); ++it)
        it->second->event (e);
    }
    return true;        // handled.
    break;
  }

  return false;         // not handled.
}

////////////////////////////////////////////////////////////////////////////////
void Layout::recalc (int w, int h)
{
  logWrite ("Layout::recalc %d,%d", w, h);

  // Apply layout definition to [w,h], yielding rectangles for each element.
  void resize (const std::string&, const int, const int, std::string& name, std::map <std::string, Rectangle>&);
  std::string name;
  std::map <std::string, Rectangle> defs;
  resize (definition, w, h, name, defs);

  // Relocate each element.
  std::map <std::string, Rectangle>::iterator it;
  for (it = defs.begin (); it != defs.end (); ++it)
  {
    Element* e = elements[it->first];
    if (e)
    {
      e->recalc (
        it->second.left,
        it->second.top,
        it->second.width,
        it->second.height);
      e->relocate ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Redrawing the layout consists of touching and refreshing all the elements.
void Layout::redraw (bool force /* = false */)
{
  logWrite ("Layout::redraw");

  // This is the magic line that makes resize work.  I wish I knew why.
  refresh ();

  std::map <std::string, Element*>::iterator it;
  for (it = elements.begin (); it != elements.end (); ++it)
  {
    it->second->redraw ();
    if (force)
    {
      pthread_mutex_lock (&conch);
      touchwin (it->second->window);
      pthread_mutex_unlock (&conch);
    }

    pthread_mutex_lock (&conch);
    wrefresh (it->second->window);
    pthread_mutex_unlock (&conch);
  }
}

////////////////////////////////////////////////////////////////////////////////
