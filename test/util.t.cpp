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

#include <cmake.h>
#include <iostream>
#include <main.h>
#include <util.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (40);

  // TODO bool confirm (const std::string&);
  // TODO int confirm3 (const std::string&);
  // TODO int confirm4 (const std::string&);
  // TODO void delay (float);

  // std::string formatBytes (size_t);
  t.is (formatBytes (0), "0 B", "0 -> 0 B");

  t.is (formatBytes (994),  "994 B", "994 -> 994 B");
  t.is (formatBytes (995),  "1.0 KiB", "995 -> 1.0 KiB");
  t.is (formatBytes (999),  "1.0 KiB", "999 -> 1.0 KiB");
  t.is (formatBytes (1000), "1.0 KiB", "1000 -> 1.0 KiB");
  t.is (formatBytes (1001), "1.0 KiB", "1001 -> 1.0 KiB");

  t.is (formatBytes (999999),  "1.0 MiB", "999999 -> 1.0 MiB");
  t.is (formatBytes (1000000), "1.0 MiB", "1000000 -> 1.0 MiB");
  t.is (formatBytes (1000001), "1.0 MiB", "1000001 -> 1.0 MiB");

  t.is (formatBytes (999999999),  "1.0 GiB", "999999999 -> 1.0 GiB");
  t.is (formatBytes (1000000000), "1.0 GiB", "1000000000 -> 1.0 GiB");
  t.is (formatBytes (1000000001), "1.0 GiB", "1000000001 -> 1.0 GiB");

  // TODO const std::string uuid ();

  // TODO These are in feedback.cpp, not util.cpp.
  // std::string taskDiff (const Task&, const Task&);
  Task left;
  left.set ("zero", "0");
  left.set ("one",  1);
  left.set ("two",  2);

  Task right;
  right.set ("zero",  "00");
  right.set ("two",   2);
  right.set ("three", 3);

  Task rightAgain (right);

  std::string output = taskDifferences (left, right);
  t.ok (taskDiff (left, right),                                                     "Detected changes");
  t.ok (output.find ("Zero will be changed from '0' to '00'") != std::string::npos, "Detected change zero:0 -> zero:00");
  t.ok (output.find ("One will be deleted")                   != std::string::npos, "Detected deletion one:1 ->");
  t.ok (output.find ("Two")                                   == std::string::npos, "Detected no change two:2 -> two:2");
  t.ok (output.find ("Three will be set to '3'")              != std::string::npos, "Detected addition -> three:3");

  t.notok (taskDiff (right, rightAgain),                                            "No changes detected");
  output = taskDifferences (right, rightAgain);
  t.ok (output.find ("No changes will be made")               != std::string::npos, "No changes detected");

  // void combine (std::vector <int>&, const std::vector <int>&);
  std::vector <int> vleft;
  vleft.push_back (1);
  vleft.push_back (2);
  vleft.push_back (3);

  std::vector <int> vright;
  vright.push_back (4);

  combine (vleft, vright);
  t.is (vleft.size (), (size_t)4, "1,2,3 + 4 -> [4]");
  t.is (vleft[0], 1,      "1,2,3 + 4 -> 1,2,3,4");
  t.is (vleft[1], 2,      "1,2,3 + 4 -> 1,2,3,4");
  t.is (vleft[2], 3,      "1,2,3 + 4 -> 1,2,3,4");
  t.is (vleft[3], 4,      "1,2,3 + 4 -> 1,2,3,4");

  vright.push_back (3);
  vright.push_back (5);
  combine (vleft, vright);

  t.is (vleft.size (), (size_t)5, "1,2,3,4 + 3,4,5 -> [5]");
  t.is (vleft[0], 1,      "1,2,3,4 + 3,4,5 -> 1,2,3,4,5");
  t.is (vleft[1], 2,      "1,2,3,4 + 3,4,5 -> 1,2,3,4,5");
  t.is (vleft[2], 3,      "1,2,3,4 + 3,4,5 -> 1,2,3,4,5");
  t.is (vleft[3], 4,      "1,2,3,4 + 3,4,5 -> 1,2,3,4,5");
  t.is (vleft[4], 5,      "1,2,3,4 + 3,4,5 -> 1,2,3,4,5");

  // see also indirect tests of indentProject and extractParents in `project.t'.
  // std::vector<std::string> indentTree (const std::vector<std::string>&, const std::string whitespace="  ", char delimiter='.');
  std::vector <std::string> flat;
  flat.push_back ("one");
  flat.push_back ("one.two");
  flat.push_back ("one.two.three");
  flat.push_back ("one.four");
  flat.push_back ("two");

  std::vector <std::string> structured = indentTree (flat, "  ", '.');
  t.is (structured.size (), (size_t) 5, "indentTree yields 5 strings");
  t.is (structured[0], "one",               "indentTree 'one'           -> 'one'");
  t.is (structured[1], "  two",         "indentTree 'one.two'       -> '  two'");
  t.is (structured[2], "    three", "indentTree 'one.two.three' -> '  three'");
  t.is (structured[3], "  four",        "indentTree 'one.four'      -> '  four'");
  t.is (structured[4], "two",               "indentTree 'two'           -> 'two'");

  // std::vector<std::string> indentProject (const std::string&, const std::string whitespace="  ", char delimiter='.');
  t.is (indentProject (""),              "",                  "indentProject '' -> ''");
  t.is (indentProject ("one"),           "one",               "indentProject 'one' -> 'one'");
  t.is (indentProject ("one.two"),       "  two",         "indentProject 'one.two' -> '  two'");
  t.is (indentProject ("one.two.three"), "    three", "indentProject 'one.two.three' -> '    three'");

  // TODO const std::string encode (const std::string& value);
  // TODO const std::string decode (const std::string& value);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

