////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_LEXER
#define INCLUDED_LEXER

#include <vector>
#include <string>

class Lexer
{
public:
  Lexer (const std::string&);
  void tokenize (std::vector <std::string>&);

  void categorizeAsAlpha (char);
  void ignoreAsAlpha (char);
  void setAlpha (const std::string&);

  void categorizeAsDigit (char);
  void ignoreAsDigit (char);
  void setDigit (const std::string&);

  void categorizeAsQuote (char);
  void ignoreAsQuote (char);
  void setQuote (const std::string&);

  void categorizeAsWhite (char);
  void ignoreAsWhite (char);
  void setWhite (const std::string&);

  void coalesceAlpha (bool);
  void coalesceDigits (bool);
  void coalesceQuoted (bool);
  void coalesceWhite (bool);
  void skipWhitespace (bool);
  void specialToken (const std::string&);

private:
  int classify (char);

  std::string mInput;

  std::string mAlpha;
  std::string mDigit;
  std::string mQuote;
  std::string mWhite;

  bool mAlphaCoalesce;
  bool mDigitCoalesce;
  bool mQuotedCoalesce;
  bool mWhiteCoalesce;
  bool mSkipWhitespace;

  std::vector <std::string> mSpecialTokens;
};

#endif

////////////////////////////////////////////////////////////////////////////////
