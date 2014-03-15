////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <stdlib.h>
#include <Context.h>
#include <Date.h>
#include <OldDuration.h>
#include <i18n.h>
#include <text.h>
#include <E9.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& out, const Arg& term)
{
  out << term._value                        << "|"
      << term._raw                          << "|"
      << Arg::type_name (term._type)        << "|"
      << Arg::category_name (term._category);

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Perform all the necessary steps prior to an eval call.
E9::E9 (const A3& args)
{
  _terms      = args;
  _dateformat = context.config.get ("dateformat");
  _dom        = context.config.getBoolean ("dom");
}

////////////////////////////////////////////////////////////////////////////////
E9::~E9 ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool E9::evalFilter (const Task& task)
{
  if (_terms.size () == 0)
    return true;

  std::vector <Arg> value_stack;
  eval (task, value_stack);

  // Coerce result to Boolean.
  Arg result = value_stack.back ();
  value_stack.pop_back ();
  return get_bool (coerce (result, Arg::type_bool));
}

////////////////////////////////////////////////////////////////////////////////
std::string E9::evalExpression (const Task& task)
{
  if (_terms.size () == 0)
    return "";

  std::vector <Arg> value_stack;
  eval (task, value_stack);

  return value_stack.back ()._value;
}

////////////////////////////////////////////////////////////////////////////////
void E9::eval (const Task& task, std::vector <Arg>& value_stack)
{
  // Case sensitivity is configurable.
  bool case_sensitive = context.config.getBoolean ("search.case.sensitive");

  std::vector <Arg>::const_iterator arg;
  for (arg = _terms.begin (); arg != _terms.end (); ++arg)
  {
    if (arg->_category == Arg::cat_op)
    {
      Arg result;

      // Unary operators.
      if (arg->_raw == "!")
      {
        if (value_stack.size () < 1)
          throw format (STRING_E9_NO_OPERANDS, "not");

        Arg right = value_stack.back ();
        value_stack.pop_back ();

        operator_not (result, right);
      }

      // TODO Not sure this is correct.
      // TODO No longer sure why I was unsure in the first place.
      else if (arg->_raw == "-" && value_stack.size () == 1)
      {
        Arg right = value_stack.back ();
        value_stack.pop_back ();

        operator_negate (result, right);
      }

      // Binary operators.
      else
      {
        if (value_stack.size () < 2)
          throw format (STRING_E9_INSUFFICIENT_OP, arg->_raw);

        Arg right = value_stack.back ();
        value_stack.pop_back ();

        Arg left = value_stack.back ();
        value_stack.pop_back ();

             if (arg->_raw == "and")      operator_and      (result, left, right);
        else if (arg->_raw == "or")       operator_or       (result, left, right);
        else if (arg->_raw == "xor")      operator_xor      (result, left, right);
        else if (arg->_raw == "<")        operator_lt       (result, left, right);
        else if (arg->_raw == "<=")       operator_lte      (result, left, right);
        else if (arg->_raw == ">=")       operator_gte      (result, left, right);
        else if (arg->_raw == ">")        operator_gt       (result, left, right);
        else if (arg->_raw == "!=")       operator_inequal  (result, left, right, case_sensitive);
        else if (arg->_raw == "=")        operator_equal    (result, left, right, case_sensitive);
        else if (arg->_raw == "~")        operator_match    (result, left, right, case_sensitive, task);
        else if (arg->_raw == "!~")       operator_nomatch  (result, left, right, case_sensitive, task);
        else if (arg->_raw == "_hastag_") operator_hastag   (result,       right, false, task);
        else if (arg->_raw == "_notag_")  operator_hastag   (result,       right, true,  task);
        else
          throw format (STRING_E9_UNSUPPORTED, arg->_raw);
      }

      // Store the result.
      value_stack.push_back (result);
    }

    // Operand (non-op).
    else
    {
      // Derive _value from _raw, and push on the stack.
      Arg operand (*arg);

      if (operand._category == Arg::cat_dom && _dom)
      {
        operand._category = Arg::cat_literal;
        operand._value    = context.dom.get (operand._raw, task);
      }
      else if (operand._type == Arg::type_date)
      {
        // Could be a date, could be a duration, added to 'now'.
        operand._category = Arg::cat_literal;
        if (operand._raw != "")
        {
          if (Date::valid (operand._raw, _dateformat))
            operand._value = Date (operand._raw, _dateformat).toEpochString ();

          else if (OldDuration::valid (operand._raw))
          {
            OldDuration dur (operand._raw);
            Date now;
            if (dur.negative ())
              now -= (int)(time_t) dur;
            else
              now += (int)(time_t) dur;
            operand._value  = now.toEpochString ();
          }
          else
            operand._value = "";
        }
        else
          operand._value = "";
      }
      else if (operand._type == Arg::type_duration)
      {
        operand._category = Arg::cat_literal;
        operand._value    = (operand._raw != "")
                            ? (std::string)OldDuration (operand._raw)
                            : "";
      }
      else
        operand._value = operand._raw;

      value_stack.push_back (operand);
    }
  }

  // Check for stack remnants.
  if (value_stack.size () != 1)
    throw std::string (STRING_E9_MORE_OP);
}

////////////////////////////////////////////////////////////////////////////////
bool E9::eval_match (Arg& left, Arg& right, bool case_sensitive)
{
  if (right._category == Arg::cat_rx)
  {
    left  = coerce (left,  Arg::type_string);
    right = coerce (right, Arg::type_string);

    // Create a cached entry, if it does not already exist.
    if (_regexes.find (right._value) == _regexes.end ())
      _regexes[right._value] = RX (right._value, case_sensitive);

    if (_regexes[right._value].match (left._value))
      return true;
  }
  else
  {
    // TODO Is there a danger of a raw date '1234567890' matching '234'?

    left  = coerce (left,  Arg::type_string);
    right = coerce (right, Arg::type_string);
    if (find (left._value, right._value, (bool) case_sensitive) != std::string::npos)
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_not (Arg& result, Arg& right)
{
  result = coerce (right, Arg::type_bool);

  if (result._value == "true")
    result._value = "false";
  else
    result._value = "true";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_negate (Arg& result, Arg& right)
{
  result = coerce (right, Arg::type_number);
  result._value = format (- strtod (result._value.c_str (), NULL));
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_and (Arg& result, Arg& left, Arg& right)
{
  result._value = "false";
  result._type = Arg::type_bool;

  if (coerce (left,  Arg::type_bool)._value == "true" &&
      coerce (right, Arg::type_bool)._value == "true" )
  {
    result._value = "true";
  }
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_or (Arg& result, Arg& left, Arg& right)
{
  result._value = "false";
  result._type = Arg::type_bool;

  if (coerce (left,  Arg::type_bool)._value == "true" ||
      coerce (right, Arg::type_bool)._value == "true" )
  {
    result._value = "true";
  }
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_xor (Arg& result, Arg& left, Arg& right)
{
  result._value = "false";
  result._type = Arg::type_bool;

  bool bool_left  = coerce (left,  Arg::type_bool)._value == "true";
  bool bool_right = coerce (right, Arg::type_bool)._value == "true";

  if ((bool_left && !bool_right) ||
      (!bool_left && bool_right))
  {
    result._value = "true";
  }
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lt (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._value != "H" && right._value == "H") result._value = "true";
    else if (left._value == "L" && right._value == "M") result._value = "true";
    else if (left._value == ""  && right._value != "")  result._value = "true";
    else                                                result._value = "false";
  }
  else if (left._type  == Arg::type_date ||
           right._type == Arg::type_date)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      Date left_date  (left._value,  _dateformat);
      Date right_date (right._value, _dateformat);

      result._value = (left_date < right_date)
                    ? "true"
                    : "false";
    }
  }
  else if (left._type  == Arg::type_duration ||
           right._type == Arg::type_duration)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      OldDuration left_duration  (left._value);
      OldDuration right_duration (right._value);

      result._value = (left_duration < right_duration)
                    ? "true"
                    : "false";
    }
  }
  else if (left._type  == Arg::type_number ||
           right._type == Arg::type_number)
  {
    float left_number  = strtof (left._value.c_str (), NULL);
    float right_number = strtof (right._value.c_str (), NULL);

    result._value = (left_number < right_number)
                  ? "true"
                  : "false";
  }
  else
  {
    result._value = (left._value < right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lte (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._value        == right._value       ) result._value = "true";
    else if (                      right._value == "H") result._value = "true";
    else if (left._value == "L" && right._value == "M") result._value = "true";
    else if (left._value == ""                        ) result._value = "true";
    else                                                result._value = "false";
  }
  else if (left._type  == Arg::type_date ||
           right._type == Arg::type_date)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      Date left_date  (left._value,  _dateformat);
      Date right_date (right._value, _dateformat);

      result._value = (left_date <= right_date)
                    ? "true"
                    : "false";
    }
  }
  else if (left._type  == Arg::type_duration ||
           right._type == Arg::type_duration)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      OldDuration left_duration  (left._value);
      OldDuration right_duration (right._value);

      result._value = (left_duration <= right_duration)
                    ? "true"
                    : "false";
    }
  }
  else if (left._type  == Arg::type_number ||
           right._type == Arg::type_number)
  {
    float left_number  = strtof (left._value.c_str (), NULL);
    float right_number = strtof (right._value.c_str (), NULL);

    result._value = (left_number <= right_number)
                  ? "true"
                  : "false";
  }
  else
  {
    result._value = (left._value <= right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gte (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._value        == right._value       ) result._value = "true";
    else if (left._value == "H"                       ) result._value = "true";
    else if (left._value == "M" && right._value == "L") result._value = "true";
    else if (                      right._value == "" ) result._value = "true";
    else                                                result._value = "false";
  }
  else if (left._type  == Arg::type_date ||
           right._type == Arg::type_date)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      Date left_date  (left._value,  _dateformat);
      Date right_date (right._value, _dateformat);

      result._value = (left_date >= right_date)
                    ? "true"
                    : "false";
    }
  }
  else if (left._type  == Arg::type_duration ||
           right._type == Arg::type_duration)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      OldDuration left_duration  (left._value);
      OldDuration right_duration (right._value);

      result._value = (left_duration >= right_duration)
                    ? "true"
                    : "false";
    }
  }
  else if (left._type  == Arg::type_number ||
           right._type == Arg::type_number)
  {
    float left_number  = strtof (left._value.c_str (), NULL);
    float right_number = strtof (right._value.c_str (), NULL);

    result._value = (left_number >= right_number)
                  ? "true"
                  : "false";
  }
  else
  {
    result._value = (left._value >= right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gt (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._value == "H" && right._value != "H") result._value = "true";
    else if (left._value == "M" && right._value == "L") result._value = "true";
    else if (left._value != ""  && right._value == "")  result._value = "true";
    else                                                result._value = "false";
  }
  else if (left._type  == Arg::type_date ||
           right._type == Arg::type_date)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      Date left_date  (left._value,  _dateformat);
      Date right_date (right._value, _dateformat);

      result._value = result._value = (left_date > right_date)
                                    ? "true"
                                    : "false";
    }
  }
  else if (left._type  == Arg::type_duration ||
           right._type == Arg::type_duration)
  {
    if (left._value == "" ||
        right._value == "")
      result._value = "false";
    else
    {
      OldDuration left_duration  (left._value);
      OldDuration right_duration (right._value);

      result._value = result._value = (left_duration > right_duration)
                                    ? "true"
                                    : "false";
    }
  }
  else if (left._type  == Arg::type_number ||
           right._type == Arg::type_number)
  {
    float left_number  = strtof (left._value.c_str (), NULL);
    float right_number = strtof (right._value.c_str (), NULL);

    result._value = (left_number > right_number)
                  ? "true"
                  : "false";
  }
  else
  {
    result._value = (left._value > right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_inequal (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive)
{
  operator_equal (result, left, right, case_sensitive);
  result._value = result._value == "false"
                                ? "true"
                                : "false";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_equal (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive)
{
  // Assume failure.
  result._value = "false";
  result._type = Arg::type_bool;

  // 'project' is matched leftmost.
  if (left._raw == "project")
  {
    // Bug 856.
    //
    // Special case for checking absent projects.  Without the explicit "" check
    // the right._value.length() is used, which is 0, and therefore generates
    // a false match.
    if (right._value == "")
    {
      if (left._value == "")
        result._value = "true";
    }
    else
    {
      unsigned int right_len = right._value.length ();
      if (compare (right._value,
                   (right_len < left._value.length ()
                     ? left._value.substr (0, right_len)
                     : left._value),
                   case_sensitive))
      {
        result._value = "true";
      }
    }
  }

  // Dates.  Note that missing data causes processing to transfer to the generic
  // string comparison below.
  else if ((left._type  == Arg::type_date ||
            right._type == Arg::type_date) &&
           left._value != "" &&
           right._value != "")
  {
    Date left_date  (left._value,  _dateformat);
    Date right_date (right._value, _dateformat);

    result._value = (left_date == right_date)
                ? "true"
                : "false";
  }

  // Case-insensitive comparison for status. Fixes #1110.
  // Also priority, fixing #1154.
  else if (left._raw == "status" ||
           left._raw == "priority")
  {
    result._value = compare (left._value, right._value, false)
                ? "true"
                : "false";
  }

  // Regular equality matching.
  else
  {
    result._value = compare (left._value, right._value, case_sensitive)
                ? "true"
                : "false";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Match may occur in description or any annotation.  Short circuits.
void E9::operator_match (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive,
  const Task& task)
{
  result._type = Arg::type_bool;

  if (eval_match (left, right, case_sensitive))
  {
    result._value = "true";
  }
  else if (left._raw == "description")
  {
    std::map <std::string, std::string> annotations;
    task.getAnnotations (annotations);

    std::map <std::string, std::string>::iterator a;
    for (a = annotations.begin (); a != annotations.end (); ++a)
    {
      // Clone 'left', override _value.
      Arg alternate (left);
      alternate._value = a->second;

      if (eval_match (alternate, right, case_sensitive))
      {
        result._value = "true";
        break;
      }
    }
  }
  else
    result._value = "false";
}

////////////////////////////////////////////////////////////////////////////////
// Match may not occur in description or any annotation.  Short circuits.
void E9::operator_nomatch (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive,
  const Task& task)
{
  result._type = Arg::type_bool;
  result._value = "true";

  if (eval_match (left, right, case_sensitive))
  {
    result._value = "false";
  }
  else if (left._raw == "description")
  {
    std::map <std::string, std::string> annotations;
    task.getAnnotations (annotations);

    std::map <std::string, std::string>::iterator a;
    for (a = annotations.begin (); a != annotations.end (); ++a)
    {
      // Clone 'left', override _value.
      Arg alternate (left);
      alternate._value = a->second;

      if (eval_match (alternate, right, case_sensitive))
      {
        result._value = "false";
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_hastag (
  Arg& result,
  Arg& right,
  bool invert,
  const Task& task)
{
  result._type = Arg::type_bool;

  if (task.hasTag (right._raw))
    result._value = invert ? "false" : "true";
  else
    result._value = invert ? "true" : "false";
}

////////////////////////////////////////////////////////////////////////////////
const Arg E9::coerce (const Arg& input, const Arg::type type)
{
  Arg result;

  if (type == Arg::type_bool)
  {
    result._raw      = input._raw;
    result._value    = get_bool (input) ? "true" : "false";
    result._type     = Arg::type_bool;
    result._category = input._category;
  }

  else if (type == Arg::type_string)
  {
    // TODO Convert date?
    result._raw      = input._raw;
    result._value    = input._value;
    result._type     = Arg::type_string;
    result._category = input._category;
  }

  else if (type == Arg::type_number)
  {
    result._raw      = input._raw;
    result._value    = input._value;
    result._type     = Arg::type_number;
    result._category = input._category;
  }

  // TODO Date
  // TODO OldDuration
  else
  {
    result = input;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool E9::get_bool (const Arg& input)
{
  std::string value = lowerCase (input._value);
  if (value == "true"   ||
      value == "t"      ||
      value == "1"      ||
      value == "+"      ||
      value == "y"      ||
      value == "yes"    ||
      value == "on"     ||
      value == "enable" ||
      value == "enabled")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
