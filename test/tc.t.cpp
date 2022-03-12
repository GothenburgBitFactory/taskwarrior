////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2022, Dustin J. Mitchell
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
#include "test.h"
#include "tc/Replica.h"
#include "tc/WorkingSet.h"
#include "tc/Task.h"
#include "tc/util.h"

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (21);

  // This function contains unit tests for the various bits of the wrappers for
  // taskchampion-lib (that is, for `src/tc/*.cpp`).

  //// util

  {
    auto s1 = std::string ("a\0string!");
    auto stc = tc::string2tc (s1);
    auto s2 = tc::tc2string (stc);
    t.is (s1, s2, "round-trip to tc string and back (containing an embedded NUL)");
  }

  {
    auto s1 = std::string ("62123ec9-c443-4f7e-919a-35362a8bef8d");
    auto tcuuid = tc::uuid2tc (s1);
    auto s2 = tc::tc2uuid (tcuuid);
    t.is(s1, s2, "round-trip to TCUuid and back");
  }

  //// Replica

  auto rep = tc::Replica ();
  t.pass ("replica constructed");

  auto maybe_task = rep.get_task("24478a28-4609-4257-bc19-44ec51391431");
  t.notok(maybe_task.has_value(), "task with fixed uuid does not exist");

  auto task = rep.new_task (tc::Status::Pending, "a test");
  t.pass ("new task constructed");
  t.is (task.get_description (), std::string ("a test"), "task description round-trip");
  t.is (task.get_status (), tc::Status::Pending, "task status round-trip");

  auto uuid = task.get_uuid();

  auto maybe_task2 = rep.get_task (uuid);
  t.ok(maybe_task2.has_value(), "task lookup by uuid finds task");
  t.is ((*maybe_task2).get_description (), std::string ("a test"), "task description round-trip");

  rep.rebuild_working_set ();
  t.pass ("rebuild_working_set");

  auto tasks = rep.all_tasks ();
  t.is ((int)tasks.size(), 1, "all_tasks returns one task");

  //// Task
  
  task = std::move(tasks[0]);

  t.is (task.get_uuid(), uuid, "returned task has correct uuid");
  t.is (task.get_status(), tc::Status::Pending, "returned task is pending");
  auto map = task.get_taskmap ();
  t.is (map["description"], "a test", "task description in taskmap");
  t.is (task.get_description(), "a test", "returned task has correct description");

  //// WorkingSet

  auto ws = rep.working_set ();

  t.is (ws.len (), (size_t)1, "WorkingSet::len");
  t.is (ws.largest_index (), (size_t)1, "WorkingSet::largest_index");
  t.is (ws.by_index (1).value(), uuid, "WorkingSet::by_index");
  t.is (ws.by_index (2).has_value(), false, "WorkingSet::by_index for unknown index");
  t.is (ws.by_uuid (uuid).value (), (size_t)1, "WorkingSet::by_uuid");
  t.is (ws.by_uuid ("3e18a306-e3a8-4a53-a85c-fa7c057759a2").has_value (), false, "WorkingSet::by_uuid for unknown uuid");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

