////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
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
//   tw.version
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
    auto c = Context::getContext ().config.find (key);
    if (c != Context::getContext ().config.end ())
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
      for (const auto& line : Context::getContext ().tdb2.backlog.get_lines ())
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
      value = Variant (Context::getContext ().cli2.getBinary ());
      return true;
    }
    else if (name == "tw.args")
    {
      std::string commandLine;
      for (auto& arg : Context::getContext ().cli2._original_args)
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
      value = Variant (static_cast<int> (Context::getContext ().terminal_width
                                           ? Context::getContext ().terminal_width
                                           : Context::getContext ().getWidth ()));
      return true;
    }
    else if (name == "tw.height")
    {
      value = Variant (static_cast<int> (Context::getContext ().terminal_height
                                           ? Context::getContext ().terminal_height
                                           : Context::getContext ().getHeight ()));
      return true;
    }

    else if (name == "tw.version")
    {
      value = Variant (VERSION);
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
      value = Variant (Context::getContext ().cli2.getBinary ());
      return true;
    }
    else if (name == "context.args")
    {
      std::string commandLine;
      for (auto& arg : Context::getContext ().cli2._original_args)
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
      value = Variant (static_cast<int> (Context::getContext ().terminal_width
                                           ? Context::getContext ().terminal_width
                                           : Context::getContext ().getWidth ()));
      return true;
    }
    else if (name == "context.height")
    {
      value = Variant (static_cast<int> (Context::getContext ().terminal_height
                                           ? Context::getContext ().terminal_height
                                           : Context::getContext ().getHeight ()));
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
//
// If task is NULL, then the contextual task will be determined from the DOM
// string, if any exists.
bool getDOM (const std::string& name, const Task* task, Variant& value)
{
  // Special case, blank refs cause problems.
  if (name == "")
    return false;

  // Quickly deal with the most common cases.
  if (task && name == "id")
  {
    value = Variant (static_cast<int> (task->id));
    return true;
  }

  if (task && name == "urgency")
  {
    value = Variant (task->urgency_c ());
    return true;
  }

  // split name on '.'
  auto elements = split (name, '.');
  Task loaded_task;

  // decide whether the reference is going to be the passed
  // "task" or whether it's going to be a newly loaded task (if id/uuid was
  // given).
  const Task* ref = task;
  Lexer lexer (elements[0]);
  std::string token;
  Lexer::Type type;

  // If this can be ID/UUID reference (the name contains '.'),
  // lex it to figure out. Otherwise don't lex, as lexing can be slow.
  if ((elements.size() > 1) and lexer.token (token, type))
  {
    bool reloaded = false;

    if (type == Lexer::Type::uuid &&
        token.length () == elements[0].length ())
    {
      if (!task || token != task->get ("uuid"))
      {
        if (Context::getContext ().tdb2.get (token, loaded_task))
          reloaded = true;
      }

      // Eat elements[0]/UUID.
      elements.erase (elements.begin ());
    }
    else if (type == Lexer::Type::number &&
             token.find ('.') == std::string::npos)
    {
      auto id = strtol (token.c_str (), nullptr, 10);
      if (id && (!task || id != task->id))
      {
        if (Context::getContext ().tdb2.get (id, loaded_task))
          reloaded = true;
      }

      // Eat elements[0]/ID.
      elements.erase (elements.begin ());
    }

    if (reloaded)
      ref = &loaded_task;
  }


  // The remainder of this method requires a contextual task, so if we do not
  // have one, delegate to the two-argument getDOM
  if (!ref)
    return getDOM (name, value);

  auto size = elements.size ();

  std::string canonical;
  if ((size == 1 || size == 2) && Context::getContext ().cli2.canonicalize (canonical, "attribute", elements[0]))
  {
    // Now that 'ref' is the contextual task, and any ID/UUID is chopped off the
    // elements vector, DOM resolution is now simple.
    if (size == 1 && canonical == "id")
    {
      value = Variant (static_cast<int> (ref->id));
      return true;
    }

    if (size == 1 && canonical == "urgency")
    {
      value = Variant (ref->urgency_c ());
      return true;
    }

    // Special handling of status required for virtual waiting status
    // implementation. Remove in 3.0.0.
    if (size == 1 && canonical == "status")
    {
      value = Variant (ref->statusToText (ref->getStatus ()));
      return true;
    }

    Column* column = Context::getContext ().columns[canonical];

    if (size == 1 && column)
    {
      if (column->is_uda () && ! ref->has (canonical))
      {
        value = Variant ("");
        return true;
      }

      if (column->type () == "date")
      {
        auto numeric = ref->get_date (canonical);
        if (numeric == 0)
          value = Variant ("");
        else
          value = Variant (numeric, Variant::type_date);
      }
      else if (column->type () == "duration" || canonical == "recur")
      {
        auto period = ref->get (canonical);

        Duration iso;
        std::string::size_type cursor = 0;
        if (iso.parse (period, cursor))
          value = Variant (iso.toTime_t (), Variant::type_duration);
        else
          value = Variant (Duration (ref->get (canonical)).toTime_t (), Variant::type_duration);
      }
      else if (column->type () == "numeric")
        value = Variant (ref->get_float (canonical));
      else
        value = Variant (ref->get (canonical));

      return true;
    }

    if (size == 2 && canonical == "tags")
    {
      value = Variant (ref->hasTag (elements[1]) ? elements[1] : "");
      return true;
    }

    if (size == 2 && column && column->type () == "date")
    {
      Datetime date (ref->get_date (canonical));
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

  if (size == 2 && elements[0] == "annotations" && elements[1] == "count")
  {
    value = Variant (static_cast<int> (ref->getAnnotationCount ()));
    return true;
  }

  if (size == 3 && elements[0] == "annotations")
  {
    auto annos = ref->getAnnotations ();

    int a = strtol (elements[1].c_str (), nullptr, 10);
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

  if (size == 4 && elements[0] == "annotations" && elements[2] == "entry")
  {
    auto annos = ref->getAnnotations ();

    int a = strtol (elements[1].c_str (), nullptr, 10);
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
DOM::~DOM ()
{
  delete _node;
}

////////////////////////////////////////////////////////////////////////////////
void DOM::addSource (
  const std::string& reference,
  bool (*provider)(const std::string&, Variant&))
{
  if (_node == nullptr)
    _node = new DOM::Node ();

  _node->addSource (reference, provider);
}

////////////////////////////////////////////////////////////////////////////////
bool DOM::valid (const std::string& reference) const
{
  return _node && _node->find (reference) != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
Variant DOM::get (const std::string& reference) const
{
  Variant v ("");

  if (_node)
  {
    auto node = _node->find (reference);
    if (node            != nullptr &&
        node->_provider != nullptr)
    {
      if (node->_provider (reference, v))
        return v;
    }
  }

  return v;
}

////////////////////////////////////////////////////////////////////////////////
int DOM::count () const
{
  if (_node)
    return _node->count ();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> DOM::decomposeReference (const std::string& reference)
{
  return split (reference, '.');
}

////////////////////////////////////////////////////////////////////////////////
std::string DOM::dump () const
{
  if (_node)
    return _node->dump ();

  return "";
}

////////////////////////////////////////////////////////////////////////////////
DOM::Node::~Node ()
{
  for (auto& branch : _branches)
    delete branch;
}

////////////////////////////////////////////////////////////////////////////////
void DOM::Node::addSource (
  const std::string& reference,
  bool (*provider)(const std::string&, Variant&))
{
  auto cursor = this;
  for (const auto& element : DOM::decomposeReference (reference))
  {
    auto found {false};
    for (auto& branch : cursor->_branches)
    {
      if (branch->_name == element)
      {
        cursor = branch;
        found = true;
        break;
      }
    }

    if (! found)
    {
      auto branch = new DOM::Node ();
      branch->_name = element;
      cursor->_branches.push_back (branch);
      cursor = branch;
    }
  }

  cursor->_provider = provider;
}

////////////////////////////////////////////////////////////////////////////////
// A valid reference is one that has a provider function.
bool DOM::Node::valid (const std::string& reference) const
{
  return find (reference) != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
const DOM::Node* DOM::Node::find (const std::string& reference) const
{
  auto cursor = this;
  for (const auto& element : DOM::decomposeReference (reference))
  {
    auto found {false};
    for (auto& branch : cursor->_branches)
    {
      if (branch->_name == element)
      {
        cursor = branch;
        found = true;
        break;
      }
    }

    if (! found)
      break;
  }

  if (reference.length () && cursor != this)
    return cursor;

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
int DOM::Node::count () const
{
  // Recurse and count the branches.
  int total {0};
  for (auto& branch : _branches)
  {
    if (branch->_provider)
      ++total;
    total += branch->count ();
  }

  return total;
}

////////////////////////////////////////////////////////////////////////////////
std::string DOM::Node::dumpNode (
  const DOM::Node* node,
  int depth) const
{
  std::stringstream out;

  // Indent.
  out << std::string (depth * 2, ' ');

  out << "\033[31m" << node->_name << "\033[0m";

  if (node->_provider)
    out << " 0x" << std::hex << (long long) (void*) node->_provider;

  out << '\n';

  // Recurse for branches.
  for (auto& b : node->_branches)
    out << dumpNode (b, depth + 1);

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string DOM::Node::dump () const
{
  std::stringstream out;
  out << "DOM::Node (" << count () << " nodes)\n"
      << dumpNode (this, 1);

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
