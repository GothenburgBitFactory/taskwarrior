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
#include "main.h"
#include "util.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (17);

  // TODO bool confirm (const std::string&);
  // TODO int confirm3 (const std::string&);
  // TODO void delay (float);
  // TODO std::string formatSeconds (time_t);
  // TODO std::string formatSecondsCompact (time_t);

  // std::string formatBytes (size_t);
  t.is (formatBytes (0), "0 B", "0 -> 0 B");

  t.is (formatBytes (999),  "999 B", "999 -> 999 B");
  t.is (formatBytes (1000), "1.0 KiB", "1000 -> 1.0 KiB");
  t.is (formatBytes (1001), "1.0 KiB", "1001 -> 1.0 KiB");

  t.is (formatBytes (999999),  "1.0 MiB", "999999 -> 1.0 MiB");
  t.is (formatBytes (1000000), "1.0 MiB", "1000000 -> 1.0 MiB");
  t.is (formatBytes (1000001), "1.0 MiB", "1000001 -> 1.0 MiB");

  t.is (formatBytes (999999999),  "1.0 GiB", "999999999 -> 1.0 GiB");
  t.is (formatBytes (1000000000), "1.0 GiB", "1000000000 -> 1.0 GiB");
  t.is (formatBytes (1000000001), "1.0 GiB", "1000000001 -> 1.0 GiB");

  // TODO const std::string uuid ();

  // std::string expandPath (const std::string&);
  t.ok (expandPath ("foo") == "foo", "expandPath nop");
  t.ok (expandPath ("~/")  != "~/",  "expandPath ~/");

  // TODO bool slurp (const std::string&, std::vector <std::string>&, bool trimLines = false);
  // TODO bool slurp (const std::string&, std::string&, bool trimLines = false);
  // TODO void spit (const std::string&, const std::string&);

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

  std::string output = taskDiff (left, right);
  t.ok (output.find ("zero was changed from '0' to '00'.\n") != std::string::npos, "Detected change zero:0 -> zero:00");
  t.ok (output.find ("one was deleted.\n")                   != std::string::npos, "Detected deletion one:1 ->");
  t.ok (output.find ("two")                                  == std::string::npos, "Detected no change two:2 -> two:2");
  t.ok (output.find ("three was set to '3'.\n")              != std::string::npos, "Detected addition -> three:3");

  output = taskDiff (right, rightAgain);
  t.ok (output.find ("No changes were made.\n")              != std::string::npos, "No changes detected");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

