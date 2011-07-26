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
#include <A3.h>
#include <E9.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Perform all the necessary steps prior to an eval call.
E9::E9 (A3& args)
: _args (args)
{
}

////////////////////////////////////////////////////////////////////////////////
E9::~E9 ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool E9::evalFilter (const Task& task)
{
  if (_args.size () == 0)
    return true;

  std::vector <Arg> value_stack;
  eval (task, value_stack);

  // TODO Coerce result to Boolean.
  return true;
}

////////////////////////////////////////////////////////////////////////////////
std::string E9::evalExpression (const Task& task)
{
  if (_args.size () == 0)
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
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_category == "op")
    {
      Arg result;

      // Unary operators.
      if (arg->_raw == "!")
      {
        Arg right = value_stack.back ();
        value_stack.pop_back ();
        operator_not (result, right);
      }

      // Binary operators.
      else
      {
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
        else if (arg->_raw == "!=")  operator_inequal  (result, left, right);
        else if (arg->_raw == "=")   operator_equal    (result, left, right);
        else if (arg->_raw == "~")   operator_match    (result, left, right);
        else if (arg->_raw == "!~")  operator_nomatch  (result, left, right);
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
void E9::operator_not (Arg& result, Arg& left)
{

  std::cout << "# not " << left._raw << "/" << left._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_and (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " and "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_or (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " or "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_xor (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " xor "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lt (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " < "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_lte (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " <= "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gte (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " >= "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_gt (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " > "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_inequal (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " != "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_equal (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " = "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_match (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " ~ "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_nomatch (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " !~ "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_multiply (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " * "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_divide (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " / "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_add (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " + "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void E9::operator_subtract (Arg& result, Arg& left, Arg& right)
{

  std::cout << "# " << left._raw << "/" << left._category
            << " - "
            << right._raw << "/" << right._category
            << " --> "
            << result._raw << "/" << result._category
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
