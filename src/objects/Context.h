////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#ifndef INCLUDED_CONTEXT
#define INCLUDED_CONTEXT

#include "Filter.h"
#include "Keymap.h"
#include "Config.h"
#include "Sequence.h"
#include "T.h"
#include "TDB.h"
#include "StringTable.h"

class Context
{
public:
  Context ();                          // Default constructor
  Context (const Context&);            // Copy constructor
  Context& operator= (const Context&); // Assignment operator
  ~Context ();                         // Destructor

  void initialize (int, char**);
  int run ();
  int interactive ();

private:
  void loadCorrectConfigFile (int, char**);

public:
  Config      config;
  Filter      filter;
  Keymap      keymap;
  Sequence    sequence;
  T           task;
  TDB         tdb;
  StringTable stringtable;

private:
};

#endif
////////////////////////////////////////////////////////////////////////////////
