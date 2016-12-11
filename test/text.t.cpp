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
#include <text.h>
#include <utf8.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (5);

  // int strippedLength (const std::string&);
  t.is (strippedLength (std::string ("")),                                  0, "strippedLength                              -> 0");
  t.is (strippedLength (std::string ("abc")),                               3, "strippedLength abc                          -> 3");
  t.is (strippedLength (std::string ("one\033[5;38;255mtwo\033[0mthree")), 11, "strippedLength one^[[5;38;255mtwo^[[0mthree -> 11");
  t.is (strippedLength (std::string ("\033[0m")),                           0, "strippedLength ^[[0m                        -> 0");
  t.is (strippedLength (std::string ("\033[1m\033[0m")),                    0, "strippedLength ^[[1m^[[0m                   -> 0");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
