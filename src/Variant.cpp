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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <text.h>
#include <i18n.h>
#include <Context.h>
#include <Variant.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Variant::Variant ()
: _type (v_unknown)
, _raw ("")
, _raw_type ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Variant& other)
{
  _type    = other._type;
  _raw     = other._raw;
  _raw_type = other._raw_type;

  // Explicitly copy only the relevant type.  This saves memory.
  switch (_type)
  {
  case v_boolean:  _bool     = other._bool;     break;
  case v_integer:  _integer  = other._integer;  break;
  case v_double:   _double   = other._double;   break;
  case v_string:   _string   = other._string;   break;
  case v_date:     _date     = other._date;     break;
  case v_duration: _duration = other._duration; break;
  case v_unknown:                               break;
  }
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const bool input)
{
  _type = v_boolean;
  _bool = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const int input)
{
  _type = v_integer;
  _integer = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const double& input)
{
  _type = v_double;
  _double = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const std::string& input)
{
  _type = v_string;
  _string = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Date& input)
{
  _type = v_date;
  _date = input;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Duration& input)
{
  _type = v_duration;
  _duration = input;
}

////////////////////////////////////////////////////////////////////////////////
// For copying.
Variant& Variant::operator= (const Variant& other)
{
  if (this != &other)
  {
    _type     = other._type;
    _bool     = other._bool;
    _integer  = other._integer;
    _double   = other._double;
    _string   = other._string;
    _date     = other._date;
    _duration = other._duration;
    _raw      = other._raw;
    _raw_type = other._raw_type;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator&& (Variant& other)
{
  cast (v_boolean);
  other.cast (v_boolean);
  return _bool && other._bool;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator|| (Variant& other)
{
  cast (v_boolean);
  other.cast (v_boolean);
  return _bool || other._bool;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator<= (Variant& other)
{
  promote (*this, other);
  bool result;

  switch (_type)
  {
  case v_boolean:
    throw std::string (STRING_VARIANT_REL_BOOL);
    break;

  case v_integer:
    result = _integer <= other._integer;
    break;

  case v_double:
    result = _double <= other._double;
    break;

  case v_string:
    {
      int collating = strcmp (_string.c_str (), other._string.c_str ());
      result = collating <= 0;
    }
    break;

  case v_date:
    result = _date <= other._date;
    break;

  case v_duration:
    result = _duration <= other._duration;
    break;

  case v_unknown:
    throw std::string (STRING_VARIANT_REL_UNKNOWN);
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator>= (Variant& other)
{
  promote (*this, other);
  bool result;

  switch (_type)
  {
  case v_boolean:
    throw std::string (STRING_VARIANT_REL_BOOL);
    break;

  case v_integer:
    result = _integer >= other._integer;
    break;

  case v_double:
    result = _double >= other._double;
    break;

  case v_string:
    {
      int collating = strcmp (_string.c_str (), other._string.c_str ());
      result = collating >= 0;
    }
    break;

  case v_date:
    result = _date >= other._date;
    break;

  case v_duration:
    result = _duration >= other._duration;
    break;

  case v_unknown:
    throw std::string (STRING_VARIANT_REL_UNKNOWN);
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator== (Variant& other)
{
  promote (*this, other);
  bool result;

  switch (_type)
  {
  case v_boolean:
    result = _bool == other._bool;
    break;

  case v_integer:
    result = _integer == other._integer;
    break;

  case v_double:
    result = _double == other._double;
    break;

  case v_string:
    {
      int collating = strcmp (_string.c_str (), other._string.c_str ());
      result = collating == 0;
    }
    break;

  case v_date:
    result = _date == other._date;
    break;

  case v_duration:
    result = _duration == other._duration;
    break;

  case v_unknown:
    throw std::string (STRING_VARIANT_REL_UNKNOWN);
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator!= (Variant& other)
{
  promote (*this, other);
  bool result;

  switch (_type)
  {
  case v_boolean:
    result = _bool != other._bool;
    break;

  case v_integer:
    result = _integer != other._integer;
    break;

  case v_double:
    result = _double != other._double;
    break;

  case v_string:
    {
      int collating = strcmp (_string.c_str (), other._string.c_str ());
      result = collating != 0;
    }
    break;

  case v_date:
    result = _date != other._date;
    break;

  case v_duration:
    result = _duration != other._duration;
    break;

  case v_unknown:
    throw std::string (STRING_VARIANT_REL_UNKNOWN);
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator! ()
{
  cast (v_boolean);
  _bool = ! _bool;
  return _bool;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator- (Variant& other)
{
  promote (*this, other);

  switch (_type)
  {
  case v_boolean:
    throw std::string ("Cannot perform subtraction on Boolean types");
    break;

  case v_integer:
    _integer -= other._integer;
    break;

  case v_double:
    _double -= other._double;
    break;

  case v_string:
    throw std::string ("Cannot perform subtraction on string types");
    break;

  case v_date:
    _duration = Duration (_date - other._date);
    _type = v_duration;
    break;

  case v_duration:
    _duration = _duration - other._duration;
    break;

  case v_unknown:
    throw std::string ("Cannot perform subtraction on unknown types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator+ (Variant& other)
{
  promote (*this, other);

  switch (_type)
  {
  case v_boolean:
    throw std::string ("Cannot perform addition on Boolean types");
    break;

  case v_integer:
    _integer += other._integer;
    break;

  case v_double:
    _double += other._double;
    break;

  case v_string:
    _string += other._string;
    break;

  case v_date:
    // TODO operator+ only works for int
    //_date += other._date;
    break;

  case v_duration:
    // TODO operator+ missing
    //_duration += other._duration;
    break;

  case v_unknown:
    throw std::string ("Cannot perform addition on unknown types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator* (Variant& other)
{
  promote (*this, other);

  switch (_type)
  {
  case v_boolean:
    throw std::string ("Cannot perform multiplication on Boolean types");
    break;

  case v_integer:
    _integer *= other._integer;
    break;

  case v_double:
    _double *= other._double;
    break;

  case v_string:
    throw std::string ("Cannot perform multiplication on string types");
    break;

  case v_date:
    throw std::string ("Cannot perform multiplication on date types");
    break;

  case v_duration:
    throw std::string ("Cannot perform multiplication on duration types");
    break;

  case v_unknown:
    throw std::string ("Cannot perform multiplication on unknown types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator/ (Variant& other)
{
  promote (*this, other);

  switch (_type)
  {
  case v_boolean:
    throw std::string ("Cannot perform division on Boolean types");
    break;

  case v_integer:
    _integer /= other._integer;
    break;

  case v_double:
    _double /= other._double;
    break;

  case v_string:
    throw std::string ("Cannot perform division on string types");
    break;

  case v_date:
    throw std::string ("Cannot perform division on date types");
    break;

  case v_duration:
    throw std::string ("Cannot perform division on duration types");
    break;

  case v_unknown:
    throw std::string ("Cannot perform division on unknown types");
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator< (Variant& other)
{
  promote (*this, other);
  bool result;

  switch (_type)
  {
  case v_boolean:
    throw std::string (STRING_VARIANT_REL_BOOL);
    break;

  case v_integer:
    result = _integer < other._integer;
    break;

  case v_double:
    result = _double < other._double;
    break;

  case v_string:
    {
      int collating = strcmp (_string.c_str (), other._string.c_str ());
      result = collating < 0;
    }
    break;

  case v_date:
    result = _date < other._date;
    break;

  case v_duration:
    result = _duration < other._duration;
    break;

  case v_unknown:
    throw std::string (STRING_VARIANT_REL_UNKNOWN);
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator> (Variant& other)
{
  promote (*this, other);
  bool result;

  switch (_type)
  {
  case v_boolean:
    throw std::string (STRING_VARIANT_REL_BOOL);
    break;

  case v_integer:
    result = _integer > other._integer;
    break;

  case v_double:
    result = _double > other._double;
    break;

  case v_string:
    {
      int collating = strcmp (_string.c_str (), other._string.c_str ());
      result = collating > 0;
    }
    break;

  case v_date:
    result = _date > other._date;
    break;

  case v_duration:
    result = _duration > other._duration;
    break;

  case v_unknown:
    throw std::string (STRING_VARIANT_REL_UNKNOWN);
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
void Variant::input (const std::string& input)
{
  if (! compare (input, "true", false) ||
      ! compare (input, "t",    false) ||
      ! compare (input, "yes",  false) ||
      ! compare (input, "on",   false))
  {
    _type = v_boolean;
    _bool = true;
    return;
  }

  if (! compare (input, "false", false) ||
      ! compare (input, "f",     false) ||
      ! compare (input, "no",    false) ||
      ! compare (input, "off",   false))
  {
    _type = v_boolean;
    _bool = false;
    return;
  }

  // Attempt Date (input) parse.
  if (Date::valid (input, context.config.get ("dateformat")))
  {
    _type = v_date;
    _date = Date (input);
    return;
  }

  // Attempt Duration (input) parse.
  if (Duration::valid (input))
  {
    _type = v_duration;
    _duration = Duration (input);
    return;
  }

  bool numeric = true;
  bool period = false;
  for (unsigned int i = 0; i < input.length (); ++i)
  {
    if (input[i] == '.')                        period = true;
    if (!isdigit (input[i]) && input[i] != '.') numeric = false;
  }

  if (numeric)
  {
    if (period)
    {
      _type = v_double;
      _double = atof (input.c_str ());
    }
    else
    {
      _type = v_integer;
      _integer = atoi (input.c_str ());
    }

    return;
  }

  _type = v_string;
  _string = input;
}

////////////////////////////////////////////////////////////////////////////////
std::string Variant::format ()
{
  std::string output;

  switch (_type)
  {
  case v_boolean:
    output = _bool ? "true" : "false";
    break;

  case v_integer:
    {
      char temp [24];
      sprintf (temp, "%d", _integer);
      output = temp;
    }
    break;

  case v_double:
    {
      char temp [24];
      sprintf (temp, "%g", _double);
      output = temp;
    }
    break;

  case v_string:
    output = _string;
    break;

  case v_date:
    output = _date.toString (context.config.get ("dateformat"));
    break;

  case v_duration:
    output = _duration.formatCompact ();
    break;

  case v_unknown:
  default:
    throw std::string ("Cannot render an unknown type.");
    break;
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
void Variant::cast (const variant_type type)
{
  if (_type == v_unknown || type == v_unknown)
    throw std::string ("Cannot coerce data either to or from an unknown type");

  // Short circuit.
  if (_type == type)
    return;

  // From v_boolean
  if (_type == v_boolean && type == v_integer)
    _integer = _bool ? 1 : 0;

  else if (_type == v_boolean && type == v_double)
    _double = _bool ? 1.0 : 0.0;

  else if (_type == v_boolean && type == v_string)
    _string = _bool ? "true" : "false";

  // From v_integer
  else if (_type == v_integer && type == v_boolean)
    _bool = _integer == 0 ? false : true;

  else if (_type == v_integer && type == v_double)
    _double = (double)_integer;

  else if (_type == v_integer && type == v_string)
  {
    char temp [24];
    sprintf (temp, "%d", _integer);
    _string = temp;
  }

  // From v_double
  else if (_type == v_double && type == v_boolean)
    _bool = _double == 0.0 ? false : true;

  else if (_type == v_double && type == v_integer)
    _integer = (int)_double;

  else if (_type == v_double && type == v_string)
  {
    char temp [24];
    sprintf (temp, "%g", _double);
    _string = temp;
  }

  // From v_string
  else if (_type == v_string && type == v_boolean)
    _bool = (_string.length () == 0 ||
             _string == "0"         ||
             _string == "0.0") ? false : true;

  else if (_type == v_string && type == v_integer)
    _integer = atol (_string.c_str ());

  else if (_type == v_string && type == v_double)
    _double = atol (_string.c_str ());

  // TODO From v_date


  // TODO From v_duration


  _type = type;
}

////////////////////////////////////////////////////////////////////////////////
void Variant::promote (Variant& lhs, Variant& rhs)
{
  // Short circuit.
  if (lhs._type == rhs._type)
    return;

  variant_type newType;
  switch (lhs._type | rhs._type)
  {
  case v_boolean | v_integer:    newType = v_integer;   break;
  case v_boolean | v_double:     newType = v_double;    break;
  case v_boolean | v_string:     newType = v_string;    break;
  case v_boolean | v_date:       newType = v_date;      break;
  case v_boolean | v_duration:   newType = v_duration;  break;
  case v_integer | v_double:     newType = v_integer;   break;
  case v_integer | v_string:     newType = v_string;    break;
  case v_integer | v_date:       newType = v_date;      break;
  case v_integer | v_duration:   newType = v_duration;  break;
  case v_double  | v_string:     newType = v_string;    break;
  case v_double  | v_date:       newType = v_date;      break;
  case v_double  | v_duration:   newType = v_duration;  break;
  case v_string  | v_date:       newType = v_date;      break;
  case v_string  | v_duration:   newType = v_duration;  break;
  case v_date    | v_duration:   newType = v_date;      break;
  }

  lhs.cast (newType);
  rhs.cast (newType);
}

////////////////////////////////////////////////////////////////////////////////
// Casts to boolean and returns the value.  Used to evaluating expression
// results.
bool Variant::boolean ()
{
  cast (v_boolean);
  return _bool;
}

////////////////////////////////////////////////////////////////////////////////
std::string Variant::dump ()
{
  return format () + "/" + _raw + "/" + _raw_type;
}

////////////////////////////////////////////////////////////////////////////////
