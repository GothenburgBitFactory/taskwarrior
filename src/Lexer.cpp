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

////////////////////////////////////////////////////////////////////////////////
// This lexer works by breaking the input stream into tokens.  The essence of
// the algorithm lies in the distinction between adjacent tokens, such that
// between the two extremes lies a good solution.
//
// At one extreme, the entire input is considered one token.  Clearly this is
// only correct for trivial input.  At the other extreme, every character of the
// input is a token.  This is also wrong.
//
// If the input is as follows:
//
//   It is almost 11:00am.
//
// The desired tokenization is:
//
//   It
//   <space>
//   is
//   <space>
//   almost
//   <space>
//   11
//   :
//   00
//   am
//   .
//   \n
//
// This can be achieved by allowing transitions to denote token boundaries.
// Given the following character classes:
//
//   letter:     a-z A-Z
//   digit:      0-9
//   whitespace: <space> <tab> <newline> <cr> <lf> <vertical-tab>
//   other:      Everything else
//
// Then a token boundary is a transition between:
//   letter     -> !letter
//   digit      -> !digit
//   whitespace -> any
//   other      -> any
//
// This has the effect of allowing groups of consecutive letters to be
// considered one token, as well as groups of digits.
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <util.h>
#include <Lexer.h>

static const int other = -1;
static const int alpha = -2;
static const int digit = -3;
static const int white = -4;
static const int quote = -5;

////////////////////////////////////////////////////////////////////////////////
Lexer::Lexer (const std::string& input)
: mInput (input)

, mAlpha ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
, mDigit ("0123456789")
, mQuote ("'\"")
, mWhite (" \t\n\r\f")

, mAlphaCoalesce (true)
, mDigitCoalesce (true)
, mQuotedCoalesce (false)
, mWhiteCoalesce (false)
, mSkipWhitespace (false)
{
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::tokenize (std::vector <std::string>& all)
{
  all.clear (); // Prevent repeated accumulation.

  std::string token;
  bool inQuote = false;
  char quoteChar = '\0';
  for (unsigned int i = 0; i < mInput.length (); ++i)
  {
    bool specialFound = false;
    for (unsigned int s = 0; s < mSpecialTokens.size (); ++s)
    {
      std::string potential = mInput.substr (
        i, min (mSpecialTokens[s].length (), mInput.length () - i));

      if (potential == mSpecialTokens[s])
      {
        // Capture currently assembled token, the special token, increment over
        // that token, and skip all remaining code in the loop.
        if (token.length ())
        {
          all.push_back (token);
          token = "";
        }

        all.push_back (potential);
        i += potential.length () - 1;
        specialFound = true;
      }
    }

    if (specialFound)
      continue;

    char c = mInput[i];
    char next = '\0';
    if (i < mInput.length () - 1)
      next = mInput[i + 1];

    // Classify current and next characters.
    int thisChar = classify (c);
    int nextChar = classify (next);

    // Properly set inQuote, quoteChar.
    if (!inQuote && thisChar == quote)
    {
      quoteChar = c;
      inQuote = true;
    }
    else if (inQuote && c == quoteChar)
    {
      inQuote = false;
    }

    // Detect transitions.
    bool transition = false;
    if (thisChar != nextChar)
      transition = true;

    token += c;

    // Transitions mean new token.  All 'other' characters are separate tokens.
    if (transition || nextChar == other)
    {
      if (!inQuote || !mQuotedCoalesce)
      {
        if (!mSkipWhitespace || thisChar != white)
          all.push_back (token);
        token = "";
      }
    }

    // Non-transitions - runs.
    else
    {
      // Runs may be optionally coalesced.
      if (!(mAlphaCoalesce && nextChar == alpha) &&
          !(mDigitCoalesce && nextChar == digit) &&
          !(mWhiteCoalesce && nextChar == white))
      {
        if (!inQuote || !mQuotedCoalesce)
        {
          if (!mSkipWhitespace || thisChar != white)
            all.push_back (token);
          token = "";
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::categorizeAsAlpha (char value)
{
  if (mAlpha.find (value) == std::string::npos)
    mAlpha += value;

  std::string::size_type pos;
  if ((pos = mDigit.find (value)) != std::string::npos) mDigit.erase (pos, 1);
  if ((pos = mQuote.find (value)) != std::string::npos) mQuote.erase (pos, 1);
  if ((pos = mWhite.find (value)) != std::string::npos) mWhite.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::ignoreAsAlpha (char value)
{
  std::string::size_type pos;
  if ((pos = mAlpha.find (value)) != std::string::npos) mAlpha.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::setAlpha (const std::string& value)
{
  mAlpha = value;

  std::string::size_type pos;
  for (unsigned int i = 0; i < mAlpha.length (); ++i)
  {
    if ((pos = mDigit.find (mAlpha[i])) != std::string::npos) mDigit.erase (pos, 1);
    if ((pos = mQuote.find (mAlpha[i])) != std::string::npos) mQuote.erase (pos, 1);
    if ((pos = mWhite.find (mAlpha[i])) != std::string::npos) mWhite.erase (pos, 1);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::categorizeAsDigit (char value)
{
  if (mDigit.find (value) == std::string::npos)
    mDigit += value;

  std::string::size_type pos;
  if ((pos = mAlpha.find (value)) != std::string::npos) mAlpha.erase (pos, 1);
  if ((pos = mQuote.find (value)) != std::string::npos) mQuote.erase (pos, 1);
  if ((pos = mWhite.find (value)) != std::string::npos) mWhite.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::ignoreAsDigit (char value)
{
  std::string::size_type pos;
  if ((pos = mDigit.find (value)) != std::string::npos) mDigit.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::setDigit (const std::string& value)
{
  mDigit = value;

  std::string::size_type pos;
  for (unsigned int i = 0; i < mDigit.length (); ++i)
  {
    if ((pos = mAlpha.find (mDigit[i])) != std::string::npos) mAlpha.erase (pos, 1);
    if ((pos = mQuote.find (mDigit[i])) != std::string::npos) mQuote.erase (pos, 1);
    if ((pos = mWhite.find (mDigit[i])) != std::string::npos) mWhite.erase (pos, 1);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::categorizeAsQuote (char value)
{
  if (mQuote.find (value) == std::string::npos)
    mQuote += value;

  std::string::size_type pos;
  if ((pos = mAlpha.find (value)) != std::string::npos) mAlpha.erase (pos, 1);
  if ((pos = mDigit.find (value)) != std::string::npos) mDigit.erase (pos, 1);
  if ((pos = mWhite.find (value)) != std::string::npos) mWhite.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::ignoreAsQuote (char value)
{
  std::string::size_type pos;
  if ((pos = mQuote.find (value)) != std::string::npos) mQuote.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::setQuote (const std::string& value)
{
  mQuote = value;

  std::string::size_type pos;
  for (unsigned int i = 0; i < mQuote.length (); ++i)
  {
    if ((pos = mAlpha.find (mQuote[i])) != std::string::npos) mAlpha.erase (pos, 1);
    if ((pos = mDigit.find (mQuote[i])) != std::string::npos) mDigit.erase (pos, 1);
    if ((pos = mWhite.find (mQuote[i])) != std::string::npos) mWhite.erase (pos, 1);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::categorizeAsWhite (char value)
{
  if (mWhite.find (value) == std::string::npos)
    mWhite += value;

  std::string::size_type pos;
  if ((pos = mAlpha.find (value)) != std::string::npos) mAlpha.erase (pos, 1);
  if ((pos = mDigit.find (value)) != std::string::npos) mDigit.erase (pos, 1);
  if ((pos = mQuote.find (value)) != std::string::npos) mQuote.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::ignoreAsWhite (char value)
{
  std::string::size_type pos;
  if ((pos = mWhite.find (value)) != std::string::npos) mWhite.erase (pos, 1);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::setWhite (const std::string& value)
{
  mWhite = value;

  std::string::size_type pos;
  for (unsigned int i = 0; i < mWhite.length (); ++i)
  {
    if ((pos = mAlpha.find (mWhite[i])) != std::string::npos) mAlpha.erase (pos, 1);
    if ((pos = mDigit.find (mWhite[i])) != std::string::npos) mDigit.erase (pos, 1);
    if ((pos = mQuote.find (mWhite[i])) != std::string::npos) mQuote.erase (pos, 1);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::coalesceAlpha (bool value)
{
  mAlphaCoalesce = value;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::coalesceDigits (bool value)
{
  mDigitCoalesce = value;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::coalesceQuoted (bool value)
{
  mQuotedCoalesce = value;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::coalesceWhite (bool value)
{
  mWhiteCoalesce = value;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::skipWhitespace (bool value)
{
  mSkipWhitespace = value;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::specialToken (const std::string& special)
{
  mSpecialTokens.push_back (special);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::classify (char c)
{
  if (mAlpha.find (c) != std::string::npos) return alpha;
  if (mDigit.find (c) != std::string::npos) return digit;
  if (mWhite.find (c) != std::string::npos) return white;
  if (mQuote.find (c) != std::string::npos) return quote;

  return other;
}

////////////////////////////////////////////////////////////////////////////////

