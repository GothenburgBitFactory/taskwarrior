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

#include <algorithm>
#include <iostream>
#include <text.h>
#include <Tree.h>

////////////////////////////////////////////////////////////////////////////////
//  - Tree, Branch and Node are synonymous.
//  - A Tree may contain any number of branches.
//  - A Branch may contain any number of name/value pairs, unique by name.
//  - The destructor will delete all branches recursively.
//  - Tree::enumerate is a snapshot, and is invalidated by modification.
//  - Branch sequence is preserved.
Tree::Tree (const std::string& name)
: _trunk (NULL)
, _name (name)
{
}

////////////////////////////////////////////////////////////////////////////////
Tree::~Tree ()
{
  for (std::vector <Tree*>::iterator i = _branches.begin ();
       i != _branches.end ();
       ++i)
    delete *i;
}

////////////////////////////////////////////////////////////////////////////////
Tree::Tree (const Tree& other)
{
  throw "Unimplemented Tree::Tree (Tree&)";
}

////////////////////////////////////////////////////////////////////////////////
Tree& Tree::operator= (const Tree& other)
{
  throw "Unimplemented Tree::operator= ()";
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Tree* Tree::addBranch (Tree* branch)
{
  if (! branch)
    throw "Failed to allocate memory for parse tree.";

  branch->_trunk = this;
  _branches.push_back (branch);
  return branch;
}

////////////////////////////////////////////////////////////////////////////////
void Tree::removeBranch (Tree* branch)
{
  for (std::vector <Tree*>::iterator i = _branches.begin ();
       i != _branches.end ();
       ++i)
  {
    if (*i == branch)
    {
      _branches.erase (i);
      delete branch;
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Tree::removeAllBranches ()
{
  for (std::vector <Tree*>::iterator i = _branches.begin ();
       i != _branches.end ();
       ++i)
  {
    delete *i;
  }

  _branches.clear ();
}

////////////////////////////////////////////////////////////////////////////////
void Tree::replaceBranch (Tree* from, Tree* to)
{
  for (unsigned int i = 0; i < _branches.size (); ++i)
  {
    if (_branches[i] == from)
    {
      to->_trunk = this;
      _branches[i] = to;
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
void Tree::attribute (const std::string& name, const std::string& value)
{
  _attributes[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
void Tree::attribute (const std::string& name, const int value)
{
  _attributes[name] = format (value);
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
void Tree::attribute (const std::string& name, const double value)
{
  _attributes[name] = format (value, 1, 8);
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
std::string Tree::attribute (const std::string& name)
{
  // Prevent autovivification.
  std::map<std::string, std::string>::iterator i = _attributes.find (name);
  if (i != _attributes.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Tree::removeAttribute (const std::string& name)
{
  _attributes.erase (name);
}

////////////////////////////////////////////////////////////////////////////////
// Recursively builds a list of Tree* objects, left to right, depth first. The
// reason for the depth-first enumeration is that a client may wish to traverse
// the tree and delete nodes.  With a depth-first iteration, this is a safe
// mechanism, and a node pointer will never be dereferenced after it has been
// deleted.
void Tree::enumerate (std::vector <Tree*>& all) const
{
  for (std::vector <Tree*>::const_iterator i = _branches.begin ();
       i != _branches.end ();
       ++i)
  {
    (*i)->enumerate (all);
    all.push_back (*i);
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Tree::hasTag (const std::string& tag) const
{
  if (std::find (_tags.begin (), _tags.end (), tag) != _tags.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Tree::tag (const std::string& tag)
{
  if (! hasTag (tag))
    _tags.push_back (tag);
}

////////////////////////////////////////////////////////////////////////////////
void Tree::unTag (const std::string& tag)
{
  std::vector <std::string>::iterator i;
  for (i = _tags.begin (); i != _tags.end (); ++i)
  {
    if (*i == tag)
    {
      _tags.erase (i);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
int Tree::countTags () const
{
  return _tags.size ();
}

////////////////////////////////////////////////////////////////////////////////
int Tree::count () const
{
  int total = 1; // this one.

  for (std::vector <Tree*>::const_iterator i = _branches.begin ();
       i != _branches.end ();
       ++i)
  {
    // Recurse and count the branches.
    total += (*i)->count ();
  }

  return total;
}

////////////////////////////////////////////////////////////////////////////////
Tree* Tree::find (const std::string& path)
{
  std::vector <std::string> elements;
  split (elements, path, '/');

  // Must start at the trunk.
  Tree* cursor = this;
  std::vector <std::string>::iterator it = elements.begin ();
  if (cursor->_name != *it)
    return NULL;

  // Perhaps the trunk is what is needed?
  if (elements.size () == 1)
    return this;

  // Now look for the next branch.
  for (++it; it != elements.end (); ++it)
  {
    bool found = false;

    // If the cursor has a branch that matches *it, proceed.
    std::vector <Tree*>::iterator i;
    for (i = cursor->_branches.begin (); i != cursor->_branches.end (); ++i)
    {
      if ((*i)->_name == *it)
      {
        cursor = *i;
        found = true;
        break;
      }
    }

    if (!found)
      return NULL;
  }

  return cursor;
}

////////////////////////////////////////////////////////////////////////////////
void Tree::dumpNode (Tree* t, int depth, std::stringstream& output)
{
  // Dump node
  for (int i = 0; i < depth; ++i)
    output << "  ";

  output
         // Useful for debugging tree node new/delete errors.
         // << std::hex << t << " "
         << "\033[1m" << t->_name << "\033[0m";

  // Dump attributes.
  std::string atts;

  std::map <std::string, std::string>::iterator a;
  for (a = t->_attributes.begin (); a != t->_attributes.end (); ++a)
  {
    if (a != t->_attributes.begin ())
      atts += " ";

    atts += a->first + "='\033[33m" + a->second + "\033[0m'";
  }

  if (atts.length ())
    output << " " << atts;

  // Dump tags.
  std::string tags;
  std::vector <std::string>::iterator tag;
  for (tag = t->_tags.begin (); tag != t->_tags.end (); ++tag)
  {
    if (tags.length ())
      tags += ' ';

         if (*tag == "BINARY")       tags += "\033[1;37;44m"           + *tag + "\033[0m";
    else if (*tag == "CMD")          tags += "\033[1;37;46m"           + *tag + "\033[0m";
    else if (*tag == "FILTER")       tags += "\033[1;37;42m"           + *tag + "\033[0m";
    else if (*tag == "MODIFICATION") tags += "\033[1;37;43m"           + *tag + "\033[0m";
    else if (*tag == "RC")           tags += "\033[1;37;41m"           + *tag + "\033[0m";
    else if (*tag == "CONFIG")       tags += "\033[1;37;101m"          + *tag + "\033[0m";
    else if (*tag == "?")            tags += "\033[38;5;255;48;5;232m" + *tag + "\033[0m";
    else                             tags += "\033[32m"                + *tag + "\033[0m";
  }

  if (tags.length ())
    output << ' ' << tags;
  output << "\n";

  // Recurse for branches.
  std::vector <Tree*>::iterator b;
  for (b = t->_branches.begin (); b != t->_branches.end (); ++b)
    dumpNode (*b, depth + 1, output);
}

////////////////////////////////////////////////////////////////////////////////
std::string Tree::dump ()
{
  std::stringstream output;
  output << "Tree (" << count () << " nodes)\n";
  dumpNode (this, 1, output);
  return output.str ();
}

////////////////////////////////////////////////////////////////////////////////

