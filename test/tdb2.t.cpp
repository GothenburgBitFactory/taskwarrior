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

    TDB2 tdb2;
    tdb2.set_location (".");

    // Try reading an empty database.
    std::vector <Task> pending          = tdb2.pending.get_tasks ();
    std::vector <Task> completed        = tdb2.completed.get_tasks ();
    std::vector <std::string> undo      = tdb2.undo.get_lines ();
    std::vector <std::string> backlog   = tdb2.backlog.get_lines ();
    std::vector <std::string> synch_key = tdb2.synch_key.get_lines ();

    t.is ((int) pending.size (),   0, "TDB2 Read empty pending");
    t.is ((int) completed.size (), 0, "TDB2 Read empty completed");
    t.is ((int) undo.size (),      0, "TDB2 Read empty undo");
    t.is ((int) backlog.size (),   0, "TDB2 Read empty backlog");
    t.is ((int) synch_key.size (), 0, "TDB2 Read empty synch_key");

    // Add a task.
    Task task ("[name:\"value\"]");
    tdb2.add (task);

    pending   = tdb2.pending.get_tasks ();
    completed = tdb2.completed.get_tasks ();
    undo      = tdb2.undo.get_lines ();
    backlog   = tdb2.backlog.get_lines ();
    synch_key = tdb2.synch_key.get_lines ();

    t.is ((int) pending.size (),   1, "TDB2 after add, 1 pending task");
    t.is ((int) completed.size (), 0, "TDB2 after add, 0 completed tasks");
    t.is ((int) undo.size (),      3, "TDB2 after add, 3 undo lines");
    t.is ((int) backlog.size (),   1, "TDB2 after add, 1 backlog line");
    t.is ((int) synch_key.size (), 0, "TDB2 after add, 0 synch_key");


    task.set ("description", "This is a test");
    tdb2.modify (task);

    pending   = tdb2.pending.get_tasks ();
    completed = tdb2.completed.get_tasks ();
    undo      = tdb2.undo.get_lines ();
    backlog   = tdb2.backlog.get_lines ();
    synch_key = tdb2.synch_key.get_lines ();

    t.is ((int) pending.size (),   1, "TDB2 after add, 1 pending task");
    t.is ((int) completed.size (), 0, "TDB2 after add, 0 completed tasks");
    t.is ((int) undo.size (),      7, "TDB2 after add, 7 undo lines");
    t.is ((int) backlog.size (),   2, "TDB2 after add, 2 backlog line");
    t.is ((int) synch_key.size (), 0, "TDB2 after add, 0 synch_key");

/*
    TDB tdb;
    tdb.location (".");
    tdb.lock ();
    Task task ("[name:\"value\"]");
    tdb.add (task);                                                  // P0 C0 N1 M0
    tdb.unlock ();

    pending.clear ();
    completed.clear ();
    get (pending, completed);

    t.ok (pending.size () == 0, "TDB add -> no commit -> empty");
    t.ok (completed.size () == 0, "TDB add -> no commit -> empty");

    // Add with commit.
    tdb.lock ();
    tdb.add (task);                                                  // P0 C0 N1 M0
    tdb.commit ();                                                   // P1 C0 N0 M0
    tdb.unlock ();

    get (pending, completed);
    t.ok (pending.size () == 1, "TDB add -> commit -> saved");
    t.is (pending[0].get ("name"), "value", "TDB load name=value");
    t.is (pending[0].id, 1, "TDB load verification id=1");
    t.ok (completed.size () == 0, "TDB add -> commit -> saved");

    // Update with commit.
    pending.clear ();
    completed.clear ();

    tdb.lock ();
    tdb.load (all);
    all[0].set ("name", "value2");
    tdb.update (all[0]);                                             // P1 C0 N0 M1
    tdb.commit ();                                                   // P1 C0 N0 M0
    tdb.unlock ();

    pending.clear ();
    completed.clear ();
    get (pending, completed);

    t.ok (all.size () == 1, "TDB update -> commit -> saved");
    t.is (all[0].get ("name"), "value2", "TDB load name=value2");
    t.is (all[0].id, 1, "TDB load verification id=1");

    // GC.
    all.clear ();

    tdb.lock ();
    tdb.loadPending (all);
    all[0].setStatus (Task::completed);
    tdb.update (all[0]);                                             // P1 C0 N0 M1
    Task t2 ("[foo:\"bar\" status:\"pending\"]");
    tdb.add (t2);                                                    // P1 C0 N1 M1
    tdb.commit ();
    tdb.unlock ();

    pending.clear ();
    completed.clear ();
    get (pending, completed);

    t.is (pending.size (), (size_t)2,               "TDB before gc pending #2");
    t.is (pending[0].id, 1,                         "TDB before gc pending id 1");
    t.is (pending[0].getStatus (), Task::completed, "TDB before gc pending status completed");
    t.is (pending[1].id, 2,                         "TDB before gc pending id 2");
    t.is (pending[1].getStatus (), Task::pending,   "TDB before gc pending status pending");
    t.is (completed.size (), (size_t)0,             "TDB before gc completed 0");

    tdb.gc ();                                                       // P1 C1 N0 M0

    pending.clear ();
    completed.clear ();
    get (pending, completed);

    t.is (pending.size (), (size_t)1,                 "TDB after gc pending #1");
    t.is (pending[0].id, 1,                           "TDB after gc pending id 2");
    t.is (pending[0].getStatus (), Task::pending,     "TDB after gc pending status pending");
    t.is (completed.size (), (size_t)1,               "TDB after gc completed #1");
    t.is (completed[0].getStatus (), Task::completed, "TDB after gc completed status completed");
*/
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

