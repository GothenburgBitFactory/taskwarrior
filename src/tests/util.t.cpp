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
  UnitTest t (74);

  // TODO bool confirm (const std::string&);
  // TODO int confirm3 (const std::string&);
  // TODO int confirm4 (const std::string&);
  // TODO void delay (float);

  // std::string formatSeconds (time_t);
  t.is (formatSeconds (0),               "-",       "0 -> -");
  t.is (formatSeconds (1),               "1 sec",   "1 -> 1 sec");
  t.is (formatSeconds (2),               "2 secs",  "2 -> 2 secs");
  t.is (formatSeconds (59),              "59 secs", "59 -> 59 secs");
  t.is (formatSeconds (60),              "1 min",   "60 -> 1 min");
  t.is (formatSeconds (119),             "1 min",   "119 -> 1 min");
  t.is (formatSeconds (120),             "2 mins",  "120 -> 2 mins");
  t.is (formatSeconds (121),             "2 mins",  "121 -> 2 mins");
  t.is (formatSeconds (3599),            "59 mins", "3599 -> 59 mins");
  t.is (formatSeconds (3600),            "1 hr",    "3600 -> 1 hr");
  t.is (formatSeconds (3601),            "1 hr",    "3601 -> 1 hr");
  t.is (formatSeconds (86399),           "23 hrs",  "86399 -> 23 hrs");
  t.is (formatSeconds (86400),           "1 day",   "86400 -> 1 day");
  t.is (formatSeconds (86401),           "1 day",   "86401 -> 1 day");
  t.is (formatSeconds (14 * 86400 - 1),  "1 wk",    "14 days - 1 sec -> 1 wk");
  t.is (formatSeconds (14 * 86400),      "2 wks",   "14 days -> 2 wks");
  t.is (formatSeconds (14 * 86400 + 1),  "2 wks",   "14 days + 1 sec -> 2 wks");
  t.is (formatSeconds (85 * 86400 - 1),  "2 mths",  "85 days - 1 sec -> 2 mths");
  t.is (formatSeconds (85 * 86400),      "2 mths",  "85 days -> 2 mths");
  t.is (formatSeconds (85 * 86400 + 1),  "2 mths",  "85 days + 1 sec -> 2 mths");
  t.is (formatSeconds (365 * 86400 - 1), "1.0 yrs", "365 days - 1 sec -> 1.0 yrs");
  t.is (formatSeconds (365 * 86400),     "1.0 yrs", "365 days -> 1.0 yrs");
  t.is (formatSeconds (365 * 86400 + 1), "1.0 yrs", "365 days + 1 sec -> 1.0 yrs");

  // std::string formatSecondsCompact (time_t);
  t.is (formatSecondsCompact (0),               "-",    "0 -> -");
  t.is (formatSecondsCompact (1),               "1s",   "1 -> 1s");
  t.is (formatSecondsCompact (2),               "2s",   "2 -> 2s");
  t.is (formatSecondsCompact (59),              "59s",  "59 -> 59s");
  t.is (formatSecondsCompact (60),              "1m",   "60 -> 1m");
  t.is (formatSecondsCompact (119),             "1m",   "119 -> 1m");
  t.is (formatSecondsCompact (120),             "2m",   "120 -> 2m");
  t.is (formatSecondsCompact (121),             "2m",   "121 -> 2m");
  t.is (formatSecondsCompact (3599),            "59m",  "3599 -> 59m");
  t.is (formatSecondsCompact (3600),            "1h",   "3600 -> 1h");
  t.is (formatSecondsCompact (3601),            "1h",   "3601 -> 1h");
  t.is (formatSecondsCompact (86399),           "23h",  "86399 -> 23h");
  t.is (formatSecondsCompact (86400),           "1d",   "86400 -> 1d");
  t.is (formatSecondsCompact (86401),           "1d",   "86401 -> 1d");
  t.is (formatSecondsCompact (14 * 86400 - 1),  "1wk",  "14 days - 1 sec -> 1wk");
  t.is (formatSecondsCompact (14 * 86400),      "2wk",  "14 days -> 2wk");
  t.is (formatSecondsCompact (14 * 86400 + 1),  "2wk",  "14 days + 1 sec -> 2wk");
  t.is (formatSecondsCompact (85 * 86400 - 1),  "2mo",  "85 days - 1 sec -> 2mo");
  t.is (formatSecondsCompact (85 * 86400),      "2mo",  "85 days -> 2mo");
  t.is (formatSecondsCompact (85 * 86400 + 1),  "2mo",  "85 days + 1 sec -> 2mo");
  t.is (formatSecondsCompact (365 * 86400 - 1), "1.0y", "365 days - 1 sec -> 1.0y");
  t.is (formatSecondsCompact (365 * 86400),     "1.0y", "365 days -> 1.0y");
  t.is (formatSecondsCompact (365 * 86400 + 1), "1.0y", "365 days + 1 sec -> 1.0y");

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

  // std::string expandPath (const std::string&);
  t.ok (expandPath ("foo") == "foo", "expandPath nop");
  t.ok (expandPath ("~/")  != "~/",  "expandPath ~/");
  t.ok (expandPath ("~")   != "~",   "expandPath ~");

  // bool isAbsolutePath (const std::string&);
  t.notok (isAbsolutePath ("."),               "isAbsolutePath .");
  t.notok (isAbsolutePath ("~"),               "isAbsolutePath ~");
  t.ok    (isAbsolutePath (expandPath ("~")),  "isAbsolutePath (expandPath ~)");
  t.ok    (isAbsolutePath (expandPath ("~/")), "isAbsolutePath (expandPath ~/)");
  t.ok    (isAbsolutePath ("/"),               "isAbsolutePath /");
  t.ok    (isAbsolutePath ("/tmp"),            "isAbsolutePath /tmp");

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

  std::string output = taskDifferences (left, right);
  t.ok (taskDiff (left, right),                                                     "Detected changes");
  t.ok (output.find ("zero will be changed from '0' to '00'") != std::string::npos, "Detected change zero:0 -> zero:00");
  t.ok (output.find ("one will be deleted")                   != std::string::npos, "Detected deletion one:1 ->");
  t.ok (output.find ("two")                                   == std::string::npos, "Detected no change two:2 -> two:2");
  t.ok (output.find ("three will be set to '3'")              != std::string::npos, "Detected addition -> three:3");

  t.notok (taskDiff (right, rightAgain),                                            "No changes detected");
  output = taskDifferences (right, rightAgain);
  t.ok (output.find ("No changes will be made")               != std::string::npos, "No changes detected");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

