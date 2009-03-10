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
#include <unistd.h>

#include "../TDB.h"
#include "../task.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  UnitTest t (38);

  try
  {
    // Remove any residual test file.
    unlink ("./pending.data");
    unlink ("./completed.data");

    // Try reading an empty database.
    TDB tdb;
    tdb.dataDirectory (".");
    std::vector <T> all;
    t.ok (!tdb.pendingT (all), "TDB::pendingT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (!tdb.allPendingT (all), "TDB::allPendingT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (!tdb.completedT (all), "TDB::completedT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (!tdb.allCompletedT (all), "TDB::allCompletedT read empty db");
    t.is ((int) all.size (), 0, "empty db");

    // Add a new task.
    T t1;
    t1.setId (1);
    t1.setStatus (T::pending);
    t1.setAttribute ("project", "p1");
    t1.setDescription ("task 1");
    t.diag (t1.compose ());
    t.ok (tdb.addT (t1), "TDB::addT t1");

    // Verify as above.
    t.ok (tdb.pendingT (all), "TDB::pendingT read db");
    t.is ((int) all.size (), 1, "empty db");
    t.ok (tdb.allPendingT (all), "TDB::allPendingT read db");
    t.is ((int) all.size (), 1, "empty db");
    t.ok (!tdb.completedT (all), "TDB::completedT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (!tdb.allCompletedT (all), "TDB::allCompletedT read empty db");
    t.is ((int) all.size (), 0, "empty db");

    // TODO Modify task.

    // Complete task.
    t.ok (tdb.completeT (t1), "TDB::completeT t1");;
    t.ok (tdb.pendingT (all), "TDB::pendingT read db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (tdb.allPendingT (all), "TDB::allPendingT read db");
    t.is ((int) all.size (), 1, "empty db");
    t.ok (!tdb.completedT (all), "TDB::completedT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (!tdb.allCompletedT (all), "TDB::allCompletedT read empty db");
    t.is ((int) all.size (), 0, "empty db");

    t.is (tdb.gc (), 1, "TDB::gc");
    t.ok (tdb.pendingT (all), "TDB::pendingT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (tdb.allPendingT (all), "TDB::allPendingT read empty db");
    t.is ((int) all.size (), 0, "empty db");
    t.ok (tdb.completedT (all), "TDB::completedT read db");
    t.is ((int) all.size (), 1, "empty db");
    t.ok (tdb.allCompletedT (all), "TDB::allCompletedT read db");
    t.is ((int) all.size (), 1, "empty db");

    // Add a new task.
    T t2;
    t2.setId (1);
    t2.setAttribute ("project", "p2");
    t2.setDescription ("task 2");
    t.ok (tdb.addT (t2), "TDB::addT t2");

    // Delete task.
    t.ok (tdb.deleteT (t2), "TDB::deleteT t2");

    // GC the files.
    t.is (tdb.gc (), 1, "1 <- TDB::gc");
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

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

