////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
  A2 (const std::string&, Lexer::Type);
  A2 (const A2&);
  A2& operator= (const A2&);
  bool hasTag (const std::string&) const;
  void tag (const std::string&);
  void unTag (const std::string&);
  void attribute (const std::string&, const std::string&);
  const std::string attribute (const std::string&) const;
  const std::string getToken () const;
  const std::string dump () const;
  void decompose ();

public:
  Lexer::Type                         _lextype     {Lexer::Type::word};
  std::vector <std::string>           _tags        {};
  std::map <std::string, std::string> _attributes  {};
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
  CLI2 () = default;
  void alias (const std::string&, const std::string&);
  void entity (const std::string&, const std::string&);

  void add (const std::string&);
  void add (const std::vector <std::string>&);
  void analyze ();
  void addFilter (const std::string& arg);
  void addContextFilter ();
  void prepareFilter ();
  const std::vector <std::string> getWords ();
  bool canonicalize (std::string&, const std::string&, const std::string&) const;
  std::string getBinary () const;
  std::string getCommand (bool canonical = true) const;
  const std::string dump (const std::string& title = "CLI2 Parser") const;

private:
  void handleArg0 ();
  void lexArguments ();
  void demotion ();
  void aliasExpansion ();
  void canonicalizeNames ();
  void categorizeArgs ();
  void parenthesizeOriginalFilter ();
  bool findCommand ();
  bool exactMatch (const std::string&, const std::string&) const;
  void desugarFilterTags ();
  void findStrayModifications ();
  void desugarFilterAttributes ();
  void desugarFilterPatterns ();
  void findIDs ();
  void findUUIDs ();
  void insertIDExpr ();
  void lexFilterArgs ();
  void desugarFilterPlainArgs ();
  void insertJunctions ();
  void defaultCommand ();
  std::vector <A2> lexExpression (const std::string&);

public:
  std::multimap <std::string, std::string>           _entities             {};
  std::map <std::string, std::string>                _aliases              {};
  std::vector <A2>                                   _original_args        {};
  std::vector <A2>                                   _args                 {};

  std::vector <std::pair <std::string, std::string>> _id_ranges            {};
  std::vector <std::string>                          _uuid_list            {};
  bool                                               _context_filter_added {false};
};

#endif

