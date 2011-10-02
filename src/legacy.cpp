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

#define L10N                                           // Localization complete.

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
