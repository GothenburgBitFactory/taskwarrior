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

#include <sstream>
#include <stdlib.h>
#include <text.h>
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
  mName  = name;
  mValue = value;

  mMods.clear ();
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, int value)
{
  mName  = name;

  std::stringstream s;
  s << value;
  mValue = s.str ();

  mMods.clear ();
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const Att& other)
{
  mName  = other.mName;
  mValue = other.mValue;
  mMods  = other.mMods;
}

////////////////////////////////////////////////////////////////////////////////
Att& Att::operator= (const Att& other)
{
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
//
// start --> name --> . --> mod --> : --> " --> value --> " --> end
//                    ^          |
//                    |__________|
//
bool Att::parse (Nibbler& n)
{
  // Ensure a clean object first.
  mName = "";
  mValue = "";
  mMods.clear ();

  if (n.getUntilOneOf (".:", mName))
  {
    while (n.skip ('.'))
    {
      std::string mod;
      if (n.getUntilOneOf (".:", mod))
        mMods.push_back (mod);
      else
        throw std::string ("Missing . or : after modifier");
    }

    if (n.skip (':'))
    {
      if (n.getQuoted ('"', mValue))
        return true;
      else if (n.getUntil (' ', mValue))
        return true;

      throw std::string ("Missing attribute value");
    }
    else
      throw std::string ("Missing : after attribute name");
  }
  else
    throw std::string ("Missing : after attribute name");

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// name : " value "
std::string Att::composeF4 () const
{
  std::string output = "";

  if (mName != "" && mValue != "")
  {
    std::string value = mValue;
    encode (value);
    enquote (value);

    output += mName + ":" + value;
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
void Att::addMod (const Mod& mod)
{
  mMods.push_back (mod);
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
  return ::atoi (mValue.c_str ());
}

////////////////////////////////////////////////////////////////////////////////
void Att::value_int (int value)
{
  std::stringstream s;
  s << value;
  mValue = s.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Add quotes.
void Att::enquote (std::string& value) const
{
  value = '"' + value + '"';
}

////////////////////////////////////////////////////////////////////////////////
// Remove quotes.  Instead of being picky, just remove them all.  There should
// be none within the value, and this will correct for one possible corruption
// that hand-editing the pending.data file could cause.
void Att::dequote (std::string& value) const
{
  std::string::size_type quote;
  while ((quote = value.find ('"')) != std::string::npos)
    value.replace (quote, 1, "");
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
