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
#include <iostream>
#include <unistd.h>
#include <cmake.h>
#include <main.h>
#include <test.h>

#ifdef HAVE_LIBLUA
extern "C"
{
  #include <lua.h>
}
#endif

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (7);

  try
  {
    // Prime the pump.
    context.a3.capture ("task");

    // TODO dom.get rc.name
    DOM dom;
    t.is (dom.get ("system.version"),     VERSION,     "DOM system.version -> VERSION");
    t.is (dom.get ("system.lua.version"), LUA_RELEASE, "DOM system.lua.version -> LUA_RELEASE");
    t.ok (dom.get ("system.os") != "<unknown>",        "DOM system.os -> != Unknown");
    t.is (dom.get ("context.program"),    "task",      "DOM context.program -> 'task'");
    t.is (dom.get ("context.args"),       "task",      "DOM context.args -> 'task'");
    t.is (dom.get ("context.width"),      "0",         "DOM context.width -> '0'");
    t.is (dom.get ("context.height"),     "0",         "DOM context.height -> '0'");

    // TODO dom.set rc.name
  }

  catch (std::string& error)
  {
    t.diag (error);
    return -1;
  }

  catch (...)
  {
    t.diag ("Unknown error.");
    return -2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

