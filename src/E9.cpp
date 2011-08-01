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

#include <iostream> // TODO Remove.
#include <Context.h>
#include <text.h>
#include <A3.h>
#include <E9.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Perform all the necessary steps prior to an eval call.
E9::E9 (const A3& args)
{
  std::vector <Arg>::const_iterator arg;
  for (arg = args.begin (); arg != args.end (); ++arg)
    _terms.push_back (Term (*arg));
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

  std::vector <Term> value_stack;
  eval (task, value_stack);

  // Coerce result to Boolean.
  Term result = value_stack.back ();
  value_stack.pop_back ();
  return get_bool (coerce (result, "bool"));
}

////////////////////////////////////////////////////////////////////////////////
std::string E9::evalExpression (const Task& task)
{
  if (_terms.size () == 0)
    return "";

  std::vector <Term> value_stack;
  eval (task, value_stack);

  return value_stack.back ()._value;
}

////////////////////////////////////////////////////////////////////////////////
void E9::eval (const Task& task, std::vector <Term>& value_stack)
{
  // Case sensitivity is configurable.
  bool case_sensitive = context.config.getBoolean ("search.case.sensitive");

  std::vector <Term>::iterator arg;
  for (arg = _terms.begin (); arg != _terms.end (); ++arg)
  {
    if (arg->_category == "op")
    {
      Term result;

      // Unary operators.
      if (arg->_raw == "!")
      {
        if (value_stack.size () < 1)
          throw std::string ("There are no operands for the 'not' operator.");

        Term right = value_stack.back ();
        value_stack.pop_back ();

        if (right._category == "dom")
        {
          right._value = context.dom.get (right._raw, task);
          right._category = "string";
        }

        operator_not (result, right);
      }

      // Binary operators.
      else
      {
        if (value_stack.size () < 2)
          throw std::string ("There are not enough operands for the '") + arg->_raw + "' operator.";

        Term right = value_stack.back ();
        value_stack.pop_back ();

        if (right._category == "dom")
        {
          right._value = context.dom.get (right._raw, task);
          right._category = "string";
        }

        Term left = value_stack.back ();
        value_stack.pop_back ();

        if (left._category == "dom")
        {
          left._value = context.dom.get (left._raw, task);
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
      value_stack.push_back (*arg);
    }
  }

  // Check for stack remnants.
  if (value_stack.size () != 1)
    throw std::string ("Error: Expression::eval found extra items on the stack.");
}

////////////////////////////////////////////////////////////////////////////////
bool E9::eval_match (Term& left, Term& right, bool case_sensitive)
{
  if (right._category == "rx")
  {
    left  = coerce (left,  "string");
    right = coerce (right, "string");

    // Create a cached entry, if it does not already exist.
    if (_regexes.find (right._value) == _regexes.end ())
      _regexes[right._value] = RX (right._value, case_sensitive);

    if (_regexes[right._value].match (left._value))
      return true;
  }
  else
  {
    left  = coerce (left,  "string");
    right = coerce (right, "string");
    if (find (left._value, right._value, (bool) case_sensitive) != std::string::npos)
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_not (Term& result, Term& right)
{
  // TODO This is not right.
  result = Term (right._raw, coerce (right, "bool")._value, "bool");

  std::cout << "# not " << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_and (Term& result, Term& left, Term& right)
{
  // Assume failure.
  result._raw = result._value = "false";
  result._category = "bool";

  if (coerce (left,  "bool")._value == "true" &&
      coerce (right, "bool")._value == "true" )
  {
    result._raw = result._value = "true";
    result._category = "bool";
  }

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " and "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_or (Term& result, Term& left, Term& right)
{
  // Assume failure.
  result._raw = result._value = "false";
  result._category = "bool";

  if (coerce (left,  "bool")._value == "true" || 
      coerce (right, "bool")._value == "true" )
  {
    result._raw = result._value = "true";
    result._category = "bool";
  }

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " or "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_xor (Term& result, Term& left, Term& right)
{
  // Assume failure.
  result._raw = result._value = "false";
  result._category = "bool";

  bool bool_left  = coerce (left,  "bool")._value == "true";
  bool bool_right = coerce (right, "bool")._value == "true";

  if ((bool_left && !bool_right) ||
      (!bool_left && bool_right))
  {
    result._raw = result._value = "true";
    result._category = "bool";
  }

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " xor "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lt (Term& result, Term& left, Term& right)
{
  if (left._raw == "priority")
  {
         if (left._value != "H" && right._value == "H") result._raw = result._value = "true";
    else if (left._value == "L" && right._value == "M") result._raw = result._value = "true";
    else if (left._value == ""  && right._value != "")  result._raw = result._value = "true";
    else                                                result._raw = result._value = "false";
  }
  else
  {
    if (left._value < right._value)
      result._raw = result._value = "true";
    else
      result._raw = result._value = "false";
  }

  result._category = "bool";

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " < "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lte (Term& result, Term& left, Term& right)
{
  if (left._raw == "priority")
  {
         if (left._value        == right._value       ) result._raw = result._value = "true";
    else if (                      right._value == "H") result._raw = result._value = "true";
    else if (left._value == "L" && right._value == "M") result._raw = result._value = "true";
    else if (left._value == ""                        ) result._raw = result._value = "true";
    else                                                result._raw = result._value = "false";
  }
  else
  {
    if (left._value <= right._value)
      result._raw = result._value = "true";
    else
      result._raw = result._value = "false";
  }

  result._category = "bool";

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " <= "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gte (Term& result, Term& left, Term& right)
{
  if (left._raw == "priority")
  {
         if (left._value        == right._value       ) result._raw = result._value = "true";
    else if (left._value == "H"                       ) result._raw = result._value = "true";
    else if (left._value == "M" && right._value == "L") result._raw = result._value = "true";
    else if (                      right._value == "" ) result._raw = result._value = "true";
    else                                                result._raw = result._value = "false";
  }
  else
  {
    if (left._value >= right._value)
      result._raw = result._value = "true";
    else
      result._raw = result._value = "false";
  }

  result._category = "bool";

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " >= "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gt (Term& result, Term& left, Term& right)
{
  if (left._raw == "priority")
  {
         if (left._value == "H" && right._value != "H") result._raw = result._value = "true";
    else if (left._value == "M" && right._value == "L") result._raw = result._value = "true";
    else if (left._value != ""  && right._value == "")  result._raw = result._value = "true";
    else                                                result._raw = result._value = "false";
  }
  else
  {
    if (left._value > right._value)
      result._raw = result._value = "true";
    else
      result._raw = result._value = "false";
  }

  result._category = "bool";

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " > "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_inequal (
  Term& result,
  Term& left,
  Term& right,
  bool case_sensitive)
{
  operator_equal (result, left, right, case_sensitive);
  if (result._raw == "false")
    result._raw = result._value = "true";
  else
    result._raw = result._value = "false";

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " != "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
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
  Term& result,
  Term& left,
  Term& right,
  bool case_sensitive)
{
  // Assume failure.
  result._raw = result._value = "false";
  result._category = "bool";

  // 'project' and 'recur' attributes are matched leftmost.
  if (left._raw == "project" || left._raw == "recur")
  {
    coerce (left, "string");
    coerce (right, "string");

    if (right._value.length () <= left._value.length () &&
        compare (right._value,
                 left._value.substr (0, right._value.length ()),
                 (bool) case_sensitive))
    {
      result._raw = result._value = "true";
      result._category = "bool";
    }
  }

  // Regular equality matching.
  else
  {
    if (left._value == right._value)
    {
      result._raw = result._value = "true";
      result._category = "bool";
    }
  }

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " = "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_match (
  Term& result,
  Term& left,
  Term& right,
  bool case_sensitive)
{
  result._category = "bool";

  if (eval_match (left, right, case_sensitive))
    result._raw = result._value = "true";
  else
    result._raw = result._value = "false";

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " ~ "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
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
  Term& result,
  Term& left,
  Term& right,
  bool case_sensitive)
{
  result._category = "bool";

  if (!eval_match (left, right, case_sensitive))
    result._raw = result._value = "true";
  else
    result._raw = result._value = "false";

/*
  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " !~ "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
*/
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_multiply (Term& result, Term& left, Term& right)
{

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " * "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_divide (Term& result, Term& left, Term& right)
{

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " / "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_add (Term& result, Term& left, Term& right)
{

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " + "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_subtract (Term& result, Term& left, Term& right)
{

  std::cout << "# " << left._raw << "/" << left._value << "/" << left._category
            << " - "
            << right._raw << "/" << right._value << "/" << right._category
            << " --> "
            << result._raw << "/" << result._value << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
const Term E9::coerce (const Term& input, const std::string& type)
{
  Term result;

  if (type == "bool")
  {
    result._category = "bool";
    result._value = get_bool (input) ? "true" : "false";
  }

  if (type == "string")
  {
    // TODO Convert date?
    result._raw      = input._raw;
    result._value    = input._value;
    result._category = "string";
  }

  // TODO Date
  // TODO Duration

  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool E9::get_bool (const Term& input)
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
