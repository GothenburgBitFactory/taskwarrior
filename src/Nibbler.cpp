////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#include <ctype.h>
#include "Nibbler.h"

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler ()
: mInput ("")
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const char* input)
: mInput (input)
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const std::string& input)
: mInput (input)
, mCursor (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::Nibbler (const Nibbler& other)
{
  mInput  = other.mInput;
  mCursor = other.mCursor;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler& Nibbler::operator= (const Nibbler& other)
{
  if (this != &other)
  {
    mInput  = other.mInput;
    mCursor = other.mCursor;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Nibbler::~Nibbler ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Extract up until the next c, or EOS.
bool Nibbler::getUntil (char c, std::string& result)
{
  if (mCursor < mInput.length ())
  {
    std::string::size_type i = mInput.find (c, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor, std::string::npos);
      mCursor = mInput.length ();
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilOneOf (const std::string& chars, std::string& result)
{
  if (mCursor < mInput.length ())
  {
    std::string::size_type i = mInput.find_first_of (chars, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
    }
    else
    {
      result = mInput.substr (mCursor, std::string::npos);
      mCursor = mInput.length ();
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntil (const std::string& terminator, std::string& result)
{
  if (mCursor < mInput.length ())
  {
    std::string::size_type i = mInput.find (terminator, mCursor);
    if (i != std::string::npos)
    {
      result = mInput.substr (mCursor, i - mCursor);
      mCursor = i;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipN (const int quantity /* = 1 */)
{
  if (mCursor >= mInput.length ())
    return false;

  if (mCursor <= mInput.length () - quantity)
  {
    mCursor += quantity;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skip (char c)
{
  if (mCursor < mInput.length () &&
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
  std::string::size_type i = mCursor;
  while (i < mInput.length () && mInput[i] == c)
    ++i;

  if (i != mCursor)
  {
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::skipAllOneOf (const std::string& chars)
{
  if (mCursor < mInput.length ())
  {
    std::string::size_type i = mInput.find_first_not_of (chars, mCursor);
    if (i == mCursor)
      return false;

    if (i == std::string::npos)
      mCursor = mInput.length ();  // Yes, off the end.
    else
      mCursor = i;

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getQuoted (char c, std::string& result)
{
  std::string::size_type start = mCursor;
  if (start < mInput.length () - 1 && mInput[start] == c)
  {
    ++start;
    if (start < mInput.length ())
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

  if (i < mInput.length ())
  {
    if (mInput[i] == '-')
      ++i;
    else if (mInput[i] == '+')
      ++i;
  }

  while (i < mInput.length () && ::isdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = ::atoi (mInput.substr (mCursor, i - mCursor).c_str ());
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUnsignedInt (int& result)
{
  std::string::size_type i = mCursor;
  while (i < mInput.length () && ::isdigit (mInput[i]))
    ++i;

  if (i > mCursor)
  {
    result = ::atoi (mInput.substr (mCursor, i - mCursor).c_str ());
    mCursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOL (std::string& result)
{
  return getUntil ('\n', result);
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::getUntilEOS (std::string& result)
{
  if (mCursor < mInput.length ())
  {
    result = mInput.substr (mCursor, std::string::npos);
    mCursor = mInput.length ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Nibbler::depleted ()
{
  if (mCursor >= mInput.length ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
