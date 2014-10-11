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
#ifndef INCLUDED_PARSER
#define INCLUDED_PARSER

#include <Tree.h>
#include <Path.h>
#include <File.h>
#include <string>
#include <map>

class Parser
{
public:
  Parser ();
  ~Parser ();

  static void getOverrides (int, const char**, std::string&);

  void initialize (int, const char**);
  void clear ();
  Tree* tree ();
  Tree* parse ();
  void alias (const std::string&, const std::string&);
  void entity (const std::string&, const std::string&);
  bool exactMatch (const std::string&, const std::string&) const;
  bool canonicalize (std::string&, const std::string&, const std::string&) const;

  enum collectType {collectLeaf, collectAll, collectTerminated};
  void collect (std::vector <Tree*>&, collectType = collectLeaf, Tree* tree = NULL) const;

  void findBinary ();
  void resolveAliases ();
  void findOverrides ();
  void findCommand ();
  void findIdSequence ();
  void findUUIDList ();

  void getOverrides (std::string&, File&);
  void getDataLocation (Path&);
  void applyOverrides ();
  void injectDefaults ();
  Tree* captureFirst (const std::string&);
  Tree* captureLast (const std::string&);

  const std::string getFilterExpression ();
  const std::vector <std::string> getWords () const;

  std::string getLimit () const;
  std::string getCommand () const;

private:
  void findTerminator ();
  void findPattern ();
  void findSubstitution ();
  void findTag ();
  void findAttribute ();
  void findAttributeModifier ();
  void findOperator ();
  void findFilter ();
  void findModifications ();
  void findStrayModifications ();
  void findPlainArgs ();
  void findFilterSubst ();
  void findMissingOperators ();
  bool insertOr ();
  bool insertAnd ();
  void validate ();

private:
  int                                      _debug;
  Tree*                                    _tree;
  std::multimap <std::string, std::string> _entities;
  std::map <std::string, std::string>      _aliases;
};

#endif

