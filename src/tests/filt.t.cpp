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
#include "Filter.h"
#include "T2.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (6);

  // Create a filter consisting of two Att criteria.
  Filter f;
  f.push_back (Att ("name1", "value1"));
  f.push_back (Att ("name2", "value2"));
  test.is (f.size (), (size_t)2, "Filter created");

  // Create a T2 to match against.
  T2 yes;
  yes.set ("name1", "value1");
  yes.set ("name2", "value2");
  test.ok (f.pass (yes), "full match");

  yes.set ("name3", "value3");
  test.ok (f.pass (yes), "over match");

  // Negative tests.
  T2 no0;
  test.notok (f.pass (no0), "no match against default T2");

  T2 no1;
  no1.set ("name3", "value3");
  test.notok (f.pass (no0), "no match against mismatch T2");

  T2 partial;
  partial.set ("name1", "value1");
  test.notok (f.pass (no0), "no match against partial T2");

  // TODO Modifiers.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

