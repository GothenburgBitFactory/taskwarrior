////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <vector>
#include <string>

class Lexer
{
public:
  static std::string dateFormat;

  enum Type
  {
    typeNone = 0,
    typeString,
    typeIdentifier,
    typeIdentifierEscape,    // Intermediate
    typeEscape,              // Intermediate
    typeEscapeHex,           // Intermediate
    typeEscapeUnicode,       // Intermediate
    typeNumber,
    typeDecimal,
    typeExponentIndicator,   // Intermediate
    typeExponent,            // Intermediate
    typeHex,
    typeOperator,
    typeDate,
    typeDuration,
    typeTag,
  };

  Lexer (const std::string&);
  virtual ~Lexer ();
  Lexer (const Lexer&);            // Not implemented.
  Lexer& operator= (const Lexer&); // Not implemented.
  bool operator== (const Lexer&);  // Not implemented.
  bool token (std::string&, Type&);
  bool word (std::string&, Type&);
  void ambiguity (bool);

  static const std::string type_name (const Type&);
  static bool is_ws (int);
  static bool boundary (int, int);
  static void word_split (std::vector <std::string>&, const std::string&);
  static void token_split (std::vector <std::string>&, const std::string&);
  static void token_split (std::vector <std::pair <std::string, Lexer::Type> >&, const std::string&);

private:
  bool is_date (std::string&);
  bool is_duration (std::string&);
  bool is_punct (int) const;
  bool is_num (int) const;
  bool is_ident_start (int) const;
  bool is_ident (int) const;
  bool is_triple_op (int, int, int) const;
  bool is_double_op (int, int, int) const;
  bool is_single_op (int) const;
  bool is_dec_digit (int) const;
  bool is_hex_digit (int) const;
  int decode_escape (int) const;
  int hex_to_int (int) const;
  int hex_to_int (int, int) const;
  int hex_to_int (int, int, int, int) const;
  void shift ();

private:
  const std::string _input;
  std::string::size_type _i;
  std::string::size_type _shift_counter;
  int _n0;
  int _n1;
  int _n2;
  int _n3;
  bool _boundary01;
  bool _boundary12;
  bool _boundary23;
  bool _ambiguity;
};

#endif

////////////////////////////////////////////////////////////////////////////////
