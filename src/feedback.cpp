////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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

#define L10N                                           // Localization complete.

#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <inttypes.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <cmake.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
bool taskDiff (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  Task::const_iterator att;
  for (att = before.begin (); att != before.end (); ++att)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  for (att = after.begin (); att != after.end (); ++att)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  if (beforeOnly.size () !=
      afterOnly.size ())
    return true;

  std::vector <std::string>::iterator name;
  for (name = beforeAtts.begin (); name != beforeAtts.end (); ++name)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string taskDifferences (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  Task::const_iterator att;
  for (att = before.begin (); att != before.end (); ++att)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  for (att = after.begin (); att != after.end (); ++att)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  std::vector <std::string>::iterator name;
  for (name = beforeOnly.begin (); name != beforeOnly.end (); ++name)
    out << "  - "
        << format (STRING_FEEDBACK_DELETED, ucFirst (*name))
        << "\n";

  for (name = afterOnly.begin (); name != afterOnly.end (); ++name)
  {
    if (*name == "depends")
    {
      std::vector <int> deps_after;
      after.getDependencies (deps_after);
      std::string to;
      join (to, ", ", deps_after);

      out << "  - "
          << format (STRING_FEEDBACK_DEP_SET, to)
          << "\n";
    }
    else
      out << "  - "
          << format (STRING_FEEDBACK_ATT_SET,
                     ucFirst (*name),
                     renderAttribute (*name, after.get (*name)))
          << "\n";
  }

  for (name = beforeAtts.begin (); name != beforeAtts.end (); ++name)
  {
    // Ignore UUID differences, and find values that changed, but are not also
    // in the beforeOnly and afterOnly lists, which have been handled above..
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name) &&
        std::find (beforeOnly.begin (), beforeOnly.end (), *name) == beforeOnly.end () &&
        std::find (afterOnly.begin (),  afterOnly.end (),  *name) == afterOnly.end ())
    {
      if (*name == "depends")
      {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        std::vector <int> deps_after;
        after.getDependencies (deps_after);
        std::string to;
        join (to, ", ", deps_after);

        out << "  - "
            << format (STRING_FEEDBACK_DEP_MOD, from, to)
            << "\n";
      }
      else
        out << "  - "
            << format (STRING_FEEDBACK_ATT_MOD,
                       ucFirst (*name),
                       renderAttribute (*name, before.get (*name)),
                       renderAttribute (*name, after.get (*name)))
            << "\n";
    }
  }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << "  - "
        << STRING_FEEDBACK_NOP
        << "\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string taskInfoDifferences (const Task& before, const Task& after)
{
  // Attributes are all there is, so figure the different attribute names
  // between before and after.
  std::vector <std::string> beforeAtts;
  Task::const_iterator att;
  for (att = before.begin (); att != before.end (); ++att)
    beforeAtts.push_back (att->first);

  std::vector <std::string> afterAtts;
  for (att = after.begin (); att != after.end (); ++att)
    afterAtts.push_back (att->first);

  std::vector <std::string> beforeOnly;
  std::vector <std::string> afterOnly;
  listDiff (beforeAtts, afterAtts, beforeOnly, afterOnly);

  // Now start generating a description of the differences.
  std::stringstream out;
  std::vector <std::string>::iterator name;
  for (name = beforeOnly.begin (); name != beforeOnly.end (); ++name)
  {
    if (*name == "depends")
    {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        out << format (STRING_FEEDBACK_DEP_DEL, from)
            << "\n";
    }
    else if (name->substr (0, 11) == "annotation_")
    {
      out << format (STRING_FEEDBACK_ANN_DEL, before.get (*name))
          << "\n";
    }
    else
    {
      out << format (STRING_FEEDBACK_ATT_DEL, ucFirst (*name))
          << "\n";
    }
  }

  for (name = afterOnly.begin (); name != afterOnly.end (); ++name)
  {
    if (*name == "depends")
    {
      std::vector <int> deps_after;
      after.getDependencies (deps_after);
      std::string to;
      join (to, ", ", deps_after);

      out << format (STRING_FEEDBACK_DEP_WAS_SET, to)
          << "\n";
    }
    else if (name->substr (0, 11) == "annotation_")
    {
      out << format (STRING_FEEDBACK_ANN_ADD, after.get (*name))
          << "\n";
    }
    else
      out << format (STRING_FEEDBACK_ATT_WAS_SET,
                     ucFirst (*name),
                     renderAttribute (*name, after.get (*name)))
          << "\n";
  }

  for (name = beforeAtts.begin (); name != beforeAtts.end (); ++name)
    if (*name              != "uuid" &&
        before.get (*name) != after.get (*name) &&
        before.get (*name) != "" && after.get (*name) != "")
    {
      if (*name == "depends")
      {
        std::vector <int> deps_before;
        before.getDependencies (deps_before);
        std::string from;
        join (from, ", ", deps_before);

        std::vector <int> deps_after;
        after.getDependencies (deps_after);
        std::string to;
        join (to, ", ", deps_after);

        out << format (STRING_FEEDBACK_DEP_WAS_MOD, from, to)
            << "\n";
      }
      else if (name->substr (0, 11) == "annotation_")
      {
        out << format (STRING_FEEDBACK_ANN_WAS_MOD, after.get (*name))
            << "\n";
      }
      else
        out << format (STRING_FEEDBACK_ATT_WAS_MOD,
                       ucFirst (*name),
                       renderAttribute (*name, before.get (*name)),
                       renderAttribute (*name, after.get (*name)))
            << "\n";
    }

  // Shouldn't just say nothing.
  if (out.str ().length () == 0)
    out << STRING_FEEDBACK_WAS_NOP
        << "\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string renderAttribute (const std::string& name, const std::string& value)
{
  Column* col = context.columns[name];
  if (col                    &&
      col->type () == "date" &&
      value != "")
  {
    Date d ((time_t)strtol (value.c_str (), NULL, 10));
    return d.toString (context.config.get ("dateformat"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Implements:
//    <string>
void feedback_affected (const std::string& effect)
{
  if (context.verbose ("affected") ||
      context.config.getBoolean ("echo.command")) // Deprecated 2.0
  {
    std::cout << effect << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Implements:
//    Deleted 3 tasks
//
// The 'effect' string should contain:
//    {1}    Quantity
void feedback_affected (const std::string& effect, int quantity)
{
  if (context.verbose ("affected") ||
      context.config.getBoolean ("echo.command")) // Deprecated 2.0
  {
    std::cout << format (effect, quantity)
              << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Implements:
//    Deleting task 123 'This is a test'
//
// The 'effect' string should contain:
//    {1}    ID
//    {2}    Description
void feedback_affected (const std::string& effect, const Task& task)
{
  if (context.verbose ("affected") ||
      context.config.getBoolean ("echo.command")) // Deprecated 2.0
  {
    if (task.id)
      std::cout << format (effect, task.id, task.get ("description"))
                << "\n";
    else
      std::cout << format (effect, task.get ("uuid"), task.get ("description"))
                << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Implements feedback when adding special tags to a task.
void feedback_special_tags (const Task& task, const std::string& tag)
{
  if (context.verbose ("special"))
  {
    std::string msg;
    std::string explanation;
         if (tag == "nocolor") msg = STRING_FEEDBACK_TAG_NOCOLOR;
    else if (tag == "nonag")   msg = STRING_FEEDBACK_TAG_NONAG;
    else if (tag == "nocal")   msg = STRING_FEEDBACK_TAG_NOCAL;
    else if (tag == "next")    msg = STRING_FEEDBACK_TAG_NEXT;

    if (msg.length ())
    {
      if (task.id)
        std::cout << format (msg, task.id)
                  << "\n";
      else
        std::cout << format (msg, task.get ("uuid"))
                  << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

