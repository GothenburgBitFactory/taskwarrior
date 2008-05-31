////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
  plan (43);

  try
  {
    // Remove any residual test file.
    unlink ("./pending.data");
    unlink ("./completed.data");

    // Try reading an empty database.
    TDB tdb;
    tdb.dataDirectory (".");
    std::vector <T> all;
    ok (!tdb.pendingT (all), "TDB::pendingT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (!tdb.allPendingT (all), "TDB::allPendingT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (!tdb.completedT (all), "TDB::completedT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (!tdb.allCompletedT (all), "TDB::allCompletedT read empty db");
    is ((int) all.size (), 0, "empty db");

    // Add a new task.
    T t1;
    t1.setId (1);
    t1.setStatus (T::pending);
    t1.setAttribute ("project", "p1");
    t1.setDescription ("task 1");
    diag (t1.compose ());
    ok (tdb.addT (t1), "TDB::addT t1");

    // Verify as above.
    ok (tdb.pendingT (all), "TDB::pendingT read db");
    is ((int) all.size (), 1, "empty db");
    ok (tdb.allPendingT (all), "TDB::allPendingT read db");
    is ((int) all.size (), 1, "empty db");
    ok (!tdb.completedT (all), "TDB::completedT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (!tdb.allCompletedT (all), "TDB::allCompletedT read empty db");
    is ((int) all.size (), 0, "empty db");

    // TODO Modify task.
    fail ("modify");
    fail ("verify");

    // Complete task.
    ok (tdb.completeT (t1), "TDB::completeT t1");;
    ok (!tdb.pendingT (all), "TDB::pendingT read db");
    is ((int) all.size (), 0, "empty db");
    ok (tdb.allPendingT (all), "TDB::allPendingT read db");
    is ((int) all.size (), 1, "empty db");
    ok (!tdb.completedT (all), "TDB::completedT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (!tdb.allCompletedT (all), "TDB::allCompletedT read empty db");
    is ((int) all.size (), 0, "empty db");

    is (tdb.gc (), 1, "TDB::gc");
    ok (!tdb.pendingT (all), "TDB::pendingT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (!tdb.allPendingT (all), "TDB::allPendingT read empty db");
    is ((int) all.size (), 0, "empty db");
    ok (tdb.completedT (all), "TDB::completedT read db");
    is ((int) all.size (), 1, "empty db");
    ok (tdb.allCompletedT (all), "TDB::allCompletedT read db");
    is ((int) all.size (), 1, "empty db");

    // Add a new task.
    T t2;
    t2.setId (2);
    t2.setAttribute ("project", "p2");
    t2.setDescription ("task 2");
    diag (t2.compose ());
    ok (tdb.addT (t2), "TDB::addT t2");

    fail ("verify");

    // Delete task.
    ok (tdb.deleteT (t2), "TDB::deleteT t2");

    fail ("verify");

    // GC the files.
    is (tdb.gc (), 1, "1 <- TDB::gc");

    // Read log file.
    std::vector <std::string> entries;
    tdb.logRead (entries);
    std::vector <std::string>::iterator it;
    for (it = entries.begin (); it != entries.end (); ++it)
      diag (*it);

    // TODO Verify contents of above transactions.
    fail ("verify");
  }

  catch (std::string& error)
  {
    diag (error);
    return -1;
  }

  catch (...)
  {
    diag ("Unknown error.");
    return -2;
  }


  unlink ("./pending.data");
  unlink ("./completed.data");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

