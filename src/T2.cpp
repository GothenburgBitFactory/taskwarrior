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

#include <string>
#include "Nibbler.h"
#include "T2.h"

////////////////////////////////////////////////////////////////////////////////
T2::T2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
T2::T2 (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
T2& T2::operator= (const T2& other)
{
  throw std::string ("unimplemented T2::operator=");
  if (this != &other)
  {
//    mOne = other.mOne;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
T2::~T2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
void T2::legacyParse (const std::string& input)
{
}

////////////////////////////////////////////////////////////////////////////////
// [name:value, name:"value",name:[name:value,name:value]]
std::string T2::composeF4 ()
{
  throw std::string ("unimplemented T2::composeF4");
  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string T2::composeCSV ()
{
  throw std::string ("unimplemented T2::composeCSV");
  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool T2::validate () const
{
  // TODO Verify until > due
  // TODO Verify entry < until, due, start, end
  return true;
}

////////////////////////////////////////////////////////////////////////////////
