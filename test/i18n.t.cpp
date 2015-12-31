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
#include <string>
#include <unistd.h>
#include <stdarg.h>

#include <Context.h>
#include <text.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (11);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  try
  {
    t.is (format ("pre {1} post",    "mid"),        "pre mid post",    "format 1a");
    t.is (format ("pre {1}{1} post", "mid"),        "pre midmid post", "format 1b");
    t.is (format ("pre {1} post",    0),            "pre 0 post",      "format 1c");
    t.is (format ("pre {1}{1} post", 0),            "pre 00 post",     "format 1d");

    t.is (format ("pre {1}{2} post", "one", "two"), "pre onetwo post", "format 2a");
    t.is (format ("pre {2}{1} post", "one", "two"), "pre twoone post", "format 2b");
    t.is (format ("pre {1}{2} post", "one", 2),     "pre one2 post",   "format 2c");
    t.is (format ("pre {1}{2} post", 1, "two"),     "pre 1two post",   "format 2d");
    t.is (format ("pre {1}{2} post", 1, 2),         "pre 12 post",     "format 2e");

    t.is (format ("pre {1}{2}{3} post", "one", "two", "three"), "pre onetwothree post", "format 3a");
    t.is (format ("pre {3}{1}{2} post", "one", "two", "three"), "pre threeonetwo post", "format 3b");
  }

  catch (const std::string& error)
  {
    t.diag (error);
    return -1;
  }

  catch (...)
  {
    t.diag ("Unknown error.");
    return -2;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
