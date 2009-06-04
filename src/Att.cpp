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
    if (mName.length () == 0)
      throw std::string ("Missing attribute name");

    while (n.skip ('.'))
    {
      std::string mod;
      if (n.getUntilOneOf (".:", mod))
      {
        if (validMod (mod))
          mMods.push_back (mod);
        else
          throw std::string ("The name '") + mod + "' is not a valid modifier";
      }
      else
        throw std::string ("Missing . or : after modifier");
    }

    if (n.skip (':'))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted ('"', mValue) ||
          n.getUntil  (' ', mValue))
      {
        decode (mValue);
        return true;
      }
      else
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
bool Att::validMod (const std::string& mod)
{
  if (mod == "before"     || mod == "after"    ||   // i18n: TODO
      mod == "not"        ||                        // i18n: TODO
      mod == "none"       || mod == "any"      ||   // i18n: TODO
      mod == "synth"      ||                        // i18n: TODO
      mod == "under"      || mod == "over"     ||   // i18n: TODO
      mod == "first"      || mod == "last"     ||   // i18n: TODO
      mod == "this"       ||                        // i18n: TODO
      mod == "next"       ||                        // i18n: TODO
      mod == "is"         || mod == "isnt"     ||   // i18n: TODO
      mod == "has"        || mod == "hasnt"    ||   // i18n: TODO
      mod == "startswith" || mod == "endswith")     // i18n: TODO
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::evalMod (Att& other)
{
  // No modifier means automatic pass.
/*
  if (*this == "") // i18n: no
    return true;
*/

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

/*
  if (*this == "is")    // i18n: TODO
    return *this == other ? true : false;

  if (*this == "isnt")  // i18n: TODO
    return *this != other ? true : false;
*/

  // TODO has
  // TODO hasnt
  // TODO startswith
  // TODO endswith

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
void Att::addMod (const std::string& mod)
{
  if (validMod (mod))
    mMods.push_back (mod);
  else
    throw std::string ("The name '") + mod + "' is not a valid modifier";
}

////////////////////////////////////////////////////////////////////////////////
void Att::mods (std::vector <std::string>& all)
{
  all = mMods;
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
