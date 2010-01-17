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
#ifndef INCLUDED_API
#define INCLUDED_API

#include "auto.h"
#ifdef HAVE_LIBLUA

#include <string>
extern "C"
{
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

class API
{
public:
  API ();
  API (const API&);
  API& operator= (const API&);
  ~API ();

  void initialize ();

public:
  lua_State* L;
};

#endif
#endif
////////////////////////////////////////////////////////////////////////////////
