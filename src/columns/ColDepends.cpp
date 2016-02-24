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
#include <ColDepends.h>
#include <algorithm>
#include <Context.h>
#include <text.h>
#include <utf8.h>
#include <i18n.h>
#include <main.h>
#include <stdlib.h>

extern Context context;

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
                context.config.get ("dependency.indicator")};

  _hyphenate = false;
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnDepends::setStyle (const std::string& value)
{
  _style = value;

       if (_style == "indicator" && _label == STRING_COLUMN_LABEL_DEP) _label = _label.substr (0, context.config.get ("dependency.indicator").length ());
  else if (_style == "count"     && _label == STRING_COLUMN_LABEL_DEP) _label = STRING_COLUMN_LABEL_DEP_S;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDepends::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  std::vector <Task> blocking;
  dependencyGetBlocking (task, blocking);

  if (_style == "indicator")
  {
    if (task.has ("depends"))
      minimum = maximum = utf8_width (context.config.get ("dependency.indicator"));
    else
      minimum = maximum = 0;
  }
  else if (_style == "count")
  {
    minimum = maximum = 2 + format ((int) blocking.size ()).length ();
  }
  else if (_style == "default" ||
           _style == "list")
  {
    minimum = maximum = 0;
    if (task.has ("depends"))
    {
      std::vector <int> blocking_ids;
      for (auto& i : blocking)
        blocking_ids.push_back (i.id);

      std::string all;
      join (all, " ", blocking_ids);
      maximum = all.length ();

      unsigned int length;
      for (auto& i : blocking)
      {
        length = format (i.id).length ();
        if (length > minimum)
          minimum = length;
      }
    }
  }
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDepends::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "indicator")
      renderStringRight (lines, width, color, context.config.get ("dependency.indicator"));
    else
    {
      std::vector <Task> blocking;
      dependencyGetBlocking (task, blocking);

      if (_style == "count")
        renderStringRight (lines, width, color, "[" + format (static_cast <int>(blocking.size ())) + "]");

      else if (_style == "default" ||
               _style == "list")
      {
        std::vector <int> blocking_ids;
        for (const auto& t : blocking)
          blocking_ids.push_back (t.id);

        std::string combined;
        join (combined, " ", blocking_ids);

        std::vector <std::string> all;
        wrapText (all, combined, width, _hyphenate);

        for (const auto& i : all)
          renderStringLeft (lines, width, color, i);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDepends::modify (Task& task, const std::string& value)
{
  // Parse IDs
  std::vector <std::string> deps;
  split (deps, value, ',');

  // Apply or remove dendencies in turn.
  for (auto& dep : deps)
  {
    if (dep[0] == '-')
    {
      if (dep.length () == 37)
        task.removeDependency (dep.substr (1));
      else
        task.removeDependency (strtol (dep.substr (1).c_str (), NULL, 10));
    }
    else
    {
      if (dep.length () == 36)
        task.addDependency (dep);
      else
        task.addDependency (strtol (dep.c_str (), NULL, 10));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
