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

#include <cmake.h>
#include <map>
#include <time.h>
#include <Context.h>
#include <Task.h>
#include <Color.h>
#include <Eval.h>
#include <text.h>
#include <i18n.h>

extern Context context;
extern Task& contextTask;

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
  {  "!=",        9,         'b',  'l' },    // Inequal (partial)
  {  "!==",       9,         'b',  'l' },    // Inequal (exact)

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
  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Infix        " + dump (tokens));

  infixParse (tokens);
  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Infix parsed " + dump (tokens));

  // Convert infix --> postfix.
  infixToPostfix (tokens);
  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Postfix      " + dump (tokens));

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
    tokens.push_back (std::pair <std::string, Lexer::Type> (token, type));

  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Postfix      " + dump (tokens));

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
    if (_debug)
      context.debug ("evaluateInfixExpression token '" + token + "' " + Lexer::type_name (type));
    _compiled.push_back (std::pair <std::string, Lexer::Type> (token, type));
  }

  // Parse for syntax checking and operator replacement.
  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Infix        " + dump (_compiled));
  infixParse (_compiled);
  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Infix parsed " + dump (_compiled));

  // Convert infix --> postfix.
  infixToPostfix (_compiled);
  if (_debug)
    context.debug ("[1;37;42mFILTER[0m Postfix      " + dump (_compiled));
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
void Eval::debug (bool value)
{
  _debug = value;
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
    throw std::string (STRING_EVAL_NO_EXPRESSION);

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
      if (_debug)
      {
        context.debug (format ("[{1}] eval pop '{2}'",      values.size () + 1, (std::string) right));
        context.debug (format ("[{1}] eval operator '{2}'", values.size (),     token->first));
        context.debug (format ("[{1}] eval result push '{2}'", values.size (),  (bool) !right));
      }
      values.push_back (! right);
    }
    else if (token->second == Lexer::typeOperator &&
             token->first == "_neg_")
    {
      Variant right = values.back ();
      values.pop_back ();
      if (_debug)
      {
        context.debug (format ("[{1}] eval pop '{2}'",      values.size () + 1, (std::string) right));
        context.debug (format ("[{1}] eval operator '{2}'", values.size (),     token->first));
        context.debug (format ("[{1}] eval result push '{2}'", values.size (),  (bool) !right));
      }
      values.push_back (Variant (0) - right);
    }
    else if (token->second == Lexer::typeOperator &&
             token->first == "_pos_")
    {
      // NOP?
      if (_debug)
      {
        context.debug (format ("[{1}] eval operator '{2}'", values.size (),     token->first));
      }
    }

    // Binary operators.
    else if (token->second == Lexer::typeOperator)
    {
      Variant right = values.back ();
      values.pop_back ();

      Variant left = values.back ();
      values.pop_back ();

      if (_debug)
      {
        context.debug (format ("[{1}] eval pop '{2}'",      values.size () + 2, (std::string) right));
        context.debug (format ("[{1}] eval pop '{2}'",      values.size () + 1, (std::string) left));
        context.debug (format ("[{1}] eval operator '{2}'", values.size (),     token->first));
      }

      // Ordering these by anticipation frequency of use is a good idea.
           if (token->first == "and")      left = left && right;
      else if (token->first == "or")       left = left || right;
      else if (token->first == "&&")       left = left && right;
      else if (token->first == "||")       left = left || right;
      else if (token->first == "<")        left = left < right;
      else if (token->first == "<=")       left = left <= right;
      else if (token->first == ">")        left = left > right;
      else if (token->first == ">=")       left = left >= right;
      else if (token->first == "==")       left = left.operator== (right);
      else if (token->first == "!==")      left = left.operator!= (right);
      else if (token->first == "=")        left = left.operator_partial (right);
      else if (token->first == "!=")       left = left.operator_nopartial (right);
      else if (token->first == "+")        left += right;
      else if (token->first == "-")        left -= right;
      else if (token->first == "*")        left *= right;
      else if (token->first == "/")        left /= right;
      else if (token->first == "^")        left ^= right;
      else if (token->first == "%")        left %= right;
      else if (token->first == "xor")      left = left.operator_xor (right);
      else if (token->first == "~")        left = left.operator_match (right, contextTask);
      else if (token->first == "!~")       left = left.operator_nomatch (right, contextTask);
      else if (token->first == "_hastag_") left = left.operator_hastag (right, contextTask);
      else if (token->first == "_notag_")  left = left.operator_notag (right, contextTask);
      else
        throw format (STRING_EVAL_UNSUPPORTED, token->first);

      if (_debug)
        context.debug (format ("[{1}] eval result push '{2}'", values.size (), (std::string) left));
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
        throw std::string (STRING_EVAL_OP_EXPECTED);
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
                context.debug (format ("[{1}] eval source '{2}' --> '{3}'", values.size (), token->first, (std::string) v));
              found = true;
              break;
            }
          }

          // An identifier that fails lookup is a string.
          if (!found)
          {
            v.cast (Variant::type_string);
            if (_debug)
              context.debug (format ("[{1}] eval source failed '{2}'", values.size (), token->first));
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

      if (_debug)
        context.debug (format ("[{1}] eval push '{2}'", values.size (), (std::string) v));
      values.push_back (v);
    }
  }

  // If there is more than one variant left on the stack, then the original
  // expression was not valid.
  if (values.size () != 1)
    throw std::string (STRING_EVAL_NO_EVAL);

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
  int i = 0;
  parseLogical (infix, i);
}

////////////////////////////////////////////////////////////////////////////////
// Logical     --> Regex {( "and" | "or" | "xor" ) Regex}
bool Eval::parseLogical (
  std::vector <std::pair <std::string, Lexer::Type> >& infix,
  int &i) const
{
  if (i < infix.size () &&
      parseRegex (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "and"  ||
            infix[i].first == "or"   ||
            infix[i].first == "xor") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size () &&
      parseEquality (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "~" ||
            infix[i].first == "!~") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size () &&
      parseComparative (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "==" ||
            infix[i].first == "="  ||
            infix[i].first == "!=") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size () &&
      parseArithmetic (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "<=" ||
            infix[i].first == "<"  ||
            infix[i].first == ">=" ||
            infix[i].first == ">") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size () &&
      parseGeometric (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "+" ||
            infix[i].first == "-") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size () &&
      parseTag (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "*" ||
            infix[i].first == "/" ||
            infix[i].first == "%") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size () &&
      parseUnary (infix, i))
  {
    while (i < infix.size () &&
           (infix[i].first == "_hastag_" ||
            infix[i].first == "_notag_") &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size ())
  {
    if (infix[i].first == "-")
    {
      infix[i].first = "_neg_";
      ++i;
    }
    else if (infix[i].first == "+")
    {
      infix[i].first = "_pos_";
      ++i;
    }
    else if (infix[i].first == "!")
    {
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
  if (i < infix.size () &&
      parsePrimitive (infix, i))
  {
    while (i < infix.size () &&
           infix[i].first == "^" &&
           infix[i].second == Lexer::typeOperator)
    {
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
  if (i < infix.size ())
  {
    if (infix[i].first == "(")
    {
      ++i;
      if (i < infix.size () &&
          parseLogical (infix, i))
      {
        if (i < infix.size () &&
            infix[i].first == ")")
        {
          ++i;
          return true;
        }
      }
    }
    else
    {
      bool found = false;
      std::vector <bool (*)(const std::string&, Variant&)>::const_iterator source;
      for (source = _sources.begin (); source != _sources.end (); ++source)
      {
        Variant v;
        if ((*source) (infix[i].first, v))
        {
          found = true;
          break;
        }
      }

      if (found)
      {
        ++i;
        return true;
      }
      else if (infix[i].second != Lexer::typeOperator)
      {
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
      throw std::string (STRING_PAREN_MISMATCH);

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
std::string Eval::dump (
  std::vector <std::pair <std::string, Lexer::Type> >& tokens) const
{
  // Set up a color mapping.
  std::map <Lexer::Type, Color> color_map;
  color_map[Lexer::typeNone]       = Color ("rgb000 on gray6");
  color_map[Lexer::typeOperator]   = Color ("gray14 on gray6");
  color_map[Lexer::typeNumber]     = Color ("rgb530 on gray6");
  color_map[Lexer::typeHex]        = Color ("rgb303 on gray6");
  color_map[Lexer::typeDecimal]    = Color ("rgb530 on gray6");
  color_map[Lexer::typeString]     = Color ("rgb550 on gray6");
  color_map[Lexer::typeIdentifier] = Color ("rgb035 on gray6");
  color_map[Lexer::typeDate]       = Color ("rgb150 on gray6");
  color_map[Lexer::typeDuration]   = Color ("rgb531 on gray6");

  std::string output;
  std::vector <std::pair <std::string, Lexer::Type> >::const_iterator i;
  for (i = tokens.begin (); i != tokens.end (); ++i)
  {
    if (i != tokens.begin ())
      output += ' ';

    Color c;
    if (color_map[i->second].nontrivial ())
      c = color_map[i->second];
    else
      c = color_map[Lexer::typeNone];

    output += c.colorize (i->first);
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////

