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

#define L10N                                           // Localization complete.

#include <algorithm>
#include <iostream>
#include <Context.h>
#include <API.h>
#include <text.h>
#include <i18n.h>

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
  std::cout << STRING_API_EXITING << std::endl;
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
: _L (NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
API::~API ()
{
  if (_L)
  {
    lua_close (_L);
    _L = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
void API::initialize ()
{
  // Initialize Lua.
  _L = lua_open ();
  luaL_openlibs (_L);  // TODO Error handling

  // Register all the API functions in Lua global space.
  lua_pushcfunction (_L, api_task_header_message);   lua_setglobal (_L, "task_header_message");
  lua_pushcfunction (_L, api_task_footnote_message); lua_setglobal (_L, "task_footnote_message");
  lua_pushcfunction (_L, api_task_debug_message);    lua_setglobal (_L, "task_debug_message");
  lua_pushcfunction (_L, api_task_exit);             lua_setglobal (_L, "task_exit");
  lua_pushcfunction (_L, api_task_get);              lua_setglobal (_L, "task_get");
  lua_pushcfunction (_L, api_task_set);              lua_setglobal (_L, "task_set");
}

////////////////////////////////////////////////////////////////////////////////
bool API::callProgramHook (
  const std::string& file,
  const std::string& function)
{
  loadFile (file);

  // Get function.
  lua_getglobal (_L, function.c_str ());
  if (!lua_isfunction (_L, -1))
  {
    lua_pop (_L, 1);
    throw format (STRING_API_NOFUNC, function);
  }

  // Make call.
  if (lua_pcall (_L, 0, 2, 0) != 0)
    throw format (STRING_API_ERROR_CALLING, function, lua_tostring (_L, -1));

  // Call successful - get return values.
  if (!lua_isnumber (_L, -2))
    throw format (STRING_API_ERROR_FAIL, function);

  if (!lua_isstring (_L, -1) && !lua_isnil (_L, -1))
    throw format (STRING_API_ERROR_NORET, function);

  int rc              = lua_tointeger (_L, -2);
  const char* message = lua_tostring  (_L, -1);

  if (rc == 0)
  {
    if (message)
      context.footnote (format (STRING_API_WARNING, message));
  }
  else
  {
    if (message)
      throw std::string (message);
  }

  lua_pop (_L, 1);
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
  _current = task;

  // Get function.
  lua_getglobal (_L, function.c_str ());
  if (!lua_isfunction (_L, -1))
  {
    lua_pop (_L, 1);
    throw format (STRING_API_NOFUNC, function);
  }

  // Prepare args.
  lua_pushnumber (_L, _current.id);

  // Make call.
  if (lua_pcall (_L, 1, 2, 0) != 0)
    throw format (STRING_API_ERROR_CALLING, function, lua_tostring (_L, -1));

  // Call successful - get return values.
  if (!lua_isnumber (_L, -2))
    throw format (STRING_API_ERROR_FAIL, function);

  if (!lua_isstring (_L, -1) && !lua_isnil (_L, -1))
    throw format (STRING_API_ERROR_NORET, function);

  int rc              = lua_tointeger (_L, -2);
  const char* message = lua_tostring  (_L, -1);

  if (rc == 0)
  {
    if (message)
      context.footnote (format (STRING_API_WARNING, message));
  }
  else
  {
    if (message)
      throw std::string (message);
  }

  lua_pop (_L, 1);
  return rc == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void API::loadFile (const std::string& file)
{
  // If the file is not loaded.
  if (std::find (_loaded.begin (), _loaded.end (), file) == _loaded.end ())
  {
    // Load the file, if possible.
    if (luaL_loadfile (_L, file.c_str ()) || lua_pcall (_L, 0, 0, 0))
      throw format (STRING_API_ERROR, lua_tostring (_L, -1));

    // Mark this as _loaded, so as to not bother again.
    _loaded.push_back (file);
  }
}

////////////////////////////////////////////////////////////////////////////////

#endif

