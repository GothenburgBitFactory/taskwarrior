////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <Variant.h>
#include <ISO8601.h>
#include <Lexer.h>
#include <RX.h>
#include <text.h>
#include <i18n.h>

std::string Variant::dateFormat = "";
bool Variant::searchCaseSensitive = true;
bool Variant::searchUsingRegex = true;

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Variant& other)
{
  _type     = other._type;
  _bool     = other._bool;
  _integer  = other._integer;
  _real     = other._real;
  _string   = other._string;
  _date     = other._date;
  _duration = other._duration;
  _source   = other._source;
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const bool value)
: _type (Variant::type_boolean)
, _bool (value)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const int value)
: _type (Variant::type_integer)
, _integer (value)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const double value)
: _type (Variant::type_real)
, _real (value)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const std::string& value)
: _type (Variant::type_string)
, _string (value)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const char* value)
: _type (Variant::type_string)
, _string (value)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const time_t value, const enum type new_type)
: _type (new_type)
{
  switch (new_type)
  {
  case type_date:     _date = value;     break;
  case type_duration: _duration = value; break;
  default:
    throw std::string (STRING_VARIANT_TIME_T);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Variant::source (const std::string& input)
{
  _source = input;
}

////////////////////////////////////////////////////////////////////////////////
const std::string& Variant::source () const
{
  return _source;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator= (const Variant& other)
{
  _type     = other._type;
  _bool     = other._bool;
  _integer  = other._integer;
  _real     = other._real;
  _string   = other._string;
  _date     = other._date;
  _duration = other._duration;
  _source   = other._source;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator&& (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_boolean);
  right.cast (type_boolean);

  return left._bool && right._bool;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator|| (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_boolean);
  right.cast (type_boolean);

  return left._bool || right._bool;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_xor (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_boolean);
  right.cast (type_boolean);

  return (left._bool && !right._bool) ||
         (!left._bool && right._bool);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator< (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:                             return !left._bool && right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  < right._integer;
    case type_real:     left.cast (type_real);     return left._real     < right._real;
    case type_string:   left.cast (type_string);   return left._string   < right._string;
    case type_date:     left.cast (type_date);     return left._date     < right._date;
    case type_duration: left.cast (type_duration); return left._duration < right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); return left._integer  < right._integer;
    case type_integer:                             return left._integer  < right._integer;
    case type_real:     left.cast (type_real);     return left._real     < right._real;
    case type_string:   left.cast (type_string);   return left._string   < right._string;
    case type_date:     left.cast (type_date);     return left._date     < right._date;
    case type_duration: left.cast (type_duration); return left._duration < right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_real);    return left._real     < right._real;
    case type_integer:  right.cast (type_real);    return left._real     < right._real;
    case type_real:                                return left._real     < right._real;
    case type_string:   left.cast (type_string);   return left._string   < right._string;
    case type_date:     left.cast (type_date);     return left._date     < right._date;
    case type_duration: left.cast (type_duration); return left._duration < right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string < right._string;

    case type_string:
      {
        if (left._string == right._string)
          return false;

        auto order = Task::customOrder.find (left.source ());
        if (order != Task::customOrder.end ())
        {
          // Guaranteed to be found, because of ColUDA::validate ().
          auto posLeft  = std::find (order->second.begin (), order->second.end (), left._string);
          auto posRight = std::find (order->second.begin (), order->second.end (), right._string);
          return posLeft < posRight;
        }
        else
        {
          if (left.trivial () || right.trivial ())
            return false;

          return left._string < right._string;
        }
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date < right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration < right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date < right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration < right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator<= (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:                             return !left._bool || right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  <= right._integer;
    case type_real:     left.cast (type_real);     return left._real     <= right._real;
    case type_string:   left.cast (type_string);   return left._string   <= right._string;
    case type_date:     left.cast (type_date);     return left._date     <= right._date;
    case type_duration: left.cast (type_duration); return left._duration <= right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); return left._integer  <= right._integer;
    case type_integer:                             return left._integer  <= right._integer;
    case type_real:     left.cast (type_real);     return left._real     <= right._real;
    case type_string:   left.cast (type_string);   return left._string   <= right._string;
    case type_date:     left.cast (type_date);     return left._date     <= right._date;
    case type_duration: left.cast (type_duration); return left._duration <= right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_real);    return left._real     <= right._real;
    case type_integer:  right.cast (type_real);    return left._real     <= right._real;
    case type_real:                                return left._real     <= right._real;
    case type_string:   left.cast (type_string);   return left._string   <= right._string;
    case type_date:     left.cast (type_date);     return left._date     <= right._date;
    case type_duration: left.cast (type_duration); return left._duration <= right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string <= right._string;

    case type_string:
      {
        if (left._string == right._string)
          return true;

        auto order = Task::customOrder.find (left.source ());
        if (order != Task::customOrder.end ())
        {
          // Guaranteed to be found, because of ColUDA::validate ().
          auto posLeft  = std::find (order->second.begin (), order->second.end (), left._string);
          auto posRight = std::find (order->second.begin (), order->second.end (), right._string);
          return posLeft <= posRight;
        }
        else
        {
          if (left.trivial () || right.trivial ())
            return false;

          return left._string <= right._string;
        }
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date <= right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration <= right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date <= right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration <= right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator> (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:                             return !left._bool && right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  > right._integer;
    case type_real:     left.cast (type_real);     return left._real     > right._real;
    case type_string:   left.cast (type_string);   return left._string   > right._string;
    case type_date:     left.cast (type_date);     return left._date     > right._date;
    case type_duration: left.cast (type_duration); return left._duration > right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); return left._integer  > right._integer;
    case type_integer:                             return left._integer  > right._integer;
    case type_real:     left.cast (type_real);     return left._real     > right._real;
    case type_string:   left.cast (type_string);   return left._string   > right._string;
    case type_date:     left.cast (type_date);     return left._date     > right._date;
    case type_duration: left.cast (type_duration); return left._duration > right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_real);    return left._real     > right._real;
    case type_integer:  right.cast (type_real);    return left._real     > right._real;
    case type_real:                                return left._real     > right._real;
    case type_string:   left.cast (type_string);   return left._string   > right._string;
    case type_date:     left.cast (type_date);     return left._date     > right._date;
    case type_duration: left.cast (type_duration); return left._duration > right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string > right._string;

    case type_string:
      {
        if (left._string == right._string)
          return false;

        auto order = Task::customOrder.find (left.source ());
        if (order != Task::customOrder.end ())
        {
          // Guaranteed to be found, because of ColUDA::validate ().
          auto posLeft  = std::find (order->second.begin (), order->second.end (), left._string);
          auto posRight = std::find (order->second.begin (), order->second.end (), right._string);
          return posLeft > posRight;
        }
        else
        {
          if (left.trivial () || right.trivial ())
            return false;

          return left._string > right._string;
        }
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date > right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration > right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date > right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration > right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator>= (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:                             return left._bool || !right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  >= right._integer;
    case type_real:     left.cast (type_real);     return left._real     >= right._real;
    case type_string:   left.cast (type_string);   return left._string   >= right._string;
    case type_date:     left.cast (type_date);     return left._date     >= right._date;
    case type_duration: left.cast (type_duration); return left._duration >= right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); return left._integer  >= right._integer;
    case type_integer:                             return left._integer  >= right._integer;
    case type_real:     left.cast (type_real);     return left._real     >= right._real;
    case type_string:   left.cast (type_string);   return left._string   >= right._string;
    case type_date:     left.cast (type_date);     return left._date     >= right._date;
    case type_duration: left.cast (type_duration); return left._duration >= right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_real);    return left._real     >= right._real;
    case type_integer:  right.cast (type_real);    return left._real     >= right._real;
    case type_real:                                return left._real     >= right._real;
    case type_string:   left.cast (type_string);   return left._string   >= right._string;
    case type_date:     left.cast (type_date);     return left._date     >= right._date;
    case type_duration: left.cast (type_duration); return left._duration >= right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string >= right._string;

    case type_string:
      {
        if (left._string == right._string)
          return true;

        auto order = Task::customOrder.find (left.source ());
        if (order != Task::customOrder.end ())
        {
          // Guaranteed to be found, because of ColUDA::validate ().
          auto posLeft  = std::find (order->second.begin (), order->second.end (), left._string);
          auto posRight = std::find (order->second.begin (), order->second.end (), right._string);
          return posLeft >= posRight;
        }
        else
        {
          if (left.trivial () || right.trivial ())
            return false;

          return left._string >= right._string;
        }
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date >= right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration >= right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date >= right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration >= right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator== (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:                             return left._bool == right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real == right._real;
    case type_string:   left.cast (type_string);   return left._string == right._string;
    case type_date:     left.cast (type_date);     return left._date == right._date;
    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); return left._integer == right._integer;
    case type_integer:                             return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real == right._real;
    case type_string:   left.cast (type_string);   return left._string == right._string;
    case type_date:     left.cast (type_date);     return left._date == right._date;
    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_real);    return left._real == right._real;
    case type_integer:  right.cast (type_real);    return left._real == right._real;
    case type_real:                                return left._real == right._real;
    case type_string:   left.cast (type_string);   return left._string == right._string;
    case type_date:     left.cast (type_date);     return left._date == right._date;
    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
      right.cast (type_string);

      // Status is always compared caseless.
      if (left.source () == "status")
        return compare (left._string, right._string, false);

      return left._string == right._string;

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date == right._date;

    case type_duration:
      left.cast (type_duration);
      return left._duration == right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date == right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      right.cast (type_duration);
      return left._duration == right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator!= (const Variant& other) const
{
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_match (const Variant& other, const Task& task) const
{
  // Simple matching case first.
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_string);
  right.cast (type_string);

  std::string pattern = right._string;
  Lexer::dequote (pattern);

  if (searchUsingRegex)
  {
    RX r (pattern, searchCaseSensitive);
    if (r.match (left._string))
      return true;

    // If the above did not match, and the left source is "description", then
    // in the annotations.
    if (left.source () == "description")
    {
      std::map <std::string, std::string> annotations;
      task.getAnnotations (annotations);

      for (auto& a : annotations)
        if (r.match (a.second))
          return true;
    }
  }
  else
  {
    // If pattern starts with '^', look for a leftmost compare only.
    if (pattern[0] == '^' &&
        find (left._string,
              pattern.substr (1),
              searchCaseSensitive) == 0)
    {
      return true;
    }

    // If pattern ends with '$', look for a rightmost compare only.
    else if (pattern[pattern.length () - 1] == '$' &&
             find (left._string,
                   pattern.substr (0, pattern.length () - 1),
                   searchCaseSensitive) == (left._string.length () - pattern.length () + 1))
    {
      return true;
    }

    else if (find (left._string, pattern, searchCaseSensitive) != std::string::npos)
      return true;

    // If the above did not match, and the left source is "description", then
    // in the annotations.
    if (left.source () == "description")
    {
      std::map <std::string, std::string> annotations;
      task.getAnnotations (annotations);

      for (auto& a : annotations)
        if (find (a.second, pattern, searchCaseSensitive) != std::string::npos)
          return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_nomatch (const Variant& other, const Task& task) const
{
  return ! operator_match (other, task);
}

////////////////////////////////////////////////////////////////////////////////
// Partial match is mostly a clone of operator==, but with some overrides:
//
//   date <partial> date     --> same day check
//   string <partial> string --> leftmost
//
bool Variant::operator_partial (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:                             return left._bool    == right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real    == right._real;
    case type_string:   left.cast (type_string);   return left._string  == right._string;

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        ISO8601d left_date (left._date);
        ISO8601d right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); return left._integer == right._integer;
    case type_integer:                             return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real    == right._real;
    case type_string:   left.cast (type_string);   return left._string  == right._string;

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        ISO8601d left_date (left._date);
        ISO8601d right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_real);
      return left._real == right._real;

    case type_string:
      left.cast (type_string);
      return left._string == right._string;

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        ISO8601d left_date (left._date);
        ISO8601d right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration:
      left.cast (type_duration);
      return left._duration == right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
      {
        right.cast (type_string);

        // Status is always compared caseless.
        if (left.source () == "status")
          return compare (left._string, right._string, false);

        int left_len  = left._string.length ();
        int right_len = right._string.length ();

        if ((left_len == 0 && right_len != 0) ||
            (left_len != 0 && right_len == 0))
          return false;

        // Dodgy.
        if (left._string.length () < right._string.length ())
          return false;

        return left._string.substr (0, right._string.length ()) == right._string;
      }

    // Same-day comparison.
    case type_date:
      {
        if (left.trivial () || right.trivial ())
          return false;

        left.cast (type_date);
        ISO8601d left_date (left._date);
        ISO8601d right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration:
      left.cast (type_duration);
      return left._duration == right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    // Same-day comparison.
    case type_string:
      if (left.trivial () || right.trivial ())
        return false;

    case type_boolean:
    case type_integer:
    case type_real:
    case type_date:
    case type_duration:
      {
        right.cast (type_date);
        ISO8601d left_date (left._date);
        ISO8601d right_date (right._date);
        return left_date.sameDay (right_date);
      }
    }
    break;

  case type_duration:
    switch (right._type)
    {
    // Same-day comparison.
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      right.cast (type_duration);
      return left._duration == right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Inverse of operator_partial.
bool Variant::operator_nopartial (const Variant& other) const
{
  return ! operator_partial (other);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_hastag (const Variant& other, const Task& task) const
{
  Variant right (other);
  right.cast (type_string);
  Lexer::dequote (right._string);
  return task.hasTag (right._string);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_notag (const Variant& other, const Task& task) const
{
  return ! operator_hastag (other, task);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator! () const
{
  Variant left (*this);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  left.cast (type_boolean);
  return ! left._bool;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator^= (const Variant& other)
{
  switch (_type)
  {
  case type_boolean:
    throw std::string (STRING_VARIANT_EXP_BOOL);
    break;

  case type_integer:
    switch (other._type)
    {
    case type_boolean:  throw std::string (STRING_VARIANT_EXP_BOOL);
    case type_integer:  _integer = (int) pow (static_cast<double>(_integer), static_cast<double>(other._integer)); break;
    case type_real:     throw std::string (STRING_VARIANT_EXP_NON_INT);
    case type_string:   throw std::string (STRING_VARIANT_EXP_STRING);
    case type_date:     throw std::string (STRING_VARIANT_EXP_DATE);
    case type_duration: throw std::string (STRING_VARIANT_EXP_DURATION);
    }
    break;

  case type_real:
    switch (other._type)
    {
    case type_boolean:  throw std::string (STRING_VARIANT_EXP_BOOL);
    case type_integer:  _real = pow (_real, static_cast<double>(other._integer)); break;
    case type_real:     throw std::string (STRING_VARIANT_EXP_NON_INT);
    case type_string:   throw std::string (STRING_VARIANT_EXP_STRING);
    case type_date:     throw std::string (STRING_VARIANT_EXP_DATE);
    case type_duration: throw std::string (STRING_VARIANT_EXP_DURATION);
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_EXP_STRING);
    break;

  case type_date:
    throw std::string (STRING_VARIANT_EXP_DATE);
    break;

  case type_duration:
    throw std::string (STRING_VARIANT_EXP_DURATION);
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator^ (const Variant& other) const
{
  Variant left (*this);
  left ^= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator-= (const Variant& other)
{
  Variant right (other);

  switch (_type)
  {
  case type_boolean:
    throw std::string (STRING_VARIANT_SUB_BOOL);
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); _integer  -= right._integer;   break;
    case type_integer:                             _integer  -= right._integer;   break;
    case type_real:     cast (type_real);          _real     -= right._real;      break;
    case type_string:   throw std::string (STRING_VARIANT_SUB_STRING);
    case type_date:     cast (type_date);          _date     -= right._date;      break;
    case type_duration: cast (type_duration);      _duration -= right._duration;  break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_string:
      throw std::string (STRING_VARIANT_SUB_STRING);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_date:
    case type_duration:
      right.cast (type_real);
      _real -= right._real;
      break;
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_SUB_STRING);
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); _date -= right._integer;    break;
    case type_integer:                             _date -= right._integer;    break;
    case type_real:                                _date -= (int) right._real; break;
    case type_string:   throw std::string (STRING_VARIANT_SUB_STRING);
    case type_date:     cast (type_duration);      _duration -= right._date;   break;
    case type_duration:                            _date -= right._duration;   break;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); _duration -= right._integer;    break;
    case type_integer:                             _duration -= right._integer;    break;
    case type_real:                                _duration -= (int) right._real; break;
    case type_string:   throw std::string (STRING_VARIANT_SUB_STRING);
    case type_date:     throw std::string (STRING_VARIANT_SUB_DATE);
    case type_duration:                            _duration -= right._duration;   break;
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator- (const Variant& other) const
{
  Variant left (*this);
  left -= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator+= (const Variant& other)
{
  Variant right (other);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (_type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:  throw std::string (STRING_VARIANT_ADD_BOOL);
    case type_integer:  cast (type_integer);  _integer  += right._integer;  break;
    case type_real:     cast (type_real);     _real     += right._real;     break;
    case type_string:   cast (type_string);   _string   += right._string;   break;
    case type_date:     cast (type_date);     _date     += right._date;     break;
    case type_duration: cast (type_duration); _duration += right._duration; break;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); _integer  += right._integer;   break;
    case type_integer:                             _integer  += right._integer;   break;
    case type_real:     cast (type_real);          _real     += right._real;      break;
    case type_string:   cast (type_string);        _string   += right._string;    break;
    case type_date:     cast (type_date);          _date     += right._date;      break;
    case type_duration: cast (type_duration);      _duration += right._duration;  break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_real);
      _real += right._real;
      break;

    case type_string:
      cast (type_string);
      _string += right._string;
      break;

    case type_date:
      _type = type_date;
      _date = (unsigned) (int) _real + right._date;
      break;

    case type_duration:
      _type = type_duration;
      _duration = (unsigned) (int) _real + right._duration;
      break;
    }
    break;

  case type_string:
    _string += (std::string) right;
    break;

  case type_date:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_date); _date += right._date;       break;
    case type_integer:                          _date += right._integer;    break;
    case type_real:                             _date += (int) right._real; break;
    case type_string:   cast (type_string);     _string += right._string;   break;
    case type_date:     throw std::string (STRING_VARIANT_ADD_DATE);
    case type_duration:                         _date += right._duration;   break;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_duration); _duration += right._duration;   break;
    case type_integer:                              _duration += right._integer;    break;
    case type_real:                                 _duration += (int) right._real; break;
    case type_string:   cast (type_string);         _string += right._string;       break;
    case type_date:     cast (type_date);           _date += right._date;           break;
    case type_duration:                             _duration += right._duration;   break;
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator+ (const Variant& other) const
{
  Variant left (*this);
  left += other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator*= (const Variant& other)
{
  Variant right (other);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (_type)
  {
  case type_boolean:
    switch (right._type)
    {
    case type_boolean:  throw std::string (STRING_VARIANT_MUL_BOOL);
    case type_integer:  cast (type_integer);       _integer  *= right._integer;      break;
    case type_real:     cast (type_real);          _real     *= right._real;         break;
    case type_string:   _string = (_bool ? right._string : ""); _type = type_string; break;
    case type_date:     throw std::string (STRING_VARIANT_MUL_DATE);
    case type_duration: cast (type_duration);      _duration *= right._duration;     break;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_integer); _integer  *= right._integer;   break;
    case type_integer:                             _integer  *= right._integer;   break;
    case type_real:     cast (type_real);          _real     *= right._real;      break;
    case type_string:
      {
        int limit = _integer;
        // assert (limit < 128);
        _type = type_string;
        _string = "";
        while (limit--)
          _string += right._string;
      }
      break;
    case type_date:     throw std::string (STRING_VARIANT_MUL_DATE);
    case type_duration: cast (type_duration);      _duration *= right._duration;  break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_real);
      _real *= right._real;
      break;

    case type_string:
      throw std::string (STRING_VARIANT_MUL_REAL_STR);

    case type_date:
      throw std::string (STRING_VARIANT_MUL_DATE);

    case type_duration:
      _type = type_duration;
      _duration = (time_t) (unsigned) (int) (_real * static_cast<double>(right._duration));
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_boolean:  if (! right._bool) _string = ""; break;
    case type_integer:
      {
        int limit = right._integer - 1;
        // assert (limit < 128);
        std::string fragment = _string;
        while (limit--)
          _string += fragment;
      }
      break;
    case type_real:     throw std::string (STRING_VARIANT_MUL_STR_REAL);
    case type_string:   throw std::string (STRING_VARIANT_MUL_STR_STR);
    case type_date:     throw std::string (STRING_VARIANT_MUL_STR_DATE);
    case type_duration: throw std::string (STRING_VARIANT_MUL_STR_DUR);
    }
    break;

  case type_date:
    throw std::string (STRING_VARIANT_MUL_DATE);

  case type_duration:
    switch (right._type)
    {
    case type_boolean:  right.cast (type_duration); _duration *= right._duration;   break;
    case type_integer:                              _duration *= right._integer;    break;
    case type_real:
      _duration = (time_t) (unsigned) (int) (static_cast<double>(_duration) * right._real);
      break;
    case type_string:   throw std::string (STRING_VARIANT_MUL_DUR_STR);
    case type_date:     throw std::string (STRING_VARIANT_MUL_DUR_DATE);
    case type_duration: throw std::string (STRING_VARIANT_MUL_DUR_DUR);
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator* (const Variant& other) const
{
  Variant left (*this);
  left *= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator/= (const Variant& other)
{
  Variant right (other);

  switch (_type)
  {
  case type_boolean:
    throw std::string (STRING_VARIANT_DIV_BOOL);
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:
      throw std::string (STRING_VARIANT_DIV_INT_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _integer /= right._integer;
      break;

    case type_real:
      if (right._real == 0.0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      cast (type_real);
      _real /= right._real;
      break;

    case type_string:
      throw std::string (STRING_VARIANT_DIV_INT_STR);

    case type_date:
      throw std::string (STRING_VARIANT_DIV_INT_DATE);

    case type_duration:
      if (right._duration == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _type = type_duration;
      _duration = (time_t) (unsigned) (int) (_integer / right._duration);
      break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:
      throw std::string (STRING_VARIANT_DIV_REAL_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _real /= static_cast<double>(right._integer);
      break;

    case type_real:
      if (right._real == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _real /= right._real;
      break;

    case type_string:
      throw std::string (STRING_VARIANT_DIV_REAL_STR);

    case type_date:
      throw std::string (STRING_VARIANT_DIV_REAL_DATE);

    case type_duration:
      if (right._duration == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _type = type_duration;
      _duration = (time_t) (unsigned) (int) (_real / right._duration);
      break;
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_DIV_REAL_STR);
    break;

  case type_date:
    throw std::string (STRING_VARIANT_DIV_REAL_DATE);

  case type_duration:
    switch (right._type)
    {
    case type_boolean:
      throw std::string (STRING_VARIANT_DIV_DUR_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _duration /= right._integer;
      break;

    case type_real:
      if (right._real == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _duration = (time_t) (unsigned) (int) (static_cast<double>(_duration) / right._real);
      break;

    case type_string:
      throw std::string (STRING_VARIANT_DIV_DUR_STR);

    case type_date:
      throw std::string (STRING_VARIANT_DIV_DUR_DATE);

    case type_duration:
      throw std::string (STRING_VARIANT_DIV_DUR_DUR);
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator/ (const Variant& other) const
{
  Variant left (*this);
  left /= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator%= (const Variant& other)
{
  Variant right (other);

  switch (_type)
  {
  case type_boolean:
    throw std::string (STRING_VARIANT_MOD_BOOL);
    break;

  case type_integer:
    switch (right._type)
    {
    case type_boolean:
      throw std::string (STRING_VARIANT_MOD_INT_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      _integer %= right._integer;
      break;

    case type_real:
      if (right._real == 0.0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      cast (type_real);
      _real = fmod (_real, right._real);
      break;

    case type_string:
      throw std::string (STRING_VARIANT_MOD_INT_STR);

    case type_date:
      throw std::string (STRING_VARIANT_MOD_INT_DATE);

    case type_duration:
      throw std::string (STRING_VARIANT_MOD_INT_DUR);
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_boolean:
      throw std::string (STRING_VARIANT_MOD_REAL_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      _real = fmod (_real, static_cast<double>(right._integer));
      break;

    case type_real:
      if (right._real == 0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      _real = fmod (_real, right._real);
      break;

    case type_string:
      throw std::string (STRING_VARIANT_MOD_REAL_STR);

    case type_date:
      throw std::string (STRING_VARIANT_MOD_REAL_DATE);

    case type_duration:
      throw std::string (STRING_VARIANT_MOD_REAL_DUR);
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_MOD_STR);

  case type_date:
    throw std::string (STRING_VARIANT_MOD_DATE);

  case type_duration:
    throw std::string (STRING_VARIANT_MOD_DUR);
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator% (const Variant& other) const
{
  Variant left (*this);
  left %= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant::operator std::string () const
{
  switch (_type)
  {
  case type_boolean:
    return std::string (_bool ? "true" : "false");

  case type_integer:
    {
      std::stringstream s;
      s << _integer;
      return s.str ();
    }

  case type_real:
    {
      std::stringstream s;
      s << _real;
      return s.str ();
    }

  case type_string:
    return _string;

  case type_date:
    return ISO8601d (_date).toISOLocalExtended ();

  case type_duration:
    return ISO8601p (_duration).format ();
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Variant::sqrt ()
{
  if (_type == type_string)
    Lexer::dequote (_string);

  cast (type_real);
  if (_real < 0.0)
    throw std::string (STRING_VARIANT_SQRT_NEG);
  _real = ::sqrt (_real);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::cast (const enum type new_type)
{
  // Short circuit.
  if (_type == new_type)
    return;

  // From type_boolean
  switch (_type)
  {
  case type_boolean:
    switch (new_type)
    {
    case type_boolean:                                        break;
    case type_integer:  _integer  = _bool ? 1 : 0;            break;
    case type_real:     _real     = _bool ? 1.0 : 0.0;        break;
    case type_string:   _string   = _bool ? "true" : "false"; break;
    case type_date:     _date     = _bool ? 1 : 0;            break;
    case type_duration: _duration = _bool ? 1 : 0;            break;
    }
    break;

  case type_integer:
    switch (new_type)
    {
    case type_boolean:  _bool = _integer == 0 ? false : true;  break;
    case type_integer:                                         break;
    case type_real:     _real = static_cast<double>(_integer); break;
    case type_string:
      {
        char temp[24];
        sprintf (temp, "%d", _integer);
        _string = temp;
      }
      break;
    case type_date:     _date = (time_t) _integer;            break;
    case type_duration: _duration = (time_t) _integer;        break;
    }
    break;

  case type_real:
    switch (new_type)
    {
    case type_boolean:  _bool = _real == 0.0 ? false : true; break;
    case type_integer:  _integer = (int) _real;              break;
    case type_real:                                          break;
    case type_string:
      {
        char temp[24];
        sprintf (temp, "%g", _real);
        _string = temp;
      }
      break;
    case type_date:     _date = (time_t) (int) _real;        break;
    case type_duration: _duration = (time_t) (int) _real;    break;
    }
    break;

  case type_string:
    Lexer::dequote (_string);
    switch (new_type)
    {
    case type_boolean:
      _bool = (_string.length () == 0 ||
               _string == "0"         ||
               _string == "0.0") ? false : true;
      break;
    case type_integer:
      _integer = (int) strtol (_string.c_str (), NULL, (_string.substr (0, 2) == "0x" ? 16 : 10));
      break;
    case type_real:     _real = strtod (_string.c_str (), NULL);              break;
    case type_string:                                                         break;
    case type_date:
      {
        _date = 0;
        ISO8601d iso;
        std::string::size_type pos = 0;
        if (iso.parse (_string, pos, dateFormat) &&
            pos == _string.length ())
        {
          _date = iso.toEpoch ();
          break;
        }

        pos = 0;
        ISO8601p isop;
        if (isop.parse (_string, pos) &&
            pos == _string.length ())
        {
          _date = ISO8601d ().toEpoch () + (time_t) isop;
          break;
        }

        if (dateFormat != "")
        {
          _date = ISO8601d (_string, dateFormat).toEpoch ();
          break;
        }
      }
      break;
    case type_duration:
      {
        _duration = 0;
        std::string::size_type pos = 0;
        ISO8601p iso;
        if (iso.parse (_string, pos) &&
            pos == _string.length ())
        {
          _duration = (time_t) iso;
        }
      }
      break;
    }
    break;

  case type_date:
    switch (new_type)
    {
    case type_boolean:  _bool = _date != 0 ? true : false;  break;
    case type_integer:  _integer = (int) _date;             break;
    case type_real:     _real = static_cast<double>(_date); break;
    case type_string:   _string = (std::string) *this;      break;
    case type_date:                                         break;
    case type_duration: _duration = _date;                  break;
    }
    break;

  case type_duration:
    switch (new_type)
    {
    case type_boolean:  _bool = _duration != 0 ? true : false;  break;
    case type_integer:  _integer = (int) _duration;             break;
    case type_real:     _real = static_cast<double>(_duration); break;
    case type_string:   _string = (std::string) *this;          break;
    case type_date:     _date = _duration;                      break;
    case type_duration:                                         break;
    }
    break;
  }

  _type = new_type;
}

////////////////////////////////////////////////////////////////////////////////
int Variant::type ()
{
  return _type;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::trivial () const
{
  return (_type == type_integer  && _integer  == 0)   ||
         (_type == type_real     && _real     == 0.0) ||
         (_type == type_string   && _string   == "")  ||
         (_type == type_date     && _date     == 0)   ||
         (_type == type_duration && _duration == 0);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::get_bool () const
{
  return _bool;
}

////////////////////////////////////////////////////////////////////////////////
int Variant::get_integer () const
{
  return _integer;
}

////////////////////////////////////////////////////////////////////////////////
double Variant::get_real () const
{
  return _real;
}

////////////////////////////////////////////////////////////////////////////////
const std::string& Variant::get_string () const
{
  return _string;
}

////////////////////////////////////////////////////////////////////////////////
time_t Variant::get_date () const
{
  return _date;
}

////////////////////////////////////////////////////////////////////////////////
time_t Variant::get_duration () const
{
  return _duration;
}

////////////////////////////////////////////////////////////////////////////////
