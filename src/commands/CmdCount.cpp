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

#include <sstream>
#include <Context.h>
#include <main.h>
#include <CmdCount.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdCount::CmdCount ()
{
  _keyword     = "count";
  _usage       = "task count [<filter>]";
  _description = "Shows only the number of matching tasks.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCount::execute (const std::string& command_line, std::string& output)
{
  // Scan the pending tasks, applying any filter.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Find number of matching tasks.  Skip recurring parent tasks.
  int count = 0;
  std::vector <Task>::iterator it;
  for (it = tasks.begin (); it != tasks.end (); ++it)
    if (it->getStatus () != Task::recurring)
      ++count;

  std::stringstream out;
  out << count << "\n";
  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
