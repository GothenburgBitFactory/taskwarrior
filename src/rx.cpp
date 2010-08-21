////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010, Paul Beckingham, Federico Hernandez.
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

#include <stdlib.h>
#include <regex.h>
#include "rx.h"

//#define _POSIX_C_SOURCE 1
#define MAX_MATCHES 8

////////////////////////////////////////////////////////////////////////////////
bool regexMatch (
  const std::string& in,
  const std::string& pattern,
  bool caseSensitive /* = true */)
{
  regex_t r = {0};
  int result;
  if ((result = regcomp (&r, pattern.c_str (),
                         REG_EXTENDED | REG_NOSUB | REG_NEWLINE |
                         (caseSensitive ? 0 : REG_ICASE))) == 0)
  {
    if ((result = regexec (&r, in.c_str (), 0, NULL, 0)) == 0)
    {
      regfree (&r);
      return true;
    }

    if (result == REG_NOMATCH)
      return false;
  }

  char message[256];
  regerror (result, &r, message, 256);
  throw std::string (message);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool regexMatch (
  std::vector<std::string>& out,
  const std::string& in,
  const std::string& pattern,
  bool caseSensitive /* = true */)
{
  regex_t r = {0};
  int result;
  if ((result = regcomp (&r, pattern.c_str (),
                         REG_EXTENDED | REG_NEWLINE |
                         (caseSensitive ? 0 : REG_ICASE))) == 0)
  {
    regmatch_t rm[MAX_MATCHES];
    if ((result = regexec (&r, in.c_str (), MAX_MATCHES, rm, 0)) == 0)
    {
      for (unsigned int i = 1; i < 1 + r.re_nsub; ++i)
        out.push_back (in.substr (rm[i].rm_so, rm[i].rm_eo - rm[i].rm_so));

      regfree (&r);
      return true;
    }

    if (result == REG_NOMATCH)
      return false;
  }

  char message[256];
  regerror (result, &r, message, 256);
  throw std::string (message);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool regexMatch (
  std::vector <int>& start,
  std::vector <int>& end,
  const std::string& in,
  const std::string& pattern,
  bool caseSensitive /* = true */)
{
  regex_t r = {0};
  int result;
  if ((result = regcomp (&r, pattern.c_str (),
                         REG_EXTENDED | REG_NEWLINE |
                         (caseSensitive ? 0 : REG_ICASE))) == 0)
  {
    regmatch_t rm[MAX_MATCHES];
    if ((result = regexec (&r, in.c_str (), MAX_MATCHES, rm, 0)) == 0)
    {
      for (unsigned int i = 1; i < 1 + r.re_nsub; ++i)
      {
        start.push_back (rm[i].rm_so);
        end.push_back   (rm[i].rm_eo);
      }

      regfree (&r);
      return true;
    }

    if (result == REG_NOMATCH)
      return false;
  }

  char message[256];
  regerror (result, &r, message, 256);
  throw std::string (message);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
