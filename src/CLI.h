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
#ifndef INCLUDED_CLI
#define INCLUDED_CLI

#include <string>
#include <vector>
#include <map>

// Represents a single argument.
class A
{
public:
  A (const std::string&);
  ~A ();
  A (const A&);
  A& operator= (const A&);
  bool hasTag (const std::string&) const;
  void tag (const std::string&);
  void unTag (const std::string&);
  void attribute (const std::string&, const std::string&);
  void attribute (const std::string&, const int);
  void attribute (const std::string&, const double);
  const std::string attribute (const std::string&);
  void removeAttribute (const std::string&);
  const std::string dump ();

public:
  std::string                         _name;
  std::vector <std::string>           _tags;
  std::map <std::string, std::string> _attributes;
};

// Represents the command line.
class CLI
{
public:
  CLI ();
  ~CLI ();
  void alias (const std::string&, const std::string&);
  void entity (const std::string&, const std::string&);
  void initialize (int, const char**);
  void add (const std::string&);
  const std::string getFilter ();
  const std::vector <std::string> getWords ();
  const std::vector <std::string> getModifications ();

private:
  void aliasExpansion ();
  void extractOverrides ();
  void categorize ();
  bool exactMatch (const std::string&, const std::string&) const;
  bool canonicalize (std::string&, const std::string&, const std::string&) const;
  void unsweetenTags ();
  void dump (const std::string&) const;

public:
  std::multimap <std::string, std::string> _entities;
  std::map <std::string, std::string>      _aliases;
  std::string                              _program;
  std::vector <std::string>                _original_args;
  std::vector <std::string>                _args;
  std::string                              _rc;
  std::map <std::string, std::string>      _overrides;
  std::string                              _command;
  bool                                     _readOnly;
  std::vector <std::string>                _filter;
  std::vector <std::string>                _modifications;
};

#endif

