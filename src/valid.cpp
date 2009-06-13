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
#include "color.h"

extern Context context;

// TODO Relocate inside Context.
static std::vector <std::string> customReports;

////////////////////////////////////////////////////////////////////////////////
bool validTag (const std::string& input)
{
  if ((input[0] == '-' || input[0] == '+') &&
       input.length () > 1                 &&
       noSpaces (input))
    return true;

  return false;
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

