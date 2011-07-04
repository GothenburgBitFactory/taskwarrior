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
#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <Date.h>
#include <Duration.h>
#include <Nibbler.h>
#include <Variant.h>
#include <RX.h>
#include <text.h>
#include <Expression.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Perform all the necessary steps prior to an eval call.
Expression::Expression (Arguments& arguments)
: _args (arguments)
{
  if (_args.size ())
  {
    _args.dump ("Expression::Expression");

    expand_sequence ();
    implicit_and ();
    expand_tag ();
    expand_pattern ();
    expand_attr ();
    expand_attmod ();
    expand_word ();
    expand_tokens ();
    postfix ();
  }
}

////////////////////////////////////////////////////////////////////////////////
Expression::~Expression ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Expression::eval (Task& task)
{
  // If there are no elements in the filter, then all tasks pass.
  if (_args.size () == 0)
    return true;

  // There are elements in the filter, so the expression must be evaluated
  // against each task.
  std::vector <Variant> value_stack;

  // Case sensitivity is configurable.
  bool case_sensitive = context.config.getBoolean ("search.case.sensitive");

  // TODO Build an on-demand regex cache.

  std::vector <Triple>::const_iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_second == "op")
    {
//      std::cout << "# operator " << arg->_first << "\n";

      // Handle the unary operator first.
      if (arg->_first == "!")
      {
        // Are there sufficient arguments?
        if (value_stack.size () < 1)
          throw std::string ("Error: Insufficient operands for '!' operator.");

        Variant right (value_stack.back ());
        if (right._raw_type == "lvalue")
        {
          right = Variant (context.dom.get (right._raw, task));
          right._raw = value_stack.back ()._raw;
          right._raw_type = value_stack.back ()._raw_type;
        }
        value_stack.pop_back ();

//        std::cout << "#   " << " ! " << right.dump () << "\n";
        bool result = !right;
        right = Variant (result);
        right._raw_type = "bool";

//        std::cout << "#     --> " << right.dump () << "\n";
        value_stack.push_back (right);

        // This only occurs here, because the unary operators are handled, and
        // now the binary operators will be processed.
        continue;
      }

      // Are there sufficient arguments?
      if (value_stack.size () < 2)
        throw std::string ("Error: Insufficient operands for '") + arg->_first + "' operator.";

      // rvalue (string, rx, int, number, dom ...).
      Variant right (value_stack.back ());
      if (right._raw_type == "lvalue")
      {
        right = Variant (context.dom.get (right._raw, task));
        right._raw = value_stack.back ()._raw;
        right._raw_type = value_stack.back ()._raw_type;
      }
      value_stack.pop_back ();
//      std::cout << "# right raw=" << right._raw << " type=" << right._type << " value=" << right._string << "\n";

      // lvalue (dom).
      Variant left (value_stack.back ());
      if (left._raw_type == "lvalue")
      {
        left = Variant (context.dom.get (left._raw, task));
        left._raw = value_stack.back ()._raw;
        left._raw_type = value_stack.back ()._raw_type;
      }
      value_stack.pop_back ();
//      std::cout << "# left raw=" << left._raw << " type=" << left._type << " value=" << left._string << "\n";

      // Now the binary operators.
      if (arg->_first == "and")
      {
//        std::cout << "#   " << left.dump () << " and " << right.dump () << "\n";
        bool result = (left && right);
        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == "xor")
      {
//        std::cout << "#   " << left.dump () << " xor " << right.dump () << "\n";
        bool left_bool  = left.boolean ();
        bool right_bool = right.boolean ();
        bool result = (left_bool && !right_bool) || (!left_bool && right_bool);
        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == "or")
      {
//        std::cout << "#   " << left.dump () << " or " << right.dump () << "\n";
        bool result = (left || right);
        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == "<=")
      {
//        std::cout << "#   " << left.dump () << " <= " << right.dump () << "\n";
        bool result = false;
        if (left._raw == "priority")
        {
          left.cast (Variant::v_string);
          right.cast (Variant::v_string);

               if (left._string        == right._string       ) result = true;
          else if (                       right._string == "H") result = true;
          else if (left._string == "L" && right._string == "M") result = true;
          else if (left._string == ""                         ) result = true;
        }
        else
          result = (left <= right);

        left = Variant (result);
        left._raw_type = "bool";

        value_stack.push_back (left);
      }

      else if (arg->_first == ">=")
      {
//        std::cout << "#   " << left.dump () << " >= " << right.dump () << "\n";
        bool result = false;
        if (left._raw == "priority")
        {
          left.cast (Variant::v_string);
          right.cast (Variant::v_string);

               if (left._string        == right._string       ) result = true;
          else if (left._string == "H"                        ) result = true;
          else if (left._string == "M" && right._string == "L") result = true;
          else if (                       right._string == "" ) result = true;
        }
        else
          result = (left >= right);

        left = Variant (result);
        left._raw_type = "bool";

        value_stack.push_back (left);
      }

      else if (arg->_first == "!~")
      {
//        std::cout << "#   " << left.dump () << " !~ " << right.dump () << "\n";
        bool case_sensitive = context.config.getBoolean ("search.case.sensitive");
        bool result = !eval_match (left, right, case_sensitive);

        // Matches against description are really against either description,
        // annotations or project.
        // Short-circuit if match already failed.
        if (result && left._raw == "description")
        {
          // TODO check further.
        }

        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == "!=")
      {
//        std::cout << "#   " << left.dump () << " != " << right.dump () << "\n";
        bool result = (left != right);
        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == "=")
      {
//        std::cout << "#   " << left.dump () << " = " << right.dump () << "\n";
        bool result = false;
        if (left._raw == "project" || left._raw == "recur")
        {
          left.cast (Variant::v_string);
          right.cast (Variant::v_string);
          if (right._string.length () <= left._string.length ())
            result = compare (right._string,
                              left._string.substr (0, right._string.length ()),
                              (bool) case_sensitive);
        }
        else
          result = (left == right);

        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == ">")
      {
//        std::cout << "#   " << left.dump () << " > " << right.dump () << "\n";
        bool result = false;
        if (left._raw == "priority")
        {
          left.cast (Variant::v_string);
          right.cast (Variant::v_string);

               if (left._string == "H" && right._string != "H") result = true;
          else if (left._string == "M" && right._string == "L") result = true;
          else if (left._string != ""  && right._string == "")  result = true;
        }
        else
          result = (left > right);

        left = Variant (result);
        left._raw_type = "bool";

        value_stack.push_back (left);
      }

      else if (arg->_first == "~")
      {
//        std::cout << "#   " << left.dump () << " ~ " << right.dump () << "\n";
        bool case_sensitive = context.config.getBoolean ("search.case.sensitive");
        bool result = eval_match (left, right, case_sensitive);

        // Matches against description are really against either description,
        // annotations or project.
        // Short-circuit if match is already found.
        if (!result && left._raw == "description")
        {
          // TODO check further.
        }

        left = Variant (result);
        left._raw_type = "bool";

//        std::cout << "#     --> " << left.dump () << "\n";
        value_stack.push_back (left);
      }

      else if (arg->_first == "*")
      {
        left = left * right;
        value_stack.push_back (left);
      }

      else if (arg->_first == "/")
      {
        left = left / right;
        value_stack.push_back (left);
      }

      else if (arg->_first == "+")
      {
        left = left + right;
        value_stack.push_back (left);
      }

      else if (arg->_first == "-")
      {
        left = left - right;
        value_stack.push_back (left);
      }

      else if (arg->_first == "<")
      {
//        std::cout << "#   " << left.dump () << " < " << right.dump () << "\n";
        bool result = false;
        if (left._raw == "priority")
        {
          left.cast (Variant::v_string);
          right.cast (Variant::v_string);

               if (left._string != "H" && right._string == "H") result = true;
          else if (left._string == "L" && right._string == "M") result = true;
          else if (left._string == ""  && right._string != "")  result = true;
        }
        else
          result = (left < right);

        left = Variant (result);
        left._raw_type = "bool";

        value_stack.push_back (left);
      }

      else
        throw std::string ("Unsupported operator '") + arg->_first + "'.";
    }

    // It's not an operator, it's either and lvalue or some form of rvalue.
    else
    {
      Variant operand;
      create_variant (operand, arg->_first, arg->_second);
      value_stack.push_back (operand);
    }
  }

  // Coerce stack element to boolean.
  Variant result (value_stack.back ());
  value_stack.pop_back ();
  bool pass_fail = result.boolean ();

  // Check for stack remnants.
  if (value_stack.size ())
    throw std::string ("Error: Expression::eval found extra items on the stack.");

  return pass_fail;
}

////////////////////////////////////////////////////////////////////////////////
bool Expression::eval_match (Variant& left, Variant& right, bool case_sensitive)
{
  if (right._raw_type == "rx")
  {
    left.cast (Variant::v_string);
    right.cast (Variant::v_string);

    // Create a cached entry, if it does not already exist.
    if (_regexes.find (right._string) == _regexes.end ())
      _regexes[right._string] = RX (right._string, case_sensitive);

    if (_regexes[right._string].match (left._string))
      return true;
  }
  else
  {
    left.cast (Variant::v_string);
    right.cast (Variant::v_string);
    if (find (left._string, right._string, (bool) case_sensitive) != std::string::npos)
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Expression::create_variant (
  Variant& variant,
  const std::string& value,
  const std::string& type)
{
//  std::cout << "# operand '" << value << "' as " << type << "\n";

  // DOM references are not resolved until the operator is processed.  This
  // preserves the original name, which helps determine how to apply the
  // operator.
  if (type == "lvalue")
    variant = Variant (value);

  else if (type == "int")
    variant = Variant ((int) strtol (value.c_str (), NULL, 10));

  else if (type == "number")
    variant = Variant (strtod (value.c_str (), NULL));

  else if (type == "rvalue" ||
           type == "string" ||
           type == "rx")
    variant = Variant (unquoteText (value));

  else
    throw std::string ("Unrecognized operand '") +   + "'.";

  variant._raw      = value;
  variant._raw_type = type;
}

////////////////////////////////////////////////////////////////////////////////
// Convert:  1,3-5,00000000-0000-0000-0000-000000000000
//
// To:       (id=1 or (id>=3 and id<=5) or
//            uuid="00000000-0000-0000-0000-000000000000")
void Expression::expand_sequence ()
{
  Arguments temp;

  // Extract all the components of a sequence.
  std::vector <int> ids;
  std::vector <std::string> uuids;
  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "id")
      Arguments::extract_id (arg->_first, ids);

    else if (arg->_third == "uuid")
      Arguments::extract_uuid (arg->_first, uuids);
  }

  // If there is no sequence, we're done.
  if (ids.size () == 0 && uuids.size () == 0)
    return;

  // Construct the algebraic form.
  std::stringstream sequence;
  sequence << "(";
  for (unsigned int i = 0; i < ids.size (); ++i)
  {
    if (i)
      sequence << " or ";

    sequence << "id=" << ids[i];
  }

  if (uuids.size ())
  {
    if (sequence.str ().length () > 1)
      sequence << " or ";

    for (unsigned int i = 0; i < uuids.size (); ++i)
    {
      if (i)
        sequence << " or ";

      sequence << "uuid=\"" << uuids[i] << "\"";
    }
  }

  sequence << ")";

  // Copy everything up to the first id/uuid.
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "id" || arg->_third == "uuid")
      break;

    temp.push_back (*arg);
  }

  // Now insert the new sequence expression.
  temp.push_back (Triple (sequence.str (), "exp", "seq"));

  // Now copy everything after the last id/uuid.
  bool found_id = false;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "id" || arg->_third == "uuid")
      found_id = true;

    else if (found_id)
      temp.push_back (*arg);
  }

  _args.swap (temp);
  _args.dump ("Expression::expand_sequence");
}

////////////////////////////////////////////////////////////////////////////////
// Nibble the whole bloody thing.  Nuke it from orbit - it's the only way to be
// sure.
void Expression::expand_tokens ()
{
  Arguments temp;
  bool delta = false;

  // Get a list of all operators.
  std::vector <std::string> operators = Arguments::operator_list ();

  // Date format, for both parsing and rendering.
  std::string date_format = context.config.get ("dateformat");

  // Fake polymorphism.
  std::string s;
  int i;
  double d;
  time_t t;

  // Look at all args.
  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_second == "exp")
    {
      // Nibble each arg token by token.
      Nibbler n (arg->_first);

      while (! n.depleted ())
      {
        if (n.getQuoted ('"', s, true) ||
            n.getQuoted ('\'', s, true))
          temp.push_back (Triple (s, "string", arg->_third));

        else if (n.getQuoted ('/', s, true))
          temp.push_back (Triple (s, "pattern", arg->_third));

        else if (n.getOneOf (operators, s))
          temp.push_back (Triple (s, "op", arg->_third));

        else if (n.getDOM (s))
          temp.push_back (Triple (s, "lvalue", arg->_third));

        else if (n.getNumber (d))
          temp.push_back (Triple (format (d), "number", arg->_third));

        else if (n.getInt (i))
          temp.push_back (Triple (format (i), "int", arg->_third));

        else if (n.getDateISO (t))
          temp.push_back (Triple (Date (t).toISO (), "date", arg->_third));

        else if (n.getDate (date_format, t))
          temp.push_back (Triple (Date (t).toString (date_format), "date", arg->_third));

        else
        {
          if (! n.getUntilWS (s))
            n.getUntilEOS (s);

          temp.push_back (Triple (s, "?", arg->_third));
        }

        n.skipWS ();
      }

      delta = true;
    }
    else
      temp.push_back (*arg);
  }

  _args.swap (temp);
  _args.dump ("Expression::expand_tokens");
}

////////////////////////////////////////////////////////////////////////////////
// Inserts the 'and' operator by default between terms that are not separated by
// at least one operator.
//
// Converts:  <term1>     <term2> <op> <term>
// to:        <term1> and <term2> <op> <term>
//
void Expression::implicit_and ()
{
  Arguments temp;
  bool delta = false;

  std::string previous = "op";
  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    // Old-style filters need 'and' conjunctions.
    if (previous    != "op" &&
        arg->_third != "op")
    {
      temp.push_back (Triple ("and", "op", "-"));
      delta = true;
    }

    // Now insert the adjacent non-operator.
    temp.push_back (*arg);
    previous = arg->_third;
  }

  if (delta)
  {
    _args.swap (temp);
    _args.dump ("Expression::implicit_and");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Convert:  +with -without
// To:       tags ~ with
//           tags !~ without
void Expression::expand_tag ()
{
  Arguments temp;
  bool delta = false;

  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "tag")
    {
      char type;
      std::string value;
      Arguments::extract_tag (arg->_first, type, value);

      temp.push_back (Triple ("tags",                   "lvalue", arg->_third));
      temp.push_back (Triple (type == '+' ? "~" : "!~", "op",     arg->_third));
      temp.push_back (Triple (value,                    "string", arg->_third));
      delta = true;
    }
    else
      temp.push_back (*arg);
  }

  if (delta)
  {
    _args.swap (temp);
    _args.dump ("Expression::expand_tag");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Convert:  /foo/
// To:       description ~ foo
void Expression::expand_pattern ()
{
  Arguments temp;
  bool delta = false;

  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "pattern")
    {
      std::string value;
      Arguments::extract_pattern (arg->_first, value);

      temp.push_back (Triple ("description", "lvalue", arg->_third));
      temp.push_back (Triple ("~",           "op",     arg->_third));
      temp.push_back (Triple (value,         "rx",     arg->_third));
      delta = true;
    }
    else
      temp.push_back (*arg);
  }

  if (delta)
  {
    _args.swap (temp);
    _args.dump ("Expression::expand_pattern");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Convert:  <name>[:=]<value>
// To:       <name> = lex<value>
void Expression::expand_attr ()
{
  Arguments temp;
  bool delta = false;

  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "attr")
    {
      // TODO Canonicalize 'name'.
      std::string name;
      std::string value;
      Arguments::extract_attr (arg->_first, name, value);
      Arguments::is_attribute (name, name);

      // Always quote the value, so that empty values, or values containing spaces
      // are preserved.
      value = "\"" + value + "\"";

      temp.push_back (Triple (name,  "lvalue", arg->_third));
      temp.push_back (Triple ("=",   "op",     arg->_third));
      temp.push_back (Triple (value, "rvalue", arg->_third));
      delta = true;
    }
    else
      temp.push_back (*arg);
  }

  if (delta)
  {
    _args.swap (temp);
    _args.dump ("Expression::expand_attr");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Convert:  <name>.<mod>[:=]<value>
// To:       <name> <op> lex<value>
void Expression::expand_attmod ()
{
  Arguments temp;
  bool delta = false;

  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "attmod")
    {
      std::string name;
      std::string mod;
      std::string value;
      std::string sense;
      Arguments::extract_attmod (arg->_first, name, mod, value, sense);
      Arguments::is_attribute (name, name);
      Arguments::is_modifier (mod);

      // Always quote the value, so that empty values, or values containing spaces
      // are preserved.
      std::string raw_value = value;
      value = "\"" + value + "\"";

      if (mod == "before" || mod == "under" || mod == "below")
      {
        temp.push_back (Triple (name,  "lvalue", arg->_third));
        temp.push_back (Triple ("<",   "op",     arg->_third));
        temp.push_back (Triple (value, "rvalue", arg->_third));
      }
      else if (mod == "after" || mod == "over" || mod == "above")
      {
        temp.push_back (Triple (name,  "lvalue", arg->_third));
        temp.push_back (Triple (">",   "op",     arg->_third));
        temp.push_back (Triple (value, "rvalue", arg->_third));
      }
      else if (mod == "none")
      {
        temp.push_back (Triple (name,   "lvalue", arg->_third));
        temp.push_back (Triple ("==",   "op",     arg->_third));
        temp.push_back (Triple ("\"\"", "string", arg->_third));
      }
      else if (mod == "any")
      {
        temp.push_back (Triple (name,   "lvalue", arg->_third));
        temp.push_back (Triple ("!=",   "op",     arg->_third));
        temp.push_back (Triple ("\"\"", "string", arg->_third));
      }
      else if (mod == "is" || mod == "equals")
      {
        temp.push_back (Triple (name,  "lvalue", arg->_third));
        temp.push_back (Triple ("=",   "op",     arg->_third));
        temp.push_back (Triple (value, "rvalue", arg->_third));
      }
      else if (mod == "isnt" || mod == "not")
      {
        temp.push_back (Triple (name,  "lvalue", arg->_third));
        temp.push_back (Triple ("!=",  "op",     arg->_third));
        temp.push_back (Triple (value, "rvalue", arg->_third));
      }
      else if (mod == "has" || mod == "contains")
      {
        temp.push_back (Triple (name,  "lvalue", arg->_third));
        temp.push_back (Triple ("~",   "op",     arg->_third));
        temp.push_back (Triple (value, "rvalue", arg->_third));
      }
      else if (mod == "hasnt")
      {
        temp.push_back (Triple (name,  "lvalue", arg->_third));
        temp.push_back (Triple ("!~",  "op",     arg->_third));
        temp.push_back (Triple (value, "rvalue", arg->_third));
      }
      else if (mod == "startswith" || mod == "left")
      {
        temp.push_back (Triple (name,            "lvalue", arg->_third));
        temp.push_back (Triple ("~",             "op",     arg->_third));
        temp.push_back (Triple ("^" + raw_value, "rx",     arg->_third));
      }
      else if (mod == "endswith" || mod == "right")
      {
        temp.push_back (Triple (name,            "lvalue", arg->_third));
        temp.push_back (Triple ("~",             "op",     arg->_third));
        temp.push_back (Triple (raw_value + "$", "rx",     arg->_third));
      }
      else if (mod == "word")
      {
        temp.push_back (Triple (name,                      "lvalue", arg->_third));
        temp.push_back (Triple ("~",                       "op",     arg->_third));
        temp.push_back (Triple ("\\b" + raw_value + "\\b", "rx",     arg->_third));
      }
      else if (mod == "noword")
      {
        temp.push_back (Triple (name,                      "lvalue", arg->_third));
        temp.push_back (Triple ("!~",                      "op",     arg->_third));
        temp.push_back (Triple ("\\b" + raw_value + "\\b", "rx",     arg->_third));
      }
      else
        throw std::string ("Error: unrecognized attribute modifier '") + mod + "'.";

      delta = true;
    }
    else
      temp.push_back (*arg);
  }

  if (delta)
  {
    _args.swap (temp);
    _args.dump ("Expression::expand_attmod");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Convert:  <word>
// To:       description ~ <word>
void Expression::expand_word ()
{
  Arguments temp;
  bool delta = false;

  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_third == "word")
    {
      temp.push_back (Triple ("description",             "lvalue", arg->_third));
      temp.push_back (Triple ("~",                       "op",     arg->_third));
      temp.push_back (Triple ("\"" + arg->_first + "\"", "rvalue", arg->_third));

      delta = true;
    }
    else
      temp.push_back (*arg);
  }

  if (delta)
  {
    _args.swap (temp);
    _args.dump ("Expression::expand_word");
  }
}

////////////////////////////////////////////////////////////////////////////////
// Dijkstra Shunting Algorithm.
// http://en.wikipedia.org/wiki/Shunting-yard_algorithm
//
//   While there are tokens to be read:
//     Read a token.
//     If the token is an operator, o1, then:
//       while there is an operator token, o2, at the top of the stack, and
//             either o1 is left-associative and its precedence is less than or
//             equal to that of o2,
//             or o1 is right-associative and its precedence is less than that
//             of o2,
//         pop o2 off the stack, onto the output queue;
//       push o1 onto the stack.
//     If the token is a left parenthesis, then push it onto the stack.
//     If the token is a right parenthesis:
//       Until the token at the top of the stack is a left parenthesis, pop
//       operators off the stack onto the output queue.
//       Pop the left parenthesis from the stack, but not onto the output queue.
//       If the token at the top of the stack is a function token, pop it onto
//       the output queue.
//       If the stack runs out without finding a left parenthesis, then there
//       are mismatched parentheses.
//     If the token is a number, then add it to the output queue.
//
//   When there are no more tokens to read:
//     While there are still operator tokens in the stack:
//       If the operator token on the top of the stack is a parenthesis, then
//       there are mismatched parentheses.
//       Pop the operator onto the output queue.
//   Exit.
//
void Expression::postfix ()
{
  Arguments temp;

  std::pair <std::string, std::string> item;
  Arguments op_stack;
  char type;
  int precedence;
  char associativity;

  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
  {
    if (arg->_first == "(")
    {
      op_stack.push_back (*arg);
    }
    else if (arg->_first == ")")
    {
      while (op_stack.size () > 0 &&
             op_stack.back ()._first != "(")
      {
        temp.push_back (op_stack.back ());
        op_stack.pop_back ();
      }

      if (op_stack.size ())
      {
        op_stack.pop_back ();
      }
      else
      {
        throw std::string ("Mismatched parentheses in expression");
      }
    }
    else if (Arguments::is_operator (arg->_first, type, precedence, associativity))
    {
      char type2;
      int precedence2;
      char associativity2;
      while (op_stack.size () > 0 &&
             Arguments::is_operator (op_stack.back ()._first, type2, precedence2, associativity2) &&
             ((associativity == 'l' && precedence <= precedence2) ||
              (associativity == 'r' && precedence <  precedence2)))
      {
        temp.push_back (op_stack.back ());
        op_stack.pop_back ();
      }

      op_stack.push_back (*arg);
    }
    else
    {
      temp.push_back (*arg);
    }
  }

  while (op_stack.size () != 0)
  {
    if (op_stack.back ()._first == "(" ||
        op_stack.back ()._first == ")")
      throw std::string ("Mismatched parentheses in expression");

    temp.push_back (op_stack.back ());
    op_stack.pop_back ();
  }

  _args.swap (temp);
  _args.dump ("Expression::toPostfix");
}

////////////////////////////////////////////////////////////////////////////////
// Test whether the _original arguments are old style or new style.
//
// Old style:  no single argument corresponds to an operator, ie no 'and', 'or',
//             etc.
//
// New style:  at least one argument that is an operator.
//
bool Expression::is_new_style ()
{
  std::vector <Triple>::iterator arg;
  for (arg = _args.begin (); arg != _args.end (); ++arg)
    if (Arguments::is_symbol_operator (arg->_first))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
