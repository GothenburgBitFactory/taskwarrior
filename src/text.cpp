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
#include <shared.h>
#include <text.h>
#include <utf8.h>
#include <i18n.h>

extern Context context;

static const char* newline = "\n";
static const char* noline  = "";

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
