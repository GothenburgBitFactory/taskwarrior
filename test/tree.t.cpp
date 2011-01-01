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
#include "Tree.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest ut (8);

  // Construct tree as shown above.
  Tree t ("");

  Tree* b = new Tree ("");
  b->attribute ("name", "c1");
  b->tag ("tag");
  t.addBranch (b);

  b = new Tree ("");
  b->attribute ("name", "c2");
  t.addBranch (b);

  b = new Tree ("");
  b->attribute ("name", "c3");
  t.addBranch (b);

  Tree* l = new Tree ("");
  l->attribute ("name", "c4");

  b->addBranch (l);

  // Iterate over tree.
  std::vector <Tree*> all;
  t.enumerate (all);
  ut.is (all[0]->attribute ("name"), "c1", "c1");
  ut.is (all[1]->attribute ("name"), "c2", "c2");
  ut.is (all[2]->attribute ("name"), "c4", "c4");
  ut.is (all[3]->attribute ("name"), "c3", "c3");

  all[3]->tag ("one");
  all[3]->tag ("two");
  ut.ok    (all[3]->hasTag ("one"),   "hasTag +");
  ut.notok (all[3]->hasTag ("three"), "hasTag -");

  ut.is (t.count (), 5, "t.count");

  all.clear ();
  b->enumerate (all);
  ut.is (all[0]->attribute ("name"), "c4", "t -> c3 -> c4");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

