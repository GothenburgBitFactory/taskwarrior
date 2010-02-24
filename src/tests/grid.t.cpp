////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include <Grid.h>
#include <test.h>

Context context;

int main (int argc, char** argv)
{
  UnitTest ut (30);

  Grid g;
  ut.is ((int) g.width (), 0, "Zero width for uninitialized grid");
  ut.is ((int) g.height (), 0, "Zero height for uninitialized grid");

  g.add (2, 2, false);
  ut.is ((int) g.width (), 3, "Width of 3 columns");
  ut.is ((int) g.height (), 3, "Height of 3 rows");

  Grid g2;
  g2.add (0, 1, "value");
  g2.add (1, 0, "value");
  ut.is ((int) g2.width (), 2, "Width of 2 columns");
  ut.is ((int) g2.height (), 2, "Height of 2 rows");
  ut.is (g2.byRow (0, 0), NULL, "Gap at 0,0");
  ut.ok (g2.byRow (0, 1), "Cell at 0,0");
  ut.ok (g2.byRow (1, 0), "Cell at 0,0");
  ut.is (g2.byRow (1, 1), NULL, "Gap at 1,1");

  Grid g3;
  for (int i = 0; i < 14; ++i)
    g3.add (i / 4, i % 4, "value");

  for (int i = 0; i < 20; ++i)
    if (i < 14)
      ut.ok (g3.byRow (i / 4, i % 4), "g3 good cell");
    else
      ut.is (g3.byRow (i / 4, i % 4), NULL, "g3 missing cell");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
