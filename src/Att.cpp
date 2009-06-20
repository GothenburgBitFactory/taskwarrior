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

#include <iostream> // TODO Remove
#include <sstream>
#include <stdlib.h>
#include "text.h"
#include "color.h"
#include "util.h"
#include "Date.h"
#include "Duration.h"
#include "Context.h"
#include "Att.h"

extern Context context;

static const char* internalNames[] =
{
  "entry",
  "start",
  "end",
  "parent",
  "uuid",
  "mask",
  "imask",
  "limit",
  "status",
  "description",
};

static const char* modifiableNames[] =
{
  "project",
  "priority",
  "fg",
  "bg",
  "due",
  "recur",
  "until",
};

// Synonyms on the same line.
static const char* modifierNames[] =
{
  "before",     "under",    "below",
  "after",      "over",     "above",
  "none",
  "any",
  "is",         "equals",
  "isnt",       "not",
  "has",        "contains",
  "hasnt",
  "startswith", "left",
  "endswith",   "right",
};

#define NUM_INTERNAL_NAMES   (sizeof (internalNames)   / sizeof (internalNames[0]))
#define NUM_MODIFIABLE_NAMES (sizeof (modifiableNames) / sizeof (modifiableNames[0]))
#define NUM_MODIFIER_NAMES   (sizeof (modifierNames)   / sizeof (modifierNames[0]))

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
// For parsing.
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
bool Att::validInternalName (const std::string& name)
{
  for (unsigned int i = 0; i < NUM_INTERNAL_NAMES; ++i)
    if (name == internalNames[i])
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::validModifiableName (const std::string& name)
{
  for (unsigned int i = 0; i < NUM_MODIFIABLE_NAMES; ++i)
    if (name == modifiableNames[i])
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::validNameValue (
  const std::string& name,
  const std::string& mod,
  const std::string& value)
{
  std::string writableName  = name;
  std::string writableMod   = mod;
  std::string writableValue = value;
  return Att::validNameValue (writableName, writableMod, writableValue);
}

////////////////////////////////////////////////////////////////////////////////
bool Att::validNameValue (
  std::string& name,
  std::string& mod,
  std::string& value)
{
  // First, guess at the full attribute name.
  std::vector <std::string> candidates;
  for (unsigned i = 0; i < NUM_INTERNAL_NAMES; ++i)
    candidates.push_back (internalNames[i]);

  for (unsigned i = 0; i < NUM_MODIFIABLE_NAMES; ++i)
    candidates.push_back (modifiableNames[i]);

  std::vector <std::string> matches;
  autoComplete (name, candidates, matches);

  if (matches.size () == 0)
//    throw std::string ("Unrecognized attribute '") + name + "'";
    return false;

  else if (matches.size () != 1)
  {
    std::string error = "Ambiguous attribute '" + name + "' - could be either of "; // TODO i18n

    std::string combined;
    join (combined, ", ", matches);

    throw error + combined;
  }

  name = matches[0];

  // Second, guess at the modifier name.
  if (mod != "")
  {
    candidates.clear ();
    for (unsigned i = 0; i < NUM_MODIFIER_NAMES; ++i)
      candidates.push_back (modifierNames[i]);

    matches.clear ();
    autoComplete (mod, candidates, matches);

    if (matches.size () == 0)
      throw std::string ("Unrecognized modifier '") + mod + "'";

    else if (matches.size () != 1)
    {
      std::string error = "Ambiguous modifier '" + mod + "' - could be either of "; // TODO i18n

      std::string combined;
      join (combined, ", ", matches);
      error += combined;

      throw error + combined;
    }

    mod = matches[0];
  }

  // Some attributes are intended to be private, unless the command is read-
  // only, in which cased these are perfectly valid elements of a filter.
  if (context.cmd.isWriteCommand () &&
      !validModifiableName (name))
    throw std::string ("\"") +
          name               +
          "\" is not an attribute you may modify directly.";

  // Thirdly, make sure the value has the expected form or values.
  if (name == "project")
  {
    if (!noSpaces (value))
      throw std::string ("The '") + name + "' attribute may not contain spaces.";
  }

  else if (name == "priority")
  {
    if (value != "")
    {
      value = upperCase (value);
      if (value != "H" &&
          value != "M" &&
          value != "L")
        throw std::string ("\"") +
              value              +
              "\" is not a valid priority.  Use H, M, L or leave blank.";
    }
  }

  else if (name == "description")
  {
    if (context.cmd.isWriteCommand ())
    {
      if (value == "")
        throw std::string ("The '") + name + "' attribute must not be blank.";

      if (!noVerticalSpace (value))
        throw std::string ("The '") + name + "' attribute must not contain vertical white space.";
    }
  }

  else if (name == "fg" || name == "bg")
  {
    if (value != "")
      Text::guessColor (value);
  }

  else if (name == "due" ||
           name == "until")
  {
    // Validate and convert to epoch.
    if (value != "")
      value = Date (value, context.config.get ("dateformat", "m/d/Y")).toEpochString ();
  }

  else if (name == "recur")
  {
    // Just validate, don't convert to days.
    Duration d;
    if (value != "")
      d.parse (value);
  }

  else if (name == "limit")
  {
    if (value == "" || !digitsOnly (value))
      throw std::string ("The '") + name + "' attribute must be an integer.";
  }

  else if (name == "status")
  {
    value = lowerCase (value);

    std::vector <std::string> matches;
    std::vector <std::string> candidates;
    candidates.push_back ("pending");
    candidates.push_back ("completed");
    candidates.push_back ("deleted");
    candidates.push_back ("recurring");
    autoComplete (value, candidates, matches);

    if (matches.size () == 1)
      value = matches[0];
    else
      throw std::string ("\"") +
            value              +
            "\" is not a valid status.  Use 'pending', 'completed', 'deleted' or 'recurring'.";
  }

  else if (! validInternalName (name) &&
           ! validModifiableName (name))
    throw std::string ("'") + name + "' is not a recognized attribute.";

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Obsolete
bool Att::validMod (const std::string& mod)
{
  for (unsigned int i = 0; i < NUM_MODIFIER_NAMES; ++i)
    if (modifierNames[i] == mod)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// The type of an attribute is useful for modifier evaluation.
std::string Att::type (const std::string& name) const
{
  if (name == "due" ||
      name == "until" ||
      name == "start" ||
      name == "entry" ||
      name == "end")
    return "date";

  else if (name == "recur")
    return "duration";

  else if (name == "limit")
    return "number";

  else
    return "text";
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
    }
    else
      throw std::string ("Missing : after attribute name"); // TODO i18n
  }
  else
    throw std::string ("Missing : after attribute name"); // TODO i18n

/* TODO This might be too slow to include.  Test.
  validNameValue (mName, mMod, mValue);
*/
}

////////////////////////////////////////////////////////////////////////////////
// "this" is the attribute that has modifiers.  "other" is the attribute from a
// Record that does not have modifiers, but may have a value.
bool Att::match (const Att& other) const
{
  // All matches are assumed to pass, any short-circuit on non-match.

  // If there are no mods, just perform a straight compare on value.
  if (mMod == "")
  {
    if (mValue != other.mValue)
      return false;
  }

  // is = equal.  Nop.
  else if (mMod == "is" || mMod == "equals") // TODO i18n
  {
    if (mValue != other.mValue)
      return false;
  }

  // isnt = not equal.
  else if (mMod == "isnt" || mMod == "not") // TODO i18n
  {
    if (mValue == other.mValue)
      return false;
  }

  // any = any value, but not empty value.
  else if (mMod == "any") // TODO i18n
  {
    if (other.mValue == "")
      return false;
  }

  // none = must have empty value.
  else if (mMod == "none") // TODO i18n
  {
    if (other.mValue != "")
      return false;
  }

  // startswith = first characters must match.
  else if (mMod == "startswith" || mMod == "left") // TODO i18n
  {
    if (other.mValue.length () < mValue.length ())
      return false;

    if (mValue != other.mValue.substr (0, mValue.length ()))
      return false;
  }

  // endswith = last characters must match.
  else if (mMod == "endswith" || mMod == "right") // TODO i18n
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
  {
    if (other.mValue.find (mValue) == std::string::npos)
      return false;
  }

  // hasnt = does not contain as a substring.
  else if (mMod == "hasnt") // TODO i18n
  {
    if (other.mValue.find (mValue) != std::string::npos)
      return false;
  }

  // before = under = below = <
  else if (mMod == "before" || mMod == "under" || mMod == "below")
  {
    std::string which = type (mName);
    if (which == "duration")
    {
      Duration literal (mValue);
      Duration variable ((time_t)::atoi (other.mValue.c_str ()));
      if (!(variable < literal))
        return false;
    }
    else if (which == "date")
    {
      Date literal (mValue);
      Date variable ((time_t)::atoi (other.mValue.c_str ()));
      if (! (variable < literal))
        return false;
    }
    else if (which == "number")
    {
      if (::atoi (mValue.c_str ()) >= ::atoi (other.mValue.c_str ()))
        return false;
    }
    else if (which == "text")
    {
      if (::strcmp (mValue.c_str (), other.mValue.c_str ()) <= 0)
        return false;
    }
  }

  // after = over = above = >
  else if (mMod == "after" || mMod == "over" || mMod == "above")
  {
    std::string which = type (mName);
    if (which == "duration")
    {
      Duration literal (mValue);
      Duration variable ((time_t)::atoi (other.mValue.c_str ()));
      if (! (variable > literal))
        return false;
    }
    else if (which == "date")
    {
      Date literal (mValue);
      Date variable ((time_t)::atoi (other.mValue.c_str ()));
      if (! (variable > literal))
        return false;
    }
    else if (which == "number")
    {
      if (::atoi (mValue.c_str ()) <= ::atoi (other.mValue.c_str ()))
        return false;
    }
    else if (which == "text")
    {
      if (::strcmp (mValue.c_str (), other.mValue.c_str ()) >= 0)
        return false;
    }
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
  if (input != "" && !validMod (input))
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
