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
#include <stdlib.h>
#include <Context.h>
#include <RX.h>
#include <test.h>

Context context;

int main (int, char**)
{
  UnitTest ut (26);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  std::string text = "This is a test.";

  RX r1 ("i. ", true);
  ut.ok (r1.match (text), text + " =~ /i. /");

  std::vector <std::string> matches;
  ut.ok (r1.match (matches, text), text + " =~ /i. /");
  ut.ok (matches.size () == 2, "2 match");
  ut.is (matches[0], "is ", "$1 == is\\s");
  ut.is (matches[1], "is ", "$1 == is\\s");

  text = "abcdefghijklmnopqrstuvwxyz";

  RX r3 ("t..", true);
  ut.ok (r3.match (text), "t..");

  RX r4 ("T..", false);
  ut.ok (r4.match (text), "T..");

  RX r5 ("T..", true);
  ut.ok (!r5.match (text), "! T..");

  text = "this is a test of the regex engine.";
  //      |...:....|....:....|....:....|....:

  RX r6 ("^this");
  ut.ok (r6.match (text),      "^this matches");

  RX r7 ("engine\\.$");
  ut.ok (r7.match (text), "engine\\.$ matches");

  std::vector <std::string> results;
  std::vector <int> start;
  std::vector <int> end;
  RX r8 ("e..", true);
  ut.ok (r8.match (results, text), "e.. there are matches");
  ut.ok (r8.match (start, end, text), "e.. there are matches");
  ut.is (results.size (), (size_t) 4, "e.. == 4 matches");
  ut.is (results[0], "est", "e..[0] == 'est'");
  ut.is (start[0],      11, "e..[0] == 11->");
  ut.is (end[0],        14, "e..[0] == ->14");

  results.clear ();
  RX r9 ("e", true);
  ut.ok (r9.match (results, text),           "e there are matches");
  ut.is (results.size (), (size_t) 6,        "e == 6 matches");

  start.clear ();
  end.clear ();
  ut.ok (r9.match (start, end, text),        "e there are matches");
  ut.is (start.size (), (size_t) 6,          "e == 6 matches");

#if defined(DARWIN) || defined(CYGWIN) || defined(FREEBSD) || defined(OPENBSD)
  text = "this is the end.";
  ut.pass (text + " =~ /\\bthe/");
  ut.pass (text + " =~ /the\\b/");
  ut.pass (text + " =~ /\\bthe\\b/");
#elif defined(SOLARIS)
  RX r10 ("\\<the");
  text = "this is the end.";
  ut.ok (r10.match (text), text + " =~ /\\<the/");

  RX r11 ("the\\>");
  ut.ok (r11.match (text), text + " =~ /the\\>/");

  RX r12 ("\\<the\\>");
  ut.ok (r12.match (text), text + " =~ /\\<the\\>/");
#else
  RX r10 ("\\bthe");
  text = "this is the end.";
  ut.ok (r10.match (text), text + " =~ /\\bthe/");

  RX r11 ("the\\b");
  ut.ok (r11.match (text), text + " =~ /the\\b/");

  RX r12 ("\\bthe\\b");
  ut.ok (r12.match (text), text + " =~ /\\bthe\\b/");
#endif

#if defined(DARWIN)
  text = "D0";
  RX r13 ("D\\d");
  ut.ok (r13.match (text), text + " =~ /D\\d/");
#else
  ut.skip (" =~ /D\\d/");
#endif

  text = "D0";
  RX r14 ("D[[:digit:]]");
  ut.ok (r14.match (text), text + " =~ /D[[:digit:]]/");

  text = "D0";
  RX r15 ("D[0-9]");
  ut.ok (r15.match (text), text + " =~ /D[0-9]/");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

