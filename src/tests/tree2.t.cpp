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
#include "Context.h"
#include "Tree.h"
#include "test.h"

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest ut (30);

  // Create the following tree:
  //             t
  //             |
  //   +---+---+-+-+---+---+
  //   |   |   |   |   |   |
  //   a   b   c   d   e   f

  // Create a tree.
  Tree t ("");

  // Create six branches.
  Tree* a = new Tree (""); a->attribute ("name", "a");
  Tree* b = new Tree (""); b->attribute ("name", "b");
  Tree* c = new Tree (""); c->attribute ("name", "c");
  Tree* d = new Tree (""); d->attribute ("name", "d");
  Tree* e = new Tree (""); e->attribute ("name", "e");
  Tree* f = new Tree (""); f->attribute ("name", "f");

  // Create two branches.
  Tree* x = new Tree (""); x->attribute ("name", "x");
  Tree* y = new Tree (""); y->attribute ("name", "y");

  // Add the six.
  t.addBranch (a);
  t.addBranch (b);
  t.addBranch (c);
  t.addBranch (d);
  t.addBranch (e);
  t.addBranch (f);

  // Verify tree structure.
  ut.ok (a->parent () == &t, "a -> t");
  ut.ok (b->parent () == &t, "b -> t");
  ut.ok (c->parent () == &t, "c -> t");
  ut.ok (d->parent () == &t, "d -> t");
  ut.ok (e->parent () == &t, "e -> t");
  ut.ok (f->parent () == &t, "f -> t");
  ut.ok (x->parent () == NULL, "x -> NULL");
  ut.ok (y->parent () == NULL, "y -> NULL");

  ut.ok (t.branches () == 6, "t[6]");

  ut.diag ("---------------------------------------------------------");

  // Modify the tree to become:
  //         t
  //         |
  //   +---+-+-+---+
  //   |   |   |   |
  //   a   b   x   f
  //           |
  //       +---+---+
  //       |   |   |
  //       c   d   e    

  // Make x the parent of c, d and e.
  x->addBranch (c);
  x->addBranch (d);
  x->addBranch (e);

  // Make x replace c as one of t's branches.
  t.replaceBranch (c, x);
  t.removeBranch (d);
  t.removeBranch (e);

  // Verify structure.
  ut.ok (a->parent () == &t, "a -> t");
  ut.ok (b->parent () == &t, "b -> t");
  ut.ok (c->parent () == x, "c -> x");
  ut.ok (d->parent () == x, "d -> x");
  ut.ok (e->parent () == x, "e -> x");
  ut.ok (f->parent () == &t, "f -> t");
  ut.ok (x->parent () == &t, "x -> t");
  ut.ok (y->parent () == NULL, "y -> NULL");

  ut.ok (t.branches () == 4, "t[4]");
  ut.ok (x->branches () == 3, "x[3]");

  ut.diag ("---------------------------------------------------------");

  // Modify the tree to become:
  //       t
  //       |
  //   +---+---+
  //   |   |   |
  //   a   y   f
  //       |
  //     +-+-+
  //     |   |
  //     b   x
  //         |
  //     +---+---+
  //     |   |   |
  //     c   d   e    

  // Now insert y to be parent of b, x.
  y->addBranch (b);
  y->addBranch (x);
  t.replaceBranch (x, y);
  t.removeBranch (b);

  ut.ok (a->parent () == &t, "a -> t");
  ut.ok (b->parent () == y, "b -> y");
  ut.ok (c->parent () == x, "c -> x");
  ut.ok (d->parent () == x, "d -> x");
  ut.ok (e->parent () == x, "e -> x");
  ut.ok (f->parent () == &t, "f -> t");
  ut.ok (x->parent () == y, "x -> y");
  ut.ok (y->parent () == &t, "y -> t");

  ut.ok (t.branches () == 3, "t[3]");
  ut.ok (x->branches () == 3, "x[3]");
  ut.ok (y->branches () == 2, "y[2]");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

