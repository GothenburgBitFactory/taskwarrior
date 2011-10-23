////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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


#define L10N                                           // Localization complete.

#include <sstream>
#include <Context.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
void legacyAttributeCheck (const std::string& name)
{
  // Legacy checks.
  if (name == "fg" || name == "bg")
    context.footnote (format (STRING_LEGACY_FEATURE, name));
}

////////////////////////////////////////////////////////////////////////////////
void legacyColumnMap (std::string& name)
{
  // One-time initialization, on demand.
  static std::map <std::string, std::string> legacyMap;
  if (! legacyMap.size ())
  {
    legacyMap["priority_long"]        = "priority.long";        // Deprecated.
    legacyMap["entry_time"]           = "entry";                // Deprecated.
    legacyMap["start_time"]           = "start";                // Deprecated.
    legacyMap["end_time"]             = "end";                  // Deprecated.
    legacyMap["countdown"]            = "due.countdown";        // Deprecated.
    legacyMap["countdown_compact"]    = "due.countdown";        // Deprecated.
    legacyMap["age"]                  = "entry.age";            // Deprecated.
    legacyMap["age_compact"]          = "entry.age";            // Deprecated.
    legacyMap["active"]               = "start.active";         // Deprecated.
    legacyMap["recurrence_indicator"] = "recur.indicator";      // Deprecated.
    legacyMap["tag_indicator"]        = "tags.indicator";       // Deprecated.
    legacyMap["description_only"]     = "description.desc";     // Deprecated.
  }

  // If a legacy column was used, complain about it, but modify it anyway.
  std::map <std::string, std::string>::iterator found = legacyMap.find (name);
  if (found != legacyMap.end ())
  {
    context.footnote (format (STRING_CMD_CUSTOM_OLD_FIELD, name, found->second));
    name = found->second;
  }
}

////////////////////////////////////////////////////////////////////////////////
void legacySortColumnMap (std::string& name)
{
  // One-time initialization, on demand.
  static std::map <std::string, std::string> legacyMap;
  if (! legacyMap.size ())
  {
    legacyMap["priority_long"]        = "priority";             // Deprecated
    legacyMap["entry_time"]           = "entry";                // Deprecated
    legacyMap["start_time"]           = "start";                // Deprecated
    legacyMap["end_time"]             = "end";                  // Deprecated
    legacyMap["countdown"]            = "due";                  // Deprecated
    legacyMap["countdown_compact"]    = "due";                  // Deprecated
    legacyMap["age"]                  = "entry";                // Deprecated
    legacyMap["age_compact"]          = "entry";                // Deprecated
    legacyMap["active"]               = "start";                // Deprecated
    legacyMap["recurrence_indicator"] = "recur";                // Deprecated
    legacyMap["tag_indicator"]        = "tags";                 // Deprecated
    legacyMap["description_only"]     = "description";          // Deprecated
  }

  // If a legacy column was used, complain about it, but modify it anyway.
  std::map <std::string, std::string>::iterator found = legacyMap.find (name);
  if (found != legacyMap.end ())
  {
    context.footnote (format (STRING_CMD_CUSTOM_OLD_SORT, name, found->second));
    name = found->second;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedColor ()
{
  std::vector <std::string> deprecated;
  std::map <std::string, std::string>::const_iterator it;
  for (it = context.config.begin (); it != context.config.end (); ++it)
  {
    if (it->first.find ("color.") != std::string::npos)
    {
      std::string value = context.config.get (it->first);
      if (value.find ("_") != std::string::npos)
        deprecated.push_back (it->first);
    }
  }

  std::stringstream out;
  if (deprecated.size ())
  {
    out << STRING_CONFIG_DEPRECATED_US
        << "\n";

    std::vector <std::string>::iterator it2;
    for (it2 = deprecated.begin (); it2 != deprecated.end (); ++it2)
      out << "  " << *it2 << "=" << context.config.get (*it2) << "\n";

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string legacyCheckForDeprecatedVariables ()
{
  std::vector <std::string> deprecated;
  std::map <std::string, std::string>::const_iterator it;
  for (it = context.config.begin (); it != context.config.end (); ++it)
  {
    // report.*.limit
    if (it->first.substr (0, 7) == "report." &&
        it->first.substr (it->first.length () - 6) == ".limit")
      deprecated.push_back (it->first);

    if (it->first == "echo.command" ||
        it->first == "edit.verbose")
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
