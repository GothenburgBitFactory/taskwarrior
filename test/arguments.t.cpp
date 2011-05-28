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
#include <Context.h>
#include <Arguments.h>
#include <text.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (14);

  const char* fake[] =
  {
    // task list proj:foo 1,3,5-10 12 pattern1 rc.name1=value1 rc.name2:value2 \
    //   234 -- pattern2 n:v
    "task",                // context.program
    "list",                // command
    "proj:foo",            // n:v
    "1,3,5-10",            // sequence
    "12",                  // sequence
    "pattern1",            // text
    "rc.name1=value1",     // n:v
    "rc.name2:value2",     // n:v
    "234",                 // text
    "--",                  // terminator
    "pattern2",            // text
    "n:v",                 // text (due to terminator)
  };

  // void capture (int, char**);
  Arguments a1;
  a1.capture (12, &fake[0]);
  t.is (a1.size (), (size_t)11, "11 arguments expected");
  t.is (a1[0], "list",          "Arguments properly strips argv[0]");

  // std::string combine ();
  t.is (a1.combine (),
        "list proj:foo 1,3,5-10 12 pattern1 rc.name1=value1 rc.name2:value2 "
        "234 -- pattern2 n:v",
        "combine good");

  // TODO void append_stdin ();
  // TODO void rc_override (std::string&, File&, std::string&);
  // TODO void get_data_location (std::string&);
  // TODO void apply_overrides (std::string&);
  // TODO void resolve_aliases ();
  // TODO bool extract_command (const std::vector <std::string>&, std::string&);

  // void extract_sequence (std::vector <int>&);
  std::vector <int> sequence;
  a1.extract_sequence (sequence);
  size_t s = sequence.size ();
  t.is (s, (size_t)9, "1,3,5-10 12 --> 1,3,5,6,7,8,9,10,12 == 9");

  if (s > 0) t.is (sequence[0],  1, "sequence 1");  else t.fail ("sequence 1");
  if (s > 1) t.is (sequence[1],  3, "sequence 3");  else t.fail ("sequence 3");
  if (s > 2) t.is (sequence[2],  5, "sequence 5");  else t.fail ("sequence 5");
  if (s > 3) t.is (sequence[3],  6, "sequence 6");  else t.fail ("sequence 6");
  if (s > 4) t.is (sequence[4],  7, "sequence 7");  else t.fail ("sequence 7");
  if (s > 5) t.is (sequence[5],  8, "sequence 8");  else t.fail ("sequence 8");
  if (s > 6) t.is (sequence[6],  9, "sequence 9");  else t.fail ("sequence 9");
  if (s > 7) t.is (sequence[7], 10, "sequence 10"); else t.fail ("sequence 10");
  if (s > 8) t.is (sequence[8], 12, "sequence 12"); else t.fail ("sequence 12");

  t.is (a1.size (), (size_t)9, "a1 - <sequence> = 9 args");

  // TODO void extract_uuids (std::vector <std::string>&);
  // TODO void extract_filter ();
  // TODO void extract_modifications ();
  // TODO void extract_text ();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

