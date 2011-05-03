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
#include <Nibbler.h>
#include <ColRecur.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::ColumnRecur ()
{
  _type  = "string";
  _style = "default";
  _label = "Recur";
}

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::~ColumnRecur ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnRecur::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "indicator" && _label == "Recur")
    _label = _label.substr (0, context.config.get ("recurrence.indicator").length ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnRecur::measure (Task& task, int& minimum, int& maximum)
{
  if (_style == "default")
  {
    minimum = maximum = task.get ("recur").length ();
  }
  else if (_style == "indicator")
  {
    if (task.has ("recur"))
      minimum = maximum = context.config.get ("recurrence.indicator").length ();
  }
  else
    throw std::string ("Unrecognized column format '") + _type + "." + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnRecur::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (_style == "default")
  {
    lines.push_back (color.colorize (rightJustify (task.get ("recur"), width)));
  }
  else if (_style == "indicator")
  {
    if (task.has ("recur"))
      lines.push_back (
        color.colorize (
          rightJustify (context.config.get ("recurrence.indicator").length (), width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
