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
#include "text.h"
#include "util.h"
#include "Att.h"

////////////////////////////////////////////////////////////////////////////////
Att::Att ()
: mName ("")
, mValue ("")
, mMod ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, const std::string& value)
{
  mName  = name;
  mValue = value;
  mMod   = mod;
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, int value)
{
  mName  = name;

  std::stringstream s;
  s << value;
  mValue = s.str ();

  mMod = mod;
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& value)
{
  mName  = name;
  mValue = value;
  mMod   = "";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, int value)
{
  mName  = name;

  std::stringstream s;
  s << value;
  mValue = s.str ();

  mMod = "";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const Att& other)
{
  mName  = other.mName;
  mValue = other.mValue;
  mMod   = other.mMod;
}

////////////////////////////////////////////////////////////////////////////////
Att& Att::operator= (const Att& other)
{
  if (this != &other)
  {
    mName  = other.mName;
    mValue = other.mValue;
    mMod   = other.mMod;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Att::~Att ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Att::valid (const std::string& input) const
{
  Nibbler n (input);
  std::string ignored;
  if (n.getUntilOneOf (".:", ignored))
  {
    if (ignored.length () == 0)
      return false;

    while (n.skip ('.'))
      if (!n.getUntilOneOf (".:", ignored))
        return false;

    if (n.skip (':') &&
        (n.getQuoted ('"', ignored) ||
         n.getUntil  (' ', ignored) ||
         n.getUntilEOS (ignored)    ||
         n.depleted ()))
      return true;

    return false;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// start --> name --> . --> mod --> : --> " --> value --> " --> end
//            |                     ^
//            |_____________________|
//
void Att::parse (const std::string& input)
{
  Nibbler n (input);
  parse (n);
}

void Att::parse (Nibbler& n)
{
  // Ensure a clean object first.
  mName  = "";
  mValue = "";
  mMod   = "";

  if (n.getUntilOneOf (".:", mName))
  {
    if (mName.length () == 0)
      throw std::string ("Missing attribute name"); // TODO i18n

    if (n.skip ('.'))
    {
      std::string mod;
      if (n.getUntil (":", mod))
      {
        if (validMod (mod))
          mMod = mod;
        else
          throw std::string ("The name '") + mod + "' is not a valid modifier"; // TODO i18n
      }
      else
        throw std::string ("Missing . or : after modifier"); // TODO i18n
    }

    if (n.skip (':'))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted ('"', mValue) ||
          n.getUntil  (' ', mValue))
      {
        decode (mValue);
      }
      else
        throw std::string ("Missing attribute value"); // TODO i18n
    }
    else
      throw std::string ("Missing : after attribute name"); // TODO i18n
  }
  else
    throw std::string ("Missing : after attribute name"); // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
bool Att::validMod (const std::string& mod) const
{
  if (mod == "before"     || mod == "after"    ||   // i18n: TODO
      mod == "under"      || mod == "over"     ||   // i18n: TODO
      mod == "below"      || mod == "above"    ||   // i18n: TODO
      mod == "none"       || mod == "any"      ||   // i18n: TODO
      mod == "is"         || mod == "isnt"     ||   // i18n: TODO
      mod == "has"        || mod == "hasnt"    ||   // i18n: TODO
      mod == "contains"   ||                        // i18n: TODO
      mod == "startswith" || mod == "endswith")     // i18n: TODO
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// "this" is the attribute that has modifiers.  "other" is the attribute from a
// Record that does not have modifiers, but may have a value.
bool Att::match (const Att& other) const
{
  // If there are no mods, just perform a straight compare on value.
  if (mMod == "" && mValue != other.mValue)
    return false;

  // Assume a match, and short-circuit on mismatch.

  // is = equal.  Nop.
  else if (mMod == "is") // TODO i18n
    if (mValue != other.mValue)
      return false;

  // isnt = not equal.
  else if (mMod == "isnt") // TODO i18n
    if (mValue == other.mValue)
      return false;

  // any = any value, but not empty value.
  else if (mMod == "any") // TODO i18n
    if (other.mValue == "")
      return false;

  // none = must have empty value.
  else if (mMod == "none") // TODO i18n
    if (other.mValue != "")
      return false;

  // startswith = first characters must match.
  else if (mMod == "startswith") // TODO i18n
  {
    if (other.mValue.length () < mValue.length ())
      return false;

    if (mValue != other.mValue.substr (0, mValue.length ()))
      return false;
  }

  // endswith = last characters must match.
  else if (mMod == "endswith") // TODO i18n
  {
    if (other.mValue.length () < mValue.length ())
      return false;

    if (mValue != other.mValue.substr (
                    other.mValue.length () - mValue.length (),
                    std::string::npos))
      return false;
  }

  // has = contains as a substring.
  else if (mMod == "has" || mMod == "contains") // TODO i18n
    if (other.mValue.find (mValue) == std::string::npos)
      return false;

  // hasnt = does not contain as a substring.
  else if (mMod == "hasnt") // TODO i18n
    if (other.mValue.find (mValue) != std::string::npos)
      return false;

  // before = under = below = <
  else if (mMod == "before" || mMod == "under" || mMod == "below")
  {
    // TODO Typed compare
    return false;
  }

  // after = over = above = >
  else if (mMod == "after" || mMod == "over" || mMod == "above")
  {
    // TODO Typed compare
    return false;
  }

  return true;
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
void Att::mod (const std::string& input)
{
  if (!validMod (input))
    throw std::string ("The name '") + input + "' is not a valid modifier"; // TODO i18n

  mMod = input;
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::mod () const
{
  return mMod;
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
    value.replace (i, 1, "&tab;"); // no i18n

  while ((i = value.find ('"')) != std::string::npos)
    value.replace (i, 1, "&quot;"); // no i18n

  while ((i = value.find (',')) != std::string::npos)
    value.replace (i, 1, "&comma;"); // no i18n

  while ((i = value.find ('[')) != std::string::npos)
    value.replace (i, 1, "&open;"); // no i18n

  while ((i = value.find (']')) != std::string::npos)
    value.replace (i, 1, "&close;"); // no i18n

  while ((i = value.find (':')) != std::string::npos)
    value.replace (i, 1, "&colon;"); // no i18n
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

  while ((i = value.find ("&tab;")) != std::string::npos) // no i18n
    value.replace (i, 5, "\t");

  while ((i = value.find ("&quot;")) != std::string::npos) // no i18n
    value.replace (i, 6, "\"");

  while ((i = value.find ("&comma;")) != std::string::npos) // no i18n
    value.replace (i, 7, ",");

  while ((i = value.find ("&open;")) != std::string::npos) // no i18n
    value.replace (i, 6, "[");

  while ((i = value.find ("&close;")) != std::string::npos) // no i18n
    value.replace (i, 7, "]");

  while ((i = value.find ("&colon;")) != std::string::npos) // no i18n
    value.replace (i, 7, ":");
}

////////////////////////////////////////////////////////////////////////////////
