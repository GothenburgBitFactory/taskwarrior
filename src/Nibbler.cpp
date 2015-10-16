////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <Lexer.h>
#include <Nibbler.h>
#include <util.h>
#ifdef NIBBLER_FEATURE_DATE
#include <ISO8601.h>
#endif
#ifdef NIBBLER_FEATURE_REGEX
#include <RX.h>
#endif
#include <Lexer.h>
#include <util.h>
#include <memory>

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
#ifdef NIBBLER_FEATURE_REGEX
bool Nibbler::getUntilRx (const std::string& regex, std::string& result)
{
  if (_cursor < _length)
  {
    RX r (regex, true);
    std::vector <int> start;
    std::vector <int> end;
    if (r.match (start, end, _input->substr (_cursor)))
    {
      result = _input->substr (_cursor, start[0]);
      _cursor += start[0];
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
#endif

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
bool Nibbler::getUntilEOL (std::string& result)
{
  return getUntil ('\n', result);
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
bool Nibbler::getQuoted (
  char c,
  std::string& result,
  bool quote /* = false */)
{
  bool inquote = false;
  bool inescape = false;
  char previous = 0;
  char current = 0;
  result = "";

  if (_cursor >= _length ||
      (*_input)[_cursor] != c)
  {
    return false;
  }

  for (auto i = _cursor; i < _length; ++i)
  {
    current = (*_input)[i];

    if (current == '\\' && !inescape)
    {
      inescape = true;
      previous = current;
      continue;
    }

    if (current == c && !inescape)
    {
      if (quote)
        result += current;

      if (!inquote)
      {
        inquote = true;
      }
      else
      {
        _cursor = i + 1;
        return true;
      }
    }
    else
    {
      if (previous)
      {
        result += previous;
        previous = 0;
      }

      result += current;
      inescape = false;
    }
  }

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
bool Nibbler::getDigit6 (int& result)
{
  auto i = _cursor;
  if (i < _length &&
      _length - i >= 6)
  {
    if (Lexer::isDigit ((*_input)[i + 0]) &&
        Lexer::isDigit ((*_input)[i + 1]) &&
        Lexer::isDigit ((*_input)[i + 2]) &&
        Lexer::isDigit ((*_input)[i + 3]) &&
        Lexer::isDigit ((*_input)[i + 4]) &&
        Lexer::isDigit ((*_input)[i + 5]))
    {
      result = strtoimax (_input->substr (_cursor, 6).c_str (), NULL, 10);
      _cursor += 6;
      return true;
    }
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
  bool isnumber;
  std::string s;

  isnumber = getNumber (s);
  if (isnumber)
  {
    result = strtof (s.c_str (), NULL);
  }
  return isnumber;
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
#ifdef NIBBLER_FEATURE_REGEX
bool Nibbler::getRx (const std::string& regex, std::string& result)
{
  if (_cursor < _length)
  {
    // Regex may be anchored to the beginning and include capturing parentheses,
    // otherwise they are added.
    std::string modified_regex;
    if (regex.substr (0, 2) != "^(")
      modified_regex = "^(" + regex + ")";
    else
      modified_regex = regex;

    RX r (modified_regex, true);
    std::vector <std::string> results;
    if (r.match (results, _input->substr (_cursor)))
    {
      result = results[0];
      _cursor += result.length ();
      return true;
    }
  }

  return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUUID (std::string& result)
{
  auto i = _cursor;

  if (i < _length &&
      _length - i >= 36)
  {
    // 88888888-4444-4444-4444-cccccccccccc
    if (isxdigit ((*_input)[i + 0]) &&
        isxdigit ((*_input)[i + 1]) &&
        isxdigit ((*_input)[i + 2]) &&
        isxdigit ((*_input)[i + 3]) &&
        isxdigit ((*_input)[i + 4]) &&
        isxdigit ((*_input)[i + 5]) &&
        isxdigit ((*_input)[i + 6]) &&
        isxdigit ((*_input)[i + 7]) &&
        (*_input)[i + 8] == '-'     &&
        isxdigit ((*_input)[i + 9]) &&
        isxdigit ((*_input)[i + 10]) &&
        isxdigit ((*_input)[i + 11]) &&
        isxdigit ((*_input)[i + 12]) &&
        (*_input)[i + 13] == '-'     &&
        isxdigit ((*_input)[i + 14]) &&
        isxdigit ((*_input)[i + 15]) &&
        isxdigit ((*_input)[i + 16]) &&
        isxdigit ((*_input)[i + 17]) &&
        (*_input)[i + 18] == '-'     &&
        isxdigit ((*_input)[i + 19]) &&
        isxdigit ((*_input)[i + 20]) &&
        isxdigit ((*_input)[i + 21]) &&
        isxdigit ((*_input)[i + 22]) &&
        (*_input)[i + 23] == '-'     &&
        isxdigit ((*_input)[i + 24]) &&
        isxdigit ((*_input)[i + 25]) &&
        isxdigit ((*_input)[i + 26]) &&
        isxdigit ((*_input)[i + 27]) &&
        isxdigit ((*_input)[i + 28]) &&
        isxdigit ((*_input)[i + 29]) &&
        isxdigit ((*_input)[i + 30]) &&
        isxdigit ((*_input)[i + 31]) &&
        isxdigit ((*_input)[i + 32]) &&
        isxdigit ((*_input)[i + 33]) &&
        isxdigit ((*_input)[i + 34]) &&
        isxdigit ((*_input)[i + 35]))
    {
      result = _input->substr (_cursor, 36);
      _cursor = i + 36;
      return true;
    }
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
// 19980119T070000Z =  YYYYMMDDThhmmssZ
bool Nibbler::getDateISO (time_t& t)
{
  auto i = _cursor;

  if (i < _length &&
      _length - i >= 16)
  {
    if (Lexer::isDigit ((*_input)[i + 0]) &&
        Lexer::isDigit ((*_input)[i + 1]) &&
        Lexer::isDigit ((*_input)[i + 2]) &&
        Lexer::isDigit ((*_input)[i + 3]) &&
        Lexer::isDigit ((*_input)[i + 4]) &&
        Lexer::isDigit ((*_input)[i + 5]) &&
        Lexer::isDigit ((*_input)[i + 6]) &&
        Lexer::isDigit ((*_input)[i + 7]) &&
        (*_input)[i + 8] == 'T' &&
        Lexer::isDigit ((*_input)[i + 9]) &&
        Lexer::isDigit ((*_input)[i + 10]) &&
        Lexer::isDigit ((*_input)[i + 11]) &&
        Lexer::isDigit ((*_input)[i + 12]) &&
        Lexer::isDigit ((*_input)[i + 13]) &&
        Lexer::isDigit ((*_input)[i + 14]) &&
        (*_input)[i + 15] == 'Z')
    {
      _cursor += 16;

      int year   = ((*_input)[i + 0] - '0') * 1000
                 + ((*_input)[i + 1] - '0') *  100
                 + ((*_input)[i + 2] - '0') *   10
                 + ((*_input)[i + 3] - '0');

      int month  = ((*_input)[i + 4] - '0') * 10
                 + ((*_input)[i + 5] - '0');

      int day    = ((*_input)[i + 6] - '0') * 10
                 + ((*_input)[i + 7] - '0');

      int hour   = ((*_input)[i + 9] - '0') * 10
                 + ((*_input)[i + 10] - '0');

      int minute = ((*_input)[i + 11] - '0') * 10
                 + ((*_input)[i + 12] - '0');

      int second = ((*_input)[i + 13] - '0') * 10
                 + ((*_input)[i + 14] - '0');

      // Convert to epoch.
      struct tm tms {};
      tms.tm_isdst  = -1;   // Requests that mktime determine summer time effect.
      tms.tm_mon    = month - 1;
      tms.tm_mday   = day;
      tms.tm_year   = year - 1900;
      tms.tm_hour   = hour;
      tms.tm_min    = minute;
      tms.tm_sec    = second;
#ifdef HAVE_TM_GMTOFF
      tms.tm_gmtoff = 0;
#endif

      t = timegm (&tms);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Parse the longest integer using the next 'limit' characters of 'result'
// following position 'i' (when strict is true, the number of digits must be
// equal to limit).
bool Nibbler::parseDigits(
  std::string::size_type& i,
  int& result,
  unsigned int limit,
  bool strict /* = true */)
{
  // If the result has already been set
  if (result != -1)
    return false;
  for (unsigned int f = limit; f > 0; --f)
  {
    // Check that the nibbler has enough unparsed characters
    if (i + f <= _length)
    {
      // Check that 'f' of them are digits
      unsigned int g;
      for (g = 0; g < f; g++)
        if (! Lexer::isDigit ((*_input)[i + g]))
            break;
      // Parse the integer when it is the case
      if (g == f)
      {
        if (f == 1)
          result = (*_input)[i] - '0';
        else
          result = atoi (_input->substr (i, f).c_str ());
        // Update the global cursor before returning
        i += f;
        return true;
      }
    }
    // Do not try smaller limits if the option is strict on the size
    if (strict)
      break;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef NIBBLER_FEATURE_DATE
bool Nibbler::getDate (const std::string& format, time_t& t)
{
  auto i = _cursor;

  int month  = -1;   // So we can check later.
  int day    = -1;
  int year   = -1;
  int hour   = -1;
  int minute = -1;
  int second = -1;

  // For parsing, unused.
  int wday    = -1;
  int week    = -1;

  for (unsigned int f = 0; f < format.length (); ++f)
  {
    switch (format[f])
    {
    case 'm':
    case 'M':
      if (! parseDigits(i, month, 2, format[f] == 'M'))
        return false;
      break;

    case 'd':
    case 'D':
      if (! parseDigits(i, day, 2, format[f] == 'D'))
        return false;
      break;

    case 'y':
    case 'Y':
      if (! parseDigits(i, year, format[f] == 'y' ? 2 : 4))
        return false;
      if (format[f] == 'y')
          year += 2000;
      break;

    case 'h':
    case 'H':
      if (! parseDigits(i, hour, 2, format[f] == 'H'))
        return false;
      break;

    case 'n':
    case 'N':
      if (! parseDigits(i, minute, 2, format[f] == 'N'))
        return false;
      break;

    case 's':
    case 'S':
      if (! parseDigits(i, second, 2, format[f] == 'S'))
        return false;
      break;

    // Merely parse, not extract.
    case 'v':
    case 'V':
      if (! parseDigits(i, week, 2, format[f] == 'V'))
        return false;
      break;

    // Merely parse, not extract.
    case 'a':
    case 'A':
      if (i + 3 <= _length          &&
          ! Lexer::isDigit ((*_input)[i + 0]) &&
          ! Lexer::isDigit ((*_input)[i + 1]) &&
          ! Lexer::isDigit ((*_input)[i + 2]))
      {
        wday = ISO8601d::dayOfWeek (_input->substr (i, 3).c_str ());
        i += (format[f] == 'a') ? 3 : ISO8601d::dayName (wday).size ();
      }
      else
        return false;
      break;

    case 'b':
    case 'B':
      if (i + 3 <= _length          &&
          ! Lexer::isDigit ((*_input)[i + 0]) &&
          ! Lexer::isDigit ((*_input)[i + 1]) &&
          ! Lexer::isDigit ((*_input)[i + 2]))
      {
        if (month != -1)
          return false;
        month = ISO8601d::monthOfYear (_input->substr (i, 3).c_str());
        i += (format[f] == 'b') ? 3 : ISO8601d::monthName (month).size ();
      }
      else
        return false;
      break;

    default:
      if (i + 1 <= _length &&
          (*_input)[i] == format[f])
        ++i;
      else
        return false;
      break;
    }
  }

  // By default, the most global date variables that are undefined are put to
  // the current date (for instance, the year to the current year for formats
  // that lack Y/y). If even 'second' is undefined, then the date is parsed as
  // now.
  if (year == -1)
  {
    ISO8601d now;
    year = now.year ();
    if (month == -1)
    {
      month = now.month ();
      if (day == -1)
      {
        day = now.day ();
        if (hour == -1)
        {
          hour = now.hour ();
          if (minute == -1)
          {
            minute = now.minute ();
            if (second == -1)
              second = now.second ();
          }
        }
      }
    }
  }

  // Put all remaining undefined date variables to their default values (0 or
  // 1).
  month  = (month  == -1) ? 1 : month;
  day    = (day    == -1) ? 1 : day;
  hour   = (hour   == -1) ? 0 : hour;
  minute = (minute == -1) ? 0 : minute;
  second = (second == -1) ? 0 : second;

  // Check that values are correct
  if (! ISO8601d::valid (month, day, year, hour, minute, second))
    return false;

  // Convert to epoch.
  struct tm tms {};
  tms.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  tms.tm_mon   = month - 1;
  tms.tm_mday  = day;
  tms.tm_year  = year - 1900;
  tms.tm_hour  = hour;
  tms.tm_min   = minute;
  tms.tm_sec   = second;

  t = mktime (&tms);
  _cursor = i;
  return true;
}
#endif

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
// A name is a string of alpha-numeric characters.
bool Nibbler::getName (std::string& result)
{
  auto i = _cursor;

  if (i < _length)
  {
    if (! Lexer::isDigit ((*_input)[i]) &&
        ! ispunct ((*_input)[i]) &&
        ! Lexer::isWhitespace ((*_input)[i]))
    {
      ++i;
      while (i < _length &&
             ((*_input)[i] == '_' || ! ispunct ((*_input)[i])) &&
             ! Lexer::isWhitespace ((*_input)[i]))
      {
        ++i;
      }
    }

    if (i > _cursor)
    {
      result = _input->substr (_cursor, i - _cursor);
      _cursor = i;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// A word is a contiguous string of non-space, non-digit, non-punct characters.
bool Nibbler::getWord (std::string& result)
{
  auto i = _cursor;

  if (i < _length)
  {
    while (!Lexer::isDigit       ((*_input)[i]) &&
           !Lexer::isPunctuation ((*_input)[i]) &&
           !Lexer::isWhitespace  ((*_input)[i]))
    {
      ++i;
    }

    if (i > _cursor)
    {
      result = _input->substr (_cursor, i - _cursor);
      _cursor = i;
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
bool Nibbler::skipAll (char c)
{
  if (_cursor < _length)
  {
    auto i = _input->find_first_not_of (c, _cursor);
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
bool Nibbler::skipWS ()
{
  return this->skipAllOneOf (" \t\n\r\f");
}

////////////////////////////////////////////////////////////////////////////////
#ifdef NIBBLER_FEATURE_REGEX
bool Nibbler::skipRx (const std::string& regex)
{
  if (_cursor < _length)
  {
    // Regex may be anchored to the beginning and include capturing parentheses,
    // otherwise they are added.
    std::string modified_regex;
    if (regex.substr (0, 2) != "^(")
      modified_regex = "^(" + regex + ")";
    else
      modified_regex = regex;

    RX r (modified_regex, true);
    std::vector <std::string> results;
    if (r.match (results, _input->substr (_cursor)))
    {
      _cursor += results[0].length ();
      return true;
    }
  }

  return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::backN (const int quantity /*= 1*/)
{
  if (_cursor >= (unsigned) quantity)
  {
    _cursor -= (unsigned) quantity;
    return true;
  }

  return false;
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
