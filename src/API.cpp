////////////////////////////////////////////////////////////////////////////////
// Taskwarrior Lua API
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include "Context.h"
#include "API.h"

extern Context context;
Task* the_task = NULL;

#ifdef HAVE_LIBLUA

////////////////////////////////////////////////////////////////////////////////
// Returns a string representing the taskwarrior version number, such as
// '1.9.0'.
static int api_task_version (lua_State* L)
{
  lua_pushstring (L, PACKAGE_VERSION);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Returns a string representing the Lua version number, such as '5.1.4'.
// Lua 5.2.0 has a 'lua_version' call, but 5.1.4 is the target.
static int api_task_lua_version (lua_State* L)
{
  // Convert "Lua 5.1.4" -> "5.1.4"
  std::string ver = LUA_RELEASE;
  lua_pushstring (L, ver.substr (4, std::string::npos).c_str ());
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Returns the type of OS that task is running on.
static int api_task_os (lua_State* L)
{
#if defined (DARWIN)
  lua_pushstring (L, "darwin");
#elif defined (SOLARIS)
  lua_pushstring (L, "solaris");
#elif defined (CYGWIN)
  lua_pushstring (L, "cygwin");
#elif defined (OPENBSD)
  lua_pushstring (L, "openbsd");
#elif defined (HAIKU)
  lua_pushstring (L, "haiku");
#elif defined (FREEBSD)
  lua_pushstring (L, "freebsd");
#elif defined (LINUX)
  lua_pushstring (L, "linux");
#else
  lua_pushstring (L, "unknown");
#endif

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_feature (lua_State* L)
{
  std::string name = luaL_checkstring (L, 1);
  bool value = false;

  if (name == "readline")
  {
#ifdef HAVE_READLINE
    value = true;
#endif
  }

  else if (name == "ncurses")
  {
#ifdef HAVE_NCURSES
     value = true;
#endif
  }

  else if (name == "lua")
    value = true;

  lua_pushboolean (L, value ? 1 : 0);
  return 1;
}

/*
////////////////////////////////////////////////////////////////////////////////
static int api_task_aliases ()
{
  return {};
}
*/

////////////////////////////////////////////////////////////////////////////////
// Returns values from .taskrc, by name.
static int api_task_get_config (lua_State* L)
{
  std::string name = luaL_checkstring (L, 1);
  lua_pushstring (L, context.config.get (name).c_str ());
  return 1;
}

/*
////////////////////////////////////////////////////////////////////////////////
-- Temporarily sets .taskrc values, by name.
static int api_task_set_config (name, value)
{
}

////////////////////////////////////////////////////////////////////////////////
-- Returns an internationalized string, by string ID, from the appropriate
-- locale-based strings file.
static int api_task_i18n_string (id)
{
  return "le foo"
}

////////////////////////////////////////////////////////////////////////////////
-- Returns a list of tips, from the appropriate locale-based tips file.
static int api_task_i18n_tips ()
{
  return {}
}

////////////////////////////////////////////////////////////////////////////////
-- Returns the name of the current command.
static int api_task_get_command ()
{
  return "list"
}

////////////////////////////////////////////////////////////////////////////////
-- Returns a list of string messages generated so far.
static int api_task_get_header_messages ()
{
  return {}
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_footnote_messages ()
{
  return {}
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_debug_messages (lua_State* L)
{
}
*/

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
static int api_task_exit (lua_State* L)
{
  // TODO Is this the correct exception?  How does the shell handle this?
  std::cout << "Exiting." << std::endl;
  exit (0); 
  return 0;
}

/*
////////////////////////////////////////////////////////////////////////////////
-- Shuts off the hook system for any subsequent hook calls for this command.
static int api_task_inhibit_further_hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
-- Returns a table that contains a complete copy of the task.
static int api_task_get (lua_State* L)
{
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
-- Creates a new task from the data specified in the table t.
static int api_task_add (t)
{
}

////////////////////////////////////////////////////////////////////////////////
-- Modifies the task described in the table t.
static int api_task_modify (t)
{
}
*/

////////////////////////////////////////////////////////////////////////////////
// -- 'id' is the task id passed to the hook function.  Date attributes are
// -- returned as a numeric epoch offset.  Tags and annotations are returned
// -- as tables.  A nil value indicates a missing value.
static int api_task_get_uuid (lua_State* L)
{
  if (the_task != NULL)
    lua_pushstring (L, the_task->get ("uuid").c_str ());
  else
    lua_pushnil (L);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_description (lua_State* L)
{
  if (the_task != NULL)
    lua_pushstring (L, the_task->get ("description").c_str ());
  else
    lua_pushnil (L);

  return 1;
}

/*
////////////////////////////////////////////////////////////////////////////////
static int api_task_get_annotations (id)
{
  return task.annotations
}
*/

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_project (lua_State* L)
{
  if (the_task != NULL)
    lua_pushstring (L, the_task->get ("project").c_str ());
  else
    lua_pushnil (L);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_priority (lua_State* L)
{
  if (the_task != NULL)
    lua_pushstring (L, the_task->get ("priority").c_str ());
  else
    lua_pushnil (L);

  return 1;
}

/*
////////////////////////////////////////////////////////////////////////////////
static int api_task_get_tags (id)
{
  return task.tags
}
*/

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_status (lua_State* L)
{
  if (the_task != NULL)
    lua_pushstring (L, Task::statusToText (the_task->getStatus ()).c_str ());
  else
    lua_pushnil (L);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_due (lua_State* L)
{
  if (the_task != NULL)
  {
    unsigned int value = (unsigned int) the_task->get_ulong ("due");
    if (value)
    {
      lua_pushinteger (L, value);
      return 1;
    }
  }

  lua_pushnil (L);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_entry (lua_State* L)
{
  if (the_task != NULL)
  {
    unsigned int value = (unsigned int) the_task->get_ulong ("entry");
    if (value)
    {
      lua_pushinteger (L, value);
      return 1;
    }
  }

  lua_pushnil (L);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_start (lua_State* L)
{
  if (the_task != NULL)
  {
    unsigned int value = (unsigned int) the_task->get_ulong ("start");
    if (value)
    {
      lua_pushinteger (L, value);
      return 1;
    }
  }

  lua_pushnil (L);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_end (lua_State* L)
{
  if (the_task != NULL)
  {
    unsigned int value = (unsigned int) the_task->get_ulong ("end");
    if (value)
    {
      lua_pushinteger (L, value);
      return 1;
    }
  }

  lua_pushnil (L);
  return 1;
}

/*
////////////////////////////////////////////////////////////////////////////////
static int api_task_get_recur (id)
{
  return task.recur
}
*/

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_until (lua_State* L)
{
  if (the_task != NULL)
  {
    unsigned int value = (unsigned int) the_task->get_ulong ("until");
    if (value)
    {
      lua_pushinteger (L, value);
      return 1;
    }
  }

  lua_pushnil (L);
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_get_wait (lua_State* L)
{
  if (the_task != NULL)
  {
    unsigned int value = (unsigned int) the_task->get_ulong ("wait");
    if (value)
    {
      lua_pushinteger (L, value);
      return 1;
    }
  }

  lua_pushnil (L);
  return 1;
}

/*
////////////////////////////////////////////////////////////////////////////////
-- 'id' is the task id passed to the hook function.  Date attributes are
-- expected as numeric epoch offsets.  Tags and annotations are expected
-- as tables.  A nil value indicates a missing value.
static int api_task_set_description (id, value)
{
  task.description = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_annotations (id, value)
{
  task.annotations = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_project (id, value)
{
  task.project = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_priority (id, value)
{
  task.priority = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_tags (id, value)
{
  task.tags = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_status (id, value)
{
  task.status = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_due (id, value)
{
  task.due_date = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_start (id, value)
{
  task.start_date = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_recur (id, value)
{
  task.recur = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_until (id, value)
{
  task.until_date = value
}

////////////////////////////////////////////////////////////////////////////////
static int api_task_set_wait (id, value)
{
  task.wait_date = value
}
*/

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
  lua_pushcfunction (L, api_task_version);               lua_setglobal (L, "task_version");
  lua_pushcfunction (L, api_task_lua_version);           lua_setglobal (L, "task_lua_version");
  lua_pushcfunction (L, api_task_os);                    lua_setglobal (L, "task_os");
  lua_pushcfunction (L, api_task_feature);               lua_setglobal (L, "task_feature");
/*
  lua_pushcfunction (L, api_task_aliases);               lua_setglobal (L, "task_aliases");
*/
  lua_pushcfunction (L, api_task_get_config);            lua_setglobal (L, "task_get_config");
/*
  lua_pushcfunction (L, api_task_set_config);            lua_setglobal (L, "task_set_config");
  lua_pushcfunction (L, api_task_i18n_string);           lua_setglobal (L, "task_i18n_string");
  lua_pushcfunction (L, api_task_i18n_tips);             lua_setglobal (L, "task_i18n_tips");
  lua_pushcfunction (L, api_task_get_command);           lua_setglobal (L, "task_get_command");
  lua_pushcfunction (L, api_task_get_header_messages);   lua_setglobal (L, "task_get_header_messages");
  lua_pushcfunction (L, api_task_get_footnote_messages); lua_setglobal (L, "task_get_footnote_messages");
  lua_pushcfunction (L, api_task_get_debug_messages);    lua_setglobal (L, "task_get_debug_messages");
*/
  lua_pushcfunction (L, api_task_header_message);        lua_setglobal (L, "task_header_message");
  lua_pushcfunction (L, api_task_footnote_message);      lua_setglobal (L, "task_footnote_message");
  lua_pushcfunction (L, api_task_debug_message);         lua_setglobal (L, "task_debug_message");
  lua_pushcfunction (L, api_task_exit);                  lua_setglobal (L, "task_exit");
/*
  lua_pushcfunction (L, api_task_inhibit_further_hooks); lua_setglobal (L, "task_inhibit_further_hooks");
  lua_pushcfunction (L, api_task_get);                   lua_setglobal (L, "task_get");
  lua_pushcfunction (L, api_task_add);                   lua_setglobal (L, "task_add");
  lua_pushcfunction (L, api_task_modify);                lua_setglobal (L, "task_modify");
*/
  lua_pushcfunction (L, api_task_get_uuid);              lua_setglobal (L, "task_get_uuid");
  lua_pushcfunction (L, api_task_get_description);       lua_setglobal (L, "task_get_description");
/*
  lua_pushcfunction (L, api_task_get_annotations);       lua_setglobal (L, "task_get_annotations");
*/
  lua_pushcfunction (L, api_task_get_project);           lua_setglobal (L, "task_get_project");
  lua_pushcfunction (L, api_task_get_priority);          lua_setglobal (L, "task_get_priority");
/*
  lua_pushcfunction (L, api_task_get_tags);              lua_setglobal (L, "task_get_tags");
*/
  lua_pushcfunction (L, api_task_get_status);            lua_setglobal (L, "task_get_status");
  lua_pushcfunction (L, api_task_get_due);               lua_setglobal (L, "task_get_due");
  lua_pushcfunction (L, api_task_get_entry);             lua_setglobal (L, "task_get_entry");
  lua_pushcfunction (L, api_task_get_start);             lua_setglobal (L, "task_get_start");
  lua_pushcfunction (L, api_task_get_end);               lua_setglobal (L, "task_get_end");
/*
  lua_pushcfunction (L, api_task_get_recur);             lua_setglobal (L, "task_get_recur");
*/
  lua_pushcfunction (L, api_task_get_until);             lua_setglobal (L, "task_get_until");
  lua_pushcfunction (L, api_task_get_wait);              lua_setglobal (L, "task_get_wait");
/*
  lua_pushcfunction (L, api_task_set_description);       lua_setglobal (L, "task_set_description");
  lua_pushcfunction (L, api_task_set_annotations);       lua_setglobal (L, "task_set_annotations");
  lua_pushcfunction (L, api_task_set_project);           lua_setglobal (L, "task_set_project");
  lua_pushcfunction (L, api_task_set_priority);          lua_setglobal (L, "task_set_priority");
  lua_pushcfunction (L, api_task_set_tags);              lua_setglobal (L, "task_set_tags");
  lua_pushcfunction (L, api_task_set_status);            lua_setglobal (L, "task_set_status");
  lua_pushcfunction (L, api_task_set_due);               lua_setglobal (L, "task_set_due");
  lua_pushcfunction (L, api_task_set_start);             lua_setglobal (L, "task_set_start");
  lua_pushcfunction (L, api_task_set_recur);             lua_setglobal (L, "task_set_recur");
  lua_pushcfunction (L, api_task_set_until);             lua_setglobal (L, "task_set_until");
  lua_pushcfunction (L, api_task_set_wait);              lua_setglobal (L, "task_set_wait");
*/
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
// TODO No intention of implementing this before task 2.0.  Why?  Because we
//      need to implement a Lua iterator, in C++, to iterate over a std::vector.
bool API::callListHook (
  const std::string& file,
  const std::string& function,
  std::vector <Task>& all)
{
  loadFile (file);

  // TODO Get function.
  // TODO Prepare args.
  // TODO Make call.
  // TODO Get exit status.

  return true;
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

  // Expose the task.
  the_task = &current;

  // Make call.
  if (lua_pcall (L, 1, 2, 0) != 0)
    throw std::string ("Error calling '") + function + "' - " + lua_tostring (L, -1) + ".";

  // Hide the task.
  the_task = NULL;

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
bool API::callFieldHook (
  const std::string& file,
  const std::string& function,
  const std::string& name,
  std::string& value)
{
  loadFile (file);

  // Get function.
  lua_getglobal (L, function.c_str ());
  if (!lua_isfunction (L, -1))
  {
    lua_pop (L, 1);
    throw std::string ("The Lua function '") + function + "' was not found.";
  }

  // Prepare args.
  lua_pushstring (L, name.c_str ());
  lua_pushstring (L, value.c_str ());

  // Make call.
  if (lua_pcall (L, 2, 3, 0) != 0)
    throw std::string ("Error calling '") + function + "' - " + lua_tostring (L, -1) + ".";

  // Call successful - get return values.
  if (!lua_isstring (L, -3))
    throw std::string ("Error: '") + function + "' did not return a modified value.";

  if (!lua_isnumber (L, -2))
    throw std::string ("Error: '") + function + "' did not return a success indicator.";

  if (!lua_isstring (L, -1) && !lua_isnil (L, -1))
    throw std::string ("Error: '") + function + "' did not return a message or nil.";

  const char* new_value = lua_tostring  (L, -3);
  int rc                = lua_tointeger (L, -2);
  const char* message   = lua_tostring  (L, -1);

  if (rc == 0)
  {
    // Overwrite with the modified value.
    value = new_value;

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

