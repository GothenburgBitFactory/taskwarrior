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
void get (std::vector <Task>& pending, std::vector <Task>& completed)
{
  TDB tdb;
  tdb.location (".");
  tdb.lock ();
  tdb.loadPending   (pending);
  tdb.loadCompleted (completed);
  tdb.unlock ();
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (22);

  try
  {
    // Remove any residual test file.
    unlink ("./pending.data");
    unlink ("./completed.data");
    unlink ("./undo.data");

    // Set the context to allow GC.
    context.config.set ("gc", "on");

    // Try reading an empty database.
    std::vector <Task> all;
    std::vector <Task> pending;
    std::vector <Task> completed;
    get (pending, completed);
    t.ok (pending.size () == 0, "TDB Read empty pending");
    t.ok (completed.size () == 0, "TDB Read empty completed");

    // Add without commit.
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

  unlink ("./pending.data");
  unlink ("./completed.data");
  unlink ("./undo.data");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

