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
#include <main.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (8);

  // 1,2,3  <=>  2,3,4
  std::vector <std::string> string_one {"1", "2", "3"};
  std::vector <std::string> string_two {"2", "3", "4"};
  std::vector <std::string> string_three {"2", "3", "4"};
  std::vector <std::string> string_four;

  // What are the differences?
  std::vector<std::string> string_leftOnly;
  std::vector<std::string> string_rightOnly;
  listDiff (string_one, string_two, string_leftOnly, string_rightOnly);
  t.is ((int) string_leftOnly.size (), 1, "std::string (1,2,3) <=> (2,3,4) = 1<-");
  t.is (string_leftOnly[0], "1",          "std::string (1,2,3) <=> (2,3,4) = 1<-");

  t.is ((int) string_rightOnly.size (), 1, "std::string (1,2,3) <=> (2,3,4) = ->4");
  t.is (string_rightOnly[0], "4",          "std::string (1,2,3) <=> (2,3,4) = ->4");

  // Now do it all again, with integers.

  // 1,2,3  <=>  2,3,4
  std::vector <int> int_one {1, 2, 3};
  std::vector <int> int_two {2, 3, 4};
  std::vector <int> int_three {2, 3, 4};
  std::vector <int> int_four;

  // What are the differences?
  std::vector<int> int_leftOnly;
  std::vector<int> int_rightOnly;
  listDiff (int_one, int_two, int_leftOnly, int_rightOnly);
  t.is ((int) int_leftOnly.size (), 1, "int (1,2,3) <=> (2,3,4) = 1<-");
  t.is (int_leftOnly[0], "1",          "int (1,2,3) <=> (2,3,4) = 1<-");

  t.is ((int) int_rightOnly.size (), 1, "int (1,2,3) <=> (2,3,4) = ->4");
  t.is (int_rightOnly[0], "4",          "int (1,2,3) <=> (2,3,4) = ->4");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

