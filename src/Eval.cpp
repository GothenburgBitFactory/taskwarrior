////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <iostream>
#include <time.h>
#include <Eval.h>

////////////////////////////////////////////////////////////////////////////////
// Supported operators, borrowed from C++, particularly the precedence.
// Note: table is sorted by length of operator string, so searches match
//       longest first.
static struct
{
  std::string op;
  int         precedence;
  char        type;
  char        associativity;
} operators[] =
{
  // Operator   Precedence  Type  Associativity
  {  "^",        16,         'b',  'r' },    // Exponent

  {  "!",        15,         'u',  'r' },    // Unary not
  {  "_neg_",    15,         'u',  'r' },    // Unary minus
  {  "_pos_",    15,         'u',  'r' },    // Unary plus

  {  "_hastag_", 14,         'b',  'l'},     // +tag  [Pseudo-op]
  {  "_notag_",  14,         'b',  'l'},     // -tag  [Pseudo-op]

  {  "*",        13,         'b',  'l' },    // Multiplication
  {  "/",        13,         'b',  'l' },    // Division
  {  "%",        13,         'b',  'l' },    // Modulus

  {  "+",        12,         'b',  'l' },    // Addition
  {  "-",        12,         'b',  'l' },    // Subtraction

  {  "<=",       10,         'b',  'l' },    // Less than or equal
  {  ">=",       10,         'b',  'l' },    // Greater than or equal
  {  ">",        10,         'b',  'l' },    // Greater than
  {  "<",        10,         'b',  'l' },    // Less than

  {  "=",         9,         'b',  'l' },    // Equal (partial)
  {  "==",        9,         'b',  'l' },    // Equal (exact)
  {  "!=",        9,         'b',  'l' },    // Inequal

  {  "~",         8,         'b',  'l' },    // Regex match
  {  "!~",        8,         'b',  'l' },    // Regex non-match

  {  "and",       5,         'b',  'l' },    // Conjunction
  {  "or",        4,         'b',  'l' },    // Disjunction
  {  "xor",       3,         'b',  'l' },    // Disjunction

  {  "(",         0,         'b',  'l' },    // Precedence start
  {  ")",         0,         'b',  'l' },    // Precedence end
};

#define NUM_OPERATORS (sizeof (operators) / sizeof (operators[0]))

////////////////////////////////////////////////////////////////////////////////
// Built-in support for some named constants.
static bool namedConstants (const std::string& name, Variant& value)
{
       if (name == "true")  value = Variant (true);
  else if (name == "false") value = Variant (false);
  else if (name == "pi")    value = Variant (3.14159165);
  else
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
Eval::Eval ()
: _ambiguity (true)
, _debug (false)
{
  addSource (namedConstants);
}

////////////////////////////////////////////////////////////////////////////////
Eval::~Eval ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Eval::addSource (bool (*source)(const std::string&, Variant&))
{
  _sources.push_back (source);
}

////////////////////////////////////////////////////////////////////////////////
void Eval::evaluateInfixExpression (const std::string& e, Variant& v) const
{
  // Reduce e to a vector of tokens.
  Lexer l (e);
  l.ambiguity (_ambiguity);
  std::vector <std::pair <std::string, Lexer::Type> > tokens;
  std::string token;
  Lexer::Type type;
  while (l.token (token, type))
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));

  // Parse for syntax checking and operator replacement.
  infixParse (tokens);
  if (_debug)
  {
    std::vector <std::pair <std::string, Lexer::Type> >::iterator i;
    for (i = tokens.begin (); i != tokens.end (); ++i)
      std::cout << "# token infix '" << i->first << "' " << Lexer::type_name (i->second) << "\n";
  }

  // Convert infix --> postfix.
  infixToPostfix (tokens);

  // Call the postfix evaluator.
  evaluatePostfixStack (tokens, v);
}

////////////////////////////////////////////////////////////////////////////////
void Eval::evaluatePostfixExpression (const std::string& e, Variant& v) const
{
  // Reduce e to a vector of tokens.
  Lexer l (e);
  l.ambiguity (_ambiguity);
  std::vector <std::pair <std::string, Lexer::Type> > tokens;
  std::string token;
  Lexer::Type type;
  while (l.token (token, type))
  {
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));
    if (_debug)
      std::cout << "# token postfix '" << token << "' " << Lexer::type_name (type) << "\n";
  }

  // Call the postfix evaluator.
  evaluatePostfixStack (tokens, v);
}

////////////////////////////////////////////////////////////////////////////////
void Eval::compileExpression (const std::string& e)
{
  // Reduce e to a vector of tokens.
  Lexer l (e);
  l.ambiguity (_ambiguity);
  std::string token;
  Lexer::Type type;
  while (l.token (token, type))
  {
    _compiled.push_back (std::pair <std::string, Lexer::Type> (token, type));
    if (_debug)
      std::cout << "# token postfix '" << token << "' " << Lexer::type_name (type) << "\n";
  }

  // Parse for syntax checking and operator replacement.
  infixParse (_compiled);
  if (_debug)
  {
    std::vector <std::pair <std::string, Lexer::Type> >::iterator i;
    for (i = _compiled.begin (); i != _compiled.end (); ++i)
      std::cout << "# token infix '" << i->first << "' " << Lexer::type_name (i->second) << "\n";
  }

  // Convert infix --> postfix.
  infixToPostfix (_compiled);
}

////////////////////////////////////////////////////////////////////////////////
void Eval::evaluateCompiledExpression (Variant& v)
{
  // Call the postfix evaluator.
  evaluatePostfixStack (_compiled, v);
}

////////////////////////////////////////////////////////////////////////////////
void Eval::ambiguity (bool value)
{
  _ambiguity = value;
}

////////////////////////////////////////////////////////////////////////////////
void Eval::debug ()
{
  _debug = true;
}

////////////////////////////////////////////////////////////////////////////////
// Static.
void Eval::getOperators (std::vector <std::string>& all)
{
  all.clear ();
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    all.push_back (operators[i].op);
}

////////////////////////////////////////////////////////////////////////////////
void Eval::evaluatePostfixStack (
  const std::vector <std::pair <std::string, Lexer::Type> >& tokens,
  Variant& result) const
{
  if (tokens.size () == 0)
  {
    std::cout << "No expression to evaluate.\n";
    result = Variant ("");
    return;
  }

  // This is stack used by the postfix evaluator.
  std::vector <Variant> values;

  std::vector <std::pair <std::string, Lexer::Type> >::const_iterator token;
  for (token = tokens.begin (); token != tokens.end (); ++token)
  {
    // Unary operators.
    if (token->second == Lexer::typeOperator &&
        token->first == "!")
    {
      Variant right = values.back ();
      values.pop_back ();
      values.push_back (! right);
    }
    else if (token->second == Lexer::typeOperator &&
             token->first == "_neg_")
    {
      Variant right = values.back ();
      values.pop_back ();
      values.push_back (Variant (0) - right);
    }
    else if (token->second == Lexer::typeOperator &&
             token->first == "_pos_")
    {
      // NOP?
    }

    // Binary operators.
    else if (token->second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "# [" << values.size () << "] eval operator '" << token->first << "'\n";

      Variant right = values.back ();
      values.pop_back ();

      Variant left = values.back ();
      values.pop_back ();

      // Ordering these by anticipation frequency of use is a good idea.
           if (token->first == "and") left = left && right;
      else if (token->first == "or")  left = left || right;
      else if (token->first == "&&")  left = left && right;
      else if (token->first == "||")  left = left || right;
      else if (token->first == "<")   left = left < right;
      else if (token->first == "<=")  left = left <= right;
      else if (token->first == ">")   left = left > right;
      else if (token->first == ">=")  left = left >= right;
      else if (token->first == "==")  left = left.operator== (right);
      else if (token->first == "=")   left = left.operator== (right);
      else if (token->first == "!=")  left = left.operator!= (right);
      else if (token->first == "+")   left += right;
      else if (token->first == "-")   left -= right;
      else if (token->first == "*")   left *= right;
      else if (token->first == "/")   left /= right;
      else if (token->first == "^")   left ^= right;
      else if (token->first == "%")   left %= right;
      else if (token->first == "xor") left = left.operator_xor (right);
      else if (token->first == "~")   left = left.operator_match (right);
      else if (token->first == "!~")  left = left.operator_nomatch (right);
      else
        std::cout << "# Unrecognized operator '" << token->first << "'\n";

      values.push_back (left);
    }

    // Literals and identifiers.
    else
    {
      Variant v (token->first);
      switch (token->second)
      {
      case Lexer::typeNumber:
      case Lexer::typeHex:
        v.cast (Variant::type_integer);
        break;

      case Lexer::typeDecimal:
        v.cast (Variant::type_real);
        break;

      case Lexer::typeOperator:
        std::cout << "# Error: operator unexpected.\n";
        break;

      case Lexer::typeIdentifier:
        {
          bool found = false;
          std::vector <bool (*)(const std::string&, Variant&)>::const_iterator source;
          for (source = _sources.begin (); source != _sources.end (); ++source)
          {
            if ((*source) (token->first, v))
            {
              if (_debug)
                std::cout << "# [" << values.size () << "] eval source '" << token->first << "' --> '" << (std::string) v << "'\n";
              found = true;
              break;
            }
          }

          // An identifier that fails lookup is a string.
          if (!found)
          {
            v.cast (Variant::type_string);
            if (_debug)
              std::cout << "# [" << values.size () << "] eval source failed '" << token->first << "'\n";
          }
        }
        break;

      case Lexer::typeDate:
        v.cast (Variant::type_date);
        break;

      case Lexer::typeDuration:
        v.cast (Variant::type_duration);
        break;

      // Nothing to do.
      case Lexer::typeString:
      default:
        break;
      }

      values.push_back (v);
      if (_debug)
        std::cout << "# [" << values.size () << "] eval push '" << token->first << "' " << Lexer::type_name (token->second) << "\n";
    }
  }

  // Should only be one value left on the stack.
  if (values.size () != 1)
    if (_debug)
      std::cout << "# Error: Unexpected stack size: " << values.size () << "\n";

  result = values[0];
}

////////////////////////////////////////////////////////////////////////////////
//
// Grammar:
//   Logical     --> Regex {( "and" | "or" | "xor" ) Regex}
//   Regex       --> Equality {( "~" | "!~" ) Equality}
//   Equality    --> Comparative {( "==" | "=" | "!=" ) Comparative}
//   Comparative --> Arithmetic {( "<=" | "<" | ">=" | ">" ) Arithmetic}
//   Arithmetic  --> Geometric {( "+" | "-" ) Geometric}
//   Geometric   --> Tag {( "*" | "/" | "%" ) Tag}
//   Tag         --> Unary {( "_hastag_" | "_notag_" ) Unary}
//   Unary       --> [( "-" | "+" | "!" )] Exponent
//   Exponent    --> Primitive ["^" Primitive]
//   Primitive   --> "(" Logical ")" | Variant
//
void Eval::infixParse (
  std::vector <std::pair <std::string, Lexer::Type> >& infix) const
{
  if (_debug)
    std::cout << "# infixParse\n";

  try
  {
    int i = 0;
    if (parseLogical (infix, i))
      if (_debug)
        std::cout << "# no errors.\n";
  }

  catch (const std::string& error)
  {
    std::cerr << error << "\n";
  }

  catch (...)
  {
    std::cerr << "Unknown error.\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Logical     --> Regex {( "and" | "or" | "xor" ) Regex}
bool Eval::parseLogical (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#   parseLogical\n";

  if (i < infix.size () &&
      parseRegex (infix, i))
  {
    while ((infix[i].first == "and"  ||
            infix[i].first == "or"   ||
            infix[i].first == "xor") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#   " << infix[i].first << "\n";

      ++i;
      if (! parseRegex (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Regex       --> Equality {( "~" | "!~" ) Equality}
bool Eval::parseRegex (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#     parseRegex\n";

  if (i < infix.size () &&
      parseEquality (infix, i))
  {
    while ((infix[i].first == "~" ||
            infix[i].first == "!~") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#     " << infix[i].first << "\n";

      ++i;
      if (! parseEquality (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Equality    --> Comparative {( "==" | "=" | "!=" ) Comparative}
bool Eval::parseEquality (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#       parseEquality\n";

  if (i < infix.size () &&
      parseComparative (infix, i))
  {
    while ((infix[i].first == "==" ||
            infix[i].first == "="  ||
            infix[i].first == "!=") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#       " << infix[i].first << "\n";

      ++i;
      if (! parseComparative (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Comparative --> Arithmetic {( "<=" | "<" | ">=" | ">" ) Arithmetic}
bool Eval::parseComparative (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#         parseComparative\n";

  if (i < infix.size () &&
      parseArithmetic (infix, i))
  {
    while ((infix[i].first == "<=" ||
            infix[i].first == "<"  ||
            infix[i].first == ">=" ||
            infix[i].first == ">") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#         " << infix[i].first << "\n";

      ++i;
      if (! parseArithmetic (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Arithmetic  --> Geometric {( "+" | "-" ) Geometric}
bool Eval::parseArithmetic (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#           parseArithmetic\n";

  if (i < infix.size () &&
      parseGeometric (infix, i))
  {
    while ((infix[i].first == "+" ||
            infix[i].first == "-") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#           " << infix[i].first << "\n";

      ++i;
      if (! parseGeometric (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Geometric   --> Tag {( "*" | "/" | "%" ) Tag}
bool Eval::parseGeometric (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#             parseGeometric\n";

  if (i < infix.size () &&
      parseTag (infix, i))
  {
    while ((infix[i].first == "*" ||
            infix[i].first == "/" ||
            infix[i].first == "%") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#             " << infix[i].first << "\n";

      ++i;
      if (! parseTag (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Tag         --> Unary {( "_hastag_" | "_notag_" ) Unary}
bool Eval::parseTag (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#               parseTag\n";

  if (i < infix.size () &&
      parseUnary (infix, i))
  {
    while ((infix[i].first == "_hastag_" ||
            infix[i].first == "_notag_") &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#               " << infix[i].first << "\n";

      ++i;
      if (! parseUnary (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Unary       --> [( "-" | "+" | "!" )] Exponent
bool Eval::parseUnary (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#                 parseUnary\n";

  if (i < infix.size ())
  {
    if (infix[i].first == "-")
    {
      if (_debug)
        std::cout << "#                 '-' --> '_neg_'\n";

      infix[i].first = "_neg_";
      ++i;
    }
    else if (infix[i].first == "+")
    {
      if (_debug)
        std::cout << "#                 '+' --> '_pos_'\n";

      infix[i].first = "_pos_";
      ++i;
    }
    else if (infix[i].first == "!")
    {
      if (_debug)
        std::cout << "#                 " << infix[i].first << "\n";
      ++i;
    }
  }

  return parseExponent (infix, i);
}

////////////////////////////////////////////////////////////////////////////////
// Exponent    --> Primitive ["^" Primitive]
bool Eval::parseExponent (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#                   parseExponent\n";

  if (i < infix.size () &&
      parsePrimitive (infix, i))
  {
    while (infix[i].first == "^" &&
           infix[i].second == Lexer::typeOperator)
    {
      if (_debug)
        std::cout << "#                   " << infix[i].first << "\n";

      ++i;
      if (! parsePrimitive (infix, i))
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Primitive   --> "(" Logical ")" | Variant
bool Eval::parsePrimitive (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (_debug)
    std::cout << "#                     parseVariant\n";

  if (i < infix.size ())
  {
    if (infix[i].first == "(")
    {
      if (_debug)
        std::cout << "#                     " << infix[i].first << "\n";

      ++i;
      if (parseLogical (infix, i))
      {
        if (i < infix.size () &&
            infix[i].first == ")")
        {
          if (_debug)
            std::cout << "#                     " << infix[i].first << "\n";

          ++i;
          return true;
        }
      }
    }
    else
    {
      if (infix[i].second != Lexer::typeOperator)
      {
        if (_debug)
          std::cout << "#                     " << infix[i].first << "\n";

        ++i;
        return true;
      }
    }
  }

  return false;
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
void Eval::infixToPostfix (
  std::vector <std::pair <std::string, Lexer::Type> >& infix) const
{
  // Short circuit.
  if (infix.size () == 1)
    return;

  // Result.
  std::vector <std::pair <std::string, Lexer::Type> > postfix;

  // Shunting yard.
  std::vector <std::pair <std::string, Lexer::Type> > op_stack;

  // Operator characteristics.
  char type;
  int precedence;
  char associativity;

  std::vector <std::pair <std::string, Lexer::Type> >::iterator token;
  for (token = infix.begin (); token != infix.end (); ++token)
  {
    if (token->second == Lexer::typeOperator &&
        token->first == "(")
    {
      op_stack.push_back (*token);
    }
    else if (token->second == Lexer::typeOperator &&
             token->first == ")")
    {
      while (op_stack.size () &&
             op_stack.back ().first != "(")
      {
        postfix.push_back (op_stack.back ());
        op_stack.pop_back ();
      }

      if (op_stack.size ())
        op_stack.pop_back ();
      else
        throw std::string ("Mismatched parentheses in expression");
    }
    else if (token->second == Lexer::typeOperator &&
             identifyOperator (token->first, type, precedence, associativity))
    {
      char type2;
      int precedence2;
      char associativity2;
      while (op_stack.size () > 0 &&
             identifyOperator (op_stack.back ().first, type2, precedence2, associativity2) &&
             ((associativity == 'l' && precedence <= precedence2) ||
              (associativity == 'r' && precedence <  precedence2)))
      {
        postfix.push_back (op_stack.back ());
        op_stack.pop_back ();
      }

      op_stack.push_back (*token);
    }
    else
    {
      postfix.push_back (*token);
    }
  }

  while (op_stack.size ())
  {
    if (op_stack.back ().first == "(" ||
        op_stack.back ().first == ")")
      throw std::string ("Mismatched parentheses in expression");

    postfix.push_back (op_stack.back ());
    op_stack.pop_back ();
  }

  infix = postfix;
}

////////////////////////////////////////////////////////////////////////////////
bool Eval::identifyOperator (
  const std::string& op,
  char& type,
  int& precedence,
  char& associativity) const
{
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
  {
    if (operators[i].op == op)
    {
      type          = operators[i].type;
      precedence    = operators[i].precedence;
      associativity = operators[i].associativity;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

