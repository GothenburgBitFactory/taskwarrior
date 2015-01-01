////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <cstddef>
#include <sstream>
#include <Context.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
void legacyAttributeCheck (const std::string& name)
{
  // 2013-07-25: Deprecated "fg" and "bg" removed.
}

////////////////////////////////////////////////////////////////////////////////
void legacyColumnMap (std::string& name)
{
  // 2014-01-26: priority_long        --> priority.long        Mapping removed
  // 2014-01-26: entry_time           --> entry                Mapping removed
  // 2014-01-26: start_time           --> start                Mapping removed
  // 2014-01-26: end_time             --> end                  Mapping removed
  // 2014-01-26: countdown            --> due.countdown        Mapping removed
  // 2014-01-26: countdown_compact    --> due.countdown        Mapping removed
  // 2014-01-26: age                  --> entry.age            Mapping removed
  // 2014-01-26: age_compact          --> entry.age            Mapping removed
  // 2014-01-26: active               --> start.active         Mapping removed
  // 2014-01-26: recurrence_indicator --> recur.indicator      Mapping removed
  // 2014-01-26: tag_indicator        --> tags.indicator       Mapping removed
  // 2014-01-26: description_only     --> description.desc     Mapping removed
}

////////////////////////////////////////////////////////////////////////////////
void legacySortColumnMap (std::string& name)
{
  // 2014-01-26: priority_long        --> priority             Mapping removed
  // 2014-01-26: entry_time           --> entry                Mapping removed
  // 2014-01-26: start_time           --> start                Mapping removed
  // 2014-01-26: end_time             --> end                  Mapping removed
  // 2014-01-26: countdown            --> due                  Mapping removed
  // 2014-01-26: countdown_compact    --> due                  Mapping removed
  // 2014-01-26: age                  --> entry                Mapping removed
  // 2014-01-26: age_compact          --> entry                Mapping removed
  // 2014-01-26: active               --> start                Mapping removed
  // 2014-01-26: recurrence_indicator --> recur                Mapping removed
  // 2014-01-26: tag_indicator        --> tags                 Mapping removed
  // 2014-01-26: description_only     --> description          Mapping removed
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedColor ()
{
  // 2014-01-26: Color defs containing '_' removed.

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedVariables ()
{
  std::vector <std::string> deprecated;
  std::map <std::string, std::string>::const_iterator it;
  for (it = context.config.begin (); it != context.config.end (); ++it)
  {
    // 2014-07-04: report.*.limit removed.

    // report.*.annotations
    if (it->first.length () > 19 &&
        it->first.substr (0, 7) == "report." &&
        it->first.substr (it->first.length () - 12) == ".annotations")
      deprecated.push_back (it->first);

    if (it->first == "next"              ||
        it->first == "annotations"       ||
        it->first == "export.ical.class")
      deprecated.push_back (it->first);

    // Deprecated іn 2.4.0.
    if (it->first == "alias._query")
      deprecated.push_back (it->first);
  }

  std::stringstream out;
  if (deprecated.size ())
  {
    out << STRING_CONFIG_DEPRECATED_VAR
        << "\n";

    std::vector <std::string>::iterator it2;
    for (it2 = deprecated.begin (); it2 != deprecated.end (); ++it2)
      out << "  " << *it2 << "\n";

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedColumns ()
{
  std::vector <std::string> deprecated;
  std::map <std::string, std::string>::const_iterator it;
  for (it = context.config.begin (); it != context.config.end (); ++it)
  {
    if (it->first.find ("report") == 0)
    {
      std::string value = context.config.get (it->first);
      if (value.find ("entry_time") != std::string::npos ||
          value.find ("start_time") != std::string::npos ||
          value.find ("end_time")   != std::string::npos)
        deprecated.push_back (it->first);
    }
  }

  std::stringstream out;
  out << "\n";

  if (deprecated.size ())
  {
    out << STRING_CONFIG_DEPRECATED_COL
        << "\n";

    std::vector <std::string>::iterator it2;
    for (it2 = deprecated.begin (); it2 != deprecated.end (); ++it2)
      out << "  " << *it2 << "=" << context.config.get (*it2) << "\n";

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
void legacyAttributeMap (std::string& name)
{
  // TW-1274
  if (name == "modification")
    name = "modified";
}

////////////////////////////////////////////////////////////////////////////////
void legacyValueMap (const std::string& name, std::string& value)
{
  // 2014-07-03: One-time initialization value mapping.
  static std::map <std::string, std::string> mapping;
  if (mapping.size () == 0)
  {
    mapping["hrs"]   = "hours";
    mapping["hrs"]   = "hours";
    mapping["hr"]    = "hours";
    mapping["mins"]  = "minutes";
    mapping["mnths"] = "months";
    mapping["mths"]  = "months";
    mapping["mth"]   = "months";
    mapping["mos"]   = "months";
    mapping["m"]     = "months";
    mapping["qrtrs"] = "quarters";
    mapping["qtrs"]  = "quarters";
    mapping["qtr"]   = "quarters";
    mapping["secs"]  = "seconds";
    mapping["sec"]   = "seconds";
    mapping["s"]     = "seconds";
    mapping["wks"]   = "weeks";
    mapping["wk"]    = "weeks";
    mapping["yrs"]   = "years";
    mapping["yr"]    = "years";
  }

  if (name == "recur")
  {
    std::size_t letter = value.find_first_not_of ("0123456789.");
    if (letter == std::string::npos)
      letter = 0;

    std::map <std::string, std::string>::iterator i = mapping.find (value.substr (letter));
    if (i != mapping.end ())
    {
      if (letter)
        value = value.substr (0, letter) + i->second;
      else
        value = i->second;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
