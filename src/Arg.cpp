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

#include <Arg.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Arg::Arg ()
: _value ("")
, _raw ("")
, _type ("")
, _category ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (const std::string& raw)
: _value ("")
, _raw (raw)
, _type ("")
, _category ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (
  const std::string& raw,
  const std::string& category)
: _value ("")
, _raw (raw)
, _type ("")
, _category (category)
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (
  const std::string& raw,
  const std::string& type,
  const std::string& category)
: _value ("")
, _raw (raw)
, _type (type)
, _category (category)
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (const Arg& other)
{
  _value    = other._value;
  _raw      = other._raw;
  _type     = other._type;
  _category = other._category;
}

////////////////////////////////////////////////////////////////////////////////
Arg& Arg::operator= (const Arg& other)
{
  if (this != &other)
  {
    _value    = other._value;
    _raw      = other._raw;
    _type     = other._type;
    _category = other._category;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Arg::operator== (const Arg& other) const
{
  return _value    == other._value    &&
         _raw      == other._raw      &&
         _type     == other._type     &&
         _category == other._category;
}

////////////////////////////////////////////////////////////////////////////////

