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
#include <text.h>
#include <i18n.h>
#include <CmdCount.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCount::CmdCount ()
{
  _keyword     = "count";
  _usage       = "task count [<filter>]";
  _description = STRING_CMD_COUNT_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCount::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  std::vector <Task> filtered;
  filter (filtered);
  context.tdb2.commit ();

  // Find number of matching tasks.  Skip recurring parent tasks.
  int count = 0;
  std::vector <Task>::iterator it;
  for (it = filtered.begin (); it != filtered.end (); ++it)
    if (it->getStatus () != Task::recurring)
      ++count;

  output = format (count) + "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
