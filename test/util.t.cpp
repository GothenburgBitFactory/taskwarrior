////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <main.h>
#include <util.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (30);

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

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

