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

#ifndef INCLUDED_LEXER
#define INCLUDED_LEXER

#include <string>
#include <map>
#include <vector>
#include <cstddef>

// Lexer: A UTF8 lexical analyzer for every construct used on the Taskwarrior
//        command line, with additional recognized types for disambiguation.

class Lexer
{
public:
  // These are overridable.
  static std::string dateFormat;
  static std::string::size_type minimumMatchLength;
  static std::map <std::string, std::string> attributes;

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
  static std::vector <std::string> split (const std::string&);
  static std::string typeToString (Lexer::Type);

  // Static helpers.
  static const std::string typeName          (const Lexer::Type&);
  static bool isWhitespace                   (int);
  static bool isAlpha                        (int);
  static bool isDigit                        (int);
  static bool isHexDigit                     (int);
  static bool isIdentifierStart              (int);
  static bool isIdentifierNext               (int);
  static bool isSingleCharOperator           (int);
  static bool isDoubleCharOperator           (int, int, int);
  static bool isTripleCharOperator           (int, int, int, int);
  static bool isBoundary                     (int, int);
  static bool isHardBoundary                 (int, int);
  static bool isPunctuation                  (int);
  static bool isAllDigits                    (const std::string&);
  static bool isDOM                          (const std::string&);
  static void dequote                        (std::string&, const std::string& quotes = "'\"");
  static bool wasQuoted                      (const std::string&);
  static bool readWord                       (const std::string&, const std::string&, std::string::size_type&, std::string&);
  static bool readWord                       (const std::string&, std::string::size_type&, std::string&);
  static bool decomposePair                  (const std::string&, std::string&, std::string&, std::string&, std::string&);
  static bool decomposeSubstitution          (const std::string&, std::string&, std::string&, std::string&);
  static bool decomposePattern               (const std::string&, std::string&, std::string&);
  static int hexToInt                        (int);
  static int hexToInt                        (int, int);
  static int hexToInt                        (int, int, int, int);
  static std::string::size_type commonLength (const std::string&, const std::string&);
  static std::string::size_type commonLength (const std::string&, std::string::size_type, const std::string&, std::string::size_type);
  static std::string commify                 (const std::string&);
  static std::string lowerCase               (const std::string&);
  static std::string ucFirst                 (const std::string&);
  static std::string trimLeft                (const std::string& in, const std::string& t = " ");
  static std::string trimRight               (const std::string& in, const std::string& t = " ");
  static std::string trim                    (const std::string& in, const std::string& t = " ");

  // Stream Classifiers.
  bool isEOS          () const;
  bool isString       (std::string&, Lexer::Type&, const std::string&);
  bool isDate         (std::string&, Lexer::Type&);
  bool isDuration     (std::string&, Lexer::Type&);
  bool isUUID         (std::string&, Lexer::Type&, bool);
  bool isNumber       (std::string&, Lexer::Type&);
  bool isInteger      (std::string&, Lexer::Type&);
  bool isHexNumber    (std::string&, Lexer::Type&);
  bool isSeparator    (std::string&, Lexer::Type&);
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
  bool isLiteral      (const std::string&, bool, bool);
  bool isOneOf        (const std::vector <std::string>&, bool, bool);
  bool isOneOf        (const std::map <std::string, std::string>&, bool, bool);

private:
  std::string _text;
  std::size_t _cursor;
  std::size_t _eos;
};

#endif

////////////////////////////////////////////////////////////////////////////////
