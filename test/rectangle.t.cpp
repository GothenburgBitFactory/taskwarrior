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
#include "Context.h"
#include "Rectangle.h"
#include "text.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (34);

  // . . . . . .
  // . 0 0 0 . .
  // . 0 0 0 . .
  // . . . . . .
  // . . . . . .
  // . . . . . .
  Rectangle r0 (1, 1, 3, 2);

  // . . . . . .
  // . . . . . .
  // . . . 1 1 .
  // . . . 1 1 .
  // . . . . . .
  // . . . . . .
  Rectangle r1 (3, 2, 2, 2);

  // . . 2 . . .
  // . . 2 . . .
  // . . 2 . . .
  // . . 2 . . .
  // . . 2 . . .
  // . . 2 . . .
  Rectangle r2 (2, 0, 1, 6);

  // . . . . . .
  // . 3 . . . .
  // . . . . . .
  // . . . . . .
  // . . . . . .
  // . . . . . .
  Rectangle r3 (1, 1, 1, 1);

  // . . . . . .
  // . 4 4 4 . .
  // . 4 4 4 . .
  // . . . . . .
  // . . . . . .
  // . . . . . .
  Rectangle r4 (1, 1, 3, 2);

  // 5 5 5 5 5 5
  // 5 5 5 5 5 5
  // 5 5 5 5 5 5
  // 5 5 5 5 5 5
  // 5 5 5 5 5 5
  // 5 5 5 5 5 5
  Rectangle r5 (0, 0, 6, 6);

  // . . . . . .
  // . . . . . .
  // . . . . . .
  // 6 6 . . . .
  // . . . . . .
  // . . . . . .
  Rectangle r6 (0, 3, 2, 1);

  // . . . . . .
  // . . . . . .
  // . . . . . .
  // . . . . . .
  // . . . . 7 7
  // . . . . 7 7
  Rectangle r7 (4, 4, 2, 2);

  // . . . . . .
  // . . . . . .
  // 8 8 . . . .
  // 8 8 . . . .
  // . . . . . .
  // . . . . . .
  Rectangle r8 (0, 2, 2, 2);

  t.ok (r0.intersects (r1),  "r0.intersects (r1)");
  t.ok (r0.intersects (r2),  "r0.intersects (r2)");
  t.ok (r0.intersects (r3),  "r0.intersects (r3)");
  t.ok (r0.intersects (r4),  "r0.intersects (r4)");
  t.ok (r0.intersects (r5),  "r0.intersects (r5)");
  t.ok (!r0.intersects (r6), "!r0.intersects (r6)");
  t.ok (!r0.intersects (r7), "!r0.intersects (r7)");
  t.ok (r0.intersects (r8),  "r0.intersects (r8)");

  t.ok (r1.intersects (r0),  "r1.intersects (r0)");
  t.ok (r2.intersects (r0),  "r2.intersects (r0)");
  t.ok (r3.intersects (r0),  "r3.intersects (r0)");
  t.ok (r4.intersects (r0),  "r4.intersects (r0)");
  t.ok (r5.intersects (r0),  "r5.intersects (r0)");
  t.ok (!r6.intersects (r0), "!r6.intersects (r0)");
  t.ok (!r7.intersects (r0), "!r8.intersects (r0)");
  t.ok (r8.intersects (r0),  "r8.intersects (r0)");

  // 2:0,0,4,12 does not overlap 1:0,10,12,4
  Rectangle rBug1 (0, 0, 4, 12);
  Rectangle rBug2 (0, 10, 12, 4);
  t.ok (rBug1.intersects (rBug2), "rBug1.intersects (rBug2)");
  t.ok (rBug2.intersects (rBug1), "rBug2.intersects (rBug1)");

  t.ok (r5.contains (r0),      "r5.contains (r0)");
  t.ok (r5.contains (r1),      "r5.contains (r1)");
  t.ok (r5.contains (r2),      "r5.contains (r2)");
  t.ok (r5.contains (r3),      "r5.contains (r3)");
  t.ok (r5.contains (r4),      "r5.contains (r4)");
  t.ok (r5.contains (r6),      "r5.contains (r6)");
  t.ok (r5.contains (r7),      "r5.contains (r7)");
  t.ok (r5.contains (r8),      "r5.contains (r8)");
  t.ok (r0.contains (r3),      "r0.contains (r3)");
  t.ok (!r0.contains (r5),     "!r0.contains (r5)");

  t.ok (r0 == r4,              "r0 == r4");
  t.ok (r0 != r1,              "r0 != r1");

  Rectangle rX = r0;
  t.ok (rX == r0,              "rX == r0");

  Rectangle rY (r0);
  t.ok (rY == r0,              "rY == r0");

  t.notok (r0.adjacentTo (r1), "r0 not adjacent to r1");
  t.ok    (r1.adjacentTo (r2), "r1 is adjacent to r2");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

