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
#include <ID.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Column* Column::factory (const std::string& name)
{
  if (name == "id")  return new ColumnID ();

  throw std::string ("Unrecognized column type '") + name + "'";
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
Column::Column ()
: _style ("default")
, _label ("")
, _minimum (0)
, _maximum (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Column::Column (const Column& other)
{
  _label   = other._label;
  _minimum = other._minimum;
  _maximum = other._maximum;
}

////////////////////////////////////////////////////////////////////////////////
Column& Column::operator= (const Column& other)
{
  if (this != &other)
  {
    _label   = other._label;
    _minimum = other._minimum;
    _maximum = other._maximum;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Column::operator== (const Column& other) const
{
  return _label   == other._label   &&
         _minimum == other._minimum &&
         _maximum == other._maximum;
}

////////////////////////////////////////////////////////////////////////////////
Column::~Column ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Column::setStyle (const std::string& style)
{
  _style = style;
}

////////////////////////////////////////////////////////////////////////////////
void Column::setLabel (const std::string& label)
{
  _label = label;
}

////////////////////////////////////////////////////////////////////////////////
