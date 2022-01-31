////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <format.h>
#include <shared.h>
// If <iostream> is included, put it after <stdio.h>, because it includes
// <stdio.h>, and therefore would ignore the _WITH_GETLINE.
#ifdef FREEBSD
#define _WITH_GETLINE
#endif
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <signal.h>
#include <sys/select.h>
#include <Lexer.h>
#include <unicode.h>
#include <utf8.h>
#include <util.h>
#include <main.h>

#define STRING_UTIL_CONFIRM_YES      "yes"
#define STRING_UTIL_CONFIRM_YES_U    "Yes"
#define STRING_UTIL_CONFIRM_NO       "no"
#define STRING_UTIL_CONFIRM_ALL      "all"
#define STRING_UTIL_CONFIRM_ALL_U    "All"
#define STRING_UTIL_CONFIRM_QUIT     "quit"

static const char* newline = "\n";
static const char* noline  = "";

//////////////////////////////////////////////////////////////////////////////// 
static void signal_handler (int s)
{
  if (s == SIGINT)
  {
    std::cout << "\n\nInterrupted: No changes made.\n";
    exit (1);
  }
}

////////////////////////////////////////////////////////////////////////////////
// 0 = no
// 1 = yes
// 2 = all
// 3 = quit
int confirm4 (const std::string& question)
{
  std::vector <std::string> options {STRING_UTIL_CONFIRM_YES_U,
                                     STRING_UTIL_CONFIRM_YES,
                                     STRING_UTIL_CONFIRM_NO,
                                     STRING_UTIL_CONFIRM_ALL_U,
                                     STRING_UTIL_CONFIRM_ALL,
                                     STRING_UTIL_CONFIRM_QUIT};
  std::vector <std::string> matches;

  signal (SIGINT, signal_handler);

  do
  {
    std::cout << question
              << " ("
              << options[1] << '/'
              << options[2] << '/'
              << options[4] << '/'
              << options[5]
              << ") ";

    std::string answer {""};
    std::getline (std::cin, answer);
    Context::getContext ().debug ("STDIN '" + answer + '\'');
    answer = std::cin.eof () ? STRING_UTIL_CONFIRM_QUIT : Lexer::lowerCase (Lexer::trim (answer));
    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (! std::cin.eof () && matches.size () != 1);

  signal (SIGINT, SIG_DFL);

  if (matches.size () == 1)
  {
         if (matches[0] == STRING_UTIL_CONFIRM_YES_U) return 1;
    else if (matches[0] == STRING_UTIL_CONFIRM_YES)   return 1;
    else if (matches[0] == STRING_UTIL_CONFIRM_ALL_U) return 2;
    else if (matches[0] == STRING_UTIL_CONFIRM_ALL)   return 2;
    else if (matches[0] == STRING_UTIL_CONFIRM_QUIT)  return 3;
  }

  return 0;
}

// Handle the generation of UUIDs on FreeBSD in a separate implementation
// of the uuid () function, since the API is quite different from Linux's.
// Also, uuid_unparse_lower is not needed on FreeBSD, because the string
// representation is always lowercase anyway.
// For the implementation details, refer to
// https://svnweb.freebsd.org/base/head/sys/kern/kern_uuid.c
#if defined(FREEBSD) || defined(OPENBSD)
const std::string uuid ()
{
  uuid_t id;
  uint32_t status;
  char *buffer (0);
  uuid_create (&id, &status);
  uuid_to_string (&id, &buffer, &status);

  std::string res (buffer);
  free (buffer);

  return res;
}
#else

////////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_UUID_UNPARSE_LOWER
// Older versions of libuuid don't have uuid_unparse_lower(), only uuid_unparse()
void uuid_unparse_lower (uuid_t uu, char *out)
{
    uuid_unparse (uu, out);
    // Characters in out are either 0-9, a-z, '-', or A-Z.  A-Z is mapped to
    // a-z by bitwise or with 0x20, and the others already have this bit set
    for (size_t i = 0; i < 36; ++i) out[i] |= 0x20;
}
#endif

const std::string uuid ()
{
  uuid_t id;
  uuid_generate (id);
  char buffer[100] {};
  uuid_unparse_lower (id, buffer);

  // Bug found by Steven de Brouwer.
  buffer[36] = '\0';

  return std::string (buffer);
}
#endif

// Collides with std::numeric_limits methods
#undef max

////////////////////////////////////////////////////////////////////////////////
// Accept a list of projects, and return an indented list that reflects the
// hierarchy.
//
//      Input  - "one"
//               "one.two"
//               "one.two.three"
//               "one.four"
//               "two"
//      Output - "one"
//               "  one.two"
//               "    one.two.three"
//               "  one.four"
//               "two"
//
// There are two optional arguments, 'whitespace', and 'delimiter',
//
//  - whitespace is the string used to build the prefixes of indented items.
//    - defaults to two spaces, "  "
//  - delimiter is the character used to split up projects into subprojects.
//    - defaults to the period, '.'
//
const std::string indentProject (
  const std::string& project,
  const std::string& whitespace /* = "  " */,
  char delimiter /* = '.' */)
{
  // Count the delimiters in *i.
  std::string prefix = "";
  std::string::size_type pos = 0;
  std::string::size_type lastpos = 0;
  while ((pos = project.find (delimiter, pos + 1)) != std::string::npos)
  {
    if (pos != project.size () - 1)
    {
      prefix += whitespace;
      lastpos = pos;
    }
  }

  std::string child = "";
  if (lastpos == 0)
    child = project;
  else
    child = project.substr (lastpos + 1);

  return prefix + child;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> extractParents (
  const std::string& project,
  const char& delimiter /* = '.' */)
{
  std::vector <std::string> vec;
  std::string::size_type pos = 0;
  std::string::size_type copyUntil = 0;
  while ((copyUntil = project.find (delimiter, pos + 1)) != std::string::npos)
  {
    if (copyUntil != project.size () - 1)
      vec.push_back (project.substr (0, copyUntil));
    pos = copyUntil;
  }
  return vec;
}

////////////////////////////////////////////////////////////////////////////////
#ifndef HAVE_TIMEGM
time_t timegm (struct tm *tm)
{
  time_t ret;
  char *tz;
  tz = getenv ("TZ");
  setenv ("TZ", "UTC", 1);
  tzset ();
  ret = mktime (tm);
  if (tz)
    setenv ("TZ", tz, 1);
  else
    unsetenv ("TZ");
  tzset ();
  return ret;
}
#endif

////////////////////////////////////////////////////////////////////////////////
bool nontrivial (const std::string& input)
{
  std::string::size_type i = 0;
  int character;
  while ((character = utf8_next_char (input, i)))
    if (! unicodeWhitespace (character))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
const char* optionalBlankLine ()
{
  return Context::getContext ().verbose ("blank") ? newline : noline;
}

////////////////////////////////////////////////////////////////////////////////
void setHeaderUnderline (Table& table)
{
  // If an alternating row color is specified, notify the table.
  if (Context::getContext ().color ())
  {
    Color alternate (Context::getContext ().config.get ("color.alternate"));
    table.colorOdd (alternate);
    table.intraColorOdd (alternate);

    if (Context::getContext ().config.getBoolean ("fontunderline"))
    {
      table.colorHeader (Color ("underline " + Context::getContext ().config.get ("color.label")));
    }
    else
    {
      table.colorHeader (Color (Context::getContext ().config.get ("color.label")));
      table.underlineHeaders ();
    }
  }
  else
  {
    if (Context::getContext ().config.getBoolean ("fontunderline"))
      table.colorHeader (Color ("underline"));
    else
      table.underlineHeaders ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Perform strtol on a string and check if the extracted value matches.
//
bool extractLongInteger (const std::string& input, long& output)
{
  output = strtol (input.c_str (), nullptr, 10);
  return (format ("{1}", output) == input);
}
