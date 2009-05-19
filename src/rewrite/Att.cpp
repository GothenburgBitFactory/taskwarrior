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

#include "Att.h"

////////////////////////////////////////////////////////////////////////////////
Att::Att ()
: mName ("")
, mValue ("")
{
  mMods.clear ();
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& value)
{
  throw std::string ("unimplemented Att::Att");
  mName  = name;
  mValue = value;

  mMods.clear ();
}
////////////////////////////////////////////////////////////////////////////////
Att::Att (const Att& other)
{
  throw std::string ("unimplemented Att::Att");
  mName  = other.mName;
  mValue = other.mValue;
  mMods  = other.mMods;
}

////////////////////////////////////////////////////////////////////////////////
Att& Att::operator= (const Att& other)
{
  throw std::string ("unimplemented Att::operator=");
  if (this != &other)
  {
    mName  = other.mName;
    mValue = other.mValue;
    mMods  = other.mMods;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Att::~Att ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Parse the following forms:
//    name [[.mod] ...] : [value]
void Att::parse (const std::string& input)
{
  throw std::string ("unimplemented Att::parse");
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::composeF4 () const
{
  throw std::string ("unimplemented Att::composeF4");
  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Att::addMod (const std::string&)
{
  throw std::string ("unimplemented Att::addMod");
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::name () const
{
  return mName;
}

////////////////////////////////////////////////////////////////////////////////
void Att::name (const std::string& name)
{
  mName = name;
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::value () const
{
  return mValue;
}

////////////////////////////////////////////////////////////////////////////////
void Att::value (const std::string& value)
{
  mValue = value;
}

////////////////////////////////////////////////////////////////////////////////
int Att::value_int () const
{
  throw std::string ("unimplemented Att::value_int");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Att::value_int (int)
{
  throw std::string ("unimplemented Att::value_int");
}

////////////////////////////////////////////////////////////////////////////////
bool Att::filter () const
{
  throw std::string ("unimplemented filter");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::required () const
{
  throw std::string ("unimplemented Att::required");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::internal () const
{
  throw std::string ("unimplemented Att::internal");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   \t -> &tab;
//   "  -> &quot;
//   ,  -> &comma;
//   [  -> &open;
//   ]  -> &close;
void Att::encode (std::string&) const
{
  throw std::string ("unimplemented Att::internal");
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   \t <- &tab;
//   "  <- &quot;
//   ,  <- &comma;
//   [  <- &open;
//   ]  <- &close;
void Att::decode (std::string&) const
{
  throw std::string ("unimplemented Att::internal");
}

////////////////////////////////////////////////////////////////////////////////
