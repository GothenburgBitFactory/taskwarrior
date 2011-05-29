////////////////////////////////////////////////////////////////////////////////
// Taskwarrior Lua API
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
// -------------
//
// Copyright © 1994–2008 Lua.org, PUC-Rio.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <iostream>
#include <Context.h>
#include <API.h>

extern Context context;

#ifdef HAVE_LIBLUA

////////////////////////////////////////////////////////////////////////////////
static int api_task_header_message (lua_State* L)
{
  std::string message = luaL_checkstring (L, 1);
  context.header (message);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_footnote_message (lua_State* L)
{
  std::string message = luaL_checkstring (L, 1);
  context.footnote (message);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_debug_message (lua_State* L)
{
  std::string message = luaL_checkstring (L, 1);
  context.debug (message);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Causes the shell or interactive mode task to exit.  Ordinarily this does not
// occur.
static int api_task_exit (lua_State*)
{
  // TODO Is this the correct exception?  How does the shell handle this?
  std::cout << "Exiting." << std::endl;
  exit (0); 
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DOM reads.
static int api_task_get (lua_State* L)
{
  std::string name = luaL_checkstring (L, 1);
  try
  {
    lua_pushstring (L, context.dom.get (name).c_str ());
  }
  catch (...)
  {
    // TODO Error!
    lua_pushstring (L, "");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DOM writes.
static int api_task_set (lua_State* L)
{
  std::string name  = luaL_checkstring (L, 1);
  std::string value = luaL_checkstring (L, 2);

  try
  {
    context.dom.set (name, value);
  }
  catch (...)
  {
    // TODO Error!
    lua_pushstring (L, "");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
API::API ()
: L (NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
API::~API ()
{
  if (L)
  {
    lua_close (L);
    L = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
void API::initialize ()
{
  // Initialize Lua.
  L = lua_open ();
  luaL_openlibs (L);  // TODO Error handling

  // Register all the API functions in Lua global space.
  lua_pushcfunction (L, api_task_header_message);        lua_setglobal (L, "task_header_message");
  lua_pushcfunction (L, api_task_footnote_message);      lua_setglobal (L, "task_footnote_message");
  lua_pushcfunction (L, api_task_debug_message);         lua_setglobal (L, "task_debug_message");
  lua_pushcfunction (L, api_task_exit);                  lua_setglobal (L, "task_exit");
  lua_pushcfunction (L, api_task_get);                   lua_setglobal (L, "task_get");
  lua_pushcfunction (L, api_task_set);                   lua_setglobal (L, "task_set");
}

////////////////////////////////////////////////////////////////////////////////
bool API::callProgramHook (
  const std::string& file,
  const std::string& function)
{
  loadFile (file);

  // Get function.
  lua_getglobal (L, function.c_str ());
  if (!lua_isfunction (L, -1))
  {
    lua_pop (L, 1);
    throw std::string ("The Lua function '") + function + "' was not found.";
  }

  // Make call.
  if (lua_pcall (L, 0, 2, 0) != 0)
    throw std::string ("Error calling '") + function + "' - " + lua_tostring (L, -1) + ".";

  // Call successful - get return values.
  if (!lua_isnumber (L, -2))
    throw std::string ("Error: '") + function + "' did not return a success indicator.";

  if (!lua_isstring (L, -1) && !lua_isnil (L, -1))
    throw std::string ("Error: '") + function + "' did not return a message or nil.";

  int rc              = lua_tointeger (L, -2);
  const char* message = lua_tostring  (L, -1);

  if (rc == 0)
  {
    if (message)
      context.footnote (std::string ("Warning: ") + message);
  }
  else
  {
    if (message)
      throw std::string (message);
  }

  lua_pop (L, 1);
  return rc == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool API::callTaskHook (
  const std::string& file,
  const std::string& function,
  Task& task)
{
  loadFile (file);

  // Save the task for reference via the API.
  current = task;

  // Get function.
  lua_getglobal (L, function.c_str ());
  if (!lua_isfunction (L, -1))
  {
    lua_pop (L, 1);
    throw std::string ("The Lua function '") + function + "' was not found.";
  }

  // Prepare args.
  lua_pushnumber (L, current.id);

  // Make call.
  if (lua_pcall (L, 1, 2, 0) != 0)
    throw std::string ("Error calling '") + function + "' - " + lua_tostring (L, -1) + ".";

  // Call successful - get return values.
  if (!lua_isnumber (L, -2))
    throw std::string ("Error: '") + function + "' did not return a success indicator.";

  if (!lua_isstring (L, -1) && !lua_isnil (L, -1))
    throw std::string ("Error: '") + function + "' did not return a message or nil.";

  int rc              = lua_tointeger (L, -2);
  const char* message = lua_tostring  (L, -1);

  if (rc == 0)
  {
    if (message)
      context.footnote (std::string ("Warning: ") + message);
  }
  else
  {
    if (message)
      throw std::string (message);
  }

  lua_pop (L, 1);
  return rc == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void API::loadFile (const std::string& file)
{
  // If the file is not loaded.
  if (std::find (loaded.begin (), loaded.end (), file) == loaded.end ())
  {
    // Load the file, if possible.
    if (luaL_loadfile (L, file.c_str ()) || lua_pcall (L, 0, 0, 0))
      throw std::string ("Error: ") + std::string (lua_tostring (L, -1));

    // Mark this as loaded, so as to not bother again.
    loaded.push_back (file);
  }
}

////////////////////////////////////////////////////////////////////////////////

#endif

