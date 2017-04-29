////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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
#include <Lexer.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// DOM Supported References:
//
// Configuration:
//   rc.<name>
//
// Taskwarrior:
//   tw.syncneeded
//   tw.program
//   tw.args
//   tw.width
//   tw.height
//
// System:
//   context.program         // 2017-02-25 Deprecated in 2.6.0
//   context.args            // 2017-02-25 Deprecated in 2.6.0
//   context.width           // 2017-02-25 Deprecated in 2.6.0
//   context.height          // 2017-02-25 Deprecated in 2.6.0
//   system.version
//   system.os
//
bool getDOM (const std::string& name, Variant& value)
{
  // Special case, blank refs cause problems.
  if (name == "")
    return false;

  auto len = name.length ();

  // rc. --> context.config
  if (len > 3 &&
      ! name.compare (0, 3, "rc.", 3))
  {
    auto key = name.substr (3);
    auto c = context.config.find (key);
    if (c != context.config.end ())
    {
      value = Variant (c->second);
      return true;
    }

    return false;
  }

  // tw.*
  if (len > 3 &&
      ! name.compare (0, 3, "tw.", 3))
  {
    if (name == "tw.syncneeded")
    {
      value = Variant (0);
      for (const auto& line : context.tdb2.backlog.get_lines ())
      {
        if (line[0] == '{')
        {
          value = Variant (1);
          break;
        }
      }

      return true;
    }
    else if (name == "tw.program")
    {
      value = Variant (context.cli2.getBinary ());
      return true;
    }
    else if (name == "tw.args")
    {
      std::string commandLine;
      for (auto& arg : context.cli2._original_args)
      {
        if (commandLine != "")
           commandLine += ' ';

        commandLine += arg.attribute("raw");
      }

      value = Variant (commandLine);
      return true;
    }
    else if (name == "tw.width")
    {
      value = Variant (static_cast<int> (context.terminal_width
                                           ? context.terminal_width
                                           : context.getWidth ()));
      return true;
    }
    else if (name == "tw.height")
    {
      value = Variant (static_cast<int> (context.terminal_height
                                           ? context.terminal_height
                                           : context.getHeight ()));
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
           commandLine += ' ';

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

    return false;
  }

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
      value = Variant (osName ());
      return true;
    }

    return false;
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
//   annotations.count
//   annotations.<N>.entry
//   annotations.<N>.description
//
// This code emphasizes speed, hence 'id' and 'urgency' being evaluated first
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
  auto elements = split (name, '.');

  Task ref (task);
  Lexer lexer (elements[0]);
  std::string token;
  Lexer::Type type;
  if (lexer.token (token, type))
  {
    if (type == Lexer::Type::uuid &&
        token.length () == elements[0].length ())
    {
      if (token != ref.get ("uuid"))
        context.tdb2.get (token, ref);

      // Eat elements[0]/UUID.
      elements.erase (elements.begin ());
    }
    else if (type == Lexer::Type::number &&
             token.find ('.') == std::string::npos)
    {
      auto id = strtol (token.c_str (), NULL, 10);
      if (id && id != ref.id)
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

        Duration iso;
        std::string::size_type cursor = 0;
        if (iso.parse (period, cursor))
          value = Variant (iso.toTime_t (), Variant::type_duration);
        else
          value = Variant (Duration (ref.get (canonical)).toTime_t (), Variant::type_duration);
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
      Datetime date (ref.get_date (canonical));
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

  if (ref.data.size () && size == 2 && elements[0] == "annotations" && elements[1] == "count")
  {
    value = Variant (static_cast<int> (ref.getAnnotationCount ()));
    return true;
  }

  if (ref.data.size () && size == 3 && elements[0] == "annotations")
  {
    auto annos = ref.getAnnotations ();

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
    auto annos = ref.getAnnotations ();

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
        Datetime date (i.first.substr (11));
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
// DOM Class
//
// References are paths into a tree structure. For example:
//
//   1.due.month.number
//   1.due.day.number
//
// Are represented internally as:
//
//   1
//   +- due
//     +- day
//     | +- number
//     +- month
//       +- number
//
// The tree is augmented by other elements:
//
//   1
//   +- due
//   | +- day
//   | | +- number
//   | +- month
//   |   +- number
//   +- system
//      +- os
//
// Each node in the tree has a name ("due", "system", "day"), and each node may
// have a data source attached to it.
//
// The DOM class is independent of the project, in that it knows nothing about
// the internal data or program structure. It knows only that certain DOM path
// elements have handlers which will provide the data.
//
// The DOM class is therefore responsible for maintaining a tree of named nodes
// with associated proividers. When a reference value is requested, the DOM
// class will decompose the reference path, and navigate the tree to the lowest
// level provider, and call it.
//
// This makes the DOM class a reusible object.

////////////////////////////////////////////////////////////////////////////////
void DOM::addSource (
  const std::string&,
  bool (*provider)(const std::string&, Variant&))
{
  // TODO Implement.
}

////////////////////////////////////////////////////////////////////////////////
bool DOM::valid (const std::string& reference) const
{
  // TODO Implement.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
Variant DOM::get (const std::string& reference) const
{
  Variant v ("");

  // Find the provider.
  // TODO Start at the root of the tree.
  for (const auto& element : decomposeReference (reference))
  {
    // TODO If tree contains a named node 'element', capture the provider, if any.
  }

  return v;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> DOM::decomposeReference (const std::string& reference) const
{
  return split (reference, '.');
}

////////////////////////////////////////////////////////////////////////////////
int DOM::count () const
{
  // Recurse and count the branches.
  int total {0};
  for (auto& i : _branches)
    total += i->count ();

  return total;
}

////////////////////////////////////////////////////////////////////////////////
std::shared_ptr <DOM> DOM::find (const std::string& path)
{
  std::vector <std::string> elements = split (path, '.');

  // Must start at the trunk.
  auto cursor = std::make_shared <DOM> (*this);
  auto it = elements.begin ();
  if (cursor->_name != *it)
    return nullptr;

  // Perhaps the trunk is what is needed?
  if (elements.size () == 1)
    return cursor;

  // Now look for the next branch.
  for (++it; it != elements.end (); ++it)
  {
    bool found = false;

    // If the cursor has a branch that matches *it, proceed.
    for (auto i = cursor->_branches.begin (); i != cursor->_branches.end (); ++i)
    {
      if ((*i)->_name == *it)
      {
        cursor = *i;
        found = true;
        break;
      }
    }

    if (! found)
      return nullptr;
  }

  return cursor;
}

////////////////////////////////////////////////////////////////////////////////
std::string DOM::dumpNode (
  const std::shared_ptr <DOM> t,
  int depth) const
{
  std::stringstream out;

  // Dump node
  for (int i = 0; i < depth; ++i)
    out << "  ";

  out
      // Useful for debugging tree node new/delete errors.
      // << std::hex << t << " "
      << "\033[1m" << t->_name << "\033[0m\n";

  // Recurse for branches.
  for (auto& b : t->_branches)
    out << dumpNode (b, depth + 1);

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string DOM::dump () const
{
  std::stringstream out;
  out << "DOM (" << count () << " nodes)\n"
      << dumpNode (std::make_shared <DOM> (*this), 1);

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
