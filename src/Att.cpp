////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <text.h>
#include <RX.h>
#include <Color.h>
#include <util.h>
#include <Date.h>
#include <Duration.h>
#include <Context.h>
#include <Att.h>
#include <main.h>

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
  "tags",
  "urgency",
  // Note that annotations are not listed.
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
  "wait",
  "depends",
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
  "word",
  "noword"
};

#define NUM_INTERNAL_NAMES   (sizeof (internalNames)   / sizeof (internalNames[0]))
#define NUM_MODIFIABLE_NAMES (sizeof (modifiableNames) / sizeof (modifiableNames[0]))
#define NUM_MODIFIER_NAMES   (sizeof (modifierNames)   / sizeof (modifierNames[0]))

////////////////////////////////////////////////////////////////////////////////
Att::Att ()
: _name ("")
, _value ("")
, _mod ("")
, _sense ("positive")
{
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, const std::string& value)
{
  _name  = name;
  _value = value;
  _mod   = mod;
  _sense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, const std::string& value,
          const std::string& sense)
{
  _name  = name;
  _value = value;
  _mod   = mod;
  _sense = sense;
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, int value)
{
  _name = name;

  std::stringstream s;
  s << value;
  _value = s.str ();

  _mod = mod;
  _sense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& value)
{
  _name  = name;
  _value = value;
  _mod   = "";
  _sense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, int value)
{
  _name = name;

  std::stringstream s;
  s << value;
  _value = s.str ();

  _mod = "";
  _sense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const Att& other)
{
  _name  = other._name;
  _value = other._value;
  _mod   = other._mod;
  _sense = other._sense;
}

////////////////////////////////////////////////////////////////////////////////
Att& Att::operator= (const Att& other)
{
  if (this != &other)
  {
    _name  = other._name;
    _value = other._value;
    _mod   = other._mod;
    _sense = other._sense;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::operator== (const Att& other) const
{
  return _name  == other._name  &&
         _mod   == other._mod   &&
         _value == other._value &&
         _sense == other._sense;
}

////////////////////////////////////////////////////////////////////////////////
Att::~Att ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Att::logicSense (bool match) const
{
  if (_sense == "positive")
    return match;
  else if (_sense == "negative")
    return ! match;
  else
  {
    context.debug ("_sense: " + _sense);
    throw ("unknown _sense " + _sense);
  }
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

    if (n.skip (':'))
    {
      if (input.find ('@') <= n.cursor () ||
          input.find ('/') <= n.cursor ())
        return false;

      if (n.getQuoted ('"', ignored) ||
          n.getUntil  (' ', ignored) ||
          n.getUntilEOS (ignored)    ||
          n.depleted ())
        return true;
    }
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
  autoComplete (name,
                candidates,
                matches,
                context.config.getInteger ("abbreviation.minimum"));

  if (matches.size () == 0)
    return false;

  else if (matches.size () != 1)
  {
    std::string error = "Ambiguous attribute '" + name + "' - could be either of "; // TODO i18n

    std::sort (matches.begin (), matches.end ());
    std::string combined;
    join (combined, ", ", matches);

    throw error + combined + ".";
  }

  name = matches[0];

  // Second, guess at the modifier name.
  if (mod != "")
  {
    candidates.clear ();
    for (unsigned i = 0; i < NUM_MODIFIER_NAMES; ++i)
      candidates.push_back (modifierNames[i]);

    matches.clear ();
    autoComplete (mod,
                  candidates,
                  matches,
                  context.config.getInteger ("abbreviation.minimum"));

    if (matches.size () == 0)
      throw std::string ("Unrecognized modifier '") + mod + "'.";

    else if (matches.size () != 1)
    {
      std::string error = "Ambiguous modifier '" + mod + "' - could be either of "; // TODO i18n

      std::sort (matches.begin (), matches.end ());
      std::string combined;
      join (combined, ", ", matches);
      error += combined;

      throw error + combined + ".";
    }

    mod = matches[0];
  }

  // Some attributes are intended to be private, unless the command is read-
  // only, in which cased these are perfectly valid elements of a filter.
/*
  if (context.cmd.isWriteCommand () &&
      !validModifiableName (name))
    throw std::string ("\"") +
          name               +
          "\" is not an attribute you may modify directly.";
*/
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
//    if (context.cmd.isWriteCommand ())
    {
      if (value == "")
        throw std::string ("The '") + name + "' attribute must not be blank.";

      if (!noVerticalSpace (value))
        throw std::string ("The '") + name + "' attribute must not contain vertical white space.";
    }
  }

  else if (name == "fg" || name == "bg")
  {
    // TODO Determine whether color abbreviations are supported, and if so,
    //      modify 'value' here accordingly.
  }

  // Dates can now be either a date, or a duration that is added as an offset
  // to the current date.
  else if (name == "due"   ||
           name == "until" ||
           name == "wait")
  {
    // Validate and convert to epoch.
    if (value != "")
    {
      // Try parsing as a duration.  If unsuccessful, try again, as a date.
      try
      {
        Date now;
        Duration dur (value);

        if (dur.negative ())
          value = (now - (time_t)dur).toEpochString ();
        else
          value = (now + (time_t)dur).toEpochString ();
      }

      // If the date parsing failed, try parsing as a duration.  If successful,
      // add the duration to the current date.  If unsuccessful, propagate the
      // original date parse error.

      // Try parsing as a date.  If unsuccessfull, throw.
      catch (...)
      {
        try
        {
          value = Date (value, context.config.get ("dateformat")).toEpochString ();
        }

        catch (std::string& e)
        {
          throw e;
        }
      }
    }
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
    if (value == "" || (value != "page" && !digitsOnly (value)))
      throw std::string ("The '") + name + "' attribute must be an integer, or the value 'page'.";
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
    candidates.push_back ("waiting");
    autoComplete (value,
                  candidates,
                  matches,
                  context.config.getInteger ("abbreviation.minimum"));

    if (matches.size () == 1)
      value = matches[0];
    else
      throw std::string ("\"") +
            value              +
            "\" is not a valid status.  Use 'pending', 'completed', 'deleted', 'recurring' or 'waiting'.";
  }

  else if (! validInternalName   (name) &&
           ! validModifiableName (name))
    throw std::string ("'") + name + "' is not a recognized attribute.";

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Deprecated - remove.
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
  if (name == "due"   ||
      name == "wait"  ||
      name == "until" ||
      name == "start" ||
      name == "entry" ||
      name == "end")
    return "date";

  else if (name == "recur")
    return "duration";

  else if (name == "limit" ||
           name == "urgency")
    return "number";

  else if (name == "priority")
    return "priority";

  else
    return "text";
}

////////////////////////////////////////////////////////////////////////////////
//                                  ______________
//                                  |            |
//                                  |            v
// start --> name --> . --> mod --> : --> " --> value --> " --> end
//            |                     ^              |             ^
//            |_____________________|              |_____________|
//
void Att::parse (const std::string& input)
{
  Nibbler n (input);
  parse (n);
}

void Att::parse (Nibbler& n)
{
  // Ensure a clean object first.
  _name  = "";
  _value = "";
  _mod   = "";
  _sense = "positive";

  if (n.getUntilOneOf (".:", _name))
  {
    if (_name.length () == 0)
      throw std::string ("Missing attribute name"); // TODO i18n

    if (n.skip ('.'))
    {
      std::string mod;

      if (n.skip ('~'))
      {
        context.debug ("using negative logic");
        _sense = "negative";
      }

      if (n.getUntil (":", mod))
      {
        if (validMod (mod))
          _mod = mod;
        else
          throw std::string ("The name '") + mod + "' is not a valid modifier."; // TODO i18n
      }
      else
        throw std::string ("Missing . or : after modifier."); // TODO i18n
    }

    if (n.skip (':'))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', _value) ||
          n.getUntilEOS (_value))
      {
        decode (_value);
      }
    }
    else
      throw std::string ("Missing : after attribute name."); // TODO i18n
  }
  else
    throw std::string ("Missing : after attribute name."); // TODO i18n

/* TODO This might be too slow to include.  Test this assumption.
  validNameValue (_name, _mod, _value);
*/
}

////////////////////////////////////////////////////////////////////////////////
// name : " value "
std::string Att::composeF4 () const
{
  std::string output = "";

  if (_name != "" && _value != "")
  {
    std::string value = _value;
    encode (value);
    enquote (value);

    output += _name + ":" + value;
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
void Att::mod (const std::string& input)
{
  if (input != "" && !validMod (input))
    throw std::string ("The name '") + input + "' is not a valid modifier."; // TODO i18n

  _mod = input;
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::mod () const
{
  return _mod;
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::name () const
{
  return _name;
}

////////////////////////////////////////////////////////////////////////////////
void Att::name (const std::string& name)
{
  _name = name;
}

////////////////////////////////////////////////////////////////////////////////
std::string Att::value () const
{
  return _value;
}

////////////////////////////////////////////////////////////////////////////////
void Att::value (const std::string& value)
{
  _value = value;
}

////////////////////////////////////////////////////////////////////////////////
void Att::allNames (std::vector <std::string>& all)
{
  all.clear ();

  unsigned int i;
  for (i = 0; i < NUM_INTERNAL_NAMES; ++i)
    all.push_back (internalNames[i]);

  for (i = 0; i < NUM_MODIFIABLE_NAMES; ++i)
    all.push_back (modifiableNames[i]);
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
  str_replace (value, "\"", "");
}

////////////////////////////////////////////////////////////////////////////////
// Encode values prior to serialization.
//   \t -> &tab;
//   "  -> &dquot;
//   [  -> &open;
//   ]  -> &close;
//   \  -> \\               (extra chars to disambiguate multi-line comment)
void Att::encode (std::string& value) const
{
  str_replace (value, "\t", "&tab;");
  str_replace (value, "\"", "&dquot;");
  str_replace (value, "[",  "&open;");
  str_replace (value, "]",  "&close;");
  str_replace (value, "\\", "\\\\");
}

////////////////////////////////////////////////////////////////////////////////
// Decode values after parse.
//   \t <- &tab;
//   "  <- &quot; or &dquot;
//   '  <- &squot;
//   ,  <- &comma;
//   [  <- &open;
//   ]  <- &close;
//   :  <- &colon;
void Att::decode (std::string& value) const
{
  // Supported encodings.
  str_replace (value, "&tab;",   "\t");
  str_replace (value, "&dquot;", "\"");
  str_replace (value, "&quot;",  "'");
  str_replace (value, "&open;",  "[");
  str_replace (value, "&close;", "]");

  // Support for deprecated encodings.  These cannot be removed or old files
  // will not be parsable.  Not just old files - completed.data can contain
  // tasks formatted/encoded using these.
  str_replace (value, "&squot;", "'");
  str_replace (value, "&comma;", ",");
  str_replace (value, "&colon;", ":");
}

////////////////////////////////////////////////////////////////////////////////
