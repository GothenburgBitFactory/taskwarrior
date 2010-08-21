////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include <Column.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
static Column* Column::factory (const std::string& name)
{
  if (name == "description")  return new ColumnDescription ();

  throw std::string ("Unrecognized column type '") + name + "'";
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
Column::Column ()
: _name ("")
, _minimum (0)
, _maximum (0)
, _wrap (false)
, _just (left)
, _sizing (minimal)
{
}

////////////////////////////////////////////////////////////////////////////////
Column::Column (const Column& other)
{
  _name    = other._name;
  _minimum = other._minimum;
  _maximum = other._maximum;
  _wrap    = other._wrap;
  _just    = other._just;
  _sizing  = other._sizing;
}

////////////////////////////////////////////////////////////////////////////////
Column& Column::operator= (const Column& other)
{
  if (this != &other)
  {
    _name    = other._name;
    _minimum = other._minimum;
    _maximum = other._maximum;
    _wrap    = other._wrap;
    _just    = other._just;
    _sizing  = other._sizing;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Column::operator== (const Column& other) const
{
  return _name    == other._name    &&
         _minimum == other._minimum &&
         _maximum == other._maximum &&
         _wrap    == other._wrap    &&
         _just    == other._just    &&
         _sizing  == other._sizing;
}

////////////////////////////////////////////////////////////////////////////////
Column::~Column ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Column::setName (const std::string& name)
{
  _name = name;
}

////////////////////////////////////////////////////////////////////////////////
