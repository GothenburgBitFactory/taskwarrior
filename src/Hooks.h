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
#ifndef INCLUDED_HOOKS
#define INCLUDED_HOOKS

#include <vector>
#include <string>
#include "API.h"

class Hooks
{
public:
  Hooks ();                        // Default constructor
  ~Hooks ();                       // Destructor
  Hooks (const Hooks&);            // Deliberately unimplemented
  Hooks& operator= (const Hooks&); // Deliberately unimplemented

  void initialize ();
  bool trigger (const std::string&);
  bool eventType (const std::string&, std::string&);

private:
  bool triggerProgramEvent (const std::string&);
  bool triggerListEvent (const std::string&);
  bool triggerTaskEvent (const std::string&);
  bool triggerFieldEvent (const std::string&);

private:
  API api;
  std::vector <std::string> scripts;
};

#endif
////////////////////////////////////////////////////////////////////////////////
