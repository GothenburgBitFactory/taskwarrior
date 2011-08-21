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
, _type (type_none)
, _category (cat_none)
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (const std::string& raw)
: _value ("")
, _raw (raw)
, _type (type_none)
, _category (cat_none)
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (
  const std::string& raw,
  const category c)
: _value ("")
, _raw (raw)
, _type (type_none)
, _category (c)
{
}

////////////////////////////////////////////////////////////////////////////////
Arg::Arg (
  const std::string& raw,
  const type t,
  const category c)
: _value ("")
, _raw (raw)
, _type (t)
, _category (c)
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
const Arg::type Arg::type_id (const std::string& input)
{
       if (input == "bool")     return Arg::type_bool;
  else if (input == "string")   return Arg::type_string;
  else if (input == "date")     return Arg::type_date;
  else if (input == "duration") return Arg::type_duration;
  else if (input == "number")   return Arg::type_number;
  else if (input == "pseudo")   return Arg::type_pseudo;

  return Arg::type_none;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Arg::type_name (Arg::type t)
{
  switch (t)
  {
  case Arg::type_none:     return "none";
  case Arg::type_pseudo:   return "pseudo";
  case Arg::type_bool:     return "bool";
  case Arg::type_string:   return "string";
  case Arg::type_date:     return "date";
  case Arg::type_duration: return "duration";
  case Arg::type_number:   return "number";
  }

  return "none";
}

////////////////////////////////////////////////////////////////////////////////
const Arg::category Arg::category_id (const std::string& input)
{
       if (input == "terminator") return Arg::cat_terminator;
  else if (input == "program")    return Arg::cat_program;
  else if (input == "command")    return Arg::cat_command;
  else if (input == "rc")         return Arg::cat_rc;
  else if (input == "rx")         return Arg::cat_rx;
  else if (input == "override")   return Arg::cat_override;
  else if (input == "attr")       return Arg::cat_attr;
  else if (input == "attmod")     return Arg::cat_attmod;
  else if (input == "id")         return Arg::cat_id;
  else if (input == "uuid")       return Arg::cat_uuid;
  else if (input == "subst")      return Arg::cat_subst;
  else if (input == "pattern")    return Arg::cat_pattern;
  else if (input == "tag")        return Arg::cat_tag;
  else if (input == "dom")        return Arg::cat_dom;
  else if (input == "op")         return Arg::cat_op;
  else if (input == "literal")    return Arg::cat_literal;

  return Arg::cat_none;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Arg::category_name (Arg::category c)
{
  switch (c)
  {
  case Arg::cat_none:       return "none";
  case Arg::cat_terminator: return "terminator";
  case Arg::cat_program:    return "program";
  case Arg::cat_command:    return "command";
  case Arg::cat_rc:         return "rc";
  case Arg::cat_rx:         return "rx";
  case Arg::cat_override:   return "override";
  case Arg::cat_attr:       return "attr";
  case Arg::cat_attmod:     return "attmod";
  case Arg::cat_id:         return "id";
  case Arg::cat_uuid:       return "uuid";
  case Arg::cat_subst:      return "subst";
  case Arg::cat_pattern:    return "pattern";
  case Arg::cat_tag:        return "tag";
  case Arg::cat_dom:        return "dom";
  case Arg::cat_op:         return "op";
  case Arg::cat_literal:    return "literal";
  }

  return "none";
}

///////////////////////////////////////////////////////////////////////////////
