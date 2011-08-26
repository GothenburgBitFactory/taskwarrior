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

#define L10N                                           // Localization complete.

#include <iostream>
#include <algorithm>
#include <Context.h>
#include <Hooks.h>
#include <Timer.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Hook::Hook ()
: _event ("")
, _file ("")
, _function ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const std::string& e, const std::string& f, const std::string& fn)
: _event (e)
, _file (f)
, _function (fn)
{
}

////////////////////////////////////////////////////////////////////////////////
Hook::Hook (const Hook& other)
{
  _event = other._event;
  _file = other._file;
  _function = other._function;
}

////////////////////////////////////////////////////////////////////////////////
Hook& Hook::operator= (const Hook& other)
{
  if (this != &other)
  {
    _event = other._event;
    _file = other._file;
    _function = other._function;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
{
  // New 2.x hooks.
  _validTaskEvents.push_back ("on-task-add");       // Unimplemented
  _validTaskEvents.push_back ("on-task-modify");    // Unimplemented
  _validTaskEvents.push_back ("on-task-complete");  // Unimplemented
  _validTaskEvents.push_back ("on-task-delete");    // Unimplemented

  _validProgramEvents.push_back ("on-launch");
  _validProgramEvents.push_back ("on-exit");
  _validProgramEvents.push_back ("on-file-read");   // Unimplemented
  _validProgramEvents.push_back ("on-file-write");  // Unimplemented
  _validProgramEvents.push_back ("on-synch");       // Unimplemented
  _validProgramEvents.push_back ("on-merge");       // Unimplemented
  _validProgramEvents.push_back ("on-gc");          // Unimplemented
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
  _api.initialize ();
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
            _all.push_back (h);

            (void) n.skip (',');
          }
          else
            throw std::string (format (STRING_LUA_BAD_HOOK_DEF, *it));
        }
      }
    }
  }
  else
    context.debug ("Hooks::initialize --> off");
}

////////////////////////////////////////////////////////////////////////////////
// Program hooks.
bool Hooks::trigger (const std::string& event)
{
#ifdef HAVE_LIBLUA
  std::vector <Hook>::iterator it;
  for (it = _all.begin (); it != _all.end (); ++it)
  {
    if (it->_event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validProgramEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! _api.callProgramHook (it->_file, it->_function))
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
  for (it = _all.begin (); it != _all.end (); ++it)
  {
    if (it->_event == event)
    {
      Timer timer (std::string ("Hooks::trigger ") + event);

      if (validTaskEvent (event))
      {
        context.debug (std::string ("Event ") + event + " triggered");
        if (! _api.callTaskHook (it->_file, it->_function, task))
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
  if (std::find (_validProgramEvents.begin (), _validProgramEvents.end (), event) != _validProgramEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::validTaskEvent (const std::string& event)
{
  if (std::find (_validTaskEvents.begin (), _validTaskEvents.end (), event) != _validTaskEvents.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
