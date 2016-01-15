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

#ifndef INCLUDED_UTIL
#define INCLUDED_UTIL

#include <cmake.h>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#if defined(FREEBSD) || defined(OPENBSD)
#include <uuid.h>
#else
#include <uuid/uuid.h>
#endif
#include <Task.h>

// util.cpp
bool confirm (const std::string&);
int confirm4 (const std::string&);
std::string formatBytes (size_t);
int autoComplete (const std::string&, const std::vector<std::string>&, std::vector<std::string>&, int minimum = 1);

#ifndef HAVE_UUID_UNPARSE_LOWER
void uuid_unparse_lower (uuid_t uu, char *out);
#endif
const std::string uuid ();

int execute (const std::string&, const std::vector <std::string>&, const std::string&, std::string&);

const std::string indentProject (
  const std::string&,
  const std::string& whitespace = "  ",
  char delimiter = '.');

const std::vector <std::string> extractParents (
  const std::string&,
  const char& delimiter = '.');

#ifndef HAVE_TIMEGM
  time_t timegm (struct tm *tm);
#endif

#endif
////////////////////////////////////////////////////////////////////////////////
