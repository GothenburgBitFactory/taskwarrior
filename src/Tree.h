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
#ifndef INCLUDED_TREE
#define INCLUDED_TREE

#include <map>
#include <vector>
#include <string>
#include <sstream>

class Tree;

class Tree
{
public:
  Tree (const std::string&);
  ~Tree ();
  Tree (const Tree&);
  Tree& operator= (const Tree&);

  Tree* addBranch (Tree*);
  void removeBranch (Tree*);
  void replaceBranch (Tree*, Tree*);

  void attribute (const std::string&, const std::string&);
  void attribute (const std::string&, const int);
  void attribute (const std::string&, const double);
  std::string attribute (const std::string&);
  void removeAttribute (const std::string&);

  void enumerate (std::vector <Tree*>& all) const;

  bool hasTag (const std::string&) const;
  void tag (const std::string&);
  void unTag (const std::string&);

  int count () const;

  Tree* find (const std::string&);

  std::string dump ();

private:
  void dumpNode (Tree*, int, std::stringstream&);

public:
  Tree* _trunk;                                    // Parent.
  std::string _name;                               // Name.
  std::vector <Tree*> _branches;                   // Children.
  std::map <std::string, std::string> _attributes; // Attributes (name->value).
  std::vector <std::string> _tags;                 // Tags (tag, tag ...).
};

#endif

////////////////////////////////////////////////////////////////////////////////
