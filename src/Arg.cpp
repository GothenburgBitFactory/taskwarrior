////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
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
  case Arg::type_none:     return "";
  case Arg::type_pseudo:   return "pseudo";
  case Arg::type_bool:     return "bool";
  case Arg::type_string:   return "string";
  case Arg::type_date:     return "date";
  case Arg::type_duration: return "duration";
  case Arg::type_number:   return "number";
  }

  return "";
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
  case Arg::cat_none:       return "";
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
  case Arg::cat_dom_:       return "[dom]";
  case Arg::cat_op:         return "op";
  case Arg::cat_literal:    return "literal";
  }

  return "";
}

///////////////////////////////////////////////////////////////////////////////
