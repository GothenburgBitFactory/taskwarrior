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
#ifndef INCLUDED_CLI
#define INCLUDED_CLI

#include <string>
#include <vector>
#include <map>
#include <Lexer.h>
#include <Path.h>
#include <File.h>

// Represents a single argument.
class A
{
public:
  A ();
  A (const std::string&, const std::string&);
  A (const std::string&, const int);
  A (const std::string&, const double);
  ~A ();
  A (const A&);
  A& operator= (const A&);
  bool hasTag (const std::string&) const;
  void tag (const std::string&);
  void unTag (const std::string&);
  void unTagAll ();
  void attribute (const std::string&, const std::string&);
  void attribute (const std::string&, const int);
  void attribute (const std::string&, const double);
  const std::string attribute (const std::string&) const;
  void removeAttribute (const std::string&);
  const std::string dump () const;

public:
  std::string                         _name;
  std::vector <std::string>           _tags;
  std::map <std::string, std::string> _attributes;
};

// Represents the command line.
class CLI
{
public:
  static int minimumMatchLength;
  static void getOverride (int, const char**, std::string&, File&);
  static void getDataLocation (int, const char**, Path&);
  static void applyOverrides (int, const char**);

public:
  CLI ();
  ~CLI ();
  void alias (const std::string&, const std::string&);
  void entity (const std::string&, const std::string&);
  void initialize (int, const char**);
  void add (const std::string&);
  void addContextFilter ();
  void addRawFilter (const std::string& arg);
  void analyze (bool parse = true, bool strict = false);
  void applyOverrides ();
  const std::string getFilter (bool applyContext = true);
  const std::vector <std::string> getWords ();
  bool canonicalize (std::string&, const std::string&, const std::string&) const;
  std::string getBinary () const;
  std::string getCommand () const;
  std::string getLimit () const;
  const std::string dump (const std::string& title = "CLI Parser") const;

private:
  void addArg (const std::string&, Lexer::Type type = Lexer::Type::word);
  void aliasExpansion ();
  void findOverrides ();
  void categorize ();
  bool exactMatch (const std::string&, const std::string&) const;
  void desugarFilterTags ();
  void findStrayModifications ();
  void desugarFilterAttributes ();
  void desugarFilterAttributeModifiers ();
  void desugarFilterPatterns ();
  void findIDs ();
  void findUUIDs ();
  void insertIDExpr ();
  void desugarFilterPlainArgs ();
  void findOperators ();
  void findAttributes ();
  void insertJunctions ();
  void injectDefaults ();
  void decomposeModAttributes ();
  void decomposeModAttributeModifiers ();
  void decomposeModTags ();
  void decomposeModSubstitutions ();

  bool isTerminator     (const std::string&) const;
  bool isRCOverride     (const std::string&) const;
  bool isConfigOverride (const std::string&) const;
  bool isCommand        (const std::string&) const;
  bool isTag            (const std::string&) const;
  bool isUUIDList       (const std::string&) const;
  bool isUUID           (const std::string&) const;
  bool isIDSequence     (const std::string&) const;
  bool isID             (const std::string&) const;
  bool isPattern        (const std::string&) const;
  bool isSubstitution   (const std::string&) const;
  bool isAttribute      (const std::string&) const;
  bool isOperator       (const std::string&) const;
  bool isName           (const std::string&) const;

  bool disqualifyInsufficientTerms (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifyNoOps             (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifyOnlyParenOps      (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifyFirstLastBinary   (const std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool disqualifySugarFree         (const std::vector <std::pair <std::string, Lexer::Type>>&) const;

public:
  std::multimap <std::string, std::string> _entities;
  std::map <std::string, std::string>      _aliases;
  std::vector <std::string>                _original_args;
  std::vector <A>                          _args;

  std::vector <std::pair <int, int>>       _id_ranges;
  std::vector <std::string>                _uuid_list;
  bool                                     _strict;
  bool                                     _terminated;
};

#endif

