////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <sstream>
#include <Context.h>
#include <Nibbler.h>
#include <text.h>
#include <i18n.h>
#include <DOM.h>

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
const std::vector <std::string> DOM::get_references () const
{
  std::vector <std::string> refs;

  refs.push_back ("context.program");
  refs.push_back ("context.args");
  refs.push_back ("context.width");
  refs.push_back ("context.height");
  refs.push_back ("system.version");
  refs.push_back ("system.os");

  return refs;
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
//   system.os
const std::string DOM::get (const std::string& name)
{
  int len = name.length ();
  Nibbler n (name);

  // rc. --> context.config
  if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    return context.config.get (name.substr (3));
  }

  // context.*
  else if (len > 8 &&
           name.substr (0, 8) == "context.")
  {
         if (name == "context.program") return context.a3[0]._raw;
    else if (name == "context.args")    return context.a3.combine ();
    else if (name == "context.width")   return format (context.terminal_width  ? context.terminal_width  : context.getWidth ());
    else if (name == "context.height")  return format (context.terminal_height ? context.terminal_height : context.getHeight ());
    else                                throw format (STRING_DOM_UNREC, name);
  }

  // TODO stats.<name>

  // system. --> Implement locally.
  else if (len > 7 &&
           name.substr (0, 7) == "system.")
  {
    // Taskwarrior version number.
    if (name == "system.version")
      return VERSION;

    // OS type.
    else if (name == "system.os")
#if defined (DARWIN)
      return "Darwin";
#elif defined (SOLARIS)
      return "Solaris";
#elif defined (CYGWIN)
      return "Cygwin";
#elif defined (HAIKU)
      return "Haiku";
#elif defined (OPENBSD)
      return "OpenBSD";
#elif defined (FREEBSD)
      return "FreeBSD";
#elif defined (NETBSD)
      return "NetBSD";
#elif defined (LINUX)
      return "Linux";
#elif defined (KFREEBSD)
      return "GNU/kFreeBSD";
#elif defined (GNUHURD)
      return "GNU/Hurd";
#else
      return STRING_DOM_UNKNOWN;
#endif

    else
      throw format (STRING_DOM_UNREC, name);
  }

  // Pass-through.
  return name;
}

////////////////////////////////////////////////////////////////////////////////
// DOM Supported References:
//
//   TODO <id>.{entry,start,end,scheduled,due,until,wait}
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
//   TODO <uuid>.{entry,start,end,scheduled,due,until,wait}
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
//   {entry,start,end,scheduled,due,until,wait}
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
const std::string DOM::get (const std::string& name, const Task& task)
{
  Nibbler n (name);
  n.save ();

  int id;
  std::string uuid;
  std::string canonical;

  // <attr>
       if (name == "id")                       return format (task.id);
  else if (name == "urgency")                  return format (task.urgency_c ());
  else if (A3::is_attribute (name, canonical)) return task.get (canonical);

  // <id>.<name>
  if (n.getInt (id))
  {
    if (n.skip ('.'))
    {
      Task ref;
      if (id == task.id)
        ref = task;
      else
        context.tdb2.get (id, ref);

      std::string attr;
      n.getUntilEOS (attr);

           if (attr == "id")                       return format (ref.id);
      else if (attr == "urgency")                  return format (ref.urgency_c ());
      else if (A3::is_attribute (attr, canonical)) return ref.get (canonical);
    }

    n.restore ();
  }

  // <uuid>.<name>
  if (n.getUUID (uuid))
  {
    if (n.skip ('.'))
    {
      Task ref;
      if (uuid == task.get ("uuid"))
        ref = task;
      else
        context.tdb2.get (uuid, ref);

      std::string attr;
      n.getUntilEOS (attr);

           if (attr == "id")                       return format (ref.id);
      else if (attr == "urgency")                  return format (ref.urgency_c (), 4, 3);
      else if (A3::is_attribute (attr, canonical)) return ref.get (canonical);
    }

    n.restore ();
  }

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
    context.config.set (name.substr (3), value);
  }

  // Unrecognized --> error.
  else
    throw format (STRING_DOM_CANNOT_SET, name);
}

////////////////////////////////////////////////////////////////////////////////
