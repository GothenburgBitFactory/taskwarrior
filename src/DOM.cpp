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
#include <map>
#include <stdlib.h>
#include <Context.h>
#include <Nibbler.h>
#include <Date.h>
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
// For certain attributes:
//   <date>.year
//   <date>.month
//   <date>.day
//   <date>.week
//   <date>.weekday
//   <date>.julian
//   <date>.hour
//   <date>.minute
//   <date>.second
//
//   tags.<literal>                  Includes virtual tags
//
//   annotations.<N>.entry
//   annotations.<N>.entry.year
//   annotations.<N>.entry.month
//   annotations.<N>.entry.day
//   annotations.<N>.entry.week
//   annotations.<N>.entry.weekday
//   annotations.<N>.entry.julian
//   annotations.<N>.entry.hour
//   annotations.<N>.entry.minute
//   annotations.<N>.entry.second
//   annotations.<N>.description
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

  // split name on '.'
  std::vector <std::string> elements;
  split (elements, name, '.');

  if (elements.size () == 1)
  {
    std::string canonical;
    if (task.size () && context.parser.canonicalize (canonical, "attribute", name))
    {
      value = task.get (canonical);
      return true;
    }
  }
  else if (elements.size () > 1)
  {
    Task ref;

    Nibbler n (elements[0]);
    n.save ();
    int id;
    std::string uuid;
    bool proceed = false;

    if (n.getInt (id) && n.depleted ())
    {
      if (id == task.id)
        ref = task;
      else
        context.tdb2.get (id, ref);

      proceed = true;
    }
    else
    {
      n.restore ();
      if (n.getUUID (uuid) && n.depleted ())
      {
        if (uuid == task.get ("uuid"))
          ref = task;
        else
          context.tdb2.get (uuid, ref);

        proceed = true;
      }
    }

    if (proceed)
    {
      if (elements[1] == "id")
      {
        value = format (ref.id);
        return true;
      }
      else if (elements[1] == "urgency")
      {
        value = format (ref.urgency_c ());
        return true;
      }

      std::string canonical;
      if (context.parser.canonicalize (canonical, "attribute", elements[1]))
      {
        if (elements.size () == 2)
        {
          value = ref.get (canonical);
          return true;
        }
        else if (elements.size () == 3)
        {
          // tags.<tag>
          if (canonical == "tags")
          {
            value = ref.hasTag (elements[2]) ? elements[2] : "";
            return true;
          }

          Column* column = context.columns[canonical];
          if (column && column->type () == "date")
          {
            // <date>.year
            // <date>.month
            // <date>.day
            // <date>.week
            // <date>.weekday
            // <date>.julian
            // <date>.hour
            // <date>.minute
            // <date>.second
            Date date (ref.get_date (canonical));
                 if (elements[2] == "year")    { value = format (date.year ());      return true; }
            else if (elements[2] == "month")   { value = format (date.month ());     return true; }
            else if (elements[2] == "day")     { value = format (date.day ());       return true; }
            else if (elements[2] == "week")    { value = format (date.week ());      return true; }
            else if (elements[2] == "weekday") { value = format (date.dayOfWeek ()); return true; }
            else if (elements[2] == "julian")  { value = format (date.dayOfYear ()); return true; }
            else if (elements[2] == "hour")    { value = format (date.hour ());      return true; }
            else if (elements[2] == "minute")  { value = format (date.minute ());    return true; }
            else if (elements[2] == "second")  { value = format (date.second ());    return true; }
          }
        }
      }
      else if (elements[1] == "annotations")
      {
        if (elements.size () == 4)
        {
          std::map <std::string, std::string> annos;
          ref.getAnnotations (annos);

          int a = strtol (elements[2].c_str (), NULL, 10);
          int count = 0;

          // Count off the 'a'th annotation.
          std::map <std::string, std::string>::iterator i;
          for (i = annos.begin (); i != annos.end (); ++i)
          {
            if (++count == a)
            {
              if (elements[3] == "entry")
              {
                // annotation_1234567890
                // 0          ^11
                value = i->first.substr (11);
                return true;
              }
              else if (elements[3] == "description")
              {
                value = i->second;
                return true;
              }
            }
          }
        }
        else if (elements.size () == 5)
        {
          std::map <std::string, std::string> annos;
          ref.getAnnotations (annos);

          int a = strtol (elements[2].c_str (), NULL, 10);
          int count = 0;

          // Count off the 'a'th annotation.
          std::map <std::string, std::string>::iterator i;
          for (i = annos.begin (); i != annos.end (); ++i)
          {
            if (++count == a)
            {
              // <annotations>.<N>.entry.year
              // <annotations>.<N>.entry.month
              // <annotations>.<N>.entry.day
              // <annotations>.<N>.entry.week
              // <annotations>.<N>.entry.weekday
              // <annotations>.<N>.entry.julian
              // <annotations>.<N>.entry.hour
              // <annotations>.<N>.entry.minute
              // <annotations>.<N>.entry.second
              Date date (i->first.substr (11));
                   if (elements[4] == "year")    { value = format (date.year ());      return true; }
              else if (elements[4] == "month")   { value = format (date.month ());     return true; }
              else if (elements[4] == "day")     { value = format (date.day ());       return true; }
              else if (elements[4] == "week")    { value = format (date.week ());      return true; }
              else if (elements[4] == "weekday") { value = format (date.dayOfWeek ()); return true; }
              else if (elements[4] == "julian")  { value = format (date.dayOfYear ()); return true; }
              else if (elements[4] == "hour")    { value = format (date.hour ());      return true; }
              else if (elements[4] == "minute")  { value = format (date.minute ());    return true; }
              else if (elements[4] == "second")  { value = format (date.second ());    return true; }
            }
          }
        }
      }
    }
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
