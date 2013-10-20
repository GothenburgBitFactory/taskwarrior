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
#include <LRParser.h>
#include <RX.h>
#include <text.h>
#include <main.h>

////////////////////////////////////////////////////////////////////////////////
LRParser::LRParser ()
{
}

////////////////////////////////////////////////////////////////////////////////
// This is called only from external code.
Tree* LRParser::parse (const std::string& tokens)
{
  Tree* tree = new Tree ("root");
  if (! tree)
    throw std::string ("Failed to allocate memory for parse tree.");

  unsigned int cursor = 0;
  if (matchRule (_primary, '=', cursor, tokens, tree))
  {
    if (_verbose)
      std::cout << "syntax pass" << std::endl;
  }
  else
  {
    if (_verbose)
      std::cout << "syntax fail" << std::endl;

    delete tree;
    tree = NULL;
  }

  return tree;
}

////////////////////////////////////////////////////////////////////////////////
void LRParser::addEntity (const std::string& name, const std::string& value)
{
  _entities.insert (std::pair <std::string, std::string> (name, value));
}

////////////////////////////////////////////////////////////////////////////////
// Wraps calls to matchRule, while properly handling the quantifier.
bool LRParser::matchRuleQuant (
  const std::string& rule,
  char quantifier,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  // Must match exactly once, so run once and return the result.
  if (quantifier == '=')
  {
    return matchRule (rule, quantifier, cursor, tokens, tree);
  }

  // May match zero or one time.  If it matches, the cursor will be advanced.
  // If it fails, the cursor will not be advanced, but this is still considered
  // successful.  Return true either way, but backtrack the cursor on failure.

  // TODO Make greedy.
  else if (quantifier == '?')
  {
    unsigned int original_cursor = cursor;
    if (! matchRule (rule, quantifier, cursor, tokens, tree))
      cursor = original_cursor;
    return true;
  }

  // May match 1 or more times.  If it matches on the first attempt, continue
  // to greedily match until it fails.  If it fails on the first attempt, then
  // the rule fails.

  // TODO Make greedy.
  else if (quantifier == '+')
  {
    if (! matchRule (rule, quantifier, cursor, tokens, tree))
      return false;

    while (matchRule (rule, quantifier, cursor, tokens, tree))
      ;
    return true;
  }

  // May match zero or more times.  Keep calling while there are matches, and
  // return true always.  Backtrack the cursor on failure.

  // TODO Make greedy.
  else if (quantifier == '*')
  {
    bool result;
    do
    {
      unsigned int original_cursor = cursor;
      result = matchRule (rule, quantifier, cursor, tokens, tree);
      if (! result)
        cursor = original_cursor;
    }
    while (result);
    return true;
  }

  throw std::string ("LRParser::matchRuleQuant - this should never happen.");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Returns true, with cursor incremented, if any of the alternates match.
bool LRParser::matchRule (
  const std::string& rule,
  char quantifier,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  if (cursor >= tokens.length ()) return false;
  unsigned int original_cursor = cursor; // Preserve

  for (unsigned int alt = 0;
       alt < _rules[rule].size () && cursor < tokens.length ();
       ++alt)
  {
    Tree* b = new Tree (rule);
    if (! b)
      throw std::string ("Failed to allocate memory for parse tree.");

    if (matchAlternate (rule, quantifier, alt, _rules[rule][alt], cursor, tokens, b))
    {
      if (_verbose)
        std::cout << "\033[32m"
                  << "matchRule                "
                  << rule
                  << quantifier
                  << "/a"
                  << alt
                  << " tokens["
                  << cursor - 1
                  << "]="
                  << visible (tokens[cursor - 1])
                  << " SUCCEED"
                  << " "
                  << tree
                  << "->"
                  << b
                  << "\033[0m"
                  << std::endl;

      tree->addBranch (b);
      return true;
    }

    delete b;
  }

  cursor = original_cursor; // Restore

  if (_verbose)
    std::cout << "\033[31m"
              << "matchRule                "
              << rule
              << quantifier
              << " FAIL"
              << "\033[0m"
              << std::endl;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Returns true, with cursor incremented, if all of the token match.
bool LRParser::matchAlternate (
  const std::string& rule,
  char quantifier,
  unsigned int alt,
  const Alternate& alternate,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  if (cursor >= tokens.length ()) return false;

  unsigned int original_cursor = cursor; // Preserve

  for (unsigned int token = 0;
       token < alternate.size () && cursor < tokens.length ();
       ++token)
  {
    if (! matchToken (rule, quantifier, alt, token, alternate[token], cursor, tokens, tree))
    {
      cursor = original_cursor; // Restore
      if (_verbose)
        std::cout << "\033[31m"
                  << "matchAlternate           "
                  << rule
                  << quantifier
                  << "/a"
                  << alt
                  << "/t"
                  << token
                  << " tokens["
                  << cursor
                  << "]="
                  << visible (tokens[cursor])
                  << " FAIL"
                  << "\033[0m"
                  << std::endl;

      return false;
    }

    if (_verbose)
      std::cout << "\033[32m"
                << "matchAlternate           "
                << rule
                << quantifier
                << "/a"
                << alt
                << "/t"
                << token
                << " SUCCEED"
                << "\033[0m"
                << std::endl;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Returns true, if the token, an optional quantifier, and all optional
// modifiers match.
bool LRParser::matchToken (
  const std::string& rule,
  char quantifier,
  unsigned int alt,
  unsigned int tok,
  const Token& token,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  if (cursor >= tokens.length ()) return false;

  unsigned int original_cursor = cursor; // Preserve

  if (tokenMatchRule           (rule, quantifier, alt, tok, token, cursor, tokens, tree) ||
      tokenMatchSpecialLiteral (token, cursor, tokens, tree)                             ||
      tokenMatchLiteral        (token, cursor, tokens, tree)                             ||
      tokenMatchRegex          (token, cursor, tokens, tree))
  {
    if (_verbose)
      std::cout << "\033[32m"
                << "matchToken               "
                << rule
                << quantifier
                << "/a"
                << alt
                << "/t"
                << tok
                << " tokens["
                << cursor
                << "]="
                << visible (tokens[cursor])
                << " token="
                << token.value
                << " SUCCEED"
                << "\033[0m"
                << std::endl;

    return true;
  }

  cursor = original_cursor; // Restore

  if (_verbose)
    std::cout << "\033[31m"
              << "matchToken               "
              << rule
              << quantifier
              << "/a"
              << alt
              << "/t"
              << tok
              << " tokens["
              << cursor
              << "]="
              << visible (tokens[cursor])
              << " token="
              << token.value
              << " FAIL"
              << "\033[0m"
              << std::endl;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool LRParser::tokenMatchSpecialLiteral (
  const Token& token,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  if (cursor >= tokens.length ()) return false;

  if ((tokens[cursor] == '\t' && token.value == "\"\\t\"") ||
      (tokens[cursor] == '\n' && token.value == "\"\\n\"") ||
      (tokens[cursor] == '\r' && token.value == "\"\\r\"") ||
      (tokens[cursor] == '\f' && token.value == "\"\\f\"") ||
      (tokens[cursor] == '\v' && token.value == "\"\\v\"") ||
      (tokens[cursor] == '"'  && token.value == "\"\\\"\""))
  {
    tree->tag ("literal");
    tree->tag ("special");
    tree->attribute ("token", tokens[cursor]);

    if (_verbose)
      std::cout << "tokenMatchSpecialLiteral "
                << token.value
                << " SUCCEED"
                << std::endl;

    cursor++;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool LRParser::tokenMatchLiteral (
  const Token& token,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  int len = token.value.length () - 2;
  if (cursor > tokens.length () - len) return false;

  std::string tok = token.value.substr (1, len);

  if (token.value[0]              == '"' &&
      token.value[len + 1]        == '"' &&
      tokens.find (tok, cursor) == cursor)
  {
    tree->tag ("literal");
    tree->attribute ("token", tok);
    cursor += len;

    if (_verbose)
      std::cout << "tokenMatchLiteral        "
                << token.value
                << " SUCCEED"
                << std::endl;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool LRParser::tokenMatchRegex (
  const Token& token,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  if (cursor >= tokens.length () - 1) return false;

  // If it looks like a regex.
  if (token.value[0]                         == '/' &&
      token.value[token.value.length () - 1] == '/')
  {
    // If the regex matches at all.
    RX rx ("(" + token.value.substr (1, token.value.length () - 2) + ")", false);
    std::vector <int> start;
    std::vector <int> end;
    if (rx.match (start,
                  end,
                  tokens.substr (cursor, std::string::npos)))
    {
      // If the match is at position 'cursor'.
      if (start[0] == 0)
      {
        tree->tag ("regex");
        tree->attribute ("token", tokens.substr (cursor + start[0], end[0]));
        cursor += end[0];

        if (_verbose)
          std::cout << "tokenMatchRegex          \""
                    << tokens.substr (cursor + start[0], end[0])
                    << "\""
                    << " SUCCEED"
                    << std::endl;

        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool LRParser::tokenMatchRule (
  const std::string& rule,
  char quantifier,
  unsigned int alt,
  unsigned int it,
  const Token& token,
  unsigned int& cursor,
  const std::string& tokens,
  Tree* tree)
{
  if (cursor >= tokens.length ()) return false;

  // If this is a definition, recurse.
  if (_rules.find (token.value) != _rules.end ())
  {
    if (_verbose)
      std::cout << "tokenMatchRule           "
                << rule
                << quantifier
                << "/a"
                << alt
                << "/t"
                << it
                << " tokens["
                << cursor
                << "]="
                << visible (tokens[cursor])
                << " token="
                << token.value
                << " RECURSING matchRuleQuant"
                << std::endl;

    return matchRuleQuant (token.value, token.quantifier, cursor, tokens, tree);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
