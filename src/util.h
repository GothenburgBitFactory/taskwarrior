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

#ifndef INCLUDED_UTIL
#define INCLUDED_UTIL

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <Task.h>

#ifdef HAVE_UUID
#include <uuid/uuid.h>
#endif

// util.cpp
bool confirm (const std::string&);
int confirm3 (const std::string&);
int confirm4 (const std::string&);
void delay (float);
std::string formatBytes (size_t);
int autoComplete (const std::string&, const std::vector<std::string>&, std::vector<std::string>&, int minimum = 1);

#if defined(HAVE_UUID) && !defined(HAVE_UUID_UNPARSE_LOWER)
void uuid_unparse_lower (uuid_t uu, char *out);
#endif
const std::string uuid ();

int execute (const std::string&, std::vector<std::string>);

#ifdef SOLARIS
  #define LOCK_SH 1
  #define LOCK_EX 2
  #define LOCK_NB 4
  #define LOCK_UN 8

  int flock (int, int);
#endif

std::string compressIds (const std::vector <int>&);
void combine (std::vector <int>&, const std::vector <int>&);

unsigned burndown_size (unsigned ntasks);

const std::string encode (const std::string&);
const std::string decode (const std::string&);

const std::string escape (const std::string&, char);

const std::vector<std::string> indentTree (
  const std::vector<std::string>&,
  const std::string& whitespace = "  ",
  char delimiter = '.');

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
