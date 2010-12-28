////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include "test.h"
#include "Context.h"
#include "Lisp.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest test (6);

  // (one)
  // t -> "one" (no tags)
  //   -> no child nodes
  Lisp l;
  Tree* t = l.parse ("(one)");
  //t->dump ();

  test.is (t->branches (), 1,       "(one) -> 1 node under root");
  test.is ((*t)[0]->tags (), 1,     "(one) -> 1 tag");
  test.is ((*t)[0]->branches (), 0, "(one) -> 0 child nodes");
  delete t;

  // (one two)
  // t -> "one" (tag: "two")
  //   -> no child nodes
  t = l.parse ("(one two)");
  //t->dump ();

  test.is (t->branches (), 1,       "(one two) -> 1 node under root");
  test.is ((*t)[0]->tags (), 2,     "(one) -> 2 tags");
  test.is ((*t)[0]->branches (), 0, "(one two) -> 0 child nodes");
  delete t;

  return 0;
}

