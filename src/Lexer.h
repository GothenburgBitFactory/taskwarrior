////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2015, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_LEXER
#define INCLUDED_LEXER

#include <string>
#include <vector>
#include <cstddef>

// Lexer: A UTF8 lexical analyzer for every construct used on the Taskwarrior
//        command line, with additional recognized types for disambiguation.

class Lexer
{
public:
  // These are overridable.
  static std::string dateFormat;
  static bool isoEnabled;

  enum class Type { uuid, number, hex,
                    string,
                    url, pair, set, separator,
                    tag,
                    path,
                    substitution, pattern,
                    op,
                    dom, identifier, word,
                    date, duration };

  Lexer (const std::string&);
  ~Lexer ();
  bool token (std::string&, Lexer::Type&);
  static std::vector <std::pair <std::string, Lexer::Type>> tokens (const std::string&);
  static std::vector <std::string> split (const std::string&);
  static std::string typeToString (Lexer::Type);
  static bool isAllDigits (const std::string&);
  static bool isOneWord (const std::string&);

  // Static helpers.
  static const std::string typeName (const Lexer::Type&);
  static bool isWhitespace          (int);
  static bool isAlpha               (int);
  static bool isDigit               (int);
  static bool isHexDigit            (int);
  static bool isIdentifierStart     (int);
  static bool isIdentifierNext      (int);
  static bool isSingleCharOperator  (int);
  static bool isDoubleCharOperator  (int, int, int);
  static bool isTripleCharOperator  (int, int, int, int);
  static bool isBoundary            (int, int);
  static bool isPunctuation         (int);
  static void dequote               (std::string&);
  static bool wasQuoted             (const std::string&);

  // Helpers.
  bool isEOS () const;
  int hexToInt (int) const;
  int hexToInt (int, int) const;
  int hexToInt (int, int, int, int) const;

  // Classifiers.
  bool isString       (std::string&, Lexer::Type&, int quote);
  bool isDate         (std::string&, Lexer::Type&);
  bool isDuration     (std::string&, Lexer::Type&);
  bool isUUID         (std::string&, Lexer::Type&);
  bool isNumber       (std::string&, Lexer::Type&);
  bool isHexNumber    (std::string&, Lexer::Type&);
  bool isSeparator    (std::string&, Lexer::Type&);
  bool isList         (std::string&, Lexer::Type&);
  bool isURL          (std::string&, Lexer::Type&);
  bool isPair         (std::string&, Lexer::Type&);
  bool isSet          (std::string&, Lexer::Type&);
  bool isTag          (std::string&, Lexer::Type&);
  bool isPath         (std::string&, Lexer::Type&);
  bool isSubstitution (std::string&, Lexer::Type&);
  bool isPattern      (std::string&, Lexer::Type&);
  bool isOperator     (std::string&, Lexer::Type&);
  bool isDOM          (std::string&, Lexer::Type&);
  bool isIdentifier   (std::string&, Lexer::Type&);
  bool isWord         (std::string&, Lexer::Type&);
  bool isContiguous   (std::string&, Lexer::Type&);

private:
  std::string _text;
  std::size_t _cursor;
  std::size_t _eos;
};

#endif

////////////////////////////////////////////////////////////////////////////////
