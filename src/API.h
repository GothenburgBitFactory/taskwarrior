////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
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
#ifndef INCLUDED_API
#define INCLUDED_API
#define L10N                                           // Localization complete.

#include "../cmake.h"
#ifdef HAVE_LIBLUA

#include <vector>
#include <string>
#include <Task.h>

extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

class API
{
public:
  API ();
  API (const API&);
  API& operator= (const API&);
  ~API ();

  void initialize ();
  bool callProgramHook (const std::string&, const std::string&);
  bool callTaskHook    (const std::string&, const std::string&, Task&);

private:
  void loadFile (const std::string&);

public:
  lua_State* L;
  std::vector <std::string> loaded;

  // Context for the API.
//  std::vector <Task> all;
  Task current;
//  std::string& name;
//  std::string& value;
};

#endif
#endif
////////////////////////////////////////////////////////////////////////////////
