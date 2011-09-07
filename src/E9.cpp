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
#include <Date.h>
#include <Duration.h>
#include <text.h>
#include <E9.h>

extern Context context;

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
          throw std::string ("There are no operands for the 'not' operator.");

        Arg right = value_stack.back ();
        value_stack.pop_back ();

        operator_not (result, right);
      }
/*
      // TODO Unary -.
      else if (arg->raw == "-")
      {
      }
*/

      // Binary operators.
      else
      {
        if (value_stack.size () < 2)
          throw std::string ("There are not enough operands for the '") + arg->_raw + "' operator.";

        Arg right = value_stack.back ();
        value_stack.pop_back ();

        Arg left = value_stack.back ();
        value_stack.pop_back ();

             if (arg->_raw == "and") operator_and      (result, left, right);
        else if (arg->_raw == "or")  operator_or       (result, left, right);
        else if (arg->_raw == "xor") operator_xor      (result, left, right);
        else if (arg->_raw == "<")   operator_lt       (result, left, right);
        else if (arg->_raw == "<=")  operator_lte      (result, left, right);
        else if (arg->_raw == ">=")  operator_gte      (result, left, right);
        else if (arg->_raw == ">")   operator_gt       (result, left, right);
        else if (arg->_raw == "!=")  operator_inequal  (result, left, right, case_sensitive);
        else if (arg->_raw == "=")   operator_equal    (result, left, right, case_sensitive);
        else if (arg->_raw == "~")   operator_match    (result, left, right, case_sensitive, task);
        else if (arg->_raw == "!~")  operator_nomatch  (result, left, right, case_sensitive, task);
        else if (arg->_raw == "*")   operator_multiply (result, left, right);
        else if (arg->_raw == "/")   operator_divide   (result, left, right);
        else if (arg->_raw == "+")   operator_add      (result, left, right);
        else if (arg->_raw == "-")   operator_subtract (result, left, right);
        else
          throw std::string ("Unsupported operator '") + arg->_raw + "'.";
      }

      // Store the result.
      value_stack.push_back (result);
    }

    // Operand.
    else
    {
      // Derive _value from _raw, and push on the stack.
      Arg operand (*arg);

      if (operand._category == Arg::cat_dom && _dom)
      {
        operand._value    = context.dom.get (operand._raw, task);
        operand._category = Arg::cat_literal;
      }
      else if (operand._type     == Arg::type_date &&
               operand._category == Arg::cat_literal)
      {
        operand._value    = Date (operand._raw, _dateformat).toEpochString ();
        operand._category = Arg::cat_literal;
      }
      else if (operand._type     == Arg::type_duration &&
               operand._category == Arg::cat_literal)
      {
        operand._value    = (std::string)Duration (operand._raw);
        operand._category = Arg::cat_literal;
      }
      else
        operand._value = operand._raw;

      // std::cout << "# E9::eval operand " << operand << "\n";
      value_stack.push_back (operand);
    }
  }

  // Check for stack remnants.
  if (value_stack.size () != 1)
    throw std::string ("Error: Expression::eval found extra items on the stack.");
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
  result = right;
  coerce (result, Arg::type_bool);

  if (result._value == "true")
    result._value = "false";
  else
    result._value = "true";

//  std::cout << "# <operator_not> " << right << " --> " << result << "\n";
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

//  std::cout << "# " << left << " <operator_and> " << right << " --> " << result << "\n";
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

//  std::cout << "# " << left << " <operator_or> " << right << " --> " << result << "\n";
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

//  std::cout << "# " << left << " <operator_xor> " << right << " --> " << result << "\n";
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
  else
  {
    result._value = (left._value < right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;

//  std::cout << "# " << left << " <operator_lt> " << right << " --> " << result << "\n";
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
  else
  {
    result._value = (left._value <= right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;

//  std::cout << "# " << left << " <operator_lte> " << right << " --> " << result << "\n";
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
  else
  {
    result._value = (left._value >= right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;

//  std::cout << "# " << left << " <operator_gte> " << right << " --> " << result << "\n";
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
  else
  {
    result._value = (left._value > right._value)
                  ? "true"
                  : "false";
  }

  result._type = Arg::type_bool;

//  std::cout << "# " << left << " <operator_gt> " << right << " --> " << result << "\n";
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

//  std::cout << "# " << left << " <operator_inequal> " << right << " --> " << result << "\n";
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

  // Regular equality matching.
  else
  {
    result._value = compare (left._value, right._value, case_sensitive)
                ? "true"
                : "false";
  }

//  std::cout << "# " << left << " <operator_equal> " << right << " --> " << result << "\n";
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

//  std::cout << "# " << left << " <operator_match> " << right << " --> " << result << "\n";
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

//  std::cout << "# " << left << " <operator_nomatch> " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_multiply (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " <operator_multiply> " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_divide (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " <operator_divide> " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_add (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " <operator_add> " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_subtract (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " <operator_subtract> " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
const Arg E9::coerce (const Arg& input, const Arg::type type)
{
  Arg result;

  if (type == Arg::type_bool)
  {
    result._raw = input._raw;
    result._value = get_bool (input) ? "true" : "false";
    result._type = Arg::type_bool;
    result._category = input._category;
  }

  else if (type == Arg::type_string)
  {
    // TODO Convert date?
    result._raw = input._raw;
    result._value = input._value;
    result._type  = Arg::type_string;
    result._category = input._category;
  }

  // TODO Date
  // TODO Duration
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
