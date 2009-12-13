////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <iostream>
#include <vector>
#include <string>
#include <ctype.h>
#include "Context.h"
#include "util.h"
#include "text.h"

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

  while (copy.length ())
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
  result = "";
  unsigned int size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    result += items[i];
    if (i < size - 1)
      result += separator;
  }
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
  if (eol == std::string::npos && text.length () <= (unsigned) length)
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
  if (context.config.get ("blanklines", true) == true) // no i18n
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

  // If pos is the first alphanumeric character of the string.
  if (pos == 0 && isalnum (input[pos]))
    return true;

  // If pos is not the first alphanumeric character, but there is a preceding
  // non-alphanumeric character.
  if (pos > 0 && isalnum (input[pos]) && !isalnum (input[pos - 1]))
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
  if (pos == input.length () - 1 && isalnum (input[pos]))
    return true;

  // If pos is not the last alphanumeric character, but there is a following
  // non-alphanumeric character.
  if (pos < input.length () - 1 && isalnum (input[pos]) && !isalnum (input[pos + 1]))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
