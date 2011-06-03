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
#include <algorithm>
#include <Context.h>
#include <Hooks.h>
#include <Timer.h>
#include <text.h>
#include <i18n.h>

#define L10N                                           // Localization complete.

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Hook::Hook ()
: event ("")
, file ("")
, function ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const std::string& e, const std::string& f, const std::string& fn)
: event (e)
, file (f)
, function (fn)
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const Hook& other)
{
  event = other.event;
  file = other.file;
  function = other.function;
}

////////////////////////////////////////////////////////////////////////////////
Hook& Hook::operator= (const Hook& other)
{
  if (this != &other)
  {
    event = other.event;
    file = other.file;
    function = other.function;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
{
  // New 2.x hooks.
  validTaskEvents.push_back ("on-task-add");       // Unimplemented
  validTaskEvents.push_back ("on-task-modify");    // Unimplemented
  validTaskEvents.push_back ("on-task-complete");  // Unimplemented
  validTaskEvents.push_back ("on-task-delete");    // Unimplemented

  validProgramEvents.push_back ("on-launch");
  validProgramEvents.push_back ("on-exit");
  validProgramEvents.push_back ("on-file-read");   // Unimplemented
  validProgramEvents.push_back ("on-file-write");  // Unimplemented
  validProgramEvents.push_back ("on-synch");       // Unimplemented
  validProgramEvents.push_back ("on-merge");       // Unimplemented
  validProgramEvents.push_back ("on-gc");          // Unimplemented
}

////////////////////////////////////////////////////////////////////////////////
Hooks::~Hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Enumerate all hooks, and tell API about the script files it must load in
// order to call them.  Note that API will perform a deferred read, which means
// that if it isn't called, a script will not be loaded.
void Hooks::initialize ()
{
#ifdef HAVE_LIBLUA
  api.initialize ();
#endif

  // Allow a master switch to turn the whole thing off.
  bool big_red_switch = context.config.getBoolean ("extensions");
  if (big_red_switch)
  {
    std::vector <std::string> vars;
    context.config.all (vars);

    std::vector <std::string>::iterator it;
    for (it = vars.begin (); it != vars.end (); ++it)
    {
      std::string type;
      std::string name;
      std::string value;

      // "<type>.<name>"
      Nibbler n (*it);
      if (n.getUntil ('.', type) &&
          type == "hook"         &&
          n.skip ('.')           &&
          n.getUntilEOS (name))
      {
        std::string value = context.config.get (*it);
        Nibbler n (value);

        // <path>:<function> [, ...]
        while (!n.depleted ())
        {
          std::string file;
          std::string function;
          if (n.getUntil (':', file) &&
              n.skip (':')           &&
              n.getUntil (',', function))
          {
            context.debug (std::string ("Event '") + name + "' hooked by " + file + ", function " + function);
            Hook h (name, Path::expand (file), function);
            all.push_back (h);

            (void) n.skip (',');
          }
          else
            throw std::string (format (STRING_LUA_BAD_HOOK_DEF, *it));
        }
      }
    }
  }
  else
    context.debug ("Hooks::initialize - hook system off");
}

////////////////////////////////////////////////////////////////////////////////
// Program hooks.
bool Hooks::trigger (const std::string& event)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validProgramEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! api.callProgramHook (it->file, it->function))
          return false;
      }
      else
        throw std::string (format (STRING_LUA_BAD_EVENT, event));
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Task hooks.
bool Hooks::trigger (const std::string& event, Task& task)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validTaskEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! api.callTaskHook (it->file, it->function, task))
          return false;
      }
      else
        throw std::string (format (STRING_LUA_BAD_EVENT, event));
    }
  }
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validProgramEvent (const std::string& event)
{
  if (std::find (validProgramEvents.begin (), validProgramEvents.end (), event) != validProgramEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validTaskEvent (const std::string& event)
{
  if (std::find (validTaskEvents.begin (), validTaskEvents.end (), event) != validTaskEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
