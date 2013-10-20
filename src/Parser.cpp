////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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
#include <Parser.h>
#include <text.h>
#include <main.h>

////////////////////////////////////////////////////////////////////////////////
Parser::Parser ()
: _primary ("")
, _verbose (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::~Parser ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Parser::grammar (const std::string& file)
{
  // Strip comments.
  std::vector <std::string> lines;
  split (lines, file, '\n');

  std::string stripped = "";
  std::string::size_type comment;
  std::vector <std::string>::iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    comment = it->find ("#");

    if (comment != std::string::npos)
      stripped += it->substr (0, comment);
    else
      stripped += *it;

    stripped += "\n";
  }

  // Now parse the grammar.
  Nibbler n (stripped);
  std::string rule;
  Production prod;
  while (bnfNibbleRule (n, rule, prod))
  {
    if (_primary == "")
      _primary = rule;

    _rules[rule] = prod;
  }

  // Now the hard part.
  checkConsistency ();
}

////////////////////////////////////////////////////////////////////////////////
bool Parser::bnfNibbleRule (Nibbler& n, std::string& rule, Production& prod)
{
  prod.clear ();
  n.skipWS ();
  if (n.getUntilOneOf (": ", rule))
  {
    std::string att;
    while (n.skip (':') &&
           n.getUntilOneOf (": ", att))
    {
      prod.tag (att);
    }

    // Definition.
    n.skipWS ();
    if (n.getLiteral ("::="))
    {
      // Alternates.
      Alternate alt;
      while (bnfNibbleAlternate (n, alt))
      {
        prod.push_back (alt);
        alt.clear ();
      }

      if (alt.size ())
        prod.push_back (alt);

      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Parser::bnfNibbleAlternate (Nibbler& n, Alternate& alt)
{
  n.skipWS ();

  Token tok;
  while (bnfNibbleToken (n, tok))
    alt.push_back (tok);

       if (n.skip ('|')) {return true;}
  else if (n.skip (';')) {return false;}
  else
    throw std::string ("Expected | or ;");
}

////////////////////////////////////////////////////////////////////////////////
bool Parser::bnfNibbleToken (Nibbler& n, Token& tok)
{
  tok.clear ();
  n.skipWS ();

  if (n.next () == '|') return false;              // Alternate
  if (n.next () == ';') return false;              // Terminator

  if (n.getQuoted ('/', tok.value, true)        || // Regex
      n.getQuoted ('"', tok.value, true)        || // Literal
      n.getUntilOneOf ("\n\t =?+*", tok.value))    // Name
  {
         if (n.skip ('=')) tok.quantifier = '=';   // 1
    else if (n.skip ('?')) tok.quantifier = '?';   // 0,1
    else if (n.skip ('+')) tok.quantifier = '+';   // 1->
    else if (n.skip ('*')) tok.quantifier = '*';   // 0->

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Check consistency of the syntax.  This is where all static analysis occurs.
void Parser::checkConsistency ()
{
  std::vector <std::string> allRules;
  std::vector <std::string> allToken;
  std::vector <std::string> allLeftRecursive;

  std::map <std::string, Production>::iterator r;
  for (r = _rules.begin (); r != _rules.end (); ++r)
  {
    allRules.push_back (r->first);

    std::vector <Alternate>::iterator a;
    for (a = r->second.begin (); a != r->second.end (); ++a)
    {
      std::vector <Token>::iterator i;
      for (i = a->begin (); i != a->end (); ++i)
      {
        if (i->value[0] != '"' &&
            i->value[0] != '/')
          allToken.push_back (i->value);

        if (i == a->begin () && r->first == i->value)
          allLeftRecursive.push_back (i->value);
      }
    }
  }

  std::vector <std::string> notUsed;
  std::vector <std::string> notDefined;
  listDiff (allRules, allToken, notUsed, notDefined);

  // Undefined value - these are definitions that appear in token, but are
  // not in _rules.
  for (unsigned int i = 0; i < notDefined.size (); ++i)
    if (notDefined[i][0] != '@')
      throw std::string ("definition '") + notDefined[i] + "' referenced, but not defined.";

  // Circular definitions - these are names in _rules that also appear as
  // token 0 in any of the alternates for that definition.
  for (unsigned int i = 0; i < allLeftRecursive.size (); ++i)
    throw std::string ("definition '") + allLeftRecursive[i] + "' is left recursive.";

  for (unsigned int i = 0; i < allRules.size (); ++i)
    if (allRules[i][0] == '"')
      throw std::string ("definition '") + allRules[i] + "' must not be a literal.";

  // Unused definitions - these are names in _rules that are never
  // referenced as token.
  for (unsigned int i = 0; i < notUsed.size (); ++i)
    if (notUsed[i] != _primary)
      throw std::string ("definition '") + notUsed[i] + "' defined, but not referenced.";
}

////////////////////////////////////////////////////////////////////////////////
void Parser::verbose ()
{
  _verbose = true;
}

////////////////////////////////////////////////////////////////////////////////
// Display the entire parsed tree.  Highlight the primary definition.
void Parser::dump () const
{
  std::map <std::string, Production>::const_iterator def;
  for (def = _rules.begin (); def != _rules.end (); ++def)
  {
    if (def->first == _primary)
      std::cout << "\033[1m" << def->first << "\033[0m";
    else
      std::cout << def->first;

    std::cout << " ::=" << std::endl;

    std::vector <Alternate>::const_iterator alt;
    for (alt = def->second.begin (); alt != def->second.end (); ++alt)
    {
      if (alt != def->second.begin ())
        std::cout << "  | ";
      else
        std::cout << "    ";

      std::vector <Token>::const_iterator tok;
      for (tok = alt->begin (); tok != alt->end (); ++tok)
      {
        std::cout << tok->value;

        if (tok->quantifier != '=')
          std::cout << tok->quantifier;

        std::cout << " ";
      }

      std::cout << std::endl;
    }

    std::cout << "  ;" << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
Parser::Token::Token ()
: value ("")
, quantifier ('=')
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::Token::Token (const Parser::Token& other)
: value (other.value)
, quantifier (other.quantifier)
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::Token& Parser::Token::operator= (const Parser::Token& other)
{
  if (this != &other)
  {
    value      = other.value;
    quantifier = other.quantifier;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void Parser::Token::clear ()
{
  value = "";
  quantifier = '=';
}

////////////////////////////////////////////////////////////////////////////////
std::string Parser::Token::dump ()
{
  return value + quantifier;
}

////////////////////////////////////////////////////////////////////////////////
Parser::Alternate::Alternate ()
: std::vector <Parser::Token> ()
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::Alternate::Alternate (const Parser::Alternate& other)
: std::vector <Parser::Token> (other)
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::Alternate& Parser::Alternate::operator= (const Parser::Alternate& other)
{
  if (this != &other)
    std::vector <Parser::Token>::operator= (other);

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
std::string Parser::Alternate::dump ()
{
  std::string result;
  std::vector <Parser::Token>::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    result += i->dump () + ' ';
  return result;
}

////////////////////////////////////////////////////////////////////////////////
Parser::Production::Production ()
: std::vector <Parser::Alternate> ()
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::Production::Production (const Parser::Production& other)
: std::vector <Parser::Alternate> (other)
, mTags (other.mTags)
{
}

////////////////////////////////////////////////////////////////////////////////
Parser::Production& Parser::Production::operator= (const Parser::Production& other)
{
  if (this != &other)
  {
    std::vector <Parser::Alternate>::operator= (other);
    mTags = other.mTags;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void Parser::Production::tag (const std::string& t)
{
  mTags.push_back (t);
}

////////////////////////////////////////////////////////////////////////////////
bool Parser::Production::hasTag (const std::string& t) const
{
  return std::find (mTags.begin (), mTags.end (), t) != mTags.end ()
    ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void Parser::Production::clear ()
{
  std::vector <Parser::Alternate>::clear ();
  mTags.clear ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Parser::Production::dump ()
{
  std::string result;
  std::vector <Parser::Alternate>::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    if (i != this->begin ())
      result += " | ";
    result += i->dump ();
  }

  result += "\n";
  return result;
}

////////////////////////////////////////////////////////////////////////////////
