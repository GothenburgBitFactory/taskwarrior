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
//#include <ColDepends.h>
#include <ColDescription.h>
//#include <ColDue.h>
//#include <ColEnd.h>
//#include <ColEntry.h>
#include <ColID.h>
#include <ColPriority.h>
#include <ColProject.h>
//#include <ColRecur.h>
//#include <ColStart.h>
#include <ColTags.h>
//#include <ColUntilDepends.h>
#include <ColUUID.h>
//#include <ColWait.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// TODO Supports the new complete column definition:
//
//   <type>[.<format>][.<key>[.<direction>][.<break>]]
//
Column* Column::factory (const std::string& name)
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
//       if (column_name == "depends")     column = new ColumnDepends ();
       if (column_name == "description") column = new ColumnDescription ();
//  else if (column_name == "due")         column = new ColumnDue ();
//  else if (column_name == "end")         column = new ColumnEnd ();
//  else if (column_name == "entry")       column = new ColumnEntry ();
  else if (column_name == "id")          column = new ColumnID ();
  else if (column_name == "priority")    column = new ColumnPriority ();
  else if (column_name == "project")     column = new ColumnProject ();
//  else if (column_name == "recur")       column = new ColumnRecur ();
//  else if (column_name == "start")       column = new ColumnStart ();
  else if (column_name == "tags")        column = new ColumnTags ();
//  else if (column_name == "until")       column = new ColumnUntil ();
  else if (column_name == "uuid")        column = new ColumnUUID ();
//  else if (column_name == "wait")        column = new ColumnWait ();
  else
    throw std::string ("Unrecognized column type '") + column_name + "'";

  column->setStyle (column_style);

/*
  // TODO Load the report column def from config
  // TODO Parse column defs
  // TODO   Create column object
  // TODO   Column: name
  // TODO   Column: style
  // TODO   Column: break

  // TODO Color: odd
  // TODO Color: even
  // TODO Color: intra_odd
  // TODO Color: intra_even
  // TODO Color: extra_odd
  // TODO Color: extra_even
  // TODO Color: header

  // Terminal width.
  view.width (getWidth ());

  // TODO Intra padding.
  // TODO Extra padding.
  // TODO Margin.
  // TODO Truncate lines/page.
*/

  return column;
}

////////////////////////////////////////////////////////////////////////////////
Column::Column ()
: _type ("string")
, _style ("default")
, _label ("")
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
  // Create a basic label.
  std::string header;
  header.reserve (width);
  header = _label;

  // Create a fungible copy.
  Color c = color;

  // Now underline the header, or add a dashed line.
  if (context.config.getBoolean ("fontunderline"))
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

////////////////////////////////////////////////////////////////////////////////
