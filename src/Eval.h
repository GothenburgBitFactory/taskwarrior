////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_EVAL
#define INCLUDED_EVAL

#include <vector>
#include <string>
#include <Lexer.h>
#include <Variant.h>

class Eval
{
public:
  Eval ();

  void addSource (bool (*fn)(const std::string&, Variant&));
  void evaluateInfixExpression (const std::string&, Variant&) const;
  void evaluatePostfixExpression (const std::string&, Variant&) const;
  void compileExpression (const std::vector <std::pair <std::string, Lexer::Type>>&);
  void evaluateCompiledExpression (Variant&);
  void debug (bool);

  static std::vector <std::string> getOperators ();
  static std::vector <std::string> getBinaryOperators ();

private:
  void evaluatePostfixStack (const std::vector <std::pair <std::string, Lexer::Type>>&, Variant&) const;
  void infixToPostfix (std::vector <std::pair <std::string, Lexer::Type>>&) const;
  void infixParse (std::vector <std::pair <std::string, Lexer::Type>>&) const;
  bool parseLogical (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseRegex (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseEquality (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseComparative (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseArithmetic (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseGeometric (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseTag (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseUnary (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parseExponent (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool parsePrimitive (std::vector <std::pair <std::string, Lexer::Type>>&, unsigned int &) const;
  bool identifyOperator (const std::string&, char&, unsigned int&, char&) const;

  std::string dump (std::vector <std::pair <std::string, Lexer::Type>>&) const;

private:
  std::vector <bool (*)(const std::string&, Variant&)> _sources {};
  bool _debug                                                   {false};
  std::vector <std::pair <std::string, Lexer::Type>> _compiled  {};
};

#endif
