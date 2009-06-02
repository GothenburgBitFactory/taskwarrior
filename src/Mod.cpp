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
: std::string ("") // i18n: no
{
}

////////////////////////////////////////////////////////////////////////////////
Mod::Mod (const char* other)
: std::string (other)
{
  if (!valid ())
    throw std::string ("Unrecognized modifier '") + other + "'"; // i18n: TODO
}

////////////////////////////////////////////////////////////////////////////////
Mod::Mod (const std::string& other)
: std::string (other)
{
  if (!valid ())
    throw std::string ("Unrecognized modifier '") + other + "'"; // i18n: TODO
}

////////////////////////////////////////////////////////////////////////////////
Mod::~Mod ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Mod::valid ()
{
  if (*this == "before"     || *this == "after"    ||   // i18n: TODO
      *this == "not"        ||                          // i18n: TODO
      *this == "none"       || *this == "any"      ||   // i18n: TODO
      *this == "synth"      ||                          // i18n: TODO
      *this == "under"      || *this == "over"     ||   // i18n: TODO
      *this == "first"      || *this == "last"     ||   // i18n: TODO
      *this == "this"       ||                          // i18n: TODO
      *this == "next"       ||                          // i18n: TODO
      *this == "is"         || *this == "isnt"     ||   // i18n: TODO
      *this == "has"        || *this == "hasnt"    ||   // i18n: TODO
      *this == "startswith" || *this == "endswith")     // i18n: TODO
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Mod::eval (const Mod& other)
{
  // No modifier means automatic pass.
  if (*this == "") // i18n: no
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

  if (*this == "is")    // i18n: TODO
    return *this == other ? true : false;

  if (*this == "isnt")  // i18n: TODO
    return *this != other ? true : false;

  // TODO has
  // TODO hasnt
  // TODO startswith
  // TODO endswith

  return false;
}

////////////////////////////////////////////////////////////////////////////////
