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
#include "task.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (3);

  T2 t;
  t.addTag ("tag1");
  t.addTag ("tag2");
  test.is (t.composeF4 (), "[tag:\"tag1&commatag2\" uuid:\"...\"]", "T2::addTag");

  T2 t2 (t.composeF4 ());
  test.is (t2.composeF4 (), "[tag:\"tag1&commatag2\" uuid:\"...\"]", "T2::composeF4 -> parse round trip");

  // Round-trip testing.
  T2 t3;
  std::string before = t3.composeF4 ();
/*
  t3 (t3.composeF4 ());
  t3 (t3.composeF4 ());
  t3 (t3.composeF4 ());
*/
  std::string after = t3.composeF4 ();
  test.is (before, after, "T2::composeF4 -> parse round trip 4 iterations");

/*

T2::composeCSV
T2::id
T2::*Status
T2::*Tag*
T2::*Annotation*

*/

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

