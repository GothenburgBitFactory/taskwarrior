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

#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <main.h>
#include <text.h>
#include <i18n.h>
#include <CmdUrgency.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdUrgency::CmdUrgency ()
{
  _keyword     = "_urgency";
  _usage       = "task <filter> _urgency";
  _description = STRING_CMD_URGENCY_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUrgency::execute (std::string& output)
{
  // Apply filter.
  std::vector <Task> filtered;
  filter (filtered);

  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Display urgency for the selected tasks.
  std::stringstream out;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    if (task->id)
      out << format (STRING_CMD_URGENCY_RESULT,
                     task->id, task->urgency ())
          << "\n";
    else
      out << format (STRING_CMD_URGENCY_RESULT,
                     task->get ("uuid"), task->urgency ())
          << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
