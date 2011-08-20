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
  out << term._raw   << "|"
      << term._type  << "|"
      << term._category;

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Perform all the necessary steps prior to an eval call.
E9::E9 (const A3& args)
{
  std::vector <Arg>::const_iterator arg;
  for (arg = args.begin (); arg != args.end (); ++arg)
    _terms.push_back (Arg (*arg));

  _dateformat = context.config.get ("dateformat");
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
  return get_bool (coerce (result, "bool"));
}

////////////////////////////////////////////////////////////////////////////////
std::string E9::evalExpression (const Task& task)
{
  if (_terms.size () == 0)
    return "";

  std::vector <Arg> value_stack;
  eval (task, value_stack);

  return value_stack.back ()._raw;
}

////////////////////////////////////////////////////////////////////////////////
void E9::eval (const Task& task, std::vector <Arg>& value_stack)
{
  // Case sensitivity is configurable.
  bool case_sensitive = context.config.getBoolean ("search.case.sensitive");

  std::vector <Arg>::iterator arg;
  for (arg = _terms.begin (); arg != _terms.end (); ++arg)
  {
    if (arg->_category == "op")
    {
      Arg result;

      // Unary operators.
      if (arg->_raw == "!")
      {
        if (value_stack.size () < 1)
          throw std::string ("There are no operands for the 'not' operator.");

        Arg right = value_stack.back ();
        value_stack.pop_back ();

        if (right._category == "dom")
        {
          if (context.config.getBoolean ("dom"))
            right._raw = context.dom.get (right._raw, task);

          right._category = "string";
        }

        operator_not (result, right);
      }

      // Binary operators.
      else
      {
        if (value_stack.size () < 2)
          throw std::string ("There are not enough operands for the '") + arg->_raw + "' operator.";

        Arg right = value_stack.back ();
        value_stack.pop_back ();

        if (right._category == "dom")
        {
          if (context.config.getBoolean ("dom"))
            right._raw = context.dom.get (right._raw, task);

          right._category = "string";
        }

        Arg left = value_stack.back ();
        value_stack.pop_back ();

        if (left._category == "dom")
        {
          if (context.config.getBoolean ("dom"))
            left._raw = context.dom.get (left._raw, task);

          left._category = "string";
        }

             if (arg->_raw == "and") operator_and      (result, left, right);
        else if (arg->_raw == "or")  operator_or       (result, left, right);
        else if (arg->_raw == "xor") operator_xor      (result, left, right);
        else if (arg->_raw == "<")   operator_lt       (result, left, right);
        else if (arg->_raw == "<=")  operator_lte      (result, left, right);
        else if (arg->_raw == ">=")  operator_gte      (result, left, right);
        else if (arg->_raw == ">")   operator_gt       (result, left, right);
        else if (arg->_raw == "!=")  operator_inequal  (result, left, right, case_sensitive);
        else if (arg->_raw == "=")   operator_equal    (result, left, right, case_sensitive);
        else if (arg->_raw == "~")   operator_match    (result, left, right, case_sensitive);
        else if (arg->_raw == "!~")  operator_nomatch  (result, left, right, case_sensitive);
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
      // Expand the value if possible.
      if (arg->_category == "dom")
        arg->_raw = context.dom.get (arg->_raw, task);
      else if (arg->_category == "date")
        arg->_raw = Date (arg->_raw, _dateformat).toEpochString ();
      else if (arg->_category == "duration")
        arg->_raw = (std::string)Duration (arg->_raw);

      //std::cout << "# E9::eval operand " << *arg << "\n";
      value_stack.push_back (*arg);
    }
  }

  // Check for stack remnants.
  if (value_stack.size () != 1)
    throw std::string ("Error: Expression::eval found extra items on the stack.");
}

////////////////////////////////////////////////////////////////////////////////
bool E9::eval_match (Arg& left, Arg& right, bool case_sensitive)
{
  if (right._category == "rx")
  {
    left  = coerce (left,  "string");
    right = coerce (right, "string");

    // Create a cached entry, if it does not already exist.
    if (_regexes.find (right._raw) == _regexes.end ())
      _regexes[right._raw] = RX (right._raw, case_sensitive);

    if (_regexes[right._raw].match (left._raw))
      return true;
  }
  else
  {
    left  = coerce (left,  "string");
    right = coerce (right, "string");
    if (find (left._raw, right._raw, (bool) case_sensitive) != std::string::npos)
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_not (Arg& result, Arg& right)
{
  // TODO This is not right.
  result = Arg (right._raw, "bool", right._category);

//  std::cout << "# not " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_and (Arg& result, Arg& left, Arg& right)
{
  // Assume failure.
  result._raw = "false";
  result._category = "bool";

  if (coerce (left,  "bool")._raw == "true" &&
      coerce (right, "bool")._raw == "true" )
  {
    result._raw = "true";
    result._category = "bool";
  }

//  std::cout << "# " << left << " and " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_or (Arg& result, Arg& left, Arg& right)
{
  // Assume failure.
  result._raw = "false";
  result._category = "bool";

  if (coerce (left,  "bool")._raw == "true" || 
      coerce (right, "bool")._raw == "true" )
  {
    result._raw = "true";
    result._category = "bool";
  }

//  std::cout << "# " << left << " or " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_xor (Arg& result, Arg& left, Arg& right)
{
  // Assume failure.
  result._raw = "false";
  result._category = "bool";

  bool bool_left  = coerce (left,  "bool")._raw == "true";
  bool bool_right = coerce (right, "bool")._raw == "true";

  if ((bool_left && !bool_right) ||
      (!bool_left && bool_right))
  {
    result._raw = "true";
    result._category = "bool";
  }

//  std::cout << "# " << left << " xor " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lt (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._raw != "H" && right._raw == "H") result._raw = "true";
    else if (left._raw == "L" && right._raw == "M") result._raw = "true";
    else if (left._raw == ""  && right._raw != "")  result._raw = "true";
    else                                            result._raw = "false";
  }
  else if (left._category  == "date" ||
           right._category == "date")
  {
    Date left_date  (left._raw,  _dateformat);
    Date right_date (right._raw, _dateformat);

    result._raw = result._raw = (left_date < right_date)
                              ? "true"
                              : "false";
  }
  else
  {
    result._raw = result._raw = (left._raw < right._raw)
                              ? "true"
                              : "false";
  }

  result._category = "bool";

//  std::cout << "# " << left << " < " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lte (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._raw        == right._raw       ) result._raw = "true";
    else if (                    right._raw == "H") result._raw = "true";
    else if (left._raw == "L" && right._raw == "M") result._raw = "true";
    else if (left._raw == ""                      ) result._raw = "true";
    else                                            result._raw = "false";
  }
  else if (left._category  == "date" ||
           right._category == "date")
  {
    Date left_date  (left._raw,  _dateformat);
    Date right_date (right._raw, _dateformat);

    result._raw = result._raw = (left_date <= right_date)
                              ? "true"
                              : "false";
  }
  else
  {
    result._raw = result._raw = (left._raw <= right._raw)
                              ? "true"
                              : "false";
  }

  result._category = "bool";

//  std::cout << "# " << left << " <= " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gte (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._raw        == right._raw       ) result._raw = "true";
    else if (left._raw == "H"                     ) result._raw = "true";
    else if (left._raw == "M" && right._raw == "L") result._raw = "true";
    else if (                    right._raw == "" ) result._raw = "true";
    else                                            result._raw = "false";
  }
  else if (left._category  == "date" ||
           right._category == "date")
  {
    Date left_date  (left._raw,  _dateformat);
    Date right_date (right._raw, _dateformat);

    result._raw = result._raw = (left_date >= right_date)
                              ? "true"
                              : "false";
  }
  else
  {
    result._raw = result._raw = (left._raw >= right._raw)
                              ? "true"
                              : "false";
  }

  result._category = "bool";

//  std::cout << "# " << left << " >= " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gt (Arg& result, Arg& left, Arg& right)
{
  if (left._raw == "priority")
  {
         if (left._raw == "H" && right._raw != "H") result._raw = "true";
    else if (left._raw == "M" && right._raw == "L") result._raw = "true";
    else if (left._raw != ""  && right._raw == "")  result._raw = "true";
    else                                            result._raw = "false";
  }
  else if (left._category  == "date" ||
           right._category == "date")
  {
    Date left_date  (left._raw,  _dateformat);
    Date right_date (right._raw, _dateformat);

    result._raw = result._raw = (left_date > right_date)
                              ? "true"
                              : "false";
  }
  else
  {
    result._raw = result._raw = (left._raw > right._raw)
                              ? "true"
                              : "false";
  }

  result._category = "bool";

//  std::cout << "# " << left << " > " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_inequal (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive)
{
  operator_equal (result, left, right, case_sensitive);
  result._raw = result._raw == "false"
                            ? "true"
                            : "false";

//  std::cout << "# " << left << " != " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
//          if (right._string.length () <= left._string.length ())
//            result = compare (right._string,
//                              left._string.substr (0, right._string.length ()),
//                              (bool) case_sensitive);
//        }
//        else
//          result = (left == right);
void E9::operator_equal (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive)
{
  // Assume failure.
  result._raw = "false";
  result._category = "bool";

  // 'project' and 'recur' attributes are matched leftmost.
  if (left._raw == "project" || left._raw == "recur")
  {
    coerce (left, "string");
    coerce (right, "string");

    if (right._raw.length () <= left._raw.length () &&
        compare (right._raw,
                 left._raw.substr (0, right._raw.length ()),
                 (bool) case_sensitive))
    {
      result._raw = "true";
      result._category = "bool";
    }
  }

  // Dates.
  else if (left._category  == "date" ||
           right._category == "date")
  {
    Date left_date  (left._raw,  _dateformat);
    Date right_date (right._raw, _dateformat);

    result._raw = (left_date == right_date)
                ? "true"
                : "false";
  }
  // Regular equality matching.
  else
  {
    result._raw = left._raw == right._raw
                ? "true"
                : "false";

    if (left._raw == right._raw)
    {
      result._raw = "true";
      result._category = "bool";
    }
  }

//  std::cout << "# " << left << " = " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_match (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive)
{
  result._category = "bool";

  result._raw = eval_match (left, right, case_sensitive)
              ? "true"
              : "false";

//  std::cout << "# " << left << " ~ " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
//        bool case_sensitive = context.config.getBoolean ("search.case.sensitive");
//        bool result = !eval_match (left, right, case_sensitive);
//
//        // Matches against description are really against either description,
//        // annotations or project.
//        // Short-circuit if match already failed.
//        if (result && left._raw == "description")
//        {
//          // TODO check further.
//        }
void E9::operator_nomatch (
  Arg& result,
  Arg& left,
  Arg& right,
  bool case_sensitive)
{
  result._category = "bool";

  result._raw = eval_match (left, right, case_sensitive)
              ? "false"
              : "true";

//  std::cout << "# " << left << " !~ " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_multiply (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " * " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_divide (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " / " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_add (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " + " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_subtract (Arg& result, Arg& left, Arg& right)
{
//  std::cout << "# " << left << " - " << right << " --> " << result << "\n";
}

////////////////////////////////////////////////////////////////////////////////
const Arg E9::coerce (const Arg& input, const std::string& type)
{
  Arg result;

  if (type == "bool")
  {
    result._category = "bool";
    result._raw = get_bool (input) ? "true" : "false";
  }

  if (type == "string")
  {
    // TODO Convert date?
    result._raw      = input._raw;
    result._category = "string";
  }

  // TODO Date
  // TODO Duration

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool E9::get_bool (const Arg& input)
{
  std::string value = lowerCase (input._raw);
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
