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
      throw format (STRING_DOM_UNREC, name);
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
      throw format (STRING_DOM_UNREC, name);
  }

  return name;
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
//   {entry,start,end,due,until,wait}
//   description
//   project
//   priority
//   parent
//   status
//   tags
//   urgency
//   recur
//   depends
//
const std::string DOM::get (const std::string& name, Task& task)
{
  Nibbler n (name);
  int id;
  std::string uuid;

  // Primitives
  if (is_primitive (name))
    return name;

  // <id>.<name>
  else if (n.getInt (id))
  {
    if (n.skip ('.'))
    {
      // TODO Obtain task 'id' from TDB2.

      std::string attr;
      n.getUntilEOS (attr);

           if (attr == "description")  return task.get ("description");
      else if (attr == "status")       return task.get ("status");
      else if (attr == "project")      return task.get ("project");
      else if (attr == "priority")     return task.get ("priority");
      else if (attr == "parent")       return task.get ("parent");
      else if (attr == "tags")         return task.get ("tags");
      else if (attr == "urgency")      return format (task.urgency (), 4, 3);
      else if (attr == "recur")        return task.get ("recur");
      else if (attr == "depends")      return task.get ("depends");
      else if (attr == "entry")        return task.get ("entry");
      else if (attr == "start")        return task.get ("start");
      else if (attr == "end")          return task.get ("end");
      else if (attr == "due")          return task.get ("due");
      else if (attr == "until")        return task.get ("until");
      else if (attr == "wait")         return task.get ("wait");
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

           if (attr == "description")  return task.get ("description");
      else if (attr == "status")       return task.get ("status");
      else if (attr == "project")      return task.get ("project");
      else if (attr == "priority")     return task.get ("priority");
      else if (attr == "parent")       return task.get ("parent");
      else if (attr == "tags")         return task.get ("tags");
      else if (attr == "urgency")      return format (task.urgency (), 4, 3);
      else if (attr == "recur")        return task.get ("recur");
      else if (attr == "depends")      return task.get ("depends");
      else if (attr == "entry")        return task.get ("entry");
      else if (attr == "start")        return task.get ("start");
      else if (attr == "end")          return task.get ("end");
      else if (attr == "due")          return task.get ("due");
      else if (attr == "until")        return task.get ("until");
      else if (attr == "wait")         return task.get ("wait");
    }
  }

  // [<task>.] <name>
       if (name == "description")  return task.get ("description");
  else if (name == "status")       return task.get ("status");
  else if (name == "project")      return task.get ("project");
  else if (name == "priority")     return task.get ("priority");
  else if (name == "parent")       return task.get ("parent");
  else if (name == "tags")         return task.get ("tags");
  else if (name == "urgency")      return format (task.urgency (), 4, 3);
  else if (name == "recur")        return task.get ("recur");
  else if (name == "depends")      return task.get ("depends");
  else if (name == "entry")        return task.get ("entry");
  else if (name == "start")        return task.get ("start");
  else if (name == "end")          return task.get ("end");
  else if (name == "due")          return task.get ("due");
  else if (name == "until")        return task.get ("until");
  else if (name == "wait")         return task.get ("wait");

  // Delegate to the context-free version of DOM::get.
  return this->get (name);
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
    throw format (STRING_DOM_CANNOT_SET, name);
}

////////////////////////////////////////////////////////////////////////////////
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
