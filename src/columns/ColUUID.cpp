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

#include <math.h>
#include <Context.h>
#include <ColUUID.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnUUID::ColumnUUID ()
{
  _type  = "string";
  _style = "default";
  _label = "UUID";
}

////////////////////////////////////////////////////////////////////////////////
ColumnUUID::~ColumnUUID ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnUUID::measure (Task& task, int& minimum, int& maximum)
{
       if (_style == "default") minimum = maximum = 36;
  else if (_style == "short")   minimum = maximum = 8;
  else
    throw std::string ("Unrecognized column format 'uuid.") + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUUID::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  // f30cb9c3-3fc0-483f-bfb2-3bf134f00694  default
  //                             34f00694  short
  if (_style == "default")
    lines.push_back (color.colorize (leftJustify (task.get ("uuid"), width)));

  else if (_style == "short")
    lines.push_back (color.colorize (leftJustify (task.get ("uuid").substr (28), width)));
}

////////////////////////////////////////////////////////////////////////////////
