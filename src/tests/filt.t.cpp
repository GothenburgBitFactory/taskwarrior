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
#include "main.h"
#include "test.h"
#include "Filter.h"
#include "Task.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (20);

  // Create a filter consisting of two Att criteria.
  Filter f;
  f.push_back (Att ("name1", "value1"));
  f.push_back (Att ("name2", "value2"));
  test.is (f.size (), (size_t)2, "Filter created");

  // Create a Task to match against.
  Task yes;
  yes.set ("name1", "value1");
  yes.set ("name2", "value2");
  test.ok (f.pass (yes), "full match");

  yes.set ("name3", "value3");
  test.ok (f.pass (yes), "over match");

  // Negative tests.
  Task no0;
  test.notok (f.pass (no0), "no match against default Task");

  Task no1;
  no1.set ("name3", "value3");
  test.notok (f.pass (no1), "no match against mismatch Task");

  Task partial;
  partial.set ("name1", "value1");
  test.notok (f.pass (partial), "no match against partial Task");

  // Modifiers.
  Task mods;
  mods.set ("name", "value");
  mods.set ("description", "hello, world.");

  Att a ("name", "is", "value");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.is:value = match");
  // TODO test inverse.

  a = Att ("name", "isnt", "value");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "name:value -> name.isnt:value = no match");
  // TODO test inverse.

  a = Att ("name", "startswith", "val");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.startswith:val = match");
  // TODO test inverse.

  a = Att ("name", "endswith", "lue");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.endswith:lue = match");
  // TODO test inverse.

  a = Att ("name", "has", "alu");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.has:alu = match");
  // TODO test inverse.

  a = Att ("name", "hasnt", "alu");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "name:value -> name.hasnt:alu = no match");
  // TODO test inverse.

  a = Att ("name", "any", "");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "name:value -> name.any: = match");
  // TODO test inverse.

  a = Att ("name", "none", "");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "name:value -> name.none: = no match");
  // TODO test inverse.

/*
"before"
"after"
"under"
"over"
"above"
"below"
*/

  a = Att ("description", "word", "hello");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "description:hello, world. -> description.word:hello = match");
  // TODO test inverse.

  a = Att ("description", "word", "world");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "description:hello, world. -> description.word:world = match");
  // TODO test inverse.

  a = Att ("description", "word", "pig");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "description:hello, world. -> description.word:pig = no match");
  // TODO test inverse.

  a = Att ("description", "noword", "hello");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "description:hello, world. -> description.noword:hello = no match");
  // TODO test inverse.

  a = Att ("description", "noword", "world");
  f.clear ();
  f.push_back (a);
  test.notok (f.pass (mods), "description:hello, world. -> description.noword:world = no match");
  // TODO test inverse.

  a = Att ("description", "noword", "pig");
  f.clear ();
  f.push_back (a);
  test.ok (f.pass (mods), "description:hello, world. -> description.noword:pig = match");
  // TODO test inverse.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

