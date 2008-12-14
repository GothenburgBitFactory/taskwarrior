////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include "task.h"

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
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + 1;
  }

  results.push_back (input.substr (start, std::string::npos));
}

////////////////////////////////////////////////////////////////////////////////
void split (
  std::vector<std::string>& results,
  const std::string& input,
  const std::string& delimiter)
{
  std::string::size_type length = delimiter.length ();

  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + length;
  }

  results.push_back (input.substr (start, std::string::npos));
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
void extractParagraphs (const std::string& input, std::vector<std::string>& output)
{
  std::string copy = input;
  while (1)
  {
    size_t so = copy.find ("<p>");
    size_t eo = copy.find ("</p>");

    if (so == std::string::npos && eo == std::string::npos)
      break;

    std::string extract = trim (copy.substr (so + 3, eo - so - 3));
    copy = copy.substr (eo + 4, std::string::npos);
    output.push_back (extract);
  }

  // There were no paragraphs.
  if (!output.size ())
    output.push_back (input);
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
void unquoteText (std::string& text)
{
  char quote = text[0];
  if (quote == '\'' || quote == '"')
    if (text[text.length () - 1] == quote)
      text = text.substr (1, text.length () - 3);
}

////////////////////////////////////////////////////////////////////////////////
void extractLine (std::string& text, std::string& line, int length)
{
  size_t eol = text.find ("\n");

  // Special case: found \n in first length characters.
  if (eol != std::string::npos && eol < (unsigned) length)
  {
    line = text.substr (0, eol); // strip \n
    text = text.substr (eol + 1, std::string::npos);
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
    text = text.substr (eol + 1, std::string::npos);
  }

  // If no space was found, hyphenate.
  else
  {
    if (length > 1)
    {
      line = text.substr (0, length - 1) + "-";
      text = text.substr (length - 1, std::string::npos);
    }
    else
    {
      line = text.substr (0, 1);
      text = text.substr (length, std::string::npos);
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
    if (::isdigit (data[i]))
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
      if (::isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && ::isdigit (data[i - 1]))
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
      if (::isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && ::isdigit (data[i - 1]))
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
    if (::isupper (input[i]))
      output[i] = ::tolower (input[i]);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string upperCase (const std::string& input)
{
  std::string output = input;
  for (int i = 0; i < (int) input.length (); ++i)
    if (::isupper (input[i]))
      output[i] = ::toupper (input[i]);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
const char* optionalBlankLine (Config& conf)
{
  if (conf.get ("blanklines", true) == true)
    return newline;

  return noline;
}

////////////////////////////////////////////////////////////////////////////////
