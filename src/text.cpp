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
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <strings.h>
#include <Context.h>
#include <Lexer.h>
#include <math.h>
#include <util.h>
#include <text.h>
#include <utf8.h>
#include <i18n.h>

extern Context context;

static const char* newline = "\n";
static const char* noline  = "";

///////////////////////////////////////////////////////////////////////////////
void wrapText (
  std::vector <std::string>& lines,
  const std::string& text,
  const int width,
  bool hyphenate)
{
  std::string line;
  unsigned int offset = 0;
  while (extractLine (line, text, width, hyphenate, offset))
    lines.push_back (line);
}

////////////////////////////////////////////////////////////////////////////////
void split (
  std::set<std::string>& results,
  const std::string& input,
  const char delimiter)
{
  results.clear ();
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.insert (input.substr (start, i - start));
    start = i + 1;
  }

  if (input.length ())
    results.insert (input.substr (start));
}

////////////////////////////////////////////////////////////////////////////////
void split (
  std::vector<std::string>& results,
  const std::string& input,
  const char delimiter)
{
  results.clear ();
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + 1;
  }

  if (input.length ())
    results.push_back (input.substr (start));
}

////////////////////////////////////////////////////////////////////////////////
void split (
  std::vector<std::string>& results,
  const std::string& input,
  const std::string& delimiter)
{
  results.clear ();
  std::string::size_type length = delimiter.length ();

  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + length;
  }

  if (input.length ())
    results.push_back (input.substr (start));
}

////////////////////////////////////////////////////////////////////////////////
void join (
  std::string& result,
  const std::string& separator,
  const std::vector<std::string>& items)
{
  std::stringstream s;
  unsigned int size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    s << items[i];
    if (i < size - 1)
      s << separator;
  }

  result = s.str ();
}

////////////////////////////////////////////////////////////////////////////////
void join (
  std::string& result,
  const std::string& separator,
  const std::vector<int>& items)
{
  std::stringstream s;
  unsigned int size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    s << items[i];
    if (i < size - 1)
      s << separator;
  }

  result = s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string trimLeft (const std::string& in, const std::string& t /*= " "*/)
{
  std::string out = in;
  return out.erase (0, in.find_first_not_of (t));
}

////////////////////////////////////////////////////////////////////////////////
std::string trimRight (const std::string& in, const std::string& t /*= " "*/)
{
  std::string out = in;
  return out.erase (out.find_last_not_of (t) + 1);
}

////////////////////////////////////////////////////////////////////////////////
std::string trim (const std::string& in, const std::string& t /*= " "*/)
{
  std::string out = in;
  return trimLeft (trimRight (out, t), t);
}

////////////////////////////////////////////////////////////////////////////////
// Remove enclosing balanced quotes.  Assumes trimmed text.
std::string unquoteText (const std::string& input)
{
  std::string output = input;

  if (output.length () > 1)
  {
    char quote = output[0];
    if ((quote == '\'' || quote == '"') &&
        output[output.length () - 1] == quote)
      return output.substr (1, output.length () - 2);
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
int longestWord (const std::string& input)
{
  int longest = 0;
  int length = 0;
  std::string::size_type i = 0;
  int character;

  while ((character = utf8_next_char (input, i)))
  {
    if (character == ' ')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      length += mk_wcwidth (character);
  }

  if (length > longest)
    longest = length;

  return longest;
}

////////////////////////////////////////////////////////////////////////////////
int longestLine (const std::string& input)
{
  int longest = 0;
  int length = 0;
  std::string::size_type i = 0;
  int character;

  while ((character = utf8_next_char (input, i)))
  {
    if (character == '\n')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      length += mk_wcwidth (character);
  }

  if (length > longest)
    longest = length;

  return longest;
}

////////////////////////////////////////////////////////////////////////////////
// Walk the input text looking for a break point.  A break point is one of:
//   - EOS
//   - \n
//   - last space before 'length' characters
//   - last punctuation (, ; . :) before 'length' characters, even if not
//     followed by a space
//   - first 'length' characters
//
// text       "one two three\n  four"
// bytes       0123456789012 3456789
// characters  1234567890a23 4567890
//
// leading_ws
// ws             ^   ^       ^^
// punct
// break                     ^
bool extractLine (
  std::string& line,
  const std::string& text,
  int width,
  bool hyphenate,
  unsigned int& offset)
{
  // Terminate processing.
  // Note: bytes vs bytes.
  if (offset >= text.length ())
    return false;

  std::string::size_type last_last_bytes = offset;
  std::string::size_type last_bytes = offset;
  std::string::size_type bytes = offset;
  unsigned int last_ws = 0;
  int character;
  int char_width = 0;
  int line_width = 0;
  while (1)
  {
    last_last_bytes = last_bytes;
    last_bytes = bytes;
    character = utf8_next_char (text, bytes);

    if (character == 0 ||
        character == '\n')
    {
      line = text.substr (offset, last_bytes - offset);
      offset = bytes;
      break;
    }
    else if (character == ' ')
      last_ws = last_bytes;

    char_width = mk_wcwidth (character);
    if (line_width + char_width > width)
    {
      int last_last_character = text[last_last_bytes];
      int last_character = text[last_bytes];

      // [case 1] one| two --> last_last != 32, last == 32, ws == 0
      if (last_last_character != ' ' &&
          last_character      == ' ')
      {
        line = text.substr (offset, last_bytes - offset);
        offset = last_bytes + 1;
        break;
      }

      // [case 2] one |two --> last_last == 32, last != 32, ws != 0
      else if (last_last_character == ' ' &&
               last_character      != ' ' &&
               last_ws             != 0)
      {
        line = text.substr (offset, last_bytes - offset - 1);
        offset = last_bytes;
        break;
      }

      else if (last_last_character != ' ' &&
               last_character      != ' ')
      {
        // [case 3] one t|wo --> last_last != 32, last != 32, ws != 0
        if (last_ws != 0)
        {
          line = text.substr (offset, last_ws - offset);
          offset = last_ws + 1;
          break;
        }
        // [case 4] on|e two --> last_last != 32, last != 32, ws == 0
        else
        {
          if (hyphenate)
          {
            line = text.substr (offset, last_bytes - offset - 1) + "-";
            offset = last_last_bytes;
          }
          else
          {
            line = text.substr (offset, last_bytes - offset);
            offset = last_bytes;
          }
        }

        break;
      }
    }

    line_width += char_width;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
std::string commify (const std::string& data)
{
  // First scan for decimal point and end of digits.
  int decimalPoint = -1;
  int end          = -1;

  int i;
  for (int i = 0; i < (int) data.length (); ++i)
  {
    if (Lexer::isDigit (data[i]))
      end = i;

    if (data[i] == '.')
      decimalPoint = i;
  }

  std::string result;
  if (decimalPoint != -1)
  {
    // In reverse order, transfer all digits up to, and including the decimal
    // point.
    for (i = (int) data.length () - 1; i >= decimalPoint; --i)
      result += data[i];

    int consecutiveDigits = 0;
    for (; i >= 0; --i)
    {
      if (Lexer::isDigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && Lexer::isDigit (data[i - 1]))
        {
          result += ',';
          consecutiveDigits = 0;
        }
      }
      else
        result += data[i];
    }
  }
  else
  {
    // In reverse order, transfer all digits up to, but not including the last
    // digit.
    for (i = (int) data.length () - 1; i > end; --i)
      result += data[i];

    int consecutiveDigits = 0;
    for (; i >= 0; --i)
    {
      if (Lexer::isDigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && Lexer::isDigit (data[i - 1]))
        {
          result += ',';
          consecutiveDigits = 0;
        }
      }
      else
        result += data[i];
    }
  }

  // reverse result into data.
  std::string done;
  for (int i = (int) result.length () - 1; i >= 0; --i)
    done += result[i];

  return done;
}

////////////////////////////////////////////////////////////////////////////////
std::string lowerCase (const std::string& input)
{
  std::string output = input;
  std::transform (output.begin (), output.end (), output.begin (), tolower);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string ucFirst (const std::string& input)
{
  std::string output = input;

  if (output.length () > 0)
    output[0] = toupper (output[0]);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::string str_replace (
  std::string &str,
  const std::string& search,
  const std::string& replacement)
{
  std::string::size_type pos = 0;
  while ((pos = str.find (search, pos)) != std::string::npos)
  {
    str.replace (pos, search.length (), replacement);
    pos += replacement.length ();
  }

  return str;
}

////////////////////////////////////////////////////////////////////////////////
const std::string str_replace (
  const std::string& str,
  const std::string& search,
  const std::string& replacement)
{
  std::string modified = str;
  return str_replace (modified, search, replacement);
}

////////////////////////////////////////////////////////////////////////////////
const char* optionalBlankLine ()
{
  return context.verbose ("blank") ? newline : noline;
}

////////////////////////////////////////////////////////////////////////////////
bool nontrivial (const std::string& input)
{
  std::string::size_type i = 0;
  int character;
  while ((character = utf8_next_char (input, i)))
    if (! Lexer::isWhitespace (character))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool compare (
  const std::string& left,
  const std::string& right,
  bool sensitive /*= true*/)
{
  // Use strcasecmp if required.
  if (!sensitive)
    return strcasecmp (left.c_str (), right.c_str ()) == 0 ? true : false;

  // Otherwise, just use std::string::operator==.
  return left == right;
}

////////////////////////////////////////////////////////////////////////////////
bool closeEnough (
  const std::string& reference,
  const std::string& attempt,
  unsigned int minLength /* = 0 */)
{
  // An exact match is accepted first.
  if (compare (reference, attempt, false))
    return true;

  // A partial match will suffice.
  if (attempt.length () < reference.length () &&
      attempt.length () >= minLength)
    return compare (reference.substr (0, attempt.length ()), attempt, false);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type find (
  const std::string& text,
  const std::string& pattern,
  bool sensitive /*= true*/)
{
  // Implement a sensitive find, which is really just a loop withing a loop,
  // comparing lower-case versions of each character in turn.
  if (!sensitive)
  {
    // Handle empty pattern.
    const char* p = pattern.c_str ();
    size_t len = pattern.length ();
    if (len == 0)
      return 0;

    // Evaluate these once, for performance reasons.
    const char* t = text.c_str ();
    const char* start = t;
    const char* end = start + text.size ();

    for (; t <= end - len; ++t)
    {
      int diff = 0;
      for (size_t i = 0; i < len; ++i)
        if ((diff = tolower (t[i]) - tolower (p[i])))
          break;

      // diff == 0 means there was no break from the loop, which only occurs
      // when a difference is detected.  Therefore, the loop terminated, and
      // diff is zero.
      if (diff == 0)
        return t - start;
    }

    return std::string::npos;
  }

  // Otherwise, just use std::string::find.
  return text.find (pattern);
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type find (
  const std::string& text,
  const std::string& pattern,
  std::string::size_type begin,
  bool sensitive /*= true*/)
{
  // Implement a sensitive find, which is really just a loop withing a loop,
  // comparing lower-case versions of each character in turn.
  if (!sensitive)
  {
    // Handle empty pattern.
    const char* p = pattern.c_str ();
    size_t len = pattern.length ();
    if (len == 0)
      return 0;

    // Handle bad begin.
    if (begin >= text.length ())
      return std::string::npos;

    // Evaluate these once, for performance reasons.
    const char* start = text.c_str ();
    const char* t = start + begin;
    const char* end = start + text.size ();

    for (; t <= end - len; ++t)
    {
      int diff = 0;
      for (size_t i = 0; i < len; ++i)
        if ((diff = tolower (t[i]) - tolower (p[i])))
          break;

      // diff == 0 means there was no break from the loop, which only occurs
      // when a difference is detected.  Therefore, the loop terminated, and
      // diff is zero.
      if (diff == 0)
        return t - start;
    }

    return std::string::npos;
  }

  // Otherwise, just use std::string::find.
  return text.find (pattern, begin);
}

////////////////////////////////////////////////////////////////////////////////
// Return the length, in characters, of the input, subtracting color control
// codes.
int strippedLength (const std::string& input)
{
  int length = input.length ();
  bool inside = false;
  int count = 0;
  for (int i = 0; i < length; ++i)
  {
    if (inside)
    {
      if (input[i] == 'm')
        inside = false;
    }
    else
    {
      if (input[i] == 033)
        inside = true;
      else
        ++count;
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////
const std::string obfuscateText (const std::string& input)
{
  std::stringstream output;
  std::string::size_type i = 0;
  int character;
  bool inside = false;

  while ((character = utf8_next_char (input, i)))
  {
    if (inside)
    {
      output << (char) character;

      if (character == 'm')
        inside = false;
    }
    else
    {
      if (character == 033)
        inside = true;

      if (inside || character == ' ')
        output << (char) character;
      else
        output << 'x';
    }
  }

  return output.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (std::string& value)
{
  return value;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (const char* value)
{
  std::string s (value);
  return s;
}

////////////////////////////////////////////////////////////////////////////////
const std::string formatHex (int value)
{
  std::stringstream s;
  s.setf (std::ios::hex, std::ios::basefield);
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (float value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  if (0 < value && value < 1)
  {
    // For value close to zero, width - 2 (2 accounts for the first zero and
    // the dot) is the number of digits after zero that are significant
    double factor = 1;
    for (int i = 2; i < width; i++)
      factor *= 10;
    value = roundf (value * factor) / factor;
  }
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (double value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  if (0 < value && value < 1)
  {
    // For value close to zero, width - 2 (2 accounts for the first zero and
    // the dot) is the number of digits after zero that are significant
    double factor = 1;
    for (int i = 2; i < width; i++)
      factor *= 10;
    value = round (value * factor) / factor;
  }
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (double value)
{
  std::stringstream s;
  s << std::fixed << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
void replace_positional (
  std::string& fmt,
  const std::string& from,
  const std::string& to)
{
  std::string::size_type pos = 0;
  while ((pos = fmt.find (from, pos)) != std::string::npos)
  {
    fmt.replace (pos, from.length (), to);
    pos += to.length ();
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string leftJustify (const int input, const int width)
{
  std::stringstream s;
  s << input;
  std::string output = s.str ();
  return output + std::string (width - output.length (), ' ');
}

////////////////////////////////////////////////////////////////////////////////
std::string leftJustify (const std::string& input, const int width)
{
  return input + std::string (width - utf8_text_width (input), ' ');
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustifyZero (const int input, const int width)
{
  std::stringstream s;
  s << std::setw (width) << std::setfill ('0') << input;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustify (const int input, const int width)
{
  std::stringstream s;
  s << std::setw (width) << std::setfill (' ') << input;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustify (const std::string& input, const int width)
{
  unsigned int len = utf8_text_width (input);
  return (((unsigned int) width > len)
           ? std::string (width - len, ' ')
           : "")
         + input;
}

////////////////////////////////////////////////////////////////////////////////
