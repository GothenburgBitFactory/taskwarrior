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
#include <ColDepends.h>
#include <algorithm>
#include <Context.h>
#include <shared.h>
#include <format.h>
#include <utf8.h>
#include <main.h>
#include <util.h>
#include <stdlib.h>
#include <regex>

#define STRING_COLUMN_LABEL_DEP "Depends"

////////////////////////////////////////////////////////////////////////////////
ColumnDepends::ColumnDepends ()
{
  _name      = "depends";
  _style     = "list";
  _label     = STRING_COLUMN_LABEL_DEP;
  _styles    = {"list",
                "count",
                "indicator"};
  _examples  = {"1 2 10",
                "[3]",
                Context::getContext ().config.get ("dependency.indicator")};

  _hyphenate = false;
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnDepends::setStyle (const std::string& value)
{
  Column::setStyle (value);

       if (_style == "indicator" && _label == STRING_COLUMN_LABEL_DEP) _label = _label.substr (0, Context::getContext ().config.get ("dependency.indicator").length ());
  else if (_style == "count"     && _label == STRING_COLUMN_LABEL_DEP) _label = "Dep";
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDepends::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;
  auto deptasks = task.getDependencyTasks ();

  if (deptasks.size () > 0)
  {
    if (_style == "indicator")
    {
      minimum = maximum = utf8_width (Context::getContext ().config.get ("dependency.indicator"));
    }

    else if (_style == "count")
    {
      minimum = maximum = 2 + format ((int) deptasks.size ()).length ();
    }

    else if (_style == "default" ||
             _style == "list")
    {
      minimum = maximum = 0;

      std::vector <int> blocking_ids;
      blocking_ids.reserve(deptasks.size());
      for (auto& i : deptasks)
        blocking_ids.push_back (i.id);

      auto all = join (" ", blocking_ids);
      maximum = all.length ();

      unsigned int length;
      for (auto& i : deptasks)
      {
        length = format (i.id).length ();
        if (length > minimum)
          minimum = length;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDepends::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  auto deptasks = task.getDependencyTasks ();

  if (deptasks.size () > 0)
  {
    if (_style == "indicator")
    {
      renderStringRight (lines, width, color, Context::getContext ().config.get ("dependency.indicator"));
    }

    else if (_style == "count")
    {
      renderStringRight (lines, width, color, '[' + format (static_cast <int>(deptasks.size ())) + ']');
    }

    else if (_style == "default" ||
             _style == "list")
    {
      std::vector <int> blocking_ids;
      blocking_ids.reserve(deptasks.size());
      for (const auto& t : deptasks)
        blocking_ids.push_back (t.id);

      auto combined = join (" ", blocking_ids);

      std::vector <std::string> all;
      wrapText (all, combined, width, _hyphenate);

      for (const auto& i : all)
        renderStringLeft (lines, width, color, i);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDepends::modify (Task& task, const std::string& value)
{
  // Apply or remove dendencies in turn.
  for (auto& dep : split (value, ','))
  {
    bool removal = false;
    if (dep[0] == '-')
    {
      removal = true;
      dep = dep.substr(1);
    }

    auto hyphen = dep.find ('-');
    long lower, upper;  // For ID ranges
    std::regex valid_uuid ("[a-f0-9]{8}([a-f0-9-]{4,28})?"); // TODO: Make more precise

    // UUID
    if (dep.length () >= 8 && std::regex_match (dep, valid_uuid))
    {
       // Full UUID, can be added directly
       if (dep.length () == 36)
         if (removal)
           task.removeDependency (dep);
         else
           task.addDependency (dep);

       // Short UUID, need to look up full form
       else
       {
         Task loaded_task;
         if (Context::getContext ().tdb2.get (dep, loaded_task))
           if (removal)
             task.removeDependency (loaded_task.get ("uuid"));
           else
             task.addDependency (loaded_task.get ("uuid"));
         else
           throw format ("Dependency could not be set - task with UUID '{1}' does not exist.", dep);
       }
    }
    // ID range
    else if (dep.find ('-') != std::string::npos &&
             extractLongInteger (dep.substr (0, hyphen), lower) &&
             extractLongInteger (dep.substr (hyphen + 1), upper))
    {
      for (long i = lower; i <= upper; i++)
         if (removal)
           task.removeDependency (i);
         else
           task.addDependency (i);
    }
    // Simple ID
    else if (extractLongInteger (dep, lower))
      if (removal)
        task.removeDependency (lower);
      else
        task.addDependency (lower);
    else
      throw format ("Invalid dependency value: '{1}'", dep);
  }
}

////////////////////////////////////////////////////////////////////////////////
