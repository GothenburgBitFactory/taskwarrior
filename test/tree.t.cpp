////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <Tree.h>
#include <Context.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest ut (8);

  // Construct tree as shown above.
  Tree t ("root");

  Tree* b = t.addBranch (new Tree ("c1"));
  b->attribute ("name", "c1");
  b->tag ("tag");

  b = t.addBranch (new Tree ("c2"));
  b->attribute ("name", "c2");

  b = t.addBranch (new Tree ("c3"));
  b->attribute ("name", "c3");

  Tree* l = b->addBranch (new Tree ("c4"));
  l->attribute ("name", "c4");

  // Iterate over tree.
  ut.is (t._branches[0]->attribute ("name"), "c1", "c1");
  ut.is (t._branches[1]->attribute ("name"), "c2", "c2");
  ut.is (t._branches[2]->attribute ("name"), "c3", "c3");
  ut.is (t._branches[2]->_branches[0]->attribute ("name"), "c4", "c4");

  t._branches[2]->_branches[0]->tag ("one");
  t._branches[2]->_branches[0]->tag ("two");
  ut.ok    (t._branches[2]->_branches[0]->hasTag ("one"),   "hasTag +");
  ut.notok (t._branches[2]->_branches[0]->hasTag ("three"), "hasTag -");

  t._branches[2]->_branches[0]->unTag ("one");
  ut.notok (t._branches[2]->_branches[0]->hasTag ("one"),   "unTag");

  ut.is (t.count (), 5, "t.count");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

