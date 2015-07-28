////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <Variant.h>
#include <Duration.h>
#include <Context.h>
#include <Nibbler.h>
#include <ISO8601.h>
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
// DOM Supported References:
//
// Configuration:
//   rc.<name>
//
// System:
//   context.program
//   context.args
//   context.width
//   context.height
//   system.version
//   system.os
//
bool DOM::get (const std::string& name, Variant& value)
{
  int len = name.length ();
  Nibbler n (name);

  // rc. --> context.config
  if (len > 3 &&
      name.substr (0, 3) == "rc.")
  {
    std::string key = name.substr (3);
    auto c = context.config.find (key);
    if (c != context.config.end ())
    {
      value = Variant (c->second);
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
      value = Variant (context.cli2.getBinary ());
      return true;
    }
    else if (name == "context.args")
    {
      std::string commandLine;
      join (commandLine, " ", context.cli2._original_args);
      value = Variant (commandLine);
      return true;
    }
    else if (name == "context.width")
    {
      value = Variant (static_cast<int> (context.terminal_width
                                           ? context.terminal_width
                                           : context.getWidth ()));
      return true;
    }
    else if (name == "context.height")
    {
      value = Variant (static_cast<int> (context.terminal_height
                                           ? context.terminal_height
                                           : context.getHeight ()));
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
      value = Variant (VERSION);
      return true;
    }

    // OS type.
    else if (name == "system.os")
    {
#if defined (DARWIN)
      value = Variant ("Darwin");
#elif defined (SOLARIS)
      value = Variant ("Solaris");
#elif defined (CYGWIN)
      value = Variant ("Cygwin");
#elif defined (HAIKU)
      value = Variant ("Haiku");
#elif defined (OPENBSD)
      value = Variant ("OpenBSD");
#elif defined (FREEBSD)
      value = Variant ("FreeBSD");
#elif defined (NETBSD)
      value = Variant ("NetBSD");
#elif defined (LINUX)
      value = Variant ("Linux");
#elif defined (KFREEBSD)
      value = Variant ("GNU/kFreeBSD");
#elif defined (GNUHURD)
      value = Variant ("GNU/Hurd");
#else
      value = Variant (STRING_DOM_UNKNOWN);
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
// Relative or absolute attribute:
//   <attribute>
//   <id>.<attribute>
//   <uuid>.<attribute>
//
// Single tag:
//   tags.<word>
//
// Date type:
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
// Annotations (entry is a date):
//   annotations.<N>.entry
//   annotations.<N>.description
//
bool DOM::get (const std::string& name, const Task& task, Variant& value)
{
  // <attr>
  if (task.size () && name == "id")
  {
    value = Variant (static_cast<int> (task.id));
    return true;
  }

  if (task.size () && name == "urgency")
  {
    value = Variant (task.urgency_c ());
    return true;
  }

  // split name on '.'
  std::vector <std::string> elements;
  split (elements, name, '.');

  if (elements.size () == 1)
  {
    std::string canonical;
    if (task.size () && context.cli2.canonicalize (canonical, "attribute", name))
    {
      Column* column = context.columns[canonical];
      if (column)
      {
        if (column->is_uda () && ! task.has (canonical))
        {
          value = Variant ("");
          return true;
        }

        if (column->type () == "date")
        {
          auto numeric = task.get_date (canonical);
          if (numeric == 0)
            value = Variant ("");
          else
            value = Variant (numeric, Variant::type_date);
        }
        else if (column->type () == "duration" || canonical == "recur")
          value = Variant ((time_t) Duration (task.get (canonical)), Variant::type_duration);
        else if (column->type () == "numeric")
          value = Variant (task.get_float (canonical));
        else // string
          value = Variant (task.get (canonical));

        return true;
      }
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
        value = Variant (static_cast<int> (ref.id));
        return true;
      }
      else if (elements[1] == "urgency")
      {
        value = Variant (ref.urgency_c ());
        return true;
      }

      std::string canonical;
      if (context.cli2.canonicalize (canonical, "attribute", elements[1]))
      {
        if (elements.size () == 2)
        {
          Column* column = context.columns[canonical];
          if (column)
          {
            if (column->is_uda () && ! ref.has (canonical))
            {
              value = Variant ("");
              return true;
            }

            if (column->type () == "date")
            {
              auto numeric = ref.get_date (canonical);
              if (numeric == 0)
                value = Variant ("");
              else
                value = Variant (numeric, Variant::type_date);
            }
            else if (column->type () == "duration")
            {
              auto period = ref.get (canonical);
              context.debug ("ref.get(" + canonical + ") --> " + period);

              ISO8601p iso;
              std::string::size_type cursor = 0;
              if (iso.parse (period, cursor))
                value = Variant ((time_t) iso._value, Variant::type_duration);
              else
                value = Variant ((time_t) Duration (ref.get (canonical)), Variant::type_duration);

              context.debug ("value --> " + (std::string) value);
            }
            else if (column->type () == "numeric")
              value = Variant (ref.get_float (canonical));
            else
              value = Variant (ref.get (canonical));

            return true;
          }
        }
        else if (elements.size () == 3)
        {
          // tags.<tag>
          if (canonical == "tags")
          {
            value = Variant (ref.hasTag (elements[2]) ? elements[2] : "");
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
                 if (elements[2] == "year")    { value = Variant (static_cast<int> (date.year ()));      return true; }
            else if (elements[2] == "month")   { value = Variant (static_cast<int> (date.month ()));     return true; }
            else if (elements[2] == "day")     { value = Variant (static_cast<int> (date.day ()));       return true; }
            else if (elements[2] == "week")    { value = Variant (static_cast<int> (date.week ()));      return true; }
            else if (elements[2] == "weekday") { value = Variant (static_cast<int> (date.dayOfWeek ())); return true; }
            else if (elements[2] == "julian")  { value = Variant (static_cast<int> (date.dayOfYear ())); return true; }
            else if (elements[2] == "hour")    { value = Variant (static_cast<int> (date.hour ()));      return true; }
            else if (elements[2] == "minute")  { value = Variant (static_cast<int> (date.minute ()));    return true; }
            else if (elements[2] == "second")  { value = Variant (static_cast<int> (date.second ()));    return true; }
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
          for (auto& i : annos)
          {
            if (++count == a)
            {
              if (elements[3] == "entry")
              {
                // annotation_1234567890
                // 0          ^11
                value = Variant ((time_t) strtol (i.first.substr (11).c_str (), NULL, 10), Variant::type_date);
                return true;
              }
              else if (elements[3] == "description")
              {
                value = Variant (i.second);
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
          for (auto& i : annos)
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
              Date date (i.first.substr (11));
                   if (elements[4] == "year")    { value = Variant (static_cast<int> (date.year ()));      return true; }
              else if (elements[4] == "month")   { value = Variant (static_cast<int> (date.month ()));     return true; }
              else if (elements[4] == "day")     { value = Variant (static_cast<int> (date.day ()));       return true; }
              else if (elements[4] == "week")    { value = Variant (static_cast<int> (date.week ()));      return true; }
              else if (elements[4] == "weekday") { value = Variant (static_cast<int> (date.dayOfWeek ())); return true; }
              else if (elements[4] == "julian")  { value = Variant (static_cast<int> (date.dayOfYear ())); return true; }
              else if (elements[4] == "hour")    { value = Variant (static_cast<int> (date.hour ()));      return true; }
              else if (elements[4] == "minute")  { value = Variant (static_cast<int> (date.minute ()));    return true; }
              else if (elements[4] == "second")  { value = Variant (static_cast<int> (date.second ()));    return true; }
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
