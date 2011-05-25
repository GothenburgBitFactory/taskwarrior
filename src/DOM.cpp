////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2011, Paul Beckingham, Federico Hernandez.
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

#include <sstream>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <DOM.h>
#include "../cmake.h"

#ifdef HAVE_LIBLUA
extern "C"
{
  #include <lua.h>
}
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
DOM::DOM ()
{
}

////////////////////////////////////////////////////////////////////////////////
DOM::~DOM ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO <id>.                  <-- context.tdb2
// TODO <uuid>.                <-- context.tdb2
// rc.<name>              <-- context.config
// TODO report.<name>.         <-- context.reports
// TODO stats.<name>           <-- context.stats
// TODO context.<name>    <-- args, ...
//
// system.<name>          <-- context.system
// system.version
// system.lua.version
// system.os
const std::string DOM::get (const std::string& name)
{
  int len = name.length ();

  // rc. --> context.config
  if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    return context.config.get (name.substr (3));
  }

  else if (len > 8 &&
           name.substr (0, 8) == "context.")
  {
    if (name == "context.program")
      return context.program;

    else if (name == "context.args")
    {
      std::string combined;
      join (combined, " ", context.args);
      return combined;
    }
    else if (name == "context.width")
    {
      std::stringstream s;
      s << context.terminal_width;
      return s.str ();
    }
    else if (name == "context.height")
    {
      std::stringstream s;
      s << context.terminal_height;
      return s.str ();
    }

    else
      throw std::string ("DOM: Cannot get unrecognized name '") + name + "'.";
  }

  // TODO <id>.
  // TODO <uuid>.
  // TODO report.
  // TODO stats.<name>

  // system. --> Implement locally.
  else if (len > 7 &&
           name.substr (0, 7) == "system.")
  {
    // Taskwarrior version number.
    if (name == "system.version")
      return VERSION;

#ifdef HAVE_LIBLUA
    // Lua version number.
    else if (name == "system.lua.version")
      return LUA_RELEASE;
#endif

    // OS type.
    else if (name == "system.os")
#if defined (DARWIN)
      return "Darwin";
#elif defined (SOLARIS)
      return "Solaris";
#elif defined (CYGWIN)
      return "Cygwin";
#elif defined (OPENBSD)
      return "OpenBSD";
#elif defined (HAIKU)
      return "Haiku";
#elif defined (FREEBSD)
      return "FreeBSD";
#elif defined (LINUX)
      return "Linux";
#else
      return STRING_DOM_UNKNOWN
#endif

    else
      throw std::string ("DOM: Cannot get unrecognized name '") + name + "'.";
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void DOM::set (const std::string& name, const std::string& value)
{
  int len = name.length ();

  // rc. --> context.config
  if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    return context.config.set (name.substr (3), value);
  }

  // Unrecognized --> error.
  else
    throw std::string ("DOM: Cannot set '") + name + "'.";
}

////////////////////////////////////////////////////////////////////////////////

