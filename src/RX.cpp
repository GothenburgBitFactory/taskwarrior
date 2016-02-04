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
#include <RX.h>
#include <cstdlib>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////
RX::RX ()
{
}

////////////////////////////////////////////////////////////////////////////////
RX::RX (
  const std::string& pattern,
  bool case_sensitive /* = true */)
: _compiled (false)
, _pattern (pattern)
, _case_sensitive (case_sensitive)
{
  compile ();
}

////////////////////////////////////////////////////////////////////////////////
RX::RX (const RX& other)
{
  _compiled       = false;
  _pattern        = other._pattern;
  _case_sensitive = other._case_sensitive;
}

////////////////////////////////////////////////////////////////////////////////
RX::~RX ()
{
  if (_compiled)
    regfree (&_regex);
}

////////////////////////////////////////////////////////////////////////////////
RX& RX::operator= (const RX& other)
{
  _compiled       = false;
  _pattern        = other._pattern;
  _case_sensitive = other._case_sensitive;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void RX::compile ()
{
  if (! _compiled)
  {
    memset (&_regex, 0, sizeof (regex_t));

    int result;
    if ((result = regcomp (&_regex, _pattern.c_str (),
#if defined REG_ENHANCED
                           REG_ENHANCED | REG_EXTENDED | REG_NEWLINE |
#else
                           REG_EXTENDED | REG_NEWLINE |
#endif
                           (_case_sensitive ? 0 : REG_ICASE))) != 0)
    {
      char message[256];
      regerror (result, &_regex, message, 256);
      throw std::string (message);
    }

    _compiled = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
bool RX::match (const std::string& in)
{
  if (! _compiled)
    compile ();

  return regexec (&_regex, in.c_str (), 0, nullptr, 0) == 0 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool RX::match (
  std::vector<std::string>& matches,
  const std::string& in)
{
  if (! _compiled)
    compile ();

  regmatch_t rm[2];
  int offset = 0;
  int length = in.length ();
  while (regexec (&_regex, in.c_str () + offset, 2, &rm[0], 0) == 0 &&
         offset < length)
  {
    matches.push_back (in.substr (rm[0].rm_so + offset, rm[0].rm_eo - rm[0].rm_so));
    offset += rm[0].rm_eo;

    // Protection against zero-width patterns causing infinite loops.
    if (rm[0].rm_so == rm[0].rm_eo)
      ++offset;
  }

  return matches.size () ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool RX::match (
  std::vector <int>& start,
  std::vector <int>& end,
  const std::string& in)
{
  if (! _compiled)
    compile ();

  regmatch_t rm[2];
  int offset = 0;
  int length = in.length ();
  while (regexec (&_regex, in.c_str () + offset, 2, &rm[0], 0) == 0 &&
         offset < length)
  {
    start.push_back (rm[0].rm_so + offset);
    end.push_back   (rm[0].rm_eo + offset);
    offset += rm[0].rm_eo;

    // Protection against zero-width patterns causing infinite loops.
    if (rm[0].rm_so == rm[0].rm_eo)
      ++offset;
  }

  return start.size () ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
