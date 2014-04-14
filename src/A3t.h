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
#ifndef INCLUDED_A3T
#define INCLUDED_A3T

#include <Tree.h>
#include <Path.h>
#include <File.h>
#include <string>
#include <map>

class A3t
{
public:
  A3t ();
  ~A3t ();
  void initialize (int, const char**);
  void append_stdin ();
  Tree* parse ();
  void entity (const std::string&, const std::string&);
  bool canonicalize (std::string&, const std::string&, const std::string&) const;

  void findFileOverride ();
  void findConfigOverride ();
  void get_overrides (std::string&, File&);
  void get_data_location (Path&);

private:
  void findBinary ();
  void findTerminator ();
  void findCommand ();
  void findPattern ();
  void findSubstitution ();
  void findTag ();
  void findAttribute ();
  void findAttributeModifier ();
  void findIdSequence ();
  void findUUIDList ();
  void findOperator ();
  void validate ();

  // TODO Resolve aliases
  // TODO Inject defaults
  // TODO Extract filter
  // TODO Extract words
  // TODO Extract modifications
  // TODO Prepare infix
  // TODO Expand operators
  // TODO Expand sequence
  // TODO Convert to postfix - not necessary given parse tree?

private:
  Tree*                                    _tree;
  std::multimap <std::string, std::string> _entities;
};

#endif

