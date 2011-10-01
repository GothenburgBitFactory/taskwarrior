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
#include <main.h>
#include <i18n.h>
#include <CmdExport.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdExport::CmdExport ()
{
  _keyword     = "export";
  _usage       = "task <filter> export";
  _description = STRING_CMD_EXPORT_USAGE;
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdExport::execute (std::string& output)
{
  int rc = 0;

  // Apply filter.
  std::vector <Task> filtered;
  filter (filtered);

  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_MATCH);
    return 1;
  }

  // Note: "limit:" feature not supported.
  // TODO Why not?

  // Is output contained within a JSON array?
  bool json_array = context.config.getBoolean ("json.array");

  // Compose output.
  if (json_array)
    output += "[\n";

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    if (task != filtered.begin ())
      output += ",\n";

    output += task->composeJSON (true);
  }

  if (json_array)
    output += "\n]";

  output += "\n";
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
