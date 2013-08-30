////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#include <iostream>
#include <A3t.h>
#include <text.h>
#include <util.h>

static const int minimumMatchLength = 3;

////////////////////////////////////////////////////////////////////////////////
A3t::A3t (int argc, char** argv)
{
  _tree = new Tree ("root");
  if (! _tree)
    throw std::string ("Failed to allocate memory for parse tree.");

  for (int i = 0; i < argc; ++i)
  {
    Tree* branch = _tree->addBranch (new Tree (format ("arg{1}", i)));
    branch->attribute ("raw", argv[i]);
    branch->tag ("ORIGINAL");
  }
}

////////////////////////////////////////////////////////////////////////////////
A3t::~A3t ()
{
}

////////////////////////////////////////////////////////////////////////////////
Tree* A3t::parse ()
{
  findCommand ();

  return _tree;
}

////////////////////////////////////////////////////////////////////////////////
void A3t::identity (const std::string& name, const std::string& value)
{
  _entities.insert (std::pair <std::string, std::string> (name, value));
}

////////////////////////////////////////////////////////////////////////////////
// Search for 'value' in _entities, return category and canonicalized value.
bool A3t::canonicalize (
  std::string& canonicalized,
  const std::string& category,
  const std::string& value) const
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range (category);

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
    options.push_back (e->second);

  // Match against the options, throw away results.
  std::vector <std::string> matches;
  if (autoComplete (value, options, matches, minimumMatchLength) == 1)
  {
//    for (auto& i: matches)
//      std::cout << "match: " << i << "\n";

    canonicalized = matches[0];
    return true;
  }


  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Walk the top-level tree branches, looking for the first raw value that
// autoCompletes to a valid command/report.
void A3t::findCommand ()
{
  std::string command;
  for (int i = 0; i < _tree->branches (); ++i)
  {
    if (canonicalize (command, "report",  (*_tree)[i]->attribute ("raw")))
    {
      (*_tree)[i]->attribute ("canonical", command);
      (*_tree)[i]->tag ("REPORT");
      (*_tree)[i]->tag ("CMD");
    }

    else if (canonicalize (command, "readcmd",  (*_tree)[i]->attribute ("raw")))
    {
      (*_tree)[i]->attribute ("canonical", command);
      (*_tree)[i]->tag ("READCMD");
      (*_tree)[i]->tag ("CMD");
    }

    else if (canonicalize (command, "writecmd",  (*_tree)[i]->attribute ("raw")))
    {
      (*_tree)[i]->attribute ("canonical", command);
      (*_tree)[i]->tag ("WRITECMD");
      (*_tree)[i]->tag ("CMD");
    }

    else if (canonicalize (command, "specialcmd",  (*_tree)[i]->attribute ("raw")))
    {
      (*_tree)[i]->attribute ("canonical", command);
      (*_tree)[i]->tag ("SPECIALCMD");
      (*_tree)[i]->tag ("CMD");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
