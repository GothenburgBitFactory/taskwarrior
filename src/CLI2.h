////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_CLI2
#define INCLUDED_CLI2

#include <string>
#include <vector>
#include <map>
#include <Lexer.h>
#include <FS.h>

// Represents a single argument.
class A2
{
public:
  A2 ();
  A2 (const std::string&, Lexer::Type);
  ~A2 ();
  A2 (const A2&);
  A2& operator= (const A2&);
  bool hasTag (const std::string&) const;
  void tag (const std::string&);
/*
  void unTag (const std::string&);
  void unTagAll ();
*/
  void attribute (const std::string&, const std::string&);
  const std::string attribute (const std::string&) const;
  const std::string dump () const;

public:
  Lexer::Type                         _lextype;
  std::vector <std::string>           _tags;
  std::map <std::string, std::string> _attributes;
};

// Represents the command line.
class CLI2
{
public:
  static int minimumMatchLength;

  static void getOverride (int, const char**, std::string&, File&);
  static void getDataLocation (int, const char**, Path&);
  static void applyOverrides (int, const char**);

public:
  CLI2 ();
  ~CLI2 ();
  void alias (const std::string&, const std::string&);
  void entity (const std::string&, const std::string&);

  void add (const std::string&);
  void analyze ();
/*
  void addContextFilter ();
  void addRawFilter (const std::string& arg);
*/
  void prepareFilter (bool applyContext = true);
  const std::vector <std::string> getWords (bool filtered = true);
  bool canonicalize (std::string&, const std::string&, const std::string&) const;
  std::string getBinary () const;
  std::string getCommand (bool canonical = true) const;
/*
  std::string getLimit () const;
*/
  const std::string dump (const std::string& title = "CLI2 Parser") const;

private:
/*
  void addArg (const std::string&);
*/
  void handleArg0 ();
  void lexArguments ();
  void handleTerminator ();
  void aliasExpansion ();
  void findOverrides ();
  bool findCommand ();
  bool exactMatch (const std::string&, const std::string&) const;
/*
  void desugarFilterTags ();
  void findStrayModifications ();
  void desugarFilterAttributes ();
  void desugarFilterAttributeModifiers ();
  void desugarFilterPatterns ();
*/
  void findIDs ();
  void findUUIDs ();
  void insertIDExpr ();
/*
  void desugarFilterPlainArgs ();
  void findOperators ();
  void findAttributes ();
  void insertJunctions ();
*/
  void defaultCommand ();
/*
  void decomposeModAttributes ();
  void decomposeModAttributeModifiers ();
  void decomposeModTags ();
  void decomposeModSubstitutions ();

  bool isUUIDList       (const std::string&) const;

  bool disqualifyInsufficientTerms (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifyNoOps             (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifyOnlyParenOps      (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifyFirstLastBinary   (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
*/

public:
  std::multimap <std::string, std::string> _entities;
  std::map <std::string, std::string>      _aliases;
  std::vector <std::string>                _original_args;
  std::vector <A2>                         _args;

  std::vector <std::pair <int, int>>       _id_ranges;
  std::vector <std::string>                _uuid_list;
};

#endif

