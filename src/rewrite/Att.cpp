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
// name : " value "
std::string Att::composeF4 () const
{
  std::string value = mValue;
  encode (value);
  enquote (value);

  return mName + ":" + value;
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
bool Att::reserved () const
{
  throw std::string ("unimplemented Att::reserved");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Add quotes.
void Att::enquote (std::string& value) const
{
  value = '"' + value + '"';
}

////////////////////////////////////////////////////////////////////////////////
// Remove quotes.
void Att::dequote (std::string& value) const
{
  if (value.length () > 2 &&
      value[0] == '"' &&
      value[value.length () - 1] == '"')
    value = value.substr (1, value.length () - 2);
}

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   \t -> &tab;
//   "  -> &quot;
//   ,  -> &comma;
//   [  -> &open;
//   ]  -> &close;
//   :  -> &colon;
void Att::encode (std::string& value) const
{
  std::string::size_type i;

  while ((i = value.find ('\t')) != std::string::npos)
    value.replace (i, 1, "&tab;");

  while ((i = value.find ('"')) != std::string::npos)
    value.replace (i, 1, "&quot;");

  while ((i = value.find (',')) != std::string::npos)
    value.replace (i, 1, "&comma;");

  while ((i = value.find ('[')) != std::string::npos)
    value.replace (i, 1, "&open;");

  while ((i = value.find (']')) != std::string::npos)
    value.replace (i, 1, "&close;");

  while ((i = value.find (':')) != std::string::npos)
    value.replace (i, 1, "&colon;");
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   \t <- &tab;
//   "  <- &quot;
//   ,  <- &comma;
//   [  <- &open;
//   ]  <- &close;
//   :  <- &colon;
void Att::decode (std::string& value) const
{
  std::string::size_type i;

  while ((i = value.find ("&tab;")) != std::string::npos)
    value.replace (i, 5, "\t");

  while ((i = value.find ("&quot;")) != std::string::npos)
    value.replace (i, 6, "\"");

  while ((i = value.find ("&comma;")) != std::string::npos)
    value.replace (i, 7, ",");

  while ((i = value.find ("&open;")) != std::string::npos)
    value.replace (i, 6, "[");

  while ((i = value.find ("&close;")) != std::string::npos)
    value.replace (i, 7, "]");

  while ((i = value.find ("&colon;")) != std::string::npos)
    value.replace (i, 7, ":");
}

////////////////////////////////////////////////////////////////////////////////
