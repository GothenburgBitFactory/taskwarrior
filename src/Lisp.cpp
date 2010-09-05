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
#include "Lisp.h"

////////////////////////////////////////////////////////////////////////////////
Lisp::Lisp ()
{
}

////////////////////////////////////////////////////////////////////////////////
Lisp::~Lisp ()
{
}

////////////////////////////////////////////////////////////////////////////////
Tree* Lisp::parse (const std::string& input)
{
  Tree* root = new Tree ("root");
  if (root)
  {
    Nibbler n (input);
    parseNode (root, n);
  }

  return root;
}

////////////////////////////////////////////////////////////////////////////////
// Grammar
//   node ::= '(' list ')'
//   list ::= item [[space item] ...]
//   item ::= word | node
void Lisp::parseNode (Tree* parent, Nibbler& n)
{
  // Work on a stack-based copy, to allow backtracking.
  Nibbler attempt (n);

  if (attempt.skip ('('))
  {
    Tree* b = new Tree ("");
    parent->addBranch (b);

    parseList (b, attempt);
    if (attempt.skip (')'))
    {
      n = attempt;
      return;
    }
  }

  throw std::string ("Error: malformed node");
}

////////////////////////////////////////////////////////////////////////////////
void Lisp::parseList (Tree* parent, Nibbler& n)
{
  // Work on a stack-based copy, to allow backtracking.
  Nibbler attempt (n);

  parseItem (parent, attempt);

  while (attempt.skip (' '))
    parseItem (parent, attempt);

  n = attempt;
}

////////////////////////////////////////////////////////////////////////////////
void Lisp::parseItem (Tree* parent, Nibbler& n)
{
  // Work on a stack-based copy, to allow backtracking.
  Nibbler attempt (n);

  if (attempt.next () == '(')
    parseNode (parent, attempt);
  else
    parseWord (parent, attempt);

  n = attempt;
  return;
}

////////////////////////////////////////////////////////////////////////////////
// A word is any group of non-whitespace followed by a space or ')'.
void Lisp::parseWord (Tree* parent, Nibbler& n)
{
  // Work on a stack-based copy, to allow backtracking.
  Nibbler attempt (n);

  std::string word;
  if (attempt.getUntilOneOf (" )", word))
  {
    parent->tag (word);
    n = attempt;
    return;
  }

  throw std::string ("Error: failed to parse word");
}

////////////////////////////////////////////////////////////////////////////////
