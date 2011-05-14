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
#include <Column.h>
#include <ColDepends.h>
#include <ColDescription.h>
#include <ColDue.h>
#include <ColEnd.h>
#include <ColEntry.h>
#include <ColID.h>
#include <ColPriority.h>
#include <ColProject.h>
#include <ColRecur.h>
#include <ColStart.h>
#include <ColStatus.h>
#include <ColString.h>
#include <ColTags.h>
#include <ColUntil.h>
#include <ColUrgency.h>
#include <ColUUID.h>
#include <ColWait.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Supports the new complete column definition:
//
//   <type>[.<format>]
//
Column* Column::factory (const std::string& name, const std::string& report)
{
  // Decompose name into type and style.
  std::string::size_type dot = name.find ('.');
  std::string column_name;
  std::string column_style;
  if (dot != std::string::npos)
  {
    column_name = name.substr (0, dot);
    column_style = name.substr (dot + 1);
  }
  else
  {
    column_name = name;
    column_style = "default";
  }

  Column* column;
       if (column_name == "depends")     column = new ColumnDepends ();
  else if (column_name == "description") column = new ColumnDescription ();
  else if (column_name == "due")         column = new ColumnDue ();
  else if (column_name == "end")         column = new ColumnEnd ();
  else if (column_name == "entry")       column = new ColumnEntry ();
  else if (column_name == "id")          column = new ColumnID ();
  else if (column_name == "priority")    column = new ColumnPriority ();
  else if (column_name == "project")     column = new ColumnProject ();
  else if (column_name == "recur")       column = new ColumnRecur ();
  else if (column_name == "start")       column = new ColumnStart ();
  else if (column_name == "status")      column = new ColumnStatus ();
  else if (column_name == "tags")        column = new ColumnTags ();
  else if (column_name == "until")       column = new ColumnUntil ();
  else if (column_name == "urgency")     column = new ColumnUrgency ();
  else if (column_name == "uuid")        column = new ColumnUUID ();
  else if (column_name == "wait")        column = new ColumnWait ();

  // Special non-task column
  else if (column_name == "string")      column = new ColumnString ();
  else
    throw std::string ("Unrecognized column name '") + column_name + "'.";

  column->setReport (report);
  column->setStyle (column_style);
  return column;
}

////////////////////////////////////////////////////////////////////////////////
Column::Column ()
: _type ("string")
, _style ("default")
, _label ("")
, _report ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Column::Column (const Column& other)
{
  _type  = other._type;
  _style = other._style;
  _label = other._label;
}

////////////////////////////////////////////////////////////////////////////////
Column& Column::operator= (const Column& other)
{
  if (this != &other)
  {
    _type    = other._type;
    _style   = other._style;
    _label   = other._label;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Column::operator== (const Column& other) const
{
  return _type  == other._type   &&
         _style == other._style   &&
         _label == other._label;
}

////////////////////////////////////////////////////////////////////////////////
Column::~Column ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Column::renderHeader (
  std::vector <std::string>& lines,
  int width,
  Color& color)
{
  if (_label != "")
  {
    // Create a basic label.
    std::string header;
    header.reserve (width);
    header = _label;

    // Create a fungible copy.
    Color c = color;

    // Now underline the header, or add a dashed line.
    if (context.color () &&
        context.config.getBoolean ("fontunderline"))
    {
      c.blend (Color (Color::nocolor, Color::nocolor, true, false, false));
      lines.push_back (c.colorize (leftJustify (header, width)));
    }
    else
    {
      lines.push_back (c.colorize (leftJustify (header, width)));
      lines.push_back (c.colorize (std::string (width, '-')));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Column::measure (const std::string& value, int& minimum, int& maximum)
{
  throw std::string ("Virtual method Column::measure not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
void Column::measure (Task& task, int& minimum, int& maximum)
{
  throw std::string ("Virtual method Column::measure not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
void Column::render (std::vector <std::string>& lines, const std::string& value, int width, Color& color)
{
  throw std::string ("Virtual method Column::render not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
void Column::render (std::vector <std::string>& lines, Task& task, int width, Color& color)
{
  throw std::string ("Virtual method Column::render not overriden.");
}

////////////////////////////////////////////////////////////////////////////////
