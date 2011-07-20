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

#define L10N                                           // Localization complete.

#include <iostream> // TODO Remove
#include <sstream>
#include <Context.h>
#include <Nibbler.h>
#include <Date.h>
#include <Duration.h>
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
//   TODO stats.<name>           <-- context.stats
//
//   system.version
//   system.lua.version
//   system.os
const std::string DOM::get (const std::string& name)
{
  // Cache test.
/*
  std::map <std::string, std::string>::iterator hit = _cache.find (name);
  if (hit != _cache.end ())
    return hit->second;
*/

  int len = name.length ();
  Nibbler n (name);
  std::string copy_name (name);

  // Primitives
  if (is_literal (copy_name))
    return /*_cache[name] =*/ copy_name;

  // rc. --> context.config
  else if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    return /*_cache[name] =*/ context.config.get (name.substr (3));
  }

  // context.*
  else if (len > 8 &&
           name.substr (0, 8) == "context.")
  {
    if (name == "context.program")
      return /*_cache[name] =*/ context.args[0]._first;

    else if (name == "context.args")
    {
      return /*_cache[name] =*/ context.args.combine ();
    }
    else if (name == "context.width")
    {
      std::stringstream s;
      s << context.terminal_width;
      return /*_cache[name] =*/ s.str ();
    }
    else if (name == "context.height")
    {
      std::stringstream s;
      s << context.terminal_height;
      return /*_cache[name] =*/ s.str ();
    }

    else
      throw format (STRING_DOM_UNREC, name);
  }

  // TODO stats.<name>

  // system. --> Implement locally.
  else if (len > 7 &&
           name.substr (0, 7) == "system.")
  {
    // Taskwarrior version number.
    if (name == "system.version")
      return /*_cache[name] =*/ VERSION;

#ifdef HAVE_LIBLUA
    // Lua version number.
    else if (name == "system.lua.version")
      return /*_cache[name] =*/ LUA_RELEASE;
#endif

    // OS type.
    else if (name == "system.os")
#if defined (DARWIN)
      return /*_cache[name] =*/ "Darwin";
#elif defined (SOLARIS)
      return /*_cache[name] =*/ "Solaris";
#elif defined (CYGWIN)
      return /*_cache[name] =*/ "Cygwin";
#elif defined (OPENBSD)
      return /*_cache[name] =*/ "OpenBSD";
#elif defined (HAIKU)
      return /*_cache[name] =*/ "Haiku";
#elif defined (FREEBSD)
      return /*_cache[name] =*/ "FreeBSD";
#elif defined (LINUX)
      return /*_cache[name] =*/ "Linux";
#else
      return /*_cache[name] =*/ STRING_DOM_UNKNOWN;
#endif

    else
      throw format (STRING_DOM_UNREC, name);
  }

  // Pass-through.
  return /*_cache[name] =*/ name;
}

////////////////////////////////////////////////////////////////////////////////
// DOM Supported References:
//
//   TODO <id>.{entry,start,end,due,until,wait}
//   TODO <id>.description
//   TODO <id>.project
//   TODO <id>.priority
//   TODO <id>.parent
//   TODO <id>.status
//   TODO <id>.tags
//   TODO <id>.urgency
//   TODO <id>.recur
//   TODO <id>.depends
//
//   TODO <uuid>.{entry,start,end,due,until,wait}
//   TODO <uuid>.description
//   TODO <uuid>.project
//   TODO <uuid>.priority
//   TODO <uuid>.parent
//   TODO <uuid>.status
//   TODO <uuid>.tags
//   TODO <uuid>.urgency
//   TODO <uuid>.recur
//   TODO <uuid>.depends
//
//   {.entry,.start,.end,.due,.until,.wait}
//   .description
//   .project
//   .priority
//   .parent
//   .status
//   .tags
//   .urgency
//   .recur
//   .depends
//
const std::string DOM::get (const std::string& name, const Task& task)
{
  // Cache test.
/*
  std::map <std::string, std::string>::iterator hit = _cache.find (name);
  if (hit != _cache.end ())
    return hit->second;
*/

  Nibbler n (name);
  int id;
  std::string uuid;
  std::string canonical;

  // Primitives
  std::string copy_name (name);
  if (is_literal (copy_name))
    return /*_cache[name] =*/ copy_name;

  // <attr>
  else if (Arguments::is_attribute (name, canonical))
    return task.get (canonical);

  // <id>.<name>
  else if (n.getInt (id))
  {
    if (n.skip ('.'))
    {
      // TODO Obtain task 'id' from TDB2.
      std::string attr;
      n.getUntilEOS (attr);

           if (attr == "id")      return format (task.id);
      else if (attr == "urgency") return format (task.urgency_c (), 4, 3);
      else                        return task.get (attr);
    }
  }

  // <uuid>.<name>
  else if (n.getUUID (uuid))
  {
    if (n.skip ('.'))
    {
      // TODO Obtain task 'uuid' from TDB2.
      std::string attr;
      n.getUntilEOS (attr);

           if (attr == "id")      return format (task.id);
      else if (attr == "urgency") return format (task.urgency_c (), 4, 3);
      else                        return task.get (attr);
    }
  }

  // [<task>.]<name>
       if (name == "id")               return format (task.id);
  else if (name == "urgency")          return format (task.urgency_c (), 4, 3);
  else if (task.has (name.substr (1))) return task.get (name.substr (1));

  // Delegate to the context-free version of DOM::get.
  return this->get (name);
}

// TODO Need a context-specific DOM::set.

////////////////////////////////////////////////////////////////////////////////
void DOM::set (const std::string& name, const std::string& value)
{
  int len = name.length ();

  // rc. --> context.config
  if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    context.config.set (name.substr (3), /*_cache[name] =*/ value);
  }

  // Unrecognized --> error.
  else
    throw format (STRING_DOM_CANNOT_SET, name);
}

////////////////////////////////////////////////////////////////////////////////
bool DOM::is_literal (std::string& input)
{
  std::string s;
  double d;
  int i;

  // Date?
  if (Date::valid (input, context.config.get ("dateformat")))
  {
    input = Date (input).toEpochString ();
    std::cout << "# DOM::is_literal '" << input << "' --> date\n";
    return true;
  }

  // Duration?
  if (Duration::valid (input))
    return true;

  // TODO Quoted Date?
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

//  std::cout << "# DOM::is_literal '" << input << "' --> unknown\n";
  return false;
}

////////////////////////////////////////////////////////////////////////////////
