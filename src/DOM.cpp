////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
//
bool DOM::get (const std::string& name, std::string& value)
{
  int len = name.length ();
  Nibbler n (name);

  // rc. --> context.config
  if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    std::string key = name.substr (3);
    std::map <std::string, std::string>::iterator c = context.config.find (key);
    if (c != context.config.end ())
    {
      value = c->second;
      return true;
    }

    return false;
  }

  // context.*
  if (len > 8 &&
           name.substr (0, 8) == "context.")
  {
    if (name == "context.program")
    {
      value = context.program;
      return true;
    }
    else if (name == "context.args")
    {
      value = "";
      std::vector <Tree*>::iterator i;
      for (i = context.parser.tree ()->_branches.begin (); i != context.parser.tree ()->_branches.end (); ++i)
      {
        if (value != "")
          value += " ";

        value += (*i)->attribute ("raw");
      }

      return true;
    }
    else if (name == "context.width")
    {
      value = format (context.terminal_width
                        ? context.terminal_width
                        : context.getWidth ());
      return true;
    }
    else if (name == "context.height")
    {
      value = format (context.terminal_height
                        ? context.terminal_height
                        : context.getHeight ());
      return true;
    }
    else
      throw format (STRING_DOM_UNREC, name);
  }

  // TODO stats.<name>

  // system. --> Implement locally.
  if (len > 7 &&
           name.substr (0, 7) == "system.")
  {
    // Taskwarrior version number.
    if (name == "system.version")
    {
      value = VERSION;
      return true;
    }

    // OS type.
    else if (name == "system.os")
    {
#if defined (DARWIN)
      value = "Darwin";
#elif defined (SOLARIS)
      value = "Solaris";
#elif defined (CYGWIN)
      value = "Cygwin";
#elif defined (HAIKU)
      value = "Haiku";
#elif defined (OPENBSD)
      value = "OpenBSD";
#elif defined (FREEBSD)
      value = "FreeBSD";
#elif defined (NETBSD)
      value = "NetBSD";
#elif defined (LINUX)
      value = "Linux";
#elif defined (KFREEBSD)
      value = "GNU/kFreeBSD";
#elif defined (GNUHURD)
      value = "GNU/Hurd";
#else
      value = STRING_DOM_UNKNOWN;
#endif
      return true;
    }
    else
      throw format (STRING_DOM_UNREC, name);
  }

  // Empty string if nothing is found.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// DOM Supported References:
//
//   <attribute>
//   <id>.<attribute>
//   <uuid>.<attribute>
//
bool DOM::get (const std::string& name, const Task& task, std::string& value)
{
  // <attr>
  if (task.size () && name == "id")
  {
    value = format (task.id);
    return true;
  }

  if (task.size () && name == "urgency")
  {
    value = format (task.urgency_c ());
    return true;
  }

  std::string canonical;
  if (task.size () && context.parser.canonicalize (canonical, "attribute", name))
  {
    value = task.get (canonical);
    return true;
  }

  // <id>.<name>
  Nibbler n (name);
  n.save ();
  int id;
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

      if (attr == "id")
      {
        value = format (ref.id);
        return true;
      }
      else if (attr == "urgency")
      {
        value = format (ref.urgency_c ());
        return true;
      }
      else if (context.parser.canonicalize (canonical, "attribute", attr))
      {
        value = ref.get (canonical);
        return true;
      }
    }

    n.restore ();
  }

  // <uuid>.<name>
  std::string uuid;
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

      if (attr == "id")
      {
        value = format (ref.id);
        return true;
      }
      else if (attr == "urgency")
      {
        value = format (ref.urgency_c (), 4, 3);
        return true;
      }
      else if (context.parser.canonicalize (canonical, "attribute", attr))
      {
        value = ref.get (canonical);
        return true;
      }
    }

    n.restore ();
  }

  // Delegate to the context-free version of DOM::get.
  return this->get (name, value);
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
