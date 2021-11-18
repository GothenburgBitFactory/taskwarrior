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
#include <iostream>
#include <stdlib.h>
#include <main.h>
#include <util.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (19);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  // TODO int confirm4 (const std::string&);

  // TODO const std::string uuid ();

  // TODO These are in feedback.cpp, not util.cpp.
  Task left;
  left.set ("zero", "0");
  left.set ("one",  1);
  left.set ("two",  2);

  Task right;
  right.set ("zero",  "00");
  right.set ("two",   2);
  right.set ("three", 3);

  Task rightAgain (right);

  std::string output = left.diff (right);
  t.ok (!(left == right),                                                           "Detected changes");
  t.ok (output.find ("Zero will be changed from '0' to '00'") != std::string::npos, "Detected change zero:0 -> zero:00");
  t.ok (output.find ("One will be deleted")                   != std::string::npos, "Detected deletion one:1 ->");
  t.ok (output.find ("Two")                                   == std::string::npos, "Detected no change two:2 -> two:2");
  t.ok (output.find ("Three will be set to '3'")              != std::string::npos, "Detected addition -> three:3");

  output = right.diff (rightAgain);
  t.ok (output.find ("No changes will be made")               != std::string::npos, "No changes detected");

  // std::vector<std::string> indentProject (const std::string&, const std::string whitespace="  ", char delimiter='.');
  t.is (indentProject (""),              "",                  "indentProject '' -> ''");
  t.is (indentProject ("one"),           "one",               "indentProject 'one' -> 'one'");
  t.is (indentProject ("one.two"),       "  two",         "indentProject 'one.two' -> '  two'");
  t.is (indentProject ("one.two.three"), "    three", "indentProject 'one.two.three' -> '    three'");

  // bool nontrivial (const std::string&);
  t.notok (nontrivial (""),                       "nontrivial '' -> false");
  t.notok (nontrivial ("   "),                    "nontrivial '   ' -> false");
  t.notok (nontrivial ("\t\t"),                   "nontrivial '\\t\\t' -> false");
  t.notok (nontrivial (" \t \t"),                 "nontrivial ' \\t \\t' -> false");
  t.ok    (nontrivial ("a"),                      "nontrivial 'a' -> true");
  t.ok    (nontrivial ("   a"),                   "nontrivial '   a' -> true");
  t.ok    (nontrivial ("a   "),                   "nontrivial 'a   ' -> true");
  t.ok    (nontrivial ("  \t\ta"),                "nontrivial '  \\t\\ta' -> true");
  t.ok    (nontrivial ("a\t\t  "),                "nontrivial 'a\\t\\t  ' -> true");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

