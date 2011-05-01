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
#include <ColStatus.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnStatus::ColumnStatus ()
{
  _type  = "string";
  _style = "default";
  _label = "Status";
}

////////////////////////////////////////////////////////////////////////////////
ColumnStatus::~ColumnStatus ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnStatus::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "short" && _label == "Status")
    _label = "St";
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnStatus::measure (Task& task, int& minimum, int& maximum)
{
  Task::status status = task.getStatus ();

  if (_style == "default")
  {
    if (status == Task::pending ||
        status == Task::deleted ||
        status == Task::waiting)
    {
      minimum = maximum = 7;
    }
    else if (status == Task::completed ||
             status == Task::recurring)
    {
      minimum = maximum = 9;
    }
  }
  else if (_style == "short")
    minimum = maximum = 1;
  else
    throw std::string ("Unrecognized column format '") + _type + "." + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnStatus::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  Task::status status = task.getStatus ();
  std::string value;

  if (_style == "default")
  {
         if (status == Task::pending)   value = "Pending";
    else if (status == Task::completed) value = "Completed";
    else if (status == Task::deleted)   value = "Deleted";
    else if (status == Task::waiting)   value = "Waiting";
    else if (status == Task::recurring) value = "Recurring";
  }

  else if (_style == "short")
  {
         if (status == Task::pending)   value = "P";
    else if (status == Task::completed) value = "C";
    else if (status == Task::deleted)   value = "D";
    else if (status == Task::waiting)   value = "W";
    else if (status == Task::recurring) value = "R";
  }

  lines.push_back (color.colorize (leftJustify (value, width)));
}

////////////////////////////////////////////////////////////////////////////////
