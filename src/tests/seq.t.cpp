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
#include <Sequence.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
Sequence parseSequence (const std::string& input)
{
  try { Sequence s (input); return s; }
  catch (...) {}
  return Sequence ();
}

int main (int argc, char** argv)
{
  UnitTest t (18);

  // 1
  Sequence seq = parseSequence ("1");
  t.is (seq.size (), (size_t)1, "seq '1' -> 1 item");
  if (seq.size () == 1)
  {
    t.is (seq[0], 1, "seq '1' -> [0] == 1");
  }
  else
  {
    t.fail ("seq '1' -> [0] == 1");
  }

  // 1,3
  seq = parseSequence ("1,3");
  t.is (seq.size (), (size_t)2, "seq '1,3' -> 2 items");
  if (seq.size () == 2)
  {
    t.is (seq[0], 1, "seq '1,3' -> [0] == 1");
    t.is (seq[1], 3, "seq '1,3' -> [1] == 3");
  }
  else
  {
    t.fail ("seq '1,3' -> [0] == 1");
    t.fail ("seq '1,3' -> [1] == 3");
  }

  // 1,3-5,7
  seq = parseSequence ("1,3-5,7");
  t.is (seq.size (), (size_t)5, "seq '1,3-5,7' -> 5 items");
  if (seq.size () == 5)
  {
    t.is (seq[0], 1, "seq '1,3-5,7' -> [0] == 1");
    t.is (seq[1], 3, "seq '1,3-5,7' -> [1] == 3");
    t.is (seq[2], 4, "seq '1,3-5,7' -> [2] == 4");
    t.is (seq[3], 5, "seq '1,3-5,7' -> [3] == 5");
    t.is (seq[4], 7, "seq '1,3-5,7' -> [4] == 7");
  }
  else
  {
    t.fail ("seq '1,3-5,7' -> [0] == 1");
    t.fail ("seq '1,3-5,7' -> [1] == 3");
    t.fail ("seq '1,3-5,7' -> [2] == 4");
    t.fail ("seq '1,3-5,7' -> [3] == 5");
    t.fail ("seq '1,3-5,7' -> [4] == 7");
  }

  // 1--2
  seq = parseSequence ("1--2");
  t.is (seq.size (), (size_t)0, "seq '1--2' -> 0 items (error)");

  // 1-1000
  seq = parseSequence ("1-1000");
  t.is (seq.size (), (size_t)1000, "seq '1-1000' -> 1000 items");
  if (seq.size () == 1000)
  {
    t.is (seq[0],      1, "seq '1-1000' -> [0] == 1");
    t.is (seq[1],      2, "seq '1-1000' -> [1] == 3");
    t.is (seq[998],  999, "seq '1-1000' -> [998] == 999");
    t.is (seq[999], 1000, "seq '1-1000' -> [999] == 1000");
  }
  else
  {
    t.fail ("seq '1-1000' -> [0] == 1");
    t.fail ("seq '1-1000' -> [1] == 2");
    t.fail ("seq '1-1000' -> [998] == 999");
    t.fail ("seq '1-1000' -> [999] == 1000");
  }

  // 1-1001
  seq = parseSequence ("1-1001");
  t.is (seq.size (), (size_t)0, "seq '1-1001' -> 0 items (error)");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
