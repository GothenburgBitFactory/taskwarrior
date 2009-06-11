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
#include <test.h>
#include <util.h>
#include <main.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (8);

  std::vector <std::string> options;
  options.push_back ("abc");
  options.push_back ("abcd");
  options.push_back ("abcde");
  options.push_back ("bcdef");
  options.push_back ("cdefg");

  std::vector <std::string> matches;
  int result = autoComplete ("", options, matches);
  t.is (result, 0, "no match on empty string");

  result = autoComplete ("x", options, matches);
  t.is (result, 0, "no match on wrong string");

  result = autoComplete ("abcd", options, matches);
  t.is (result, 1, "exact match on 'abcd'");
  t.is (matches[0], "abcd", "exact match on 'abcd'");

  result = autoComplete ("ab", options, matches);
  t.is (result, 3, "partial match on 'ab'");
  t.is (matches[0], "abc", "partial match on 'abc'");
  t.is (matches[1], "abcd", "partial match on 'abcd'");
  t.is (matches[2], "abcde", "partial match on 'abcde'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
