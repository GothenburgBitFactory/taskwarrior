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
#include <Nibbler.h>
#include <text.h>
#include <i18n.h>
#include <DOM.h>
#include <cmake.h>

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
// DOM Supported References:
//   rc.<name>
//
//   context.program
//   context.args
//   context.width
//   context.height
//
//   <id>.<?>
//   <id>.{entry,start,end,due,until,wait}
//   <id>.{entry,start,end,due,until,wait}.year
//   <id>.{entry,start,end,due,until,wait}.month
//   <id>.{entry,start,end,due,until,wait}.day
//   <id>.{entry,start,end,due,until,wait}.hour
//   <id>.{entry,start,end,due,until,wait}.minute
//   <id>.{entry,start,end,due,until,wait}.second
//   <id>.description
//   <id>.project
//   <id>.priority
//   <id>.parent
//   <id>.status
//   <id>.tags
//   <id>.urgency
//   <id>.recur
//   <id>.depends
//
//   <uuid>.<?>
//
//   TODO report.<name>.         <-- context.reports
//   TODO stats.<name>           <-- context.stats
//
//   system.version
//   system.lua.version
//   system.os
const std::string DOM::get (const std::string& name)
{
  int len = name.length ();
  Nibbler n (name);
  int id;
  std::string uuid;

  // Primitives
  if (is_primitive (name))
    return name;

  // rc. --> context.config
  else if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    return context.config.get (name.substr (3));
  }

  // context.*
  else if (len > 8 &&
           name.substr (0, 8) == "context.")
  {
    if (name == "context.program")
      return context.args[0].first;

    else if (name == "context.args")
    {
      std::string combined;
      join (combined, " ", context.args.list ());
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

  // <id>.<name>
  else if (n.getInt (id))
  {
    if (n.skip ('.'))
    {
      std::string ref;
      n.getUntilEOS (ref);

      if (ref == "description")
        ;
        // TODO return task.get ("description");
    }
  }

  // TODO <uuid>.<name>
  else if (n.getUUID (uuid))
  {
  }

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
// TODO This should return a Variant.
bool DOM::is_primitive (const std::string& input)
{
  std::string s;
  double d;
  int i;

  // TODO Date?
  // TODO Quoted Date?
  // TODO Duration?
  // TODO Quoted Duration?

  // String?
  Nibbler n (input);
  if ((n.getQuoted ('"', s) ||
       n.getQuoted ('\'', s)) &&
      n.depleted ())
    return true;

  // Number?
  n = Nibbler (input);
  if (n.getNumber (d) && n.depleted ())
    return true;

  // Integer?
  n = Nibbler (input);
  if (n.getInt (i) && n.depleted ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
