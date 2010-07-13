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
#include "auto.h"

// Hook class representing a single hook, which is just a three-way map.
class Hook
{
public:
  Hook ();
  Hook (const std::string&, const std::string&, const std::string&);
  Hook (const Hook&);
  Hook& operator= (const Hook&);

public:
  std::string event;
  std::string file;
  std::string function;
};

// Hooks class for managing the loading and calling of hook functions.
class Hooks
{
public:
  Hooks ();                         // Default constructor
  ~Hooks ();                        // Destructor
  Hooks (const Hooks&);             // Deliberately unimplemented
  Hooks& operator= (const Hooks&);  // Deliberately unimplemented

  void initialize ();

  bool trigger (const std::string&);                                   // Program
  bool trigger (const std::string&, std::vector <Task>&);              // List
  bool trigger (const std::string&, Task&);                            // Task
  bool trigger (const std::string&, const std::string&, std::string&); // Field

private:
  bool validProgramEvent (const std::string&);
  bool validListEvent (const std::string&);
  bool validTaskEvent (const std::string&);
  bool validFieldEvent (const std::string&);

private:
#ifdef HAVE_LIBLUA
  API api;
#endif
  std::vector <Hook> all;           // All current hooks.

  std::vector <std::string> validProgramEvents;
  std::vector <std::string> validListEvents;
  std::vector <std::string> validTaskEvents;
  std::vector <std::string> validFieldEvents;
};

#endif
////////////////////////////////////////////////////////////////////////////////
