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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <strings.h>
#include <ctype.h>
#include "Context.h"
#include "util.h"
#include "text.h"
#include "utf8.h"

extern Context context;

static const char* newline = "\n";
static const char* noline  = "";

///////////////////////////////////////////////////////////////////////////////
void wrapText (
  std::vector <std::string>& lines,
  const std::string& text,
  const int width)
{
  std::string copy = text;
  std::string line;

  while (copy.length ())  // Used as Boolean, therefore UTF8 safe.
  {
    extractLine (copy, line, width);
    lines.push_back (line);
  }
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
void split_minimal (
  std::vector<std::string>& results,
  const std::string& input,
  const char delimiter)
{
  results.clear ();
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    if (i != start)
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
void split_minimal (
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
    if (i != start)
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

  while (character = utf8_next_char (input, i))
  {
    if (character == ' ')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      ++length;
  }

  return longest;
}

////////////////////////////////////////////////////////////////////////////////
void extractLine (std::string& text, std::string& line, int length)
{
  size_t eol = text.find ("\n");

  // Special case: found \n in first length characters.
  if (eol != std::string::npos && eol < (unsigned) length)
  {
    line = text.substr (0, eol); // strip \n
    text = text.substr (eol + 1);
    return;
  }

  // Special case: no \n, and less than length characters total.
  // special case: text.find ("\n") == std::string::npos && text.length () < length
  if (eol == std::string::npos && utf8_length (text) <= length)
  {
    line = text;
    text = "";
    return;
  }

  // Safe to ASSERT text.length () > length

  // Look for the last space prior to length
  eol = length;
  while (eol && text[eol] != ' ' && text[eol] != '\n')
    --eol;

  // If a space was found, break there.
  if (eol)
  {
    line = text.substr (0, eol);
    text = text.substr (eol + 1);
  }

  // If no space was found, hyphenate.
  else
  {
    if (length > 1)
    {
      line = text.substr (0, length - 1) + "-";
      text = text.substr (length - 1);
    }
    else
    {
      line = text.substr (0, 1);
      text = text.substr (length);
    }
  }
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
    if (isdigit (data[i]))
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
      if (isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && isdigit (data[i - 1]))
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
      if (isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && isdigit (data[i - 1]))
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
  for (int i = 0; i < (int) input.length (); ++i)
    if (isupper (input[i]))
      output[i] = tolower (input[i]);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string upperCase (const std::string& input)
{
  std::string output = input;
  for (int i = 0; i < (int) input.length (); ++i)
    if (islower (input[i]))
      output[i] = toupper (input[i]);

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
const char* optionalBlankLine ()
{
  if (context.config.getBoolean ("blanklines") == true) // no i18n
    return newline;

  return noline;
}

////////////////////////////////////////////////////////////////////////////////
void guess (
  const std::string& type,
  std::vector<std::string>& options,
  std::string& candidate)
{
  std::vector <std::string> matches;
  autoComplete (candidate, options, matches);
  if (1 == matches.size ())
    candidate = matches[0];

  else if (0 == matches.size ())
    candidate = "";

  else
  {
    std::sort (matches.begin (), matches.end ());

    std::string error = "Ambiguous "; // TODO i18n
    error += type;
    error += " '";
    error += candidate;
    error += "' - could be either of "; // TODO i18n
    for (size_t i = 0; i < matches.size (); ++i)
    {
      if (i)
        error += ", ";
      error += matches[i];
    }

    throw error;
  }
}

////////////////////////////////////////////////////////////////////////////////
bool digitsOnly (const std::string& input)
{
  for (size_t i = 0; i < input.length (); ++i)
    if (!isdigit (input[i]))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool noSpaces (const std::string& input)
{
  for (size_t i = 0; i < input.length (); ++i)
    if (isspace (input[i]))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool noVerticalSpace (const std::string& input)
{
  if (input.find_first_of ("\n\r\f") != std::string::npos)
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
//          Input: hello, world
// Result for pos: y......y....
bool isWordStart (const std::string& input, std::string::size_type pos)
{
  // Short circuit: no input means no word start.
  if (input.length () == 0)
    return false;

  // If pos is the first non space/punct character of the string.
  if (pos == 0 && !isspace (input[pos]) && !isPunctuation (input[pos]))
    return true;

  // If pos is not the first alphanumeric character, but there is a preceding
  // space/punct character.
  if (pos > 0 && !isspace (input[pos]) && !isPunctuation (input[pos])
              && (isspace (input[pos - 1]) || isPunctuation (input[pos - 1])))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//          Input: hello, world
// Result for pos: ....y......y
bool isWordEnd (const std::string& input, std::string::size_type pos)
{
  // Short circuit: no input means no word start.
  if (input.length () == 0)
    return false;

  // If pos is the last alphanumeric character of the string.
  if (pos == input.length () - 1 && !isspace (input[pos]) && !isPunctuation (input[pos]))
    return true;

  // If pos is not the last alphanumeric character, but there is a following
  // non-alphanumeric character.
  if (pos < input.length () - 1 && !isspace (input[pos]) && !isPunctuation (input[pos])
                                && (isspace (input[pos + 1]) || isPunctuation (input[pos + 1])))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Override of ispunct, that considers #, $ and @ not to be punctuation.
//
// ispunct:      ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
// Punctuation:  ! "     % & ' ( ) * + , - . / : ; < = > ?   [ \ ] ^ _ ` { | } ~
// delta:            # $                                   @
//
bool isPunctuation (char c)
{
  if (c == '@' || c == '#' || c == '$')
    return false;

  return ispunct (c);
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
// Truncates a long line, and include a two-character ellipsis.
std::string cutOff (const std::string& str, std::string::size_type len)
{
  if (str.length () > len)
  {
    return (str.substr (0, len - 2) + "..");
  }
  else
  {
    std::string res = str;
    res.resize (len, ' ');
    return res;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string format (char value)
{
  std::stringstream s;
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string format (int value)
{
  std::stringstream s;
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string formatHex (int value)
{
  std::stringstream s;
  s.setf (std::ios::hex, std::ios::basefield);
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string format (float value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string format (double value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  s << value;
  return s.str ();
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
  return input + std::string (width - utf8_length (input), ' ');
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
  return std::string (width - utf8_length (input), ' ') + input;
}

////////////////////////////////////////////////////////////////////////////////
