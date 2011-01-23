////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_TREE
#define INCLUDED_TREE

#include <map>
#include <vector>
#include <string>

class Tree;

class Tree
{
public:
  Tree (const std::string&);
  ~Tree ();
  Tree (const Tree&);
  Tree& operator= (const Tree&);
  Tree* operator[] (const int);

  void addBranch (Tree*);
  void removeBranch (Tree*);
  void replaceBranch (Tree*, Tree*);
  int branches ();

  void name (const std::string&);
  std::string name () const;
  void attribute (const std::string&, const std::string&);
  void attribute (const std::string&, const int);
  void attribute (const std::string&, const double);
  std::string attribute (const std::string&);
  void removeAttribute (const std::string&);
  int attributes () const;
  std::vector <std::string> allAttributes () const;

  bool hasTag (const std::string&) const;
  void tag (const std::string&);
  int tags () const;
  std::vector <std::string> allTags () const;

  void enumerate (std::vector <Tree*>& all) const;
  Tree* parent () const;

  int count () const;

  Tree* find (const std::string&);

  void dump ();

private:
  void dumpNode (Tree*, int);

private:
  Tree* _trunk;                                    // Parent.
  std::string _name;                               // Name.
  std::vector <Tree*> _branches;                   // Children.
  std::map <std::string, std::string> _attributes; // Attributes (name->value).
  std::vector <std::string> _tags;                 // Tags (tag, tag ...).
};

#endif

////////////////////////////////////////////////////////////////////////////////
