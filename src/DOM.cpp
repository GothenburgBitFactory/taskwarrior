////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <DOM.h>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <Variant.h>
#include <Context.h>
#include <Nibbler.h>
#include <ISO8601.h>
#include <text.h>
#include <i18n.h>

extern Context context;

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
bool getDOM (const std::string& name, Variant& value)
{
  // Special case, blank refs cause problems.
  if (name == "")
    return false;

  int len = name.length ();
  Nibbler n (name);

  // rc. --> context.config
  if (len > 3 &&
      ! name.compare (0, 3, "rc.", 3))
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
      ! name.compare (0, 8, "context.", 8))
  {
    if (name == "context.program")
    {
      value = Variant (context.cli2.getBinary ());
      return true;
    }
    else if (name == "context.args")
    {
      std::string commandLine;
      for (auto& arg : context.cli2._original_args)
      {
        if (commandLine != "")
           commandLine += " ";

        commandLine += arg.attribute("raw");
      }

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
      ! name.compare (0, 7, "system.", 7))
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
// This code emphasizes speed, hence 'id' and 'urgecny' being evaluated first
// as special cases.
bool getDOM (const std::string& name, const Task& task, Variant& value)
{
  // Special case, blank refs cause problems.
  if (name == "")
    return false;

  // Quickly deal with the most common cases.
  if (task.data.size () && name == "id")
  {
    value = Variant (static_cast<int> (task.id));
    return true;
  }

  if (task.data.size () && name == "urgency")
  {
    value = Variant (task.urgency_c ());
    return true;
  }

  // split name on '.'
  std::vector <std::string> elements;
  split (elements, name, '.');

  Task ref (task);
  Nibbler n (elements[0]);
  n.save ();
  int id;
  std::string uuid;

  // If elements[0] is a UUID, load that task (if necessary), and clobber ref.
  if (n.getPartialUUID (uuid) && n.depleted ())
  {
    if (uuid != ref.get ("uuid"))
      context.tdb2.get (uuid, ref);

    // Eat elements[0]/UUID.
    elements.erase (elements.begin ());
  }
  else
  {
    // If elements[0] is a ID, load that task (if necessary), and clobber ref.
    if (n.getInt (id) && n.depleted ())
    {
      if (id != ref.id)
        context.tdb2.get (id, ref);

      // Eat elements[0]/ID.
      elements.erase (elements.begin ());
    }
  }

  auto size = elements.size ();

  std::string canonical;
  if ((size == 1 || size == 2) && context.cli2.canonicalize (canonical, "attribute", elements[0]))
  {
    // Now that 'ref' is the contextual task, and any ID/UUID is chopped off the
    // elements vector, DOM resolution is now simple.
    if (ref.data.size () && size == 1 && canonical == "id")
    {
      value = Variant (static_cast<int> (ref.id));
      return true;
    }

    if (ref.data.size () && size == 1 && canonical == "urgency")
    {
      value = Variant (ref.urgency_c ());
      return true;
    }

    Column* column = context.columns[canonical];

    if (ref.data.size () && size == 1 && column)
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
      else if (column->type () == "duration" || canonical == "recur")
      {
        auto period = ref.get (canonical);

        ISO8601p iso;
        std::string::size_type cursor = 0;
        if (iso.parse (period, cursor))
          value = Variant ((time_t) iso, Variant::type_duration);
        else
          value = Variant ((time_t) ISO8601p (ref.get (canonical)), Variant::type_duration);
      }
      else if (column->type () == "numeric")
        value = Variant (ref.get_float (canonical));
      else
        value = Variant (ref.get (canonical));

      return true;
    }

    if (ref.data.size () && size == 2 && canonical == "tags")
    {
      value = Variant (ref.hasTag (elements[1]) ? elements[1] : "");
      return true;
    }

    if (ref.data.size () && size == 2 && column && column->type () == "date")
    {
      ISO8601d date (ref.get_date (canonical));
           if (elements[1] == "year")    { value = Variant (static_cast<int> (date.year ()));      return true; }
      else if (elements[1] == "month")   { value = Variant (static_cast<int> (date.month ()));     return true; }
      else if (elements[1] == "day")     { value = Variant (static_cast<int> (date.day ()));       return true; }
      else if (elements[1] == "week")    { value = Variant (static_cast<int> (date.week ()));      return true; }
      else if (elements[1] == "weekday") { value = Variant (static_cast<int> (date.dayOfWeek ())); return true; }
      else if (elements[1] == "julian")  { value = Variant (static_cast<int> (date.dayOfYear ())); return true; }
      else if (elements[1] == "hour")    { value = Variant (static_cast<int> (date.hour ()));      return true; }
      else if (elements[1] == "minute")  { value = Variant (static_cast<int> (date.minute ()));    return true; }
      else if (elements[1] == "second")  { value = Variant (static_cast<int> (date.second ()));    return true; }
    }
  }

  if (ref.data.size () && size == 3 && elements[0] == "annotations")
  {
    std::map <std::string, std::string> annos;
    ref.getAnnotations (annos);

    int a = strtol (elements[1].c_str (), NULL, 10);
    int count = 0;

    // Count off the 'a'th annotation.
    for (const auto& i : annos)
    {
      if (++count == a)
      {
        if (elements[2] == "entry")
        {
          // annotation_1234567890
          // 0          ^11
          value = Variant ((time_t) strtol (i.first.substr (11).c_str (), NULL, 10), Variant::type_date);
          return true;
        }
        else if (elements[2] == "description")
        {
          value = Variant (i.second);
          return true;
        }
      }
    }
  }

  if (ref.data.size () && size == 4 && elements[0] == "annotations" && elements[2] == "entry")
  {
    std::map <std::string, std::string> annos;
    ref.getAnnotations (annos);

    int a = strtol (elements[1].c_str (), NULL, 10);
    int count = 0;

    // Count off the 'a'th annotation.
    for (const auto& i : annos)
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
        ISO8601d date (i.first.substr (11));
             if (elements[3] == "year")    { value = Variant (static_cast<int> (date.year ()));      return true; }
        else if (elements[3] == "month")   { value = Variant (static_cast<int> (date.month ()));     return true; }
        else if (elements[3] == "day")     { value = Variant (static_cast<int> (date.day ()));       return true; }
        else if (elements[3] == "week")    { value = Variant (static_cast<int> (date.week ()));      return true; }
        else if (elements[3] == "weekday") { value = Variant (static_cast<int> (date.dayOfWeek ())); return true; }
        else if (elements[3] == "julian")  { value = Variant (static_cast<int> (date.dayOfYear ())); return true; }
        else if (elements[3] == "hour")    { value = Variant (static_cast<int> (date.hour ()));      return true; }
        else if (elements[3] == "minute")  { value = Variant (static_cast<int> (date.minute ()));    return true; }
        else if (elements[3] == "second")  { value = Variant (static_cast<int> (date.second ()));    return true; }
      }
    }
  }

  // Delegate to the context-free version of DOM::get.
  return getDOM (name, value);
}

////////////////////////////////////////////////////////////////////////////////
