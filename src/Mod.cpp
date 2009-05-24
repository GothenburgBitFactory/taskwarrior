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

#include "Mod.h"

////////////////////////////////////////////////////////////////////////////////
Mod::Mod ()
: std::string ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Mod::Mod (const char* other)
: std::string (other)
{
  if (!valid ())
    throw std::string ("Unrecognized modifier '") + other + "'";
}

////////////////////////////////////////////////////////////////////////////////
Mod::Mod (const std::string& other)
: std::string (other)
{
  if (!valid ())
    throw std::string ("Unrecognized modifier '") + other + "'";
}

////////////////////////////////////////////////////////////////////////////////
Mod::~Mod ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Mod::valid ()
{
  if (*this == "before"     || *this == "after"    ||
      *this == "not"        ||
      *this == "none"       || *this == "any"      ||
      *this == "synth"      ||
      *this == "under"      || *this == "over"     ||
      *this == "first"      || *this == "last"     ||
      *this == "this"       ||
      *this == "next"       ||
      *this == "is"         || *this == "isnt"     ||
      *this == "has"        || *this == "hasnt"    ||
      *this == "startswith" || *this == "endswith")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Mod::eval (const Mod& other)
{
  // No modifier means automatic pass.
  if (*this == "")
    return true;

  // TODO before
  // TODO after
  // TODO not
  // TODO none
  // TODO any
  // TODO synth
  // TODO under
  // TODO over
  // TODO first
  // TODO last
  // TODO this
  // TODO next

  if (*this == ".is")
    return *this == other ? true : false;

  if (*this == ".isnt")
    return *this != other ? true : false;

  // TODO has
  // TODO hasnt
  // TODO startswith
  // TODO endswith

  return false;
}

////////////////////////////////////////////////////////////////////////////////
