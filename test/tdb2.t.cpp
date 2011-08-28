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
#include <iostream>
#include <unistd.h>

#include <main.h>
#include <test.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (15);

  try
  {
    // Remove any residual test files.
    rmdir ("./extensions");
    unlink ("./pending.data");
    unlink ("./completed.data");
    unlink ("./undo.data");
    unlink ("./backlog.data");
    unlink ("./synch.key");

    // Set the context to allow GC.
    context.config.set ("gc", "on");
    context.config.set ("debug", "on");

    context.tdb2.set_location (".");

    // Try reading an empty database.
    std::vector <Task> pending          = context.tdb2.pending.get_tasks ();
    std::vector <Task> completed        = context.tdb2.completed.get_tasks ();
    std::vector <std::string> undo      = context.tdb2.undo.get_lines ();
    std::vector <Task> backlog          = context.tdb2.backlog.get_tasks ();
    std::vector <std::string> synch_key = context.tdb2.synch_key.get_lines ();

    t.is ((int) pending.size (),   0, "TDB2 Read empty pending");
    t.is ((int) completed.size (), 0, "TDB2 Read empty completed");
    t.is ((int) undo.size (),      0, "TDB2 Read empty undo");
    t.is ((int) backlog.size (),   0, "TDB2 Read empty backlog");
    t.is ((int) synch_key.size (), 0, "TDB2 Read empty synch_key");

    // Add a task.
    Task task ("[name:\"value\"]");
    context.tdb2.add (task);

    pending   = context.tdb2.pending.get_tasks ();
    completed = context.tdb2.completed.get_tasks ();
    undo      = context.tdb2.undo.get_lines ();
    backlog   = context.tdb2.backlog.get_tasks ();
    synch_key = context.tdb2.synch_key.get_lines ();

    t.is ((int) pending.size (),   1, "TDB2 after add, 1 pending task");
    t.is ((int) completed.size (), 0, "TDB2 after add, 0 completed tasks");
    t.is ((int) undo.size (),      3, "TDB2 after add, 3 undo lines");
    t.is ((int) backlog.size (),   1, "TDB2 after add, 1 backlog task");
    t.is ((int) synch_key.size (), 0, "TDB2 after add, 0 synch_key");

    task.set ("description", "This is a test");
    context.tdb2.modify (task);

    pending   = context.tdb2.pending.get_tasks ();
    completed = context.tdb2.completed.get_tasks ();
    undo      = context.tdb2.undo.get_lines ();
    backlog   = context.tdb2.backlog.get_tasks ();
    synch_key = context.tdb2.synch_key.get_lines ();

    t.is ((int) pending.size (),   1, "TDB2 after add, 1 pending task");
    t.is ((int) completed.size (), 0, "TDB2 after add, 0 completed tasks");
    t.is ((int) undo.size (),      7, "TDB2 after add, 7 undo lines");
    t.is ((int) backlog.size (),   2, "TDB2 after add, 2 backlog task");
    t.is ((int) synch_key.size (), 0, "TDB2 after add, 0 synch_key");

    // TODO commit
    // TODO complete a task
    // TODO gc
  }

  catch (std::string& error)
  {
    t.diag (error);
    return -1;
  }

  catch (...)
  {
    t.diag ("Unknown error.");
    return -2;
  }

  rmdir ("./extensions");
  unlink ("./pending.data");
  unlink ("./completed.data");
  unlink ("./undo.data");
  unlink ("./backlog.data");
  unlink ("./synch.key");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

