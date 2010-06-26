////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Nibbler.h"
#include "rx.h"

const char* c_digits = "0123456789";

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler ()
: mInput ("")
, mLength (0)
, mCursor (0)
, mSaved (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const char* input)
: mInput (input)
, mLength (strlen (input))
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const std::string& input)
: mInput (input)
, mLength (input.length ())
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const Nibbler& other)
{
  mInput  = other.mInput;
  mLength = other.mLength;
  mCursor = other.mCursor;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler& Nibbler::operator= (const Nibbler& other)
{
  if (this != &other)
  {
    mInput  = other.mInput;
    mLength = other.mLength;
    mCursor = other.mCursor;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::~Nibbler ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Extract up until the next c (but not including) or EOS.
bool Nibbler::getUntil (char c, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find (c, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntil (const std::string& terminator, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find (terminator, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilRx (const std::string& regex, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string modified_regex;
    if (regex[0] != '(')
      modified_regex = "(" + regex + ")";
    else
      modified_regex = regex;

    std::vector <int> start;
    std::vector <int> end;
    if (regexMatch (start, end, mInput.substr (mCursor), modified_regex, true))
    {
      result = mInput.substr (mCursor, start[0]);
      mCursor += start[0];
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilOneOf (const std::string& chars, std::string& result)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find_first_of (chars, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor);
      mCursor = mLength;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilWS (std::string& result)
{
  return this->getUntilOneOf (" \t\r\n\f", result);
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOL (std::string& result)
{
  return getUntil ('\n', result);
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOS (std::string& result)
{
  if (mCursor < mLength)
  {
    result = mInput.substr (mCursor);
    mCursor = mLength;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getQuoted (char c, std::string& result)
{
  std::string::size_type start = mCursor;
  if (start < mLength && mInput[start] == c)
  {
    ++start;
    if (start < mLength)
    {
      std::string::size_type end = mInput.find (c, start);
      if (end != std::string::npos)
      {
        result = mInput.substr (start, end - start);
        mCursor = end + 1;
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getInt (int& result)
{
  std::string::size_type i = mCursor;

  if (i < mLength)
  {
    if (mInput[i] == '-')
      ++i;
    else if (mInput[i] == '+')
      ++i;
  }

  // TODO Potential for use of find_first_not_of
  while (i < mLength && isdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = atoi (mInput.substr (mCursor, i - mCursor).c_str ());
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUnsignedInt (int& result)
{
  std::string::size_type i = mCursor;
  // TODO Potential for use of find_first_not_of
  while (i < mLength && isdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = atoi (mInput.substr (mCursor, i - mCursor).c_str ());
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getLiteral (const std::string& literal)
{
  if (mCursor < mLength &&
      mInput.find (literal, mCursor) == mCursor)
  {
    mCursor += literal.length ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getRx (const std::string& regex, std::string& result)
{
  if (mCursor < mLength)
  {
    // Regex may be anchored to the beginning and include capturing parentheses,
    // otherwise they are added.
    std::string modified_regex;
    if (regex.substr (0, 2) != "^(")
      modified_regex = "^(" + regex + ")";
    else
      modified_regex = regex;

    std::vector <std::string> results;
    if (regexMatch (results, mInput.substr (mCursor), modified_regex, true))
    {
      result = results[0];
      mCursor += result.length ();
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipN (const int quantity /* = 1 */)
{
  if (mCursor >= mLength)
    return false;

  if (mCursor <= mLength - quantity)
  {
    mCursor += quantity;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skip (char c)
{
  if (mCursor < mLength &&
      mInput[mCursor] == c)
  {
    ++mCursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipAll (char c)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find_first_not_of (c, mCursor);
    if (i == mCursor)
      return false;

    if (i == std::string::npos)
      mCursor = mLength;  // Yes, off the end.
    else
      mCursor = i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipWS ()
{
  return this->skipAllOneOf (" \t\n\r\f");
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipRx (const std::string& regex)
{
  if (mCursor < mLength)
  {
    // Regex may be anchored to the beginning and include capturing parentheses,
    // otherwise they are added.
    std::string modified_regex;
    if (regex.substr (0, 2) != "^(")
      modified_regex = "^(" + regex + ")";
    else
      modified_regex = regex;

    std::vector <std::string> results;
    if (regexMatch (results, mInput.substr (mCursor), modified_regex, true))
    {
      mCursor += results[0].length ();
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipAllOneOf (const std::string& chars)
{
  if (mCursor < mLength)
  {
    std::string::size_type i = mInput.find_first_not_of (chars, mCursor);
    if (i == mCursor)
      return false;

    if (i == std::string::npos)
      mCursor = mLength;  // Yes, off the end.
    else
      mCursor = i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
char Nibbler::next ()
{
  if (mCursor < mLength)
    return mInput[mCursor];

  return '\0';
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
std::string Nibbler::next (const int quantity)
{
  if (           mCursor  <  mLength &&
      (unsigned) quantity <= mLength &&
                 mCursor  <= mLength - quantity)
    return mInput.substr (mCursor, quantity);

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Nibbler::save ()
{
  mSaved = mCursor;
}

////////////////////////////////////////////////////////////////////////////////
void Nibbler::restore ()
{
  mCursor = mSaved;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::depleted ()
{
  if (mCursor >= mLength)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Nibbler::dump ()
{
  return std::string ("Nibbler ‹")
         + mInput.substr (mCursor)
         + "›";
}

////////////////////////////////////////////////////////////////////////////////
