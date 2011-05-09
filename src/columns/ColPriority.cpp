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

#include <Context.h>
#include <ColPriority.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnPriority::ColumnPriority ()
{
  _type  = "string";
  _style = "default";
  _label = "Pri";
}

////////////////////////////////////////////////////////////////////////////////
ColumnPriority::~ColumnPriority ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnPriority::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "long" && _label == "Pri")
    _label = "Priority";
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnPriority::measure (Task& task, int& minimum, int& maximum)
{
  std::string priority = task.get ("priority");

  minimum = maximum = 1;
  if (_style == "long")
  {
         if (priority == "H") minimum = maximum = 4;
    else if (priority == "M") minimum = maximum = 6;
    else if (priority == "L") minimum = maximum = 3;
  }
  else if (_style != "default")
    throw std::string ("Unrecognized column format 'priority.") + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnPriority::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string priority = task.get ("priority");
  if (_style == "long")
  {
         if (priority == "H") priority = "High";
    else if (priority == "M") priority = "Medium";
    else if (priority == "L") priority = "Low";
  }

  lines.push_back (color.colorize (leftJustify (priority, width)));
}

////////////////////////////////////////////////////////////////////////////////
