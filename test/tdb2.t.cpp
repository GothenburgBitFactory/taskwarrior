////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <main.h>
#include <test.h>

Context context;

void cleardb ()
{
    // Remove any residual test files.
    rmdir ("./extensions");
    unlink ("./pending.data");
    unlink ("./completed.data");
    unlink ("./undo.data");
    unlink ("./backlog.data");
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (12);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  try
  {
    cleardb ();

    // Set the context to allow GC.
    context.config.set ("gc", 1);
    context.config.set ("debug", 1);

    context.tdb2.open_replica (".", true);

    // Try reading an empty database.
    std::vector <Task> pending          = context.tdb2.pending_tasks ();
    std::vector <Task> completed        = context.tdb2.completed_tasks ();
    int num_reverts_possible            = context.tdb2.num_reverts_possible ();
    int num_local_changes               = context.tdb2.num_local_changes ();

    t.is ((int) pending.size (),      0, "TDB2 Read empty pending");
    t.is ((int) completed.size (),    0, "TDB2 Read empty completed");
    t.is ((int) num_reverts_possible, 0, "TDB2 Read empty undo");
    t.is ((int) num_local_changes,    0, "TDB2 Read empty backlog");

    // Add a task.
    Task task (R"([description:"description" name:"value"])");
    context.tdb2.add (task);

    pending              = context.tdb2.pending_tasks ();
    completed            = context.tdb2.completed_tasks ();
    num_reverts_possible = context.tdb2.num_reverts_possible ();
    num_local_changes    = context.tdb2.num_local_changes ();

    t.is ((int) pending.size (),      1, "TDB2 after add, 1 pending task");
    t.is ((int) completed.size (),    0, "TDB2 after add, 0 completed tasks");
    t.is ((int) num_reverts_possible, 3, "TDB2 after add, 3 undo lines");
    t.is ((int) num_local_changes,    1, "TDB2 after add, 1 backlog task");

    task.set ("description", "This is a test");
    context.tdb2.modify (task);

    pending              = context.tdb2.pending_tasks ();
    completed            = context.tdb2.completed_tasks ();
    num_reverts_possible = context.tdb2.num_reverts_possible ();
    num_local_changes    = context.tdb2.num_local_changes ();

    t.is ((int) pending.size (),      1, "TDB2 after add, 1 pending task");
    t.is ((int) completed.size (),    0, "TDB2 after add, 0 completed tasks");
    t.is ((int) num_reverts_possible, 7, "TDB2 after add, 7 undo lines");
    t.is ((int) num_local_changes,    2, "TDB2 after add, 2 backlog task");

    // Reset for reuse.
    cleardb ();
    context.tdb2.open_replica (".", true);

    // TODO complete a task
    // TODO gc
  }

  catch (const std::string& error)
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

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

