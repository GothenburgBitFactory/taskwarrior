////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <Context.h>
#include <Nibbler.h>
#include <ColDepends.h>
#include <text.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDepends::ColumnDepends ()
{
  _type  = "string";
  _style = "default";
  _label = "Depends";
}

////////////////////////////////////////////////////////////////////////////////
ColumnDepends::~ColumnDepends ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnDepends::setStyle (const std::string& value)
{
  _style = value;

       if (_style == "indicator" && _label == "Depends") _label = _label.substr (0, context.config.get ("dependency.indicator").length ());
  else if (_style == "count"     && _label == "Depends") _label = "Dep";
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDepends::measure (Task& task, int& minimum, int& maximum)
{
  std::vector <Task> blocked;
  dependencyGetBlocking (task, blocked);

       if (_style == "indicator") minimum = maximum = context.config.get ("dependency.indicator").length ();
  else if (_style == "count")     minimum = maximum = 2 + format ((int) blocked.size ()).length ();
  else if (_style == "default")
  {
    minimum = maximum = 0;
    if (task.has ("depends"))
    {
      std::vector <int> blocked_ids;
      std::vector <Task>::iterator i;
      for (i = blocked.begin (); i != blocked.end (); ++i)
        blocked_ids.push_back (i->id);

      std::string all;
      join (all, " ", blocked_ids);
      maximum = all.length ();

      int length;
      for (i = blocked.begin (); i != blocked.end (); ++i)
      {
        length = format (i->id).length ();
        if (length > minimum)
          minimum = length;
      }
    }
  }
  else
    throw std::string ("Unrecognized column format '") + _type + "." + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDepends::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has ("depends"))
  {
    if (_style == "indicator")
    {
      lines.push_back (
        color.colorize (
          rightJustify (context.config.get ("dependency.indicator"), width)));
      return;
    }

    std::vector <Task> blocked;
    dependencyGetBlocking (task, blocked);

    if (_style == "count")
    {
      lines.push_back (
        color.colorize (
          rightJustify ("[" + format ((int)blocked.size ()) + "]", width)));
    }
    else if (_style == "default")
    {
      std::vector <Task> blocked;
      dependencyGetBlocking (task, blocked);

      std::string combined;
      std::vector <int> blocked_ids;
      join (combined, " ", blocked_ids);

      std::vector <std::string> all;
      wrapText (all, combined, width);

      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        lines.push_back (color.colorize (leftJustify (*i, width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
