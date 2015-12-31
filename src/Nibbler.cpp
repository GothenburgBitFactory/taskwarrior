////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <Nibbler.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <Lexer.h>
#include <util.h>

static const char*        _uuid_pattern    = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
static const unsigned int _uuid_min_length = 8;

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler ()
: _length (0)
, _cursor (0)
, _saved (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const std::string& input)
: _input (std::make_shared <std::string> (input))
, _length (input.length ())
, _cursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const Nibbler& other)
: _input (other._input)
, _length (other._length)
, _cursor (other._cursor)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler& Nibbler::operator= (const Nibbler& other)
{
  if (this != &other)
  {
    _input  = other._input;
    _length = other._length;
    _cursor = other._cursor;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::~Nibbler ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Extract up until the next c (but not including) or EOS.
bool Nibbler::getUntil (char c, std::string& result)
{
  if (_cursor < _length)
  {
    auto i = _input->find (c, _cursor);
    if (i != std::string::npos)
    {
      result = _input->substr (_cursor, i - _cursor);
      _cursor = i;
    }
    else
    {
      result = _input->substr (_cursor);
      _cursor = _length;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntil (const std::string& terminator, std::string& result)
{
  if (_cursor < _length)
  {
    auto i = _input->find (terminator, _cursor);
    if (i != std::string::npos)
    {
      result = _input->substr (_cursor, i - _cursor);
      _cursor = i;
    }
    else
    {
      result = _input->substr (_cursor);
      _cursor = _length;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilOneOf (const std::string& chars, std::string& result)
{
  if (_cursor < _length)
  {
    auto i = _input->find_first_of (chars, _cursor);
    if (i != std::string::npos)
    {
      result = _input->substr (_cursor, i - _cursor);
      _cursor = i;
    }
    else
    {
      result = _input->substr (_cursor);
      _cursor = _length;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilWS (std::string& result)
{
  return this->getUntilOneOf (" \t\r\n\f", result);
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOS (std::string& result)
{
  if (_cursor < _length)
  {
    result = _input->substr (_cursor);
    _cursor = _length;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getN (const int quantity, std::string& result)
{
  if (_cursor + quantity <= _length)
  {
    result = _input->substr (_cursor, quantity);
    _cursor += quantity;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Gets quote content:      "foobar" -> foobar      (for c = '"')
// Handles escaped quotes:  "foo\"bar" -> foo\"bar  (for c = '"')
// Returns false if first character is not c, or if there is no closing c
bool Nibbler::getQuoted (char c, std::string& result)
{
  result = "";

  if (_cursor >= _length ||
      (*_input)[_cursor] != c)
  {
    return false;
  }

  std::string::size_type start = _cursor + 1;  // Skip first quote char
  std::string::size_type i = start;

  while (i < _length)
  {
    i = (*_input).find (c, i);

    if (i == std::string::npos)
      return false;  // Unclosed quote

    if (i == start)
    {
      // Empty quote
      _cursor += 2;  // Skip both quote chars
      return true;
    }

    if ((*_input)[i-1] == '\\')
    {
      // Check for escaped backslashes.  Backtracking like this is not very
      // efficient, but is only done in extreme corner cases.

      auto j = i-2;  // Start one character further left
      bool is_escaped_quote = true;
      while (j >= start && (*_input)[j] == '\\')
      {
        // Toggle flag for each further backslash encountered.
        is_escaped_quote = is_escaped_quote ? false : true;
        --j;
      }

      if (is_escaped_quote)
      {
        // Keep searching
        ++i;
        continue;
      }
    }

    // None of the above applied, we must have found the closing quote char.
    result.assign ((*_input), start, i - start);
    _cursor = i + 1;  // Skip closing quote char
    return true;
  }

  // This should never be reached.  We could throw here instead.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDigit (int& result)
{
  if (_cursor < _length &&
      Lexer::isDigit ((*_input)[_cursor]))
  {
    result = (*_input)[_cursor++] - '0';
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDigit4 (int& result)
{
  auto i = _cursor;
  if (i < _length &&
      _length - i >= 4)
  {
    if (Lexer::isDigit ((*_input)[i + 0]) &&
        Lexer::isDigit ((*_input)[i + 1]) &&
        Lexer::isDigit ((*_input)[i + 2]) &&
        Lexer::isDigit ((*_input)[i + 3]))
    {
      result = strtoimax (_input->substr (_cursor, 4).c_str (), NULL, 10);
      _cursor += 4;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDigit3 (int& result)
{
  auto i = _cursor;
  if (i < _length &&
      _length - i >= 3)
  {
    if (Lexer::isDigit ((*_input)[i + 0]) &&
        Lexer::isDigit ((*_input)[i + 1]) &&
        Lexer::isDigit ((*_input)[i + 2]))
    {
      result = strtoimax (_input->substr (_cursor, 3).c_str (), NULL, 10);
      _cursor += 3;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getDigit2 (int& result)
{
  auto i = _cursor;
  if (i < _length &&
      _length - i >= 2)
  {
    if (Lexer::isDigit ((*_input)[i + 0]) &&
        Lexer::isDigit ((*_input)[i + 1]))
    {
      result = strtoimax (_input->substr (_cursor, 2).c_str (), NULL, 10);
      _cursor += 2;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getInt (int& result)
{
  auto i = _cursor;

  if (i < _length)
  {
    if ((*_input)[i] == '-')
      ++i;
    else if ((*_input)[i] == '+')
      ++i;
  }

  // TODO Potential for use of find_first_not_of
  while (i < _length && Lexer::isDigit ((*_input)[i]))
    ++i;

  if (i > _cursor)
  {
    result = strtoimax (_input->substr (_cursor, i - _cursor).c_str (), NULL, 10);
    _cursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUnsignedInt (int& result)
{
  auto i = _cursor;
  // TODO Potential for use of find_first_not_of
  while (i < _length && Lexer::isDigit ((*_input)[i]))
    ++i;

  if (i > _cursor)
  {
    result = strtoimax (_input->substr (_cursor, i - _cursor).c_str (), NULL, 10);
    _cursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// number:
//   int frac? exp?
// 
// int:
//   (-|+)? digit+
// 
// frac:
//   . digit+
// 
// exp:
//   e digit+
// 
// e:
//   e|E (+|-)?
// 
bool Nibbler::getNumber (std::string& result)
{
  auto i = _cursor;

  // [+-]?
  if (i < _length && ((*_input)[i] == '-' || (*_input)[i] == '+'))
    ++i;

  // digit+
  if (i < _length && Lexer::isDigit ((*_input)[i]))
  {
    ++i;

    while (i < _length && Lexer::isDigit ((*_input)[i]))
      ++i;

    // ( . digit+ )?
    if (i < _length && (*_input)[i] == '.')
    {
      ++i;

      while (i < _length && Lexer::isDigit ((*_input)[i]))
        ++i;
    }

    // ( [eE] [+-]? digit+ )?
    if (i < _length && ((*_input)[i] == 'e' || (*_input)[i] == 'E'))
    {
      ++i;

      if (i < _length && ((*_input)[i] == '+' || (*_input)[i] == '-'))
        ++i;

      if (i < _length && Lexer::isDigit ((*_input)[i]))
      {
        ++i;

        while (i < _length && Lexer::isDigit ((*_input)[i]))
          ++i;

        result = _input->substr (_cursor, i - _cursor);
        _cursor = i;
        return true;
      }

      return false;
    }

    result = _input->substr (_cursor, i - _cursor);
    _cursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getNumber (double &result)
{
  std::string s;
  if (getNumber (s))
  {
    result = strtof (s.c_str (), NULL);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getLiteral (const std::string& literal)
{
  if (_cursor < _length &&
      _input->find (literal, _cursor) == _cursor)
  {
    _cursor += literal.length ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getPartialUUID (std::string& result)
{
  std::string::size_type i;
  for (i = 0; i < 36 && i < (_length - _cursor); i++)
  {
    if (_uuid_pattern[i] == 'x' && !isxdigit ((*_input)[_cursor + i]))
      break;

    else if (_uuid_pattern[i] == '-' && (*_input)[_cursor + i] != '-')
      break;
  }

  // If the partial match found is long enough, consider it a match.
  if (i >= _uuid_min_length)
  {
    // Fail if there is another hex digit.
    if (_cursor + i < _length &&
        isxdigit ((*_input)[_cursor + i]))
      return false;

    result = _input->substr (_cursor, i);
    _cursor += i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Assumes that the options are sorted by decreasing length, so that if the
// options contain 'fourteen' and 'four', the stream is first matched against
// the longer entry.
bool Nibbler::getOneOf (
  const std::vector <std::string>& options,
  std::string& found)
{
  for (auto& option : options)
  {
    if (getLiteral (option))
    {
      found = option;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipN (const int quantity /* = 1 */)
{
  if (_cursor < _length &&
      _cursor <= _length - quantity)
  {
    _cursor += quantity;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skip (char c)
{
  if (_cursor < _length &&
      (*_input)[_cursor] == c)
  {
    ++_cursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipWS ()
{
  return this->skipAllOneOf (" \t\n\r\f");
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipAllOneOf (const std::string& chars)
{
  if (_cursor < _length)
  {
    auto i = _input->find_first_not_of (chars, _cursor);
    if (i == _cursor)
      return false;

    if (i == std::string::npos)
      _cursor = _length;  // Yes, off the end.
    else
      _cursor = i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
char Nibbler::next ()
{
  if (_cursor < _length)
    return (*_input)[_cursor];

  return '\0';
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type Nibbler::cursor ()
{
  return _cursor;
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
std::string Nibbler::next (const int quantity)
{
  if (           _cursor  <  _length &&
      (unsigned) quantity <= _length &&
                 _cursor  <= _length - quantity)
    return _input->substr (_cursor, quantity);

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type Nibbler::save ()
{
  return _saved = _cursor;
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type Nibbler::restore ()
{
  return _cursor = _saved;
}

////////////////////////////////////////////////////////////////////////////////
const std::string& Nibbler::str () const
{
  return *_input;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::depleted ()
{
  if (_cursor >= _length)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Nibbler::dump ()
{
  return std::string ("Nibbler ‹")
         + _input->substr (_cursor)
         + "›";
}

////////////////////////////////////////////////////////////////////////////////
