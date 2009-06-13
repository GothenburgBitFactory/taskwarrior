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
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "Context.h"
#include "Date.h"
#include "Duration.h"
#include "T.h"
#include "text.h"
#include "util.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// NOTE: These are static arrays only because there is no initializer list for
//       std::vector.
// TODO Obsolete
static const char* colors[] =
{
  "bold",
  "underline",
  "bold_underline",

  "black",
  "red",
  "green",
  "yellow",
  "blue",
  "magenta",
  "cyan",
  "white",

  "bold_black",
  "bold_red",
  "bold_green",
  "bold_yellow",
  "bold_blue",
  "bold_magenta",
  "bold_cyan",
  "bold_white",

  "underline_black",
  "underline_red",
  "underline_green",
  "underline_yellow",
  "underline_blue",
  "underline_magenta",
  "underline_cyan",
  "underline_white",

  "bold_underline_black",
  "bold_underline_red",
  "bold_underline_green",
  "bold_underline_yellow",
  "bold_underline_blue",
  "bold_underline_magenta",
  "bold_underline_cyan",
  "bold_underline_white",

  "on_black",
  "on_red",
  "on_green",
  "on_yellow",
  "on_blue",
  "on_magenta",
  "on_cyan",
  "on_white",

  "on_bright_black",
  "on_bright_red",
  "on_bright_green",
  "on_bright_yellow",
  "on_bright_blue",
  "on_bright_magenta",
  "on_bright_cyan",
  "on_bright_white",
  "",
};

// TODO Obsolete
static const char* attributes[] =
{
  "project",
  "priority",
  "fg",
  "bg",
  "due",
  "entry",
  "start",
  "end",
  "recur",
  "until",
  "mask",
  "imask",
//  "limit",
  "",
};

static const char* modifiableAttributes[] =
{
  "project",
  "priority",
  "fg",
  "bg",
  "due",
  "recur",
  "until",
  "",
};

// TODO Relocate inside Context.
static std::vector <std::string> customReports;

////////////////////////////////////////////////////////////////////////////////
void guess (
  const std::string& type,
  const char** list,
  std::string& candidate)
{
  std::vector <std::string> options;
  for (int i = 0; list[i][0]; ++i)
    options.push_back (list[i]);

  guess (type, options, candidate);
}

////////////////////////////////////////////////////////////////////////////////
bool validPriority (const std::string& input)
{
  if (input != "H" &&
      input != "M" &&
      input != "L" &&
      input != "")
    throw std::string ("\"") +
          input              +
          "\" is not a valid priority.  Use H, M, L or leave blank.";

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Only attributes that are written to the data files.
// TODO Relocate to Att.cpp.
bool isModifiableAttribute (const std::string& name)
{
  if (name == "project"  ||
      name == "priority" ||
      name == "fg"       ||
      name == "bg"       ||
      name == "due"      ||
      name == "recur"    ||
      name == "until")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// All attributes, regardless of usage.
// TODO Relocate to Att.cpp.
bool validAttribute (std::string& name, std::string& value)
{
  guess ("attribute", attributes, name);
  if (name != "")
  {
    if ((name == "fg" || name == "bg") && value != "")
      guess ("color", colors, value);

    else if (name == "due" && value != "")
      Date (value);

    else if (name == "until" && value != "")
      Date (value);

    else if (name == "priority")
    {
      value = upperCase (value);
      return validPriority (value);
    }

    // Some attributes are intended to be private.
    else if (name == "entry" ||
             name == "start" ||
             name == "end"   ||
             name == "mask"  ||
             name == "imask")
      throw std::string ("\"") +
            name               +
            "\" is not an attribute you may modify directly.";

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool validId (const std::string& input)
{
  if (input.length () == 0)
    return false;

  for (size_t i = 0; i < input.length (); ++i)
    if (!::isdigit (input[i]))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool validTag (const std::string& input)
{
  if ((input[0] == '-' || input[0] == '+') &&
       input.length () > 1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool validDescription (const std::string& input)
{
  if (input.length ()                        &&
      input.find ("\r") == std::string::npos &&
      input.find ("\f") == std::string::npos &&
      input.find ("\n") == std::string::npos)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool validDuration (std::string& input)
{
  try         { Duration (input); }
  catch (...) { return false;     }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
void validReportColumns (const std::vector <std::string>& columns)
{
  std::vector <std::string> bad;

  std::vector <std::string>::const_iterator it;
  for (it = columns.begin (); it != columns.end (); ++it)
    if (*it != "id"                   &&
        *it != "uuid"                 &&
        *it != "project"              &&
        *it != "priority"             &&
        *it != "entry"                &&
        *it != "start"                &&
        *it != "end"                  &&
        *it != "due"                  &&
        *it != "age"                  &&
        *it != "age_compact"          &&
        *it != "active"               &&
        *it != "tags"                 &&
        *it != "recur"                &&
        *it != "recurrence_indicator" &&
        *it != "tag_indicator"        &&
        *it != "description_only"     &&
        *it != "description")
      bad.push_back (*it);

  if (bad.size ())
  {
    std::string error;
    join (error, ", ", bad);
    throw std::string ("Unrecognized column name: ") + error;
  }
}

////////////////////////////////////////////////////////////////////////////////
void validSortColumns (
  const std::vector <std::string>& columns,
  const std::vector <std::string>& sortColumns)
{
  std::vector <std::string> bad;
  std::vector <std::string>::const_iterator sc;
  for (sc = sortColumns.begin (); sc != sortColumns.end (); ++sc)
  {
    std::vector <std::string>::const_iterator co;
    for (co = columns.begin (); co != columns.end (); ++co)
      if (sc->substr (0, sc->length () - 1) == *co)
        break;

    if (co == columns.end ())
      bad.push_back (*sc);
  }

  if (bad.size ())
  {
    std::string error;
    join (error, ", ", bad);
    throw std::string ("Sort column is not part of the report: ") + error;
  }
}

////////////////////////////////////////////////////////////////////////////////

