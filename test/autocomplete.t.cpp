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
#include <iostream>
#include <stdlib.h>
#include <test.h>
#include <util.h>
#include <main.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (8);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  std::vector <std::string> options {"abc", "abcd", "abcde", "bcdef", "cdefg"};
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
