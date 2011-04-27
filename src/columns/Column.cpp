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

#include <iostream>
#include <Context.h>
#include <Column.h>
#include <ColID.h>
#include <ColProject.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Column* Column::factory (const std::string& name)
{
  if (name == "id")       return new ColumnID ();
  if (name == "project")  return new ColumnProject ();

  throw std::string ("Unrecognized column type '") + name + "'";
  return NULL;
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
void Column::renderHeader (std::vector <std::string>& lines, int width)
{
  // Create a basic label.
  std::string header;
  header.reserve (width);
  header = _label;

  // Right pad with spaces, if necessary.
  int length = characters (_label);
  if (length < width)
    _label += std::string (' ', width - length);

  // Now underline the header, or add a dashed line.
  if (context.config.getBoolean ("fontunderline"))
  {
    lines.push_back (header);
  }
  else
  {
    lines.push_back (header);
    lines.push_back (std::string ('-', width));
  }
}

////////////////////////////////////////////////////////////////////////////////
