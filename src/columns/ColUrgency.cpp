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
#include <ColUrgency.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnUrgency::ColumnUrgency ()
{
  _type  = "number";
  _style = "default";
  _label = "Urgency";
}

////////////////////////////////////////////////////////////////////////////////
ColumnUrgency::~ColumnUrgency ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnUrgency::measure (Task& task, int& minimum, int& maximum)
{
  minimum = maximum = format (task.urgency (), 4, 3).length ();

  if (_style != "default")
    throw std::string ("Unrecognized column format 'urgency.") + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUrgency::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  lines.push_back (
    color.colorize (
      rightJustify (
        format (task.urgency (), 4, 3), width)));
}

////////////////////////////////////////////////////////////////////////////////
