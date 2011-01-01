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
#ifndef INCLUDED_UI
#define INCLUDED_UI

#include <map>
#include <string>
#include "Layout.h"

class UI
{
public:
  UI ();
  UI (const UI&);
  UI& operator= (const UI&);
  ~UI ();

  void add (const std::string&, Layout*);
  void switchLayout (const std::string&);
  void initialize ();
  void deinitialize ();
  void interactive ();
  void recalc (int, int);
  bool event (int);

private:
  std::map <std::string, Layout*> layouts;
  Layout* current;      // Points to one of the layouts.
};

#endif
////////////////////////////////////////////////////////////////////////////////

