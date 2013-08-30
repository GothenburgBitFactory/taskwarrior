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

#ifndef INCLUDED_PARSER
#define INCLUDED_PARSER

#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <Nibbler.h>
#include <Tree.h>

class Parser
{
public:
  Parser ();
  virtual ~Parser ();
  void grammar (const std::string&);
  virtual Tree* parse (const std::string&) = 0;

  void verbose ();
  void dump () const;

protected:
  class Token
  {
  public:
    Token ();
    Token (const Token&);
    Token& operator= (const Token&);
    void clear ();
    std::string dump ();

  public:
    std::string value;
    char quantifier;
  };

  class Alternate : public std::vector <Token>
  {
  public:
    Alternate ();
    Alternate (const Alternate&);
    Alternate& operator= (const Alternate&);
    std::string dump ();
  };

  class Production : public std::vector <Alternate>
  {
  public:
    Production ();
    Production (const Production&);
    Production& operator= (const Production&);
    void tag    (const std::string&);
    bool hasTag (const std::string&) const;
    void clear ();
    std::string dump ();

  private:
    std::vector <std::string> mTags;
  };

private:
  bool bnfNibbleRule (Nibbler&, std::string&, Production&);
  bool bnfNibbleAlternate (Nibbler&, Alternate&);
  bool bnfNibbleToken (Nibbler&, Token&);

  void checkConsistency ();

protected:
  std::string _primary;
  std::map <std::string, Production> _rules;
  bool _verbose;
};

#endif

////////////////////////////////////////////////////////////////////////////////
