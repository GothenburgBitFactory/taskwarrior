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
#include <rx.h>
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
static inline std::string& str_replace (
  std::string &str,
  const std::string& search,
  const std::string& replacement)
{
  std::string::size_type pos = 0;
  while ((pos = str.find (search, pos)) != std::string::npos)
  {
    str.replace (pos, search.length (), replacement);
    pos += replacement.length ();
  }

  return str;
}

////////////////////////////////////////////////////////////////////////////////
Att::Att ()
: mName ("")
, mValue ("")
, mMod ("")
, mSense ("positive")
{
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, const std::string& value)
{
  mName  = name;
  mValue = value;
  mMod   = mod;
  mSense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, const std::string& value,
          const std::string& sense)
{
  mName  = name;
  mValue = value;
  mMod   = mod;
  mSense = sense;
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& mod, int value)
{
  mName = name;

  std::stringstream s;
  s << value;
  mValue = s.str ();

  mMod = mod;
  mSense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, const std::string& value)
{
  mName  = name;
  mValue = value;
  mMod   = "";
  mSense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const std::string& name, int value)
{
  mName = name;

  std::stringstream s;
  s << value;
  mValue = s.str ();

  mMod = "";
  mSense = "positive";
}

////////////////////////////////////////////////////////////////////////////////
Att::Att (const Att& other)
{
  mName  = other.mName;
  mValue = other.mValue;
  mMod   = other.mMod;
  mSense = other.mSense;
}

////////////////////////////////////////////////////////////////////////////////
Att& Att::operator= (const Att& other)
{
  if (this != &other)
  {
    mName  = other.mName;
    mValue = other.mValue;
    mMod   = other.mMod;
    mSense = other.mSense;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Att::operator== (const Att& other) const
{
  return mName  == other.mName  &&
         mMod   == other.mMod   &&
         mValue == other.mValue &&
         mSense == other.mSense;
}

////////////////////////////////////////////////////////////////////////////////
Att::~Att ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Att::logicSense (bool match) const
{
  if (mSense == "positive")
    return match;
  else if (mSense == "negative")
    return ! match;
  else
  {
    context.debug ("mSense: " + mSense);
    throw ("unknown mSense " + mSense);
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
  autoComplete (name, candidates, matches);

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
    autoComplete (mod, candidates, matches);

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
    autoComplete (value, candidates, matches);

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
// The type of an attribute is useful for modifier evaluation.
std::string Att::modType (const std::string& name) const
{
  if (name == "hasnt" ||
      name == "isnt"  ||
      name == "not"   ||  // TODO Verify this.
      name == "noword")
    return "negative";

  return "positive";
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
  mName  = "";
  mValue = "";
  mMod   = "";
  mSense = "positive";

  if (n.getUntilOneOf (".:", mName))
  {
    if (mName.length () == 0)
      throw std::string ("Missing attribute name"); // TODO i18n

    if (n.skip ('.'))
    {
      std::string mod;

      if (n.skip ('~'))
      {
        context.debug ("using negative logic");
        mSense = "negative";
      }

      if (n.getUntil (":", mod))
      {
        if (validMod (mod))
          mMod = mod;
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
      if (n.getQuoted   ('"', mValue) ||
          n.getUntilEOS (mValue))
      {
        decode (mValue);
      }
    }
    else
      throw std::string ("Missing : after attribute name."); // TODO i18n
  }
  else
    throw std::string ("Missing : after attribute name."); // TODO i18n

/* TODO This might be too slow to include.  Test this assumption.
  validNameValue (mName, mMod, mValue);
*/
}

////////////////////////////////////////////////////////////////////////////////
// "this" is the attribute that has modifiers.  "other" is the attribute from a
// Record that does not have modifiers, but may have a value.
//
// In other words, the filter:
//   task list description.contains:foo
//
// Is represented with:
//   this  = filter (description.contains:foo)
//   other = actual task data to be matched
//
bool Att::match (const Att& other) const
{
  // All matches are assumed to pass, any short-circuit on non-match.
  bool case_sensitive = context.config.getBoolean ("search.case.sensitive");

  // Are regular expressions being used in place of string comparison?
#ifdef FEATURE_REGEX
  bool regex = context.config.getBoolean ("regex");
#endif

  // If there are no mods, just perform a straight compare on value.
  if (mMod == "")
  {
    // Exact matches on dates should only compare m/d/y, not h:m:s.  This allows
    // comparisons like "task list due:today" (bug #405).
    std::string which = type (mName);
    if (which == "date")
    {
      if (other.mValue == "")
        return false;

      Date left (mValue);
      Date right (other.mValue);

      if (! left.sameDay (right))
        return false;
    }
    else
    {
#ifdef FEATURE_REGEX
      if (regex)
      {
        std::string pattern = "^" + mValue + "$";
        if (!regexMatch (other.mValue, pattern, case_sensitive))
          return false;
      }
      else if (!compare (mValue, other.mValue, (bool) case_sensitive))
        return false;
#else
      if (!compare (mValue, other.mValue, (bool) case_sensitive))
        return false;
#endif
    }
  }

  // has = contains as a substring.
  else if (mMod == "has" || mMod == "contains") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex)
    {
      if (!regexMatch (other.mValue, mValue, case_sensitive))
        return false;
    }
    else if (find (other.mValue, mValue, (bool) case_sensitive) == std::string::npos)
      return false;
#else
    if (find (other.mValue, mValue, (bool) case_sensitive) == std::string::npos)
      return false;
#endif
  }

  // is = equal.  Nop.
  else if (mMod == "is" || mMod == "equals") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex)
    {
      std::string pattern = "^" + mValue + "$";
      if (!regexMatch (other.mValue, pattern, case_sensitive))
        return false;
    }
    else if (!compare (mValue, other.mValue, (bool) case_sensitive))
      return false;
#else
    if (!compare (mValue, other.mValue, (bool) case_sensitive))
      return false;
#endif
  }

  // isnt = not equal.
  else if (mMod == "isnt" || mMod == "not") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex)
    {
      std::string pattern = "^" + mValue + "$";
      if (regexMatch (other.mValue, pattern, case_sensitive))
        return false;
    }
    else if (compare (mValue, other.mValue, (bool) case_sensitive))
      return false;
#else
    if (compare (mValue, other.mValue, (bool) case_sensitive))
      return false;
#endif
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
#ifdef FEATURE_REGEX
    if (regex)
    {
      std::string pattern = "^" + mValue;
      if (!regexMatch (other.mValue, pattern, case_sensitive))
        return false;
    }
    else
    {
#endif
      if (other.mValue.length () < mValue.length ())
        return false;

      if (!compare (mValue, other.mValue.substr (0, mValue.length ()), (bool) case_sensitive))
        return false;
#ifdef FEATURE_REGEX
    }
#endif
  }

  // endswith = last characters must match.
  else if (mMod == "endswith" || mMod == "right") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex)
    {
      std::string pattern = mValue + "$";
      if (!regexMatch (other.mValue, pattern, case_sensitive))
        return false;
    }
    else
    {
#endif
      if (other.mValue.length () < mValue.length ())
        return false;

      if (!compare (mValue, other.mValue.substr (
                      other.mValue.length () - mValue.length (),
                      std::string::npos), (bool) case_sensitive))
        return false;
#ifdef FEATURE_REGEX
    }
#endif
  }

  // hasnt = does not contain as a substring.
  else if (mMod == "hasnt") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex)
    {
      if (regexMatch (other.mValue, mValue, case_sensitive))
        return false;
    }
    else if (find (other.mValue, mValue, (bool) case_sensitive) != std::string::npos)
      return false;
#else
    if (find (other.mValue, mValue, (bool) case_sensitive) != std::string::npos)
      return false;
#endif
  }

  // before = under = below = <
  else if (mMod == "before" || mMod == "under" || mMod == "below")
  {
    std::string which = type (mName);
    if (which == "duration")
    {
      Duration literal (mValue);
      Duration variable ((time_t)atoi (other.mValue.c_str ()));
      if (!(variable < literal))
        return false;
    }
    else if (which == "date")
    {
      Date literal (mValue.c_str (), context.config.get ("dateformat"));
      Date variable ((time_t)atoi (other.mValue.c_str ()));
      if (other.mValue == "" || ! (variable < literal))
        return false;
    }
    else if (which == "number")
    {
      if (atoi (mValue.c_str ()) >= atoi (other.mValue.c_str ()))
        return false;
    }
    else if (which == "text")
    {
      if (mValue <= other.mValue)
        return false;
    }
    else if (which == "priority")
    {
      if (mValue == "" ||
          other.mValue == "H" ||
          mValue == other.mValue ||
          (mValue == "L" && other.mValue == "M"))
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
      Duration variable ((time_t)atoi (other.mValue.c_str ()));
      if (! (variable > literal))
        return false;
    }
    else if (which == "date")
    {
      Date literal (mValue.c_str (), context.config.get ("dateformat"));
      Date variable ((time_t)atoi (other.mValue.c_str ()));
      if (! (variable > literal))
        return false;
    }
    else if (which == "number")
    {
      if (atoi (mValue.c_str ()) <= atoi (other.mValue.c_str ()))
        return false;
    }
    else if (which == "text")
    {
      if (mValue >= other.mValue)
        return false;
    }
    else if (which == "priority")
    {
      if (mValue == "H" ||
          other.mValue == "" ||
          mValue == other.mValue ||
          (mValue == "M" && other.mValue == "L"))
        return false;
    }
  }

  // word = contains as a substring, with word boundaries.
  else if (mMod == "word") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex && other.mName != "tags")
    {
      std::vector <int> start;
      std::vector <int> end;
      if (!regexMatch (start, end, other.mValue, mValue, case_sensitive))
        return false;

      if (!isWordStart (other.mValue, start[0]))
        return false;

      if (!isWordEnd (other.mValue, end[0]))
        return false;
    }
    else
    {
#endif
      // Fail if the substring is not found.
      std::string::size_type sub = find (other.mValue, mValue, (bool) case_sensitive);
      if (sub == std::string::npos)
        return false;

      // Also fail if there is no word boundary at beginning and end.
      if (!isWordStart (other.mValue, sub))
        return false;

      if (!isWordEnd (other.mValue, sub + mValue.length () - 1))
        return false;
#ifdef FEATURE_REGEX
    }
#endif
  }

  // noword = does not contain as a substring, with word boundaries.
  else if (mMod == "noword") // TODO i18n
  {
#ifdef FEATURE_REGEX
    if (regex && other.mName != "tags")
    {
      std::vector <int> start;
      std::vector <int> end;
      if (regexMatch (start, end, other.mValue, mValue, case_sensitive) &&
          isWordStart (other.mValue, start[0])                     &&
          isWordEnd (other.mValue, end[0]))
        return false;
    }
    else
    {
#endif
      // Fail if the substring is not found.
      std::string::size_type sub = find (other.mValue, mValue);
      if (sub != std::string::npos &&
          isWordStart (other.mValue, sub) &&
          isWordEnd (other.mValue, sub + mValue.length () - 1))
      {
        return false;
      }
#ifdef FEATURE_REGEX
    }
#endif
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
    throw std::string ("The name '") + input + "' is not a valid modifier."; // TODO i18n

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
  return atoi (mValue.c_str ());
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
