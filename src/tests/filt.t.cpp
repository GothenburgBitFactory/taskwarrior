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
#include "main.h"
#include "test.h"
#include "Filter.h"
#include "T2.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (14);

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
  test.notok (f.pass (no1), "no match against mismatch T2");

  T2 partial;
  partial.set ("name1", "value1");
  test.notok (f.pass (partial), "no match against partial T2");

  // Modifiers.
  T2 mods;
  mods.set ("name", "value");

  Att a ("name", "value");
  a.addMod ("is");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.is:value = match");
  // TODO test inverse.

  a = Att ("name", "value");
  a.addMod ("isnt");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "name:value -> name.isnt:value = no match");
  // TODO test inverse.

  a = Att ("name", "val");
  a.addMod ("startswith");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.startswith:val = match");
  // TODO test inverse.

  a = Att ("name", "lue");
  a.addMod ("endswith");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.endswith:lue = match");
  // TODO test inverse.

  a = Att ("name", "value");
  a.addMod ("has");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.has:alu = match");
  // TODO test inverse.

  a = Att ("name", "value");
  a.addMod ("hasnt");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "name:value -> name.hasnt:alu = no match");
  // TODO test inverse.

  a = Att ("name", "");
  a.addMod ("any");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.any: = match");
  // TODO test inverse.

  a = Att ("name", "");
  a.addMod ("none");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "name:value -> name.none: = no match");
  // TODO test inverse.

/*
"before"
"after"
"not"
"synth"
"under"
"over"
"first"
"last"
"this"
"next"
*/

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

