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
// cmake.h include header must come first

#include <test.h>
#include <unistd.h>

#include "Context.h"

namespace {

void cleardb() {
  // Remove any residual test files.
  rmdir("./extensions");
  unlink("./taskchampion.sqlite3");
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
int TEST_NAME(int, char**) {
  UnitTest t(27);
  Context context;
  Context::setContext(&context);

  // Ensure environment has no influence.
  unsetenv("TASKDATA");
  unsetenv("TASKRC");

  try {
    cleardb();

    // Set the context to allow GC.
    context.config.set("gc", 1);
    context.config.set("debug", 1);

    context.tdb2.open_replica(".", /*create_if_missing=*/true, /*read_write=*/true);

    // Try reading an empty database.
    std::vector<Task> pending = context.tdb2.pending_tasks();
    std::vector<Task> completed = context.tdb2.completed_tasks();
    int num_reverts_possible = context.tdb2.num_reverts_possible();
    int num_local_changes = context.tdb2.num_local_changes();

    t.is((int)pending.size(), 0, "TDB2 Read empty pending");
    t.is((int)completed.size(), 0, "TDB2 Read empty completed");
    t.is((int)num_reverts_possible, 0, "TDB2 Read empty undo");
    t.is((int)num_local_changes, 0, "TDB2 Read empty backlog");

    // Add a task.
    Task task(R"([description:"description" name:"value"])");
    context.tdb2.add(task);

    pending = context.tdb2.pending_tasks();
    completed = context.tdb2.completed_tasks();
    num_reverts_possible = context.tdb2.num_reverts_possible();
    num_local_changes = context.tdb2.num_local_changes();

    t.is((int)pending.size(), 1, "TDB2 after add, 1 pending task");
    t.is((int)completed.size(), 0, "TDB2 after add, 0 completed tasks");
    t.is((int)num_reverts_possible, 1, "TDB2 after add, 1 revert possible");
    t.is((int)num_local_changes, 6, "TDB2 after add, 6 local changes");

    task.set("description", "This is a test");
    context.tdb2.modify(task);

    pending = context.tdb2.pending_tasks();
    completed = context.tdb2.completed_tasks();
    num_reverts_possible = context.tdb2.num_reverts_possible();
    num_local_changes = context.tdb2.num_local_changes();

    t.is((int)pending.size(), 1, "TDB2 after set, 1 pending task");
    t.is((int)completed.size(), 0, "TDB2 after set, 0 completed tasks");
    t.is((int)num_reverts_possible, 1, "TDB2 after set, 1 revert possible");

    // At this point, there may be 7 or 8 local changes, depending on whether
    // the `modified` property changed between the `add` and `modify`
    // invocation. That only happens if the clock ticks over to the next second
    // between those invocations.
    t.ok(num_local_changes == 7 || num_local_changes == 8, "TDB2 after set, 7 or 8 local changes");

    // Reset for reuse.
    cleardb();
    context.tdb2.open_replica(".", /*create_if_missing=*/true, /*read_write=*/true);

    Task blocker(R"([description:"blocking"])");
    Task first(R"([description:"first"])");
    Task second(R"([description:"second"])");

    first.addDependency(blocker.get_ref("uuid"));
    context.tdb2.modify(first);
    second.addDependency(blocker.get_ref("uuid"));
    context.tdb2.modify(second);

    t.ok(context.tdb2.find_pending(blocker.get_ref("uuid"))->is_blocking,
         "TDB2 dependency is blocking");
    t.ok(context.tdb2.find_pending(first.get_ref("uuid"))->is_blocked,
         "TDB2 first dependent is blocked");
    t.ok(context.tdb2.find_pending(second.get_ref("uuid"))->is_blocked,
         "TDB2 second dependent is blocked");
    t.is((int)context.tdb2.dependency_graph().dependencies.size(), 2,
         "TDB2 dependency graph contains two dependents");
    t.is((int)context.tdb2.dependency_graph().dependents.at(blocker.get_ref("uuid")).size(), 2,
         "TDB2 dependency map contains both edges");

    first.removeDependency(blocker.get_ref("uuid"));
    context.tdb2.modify(first);

    t.ok(context.tdb2.find_pending(blocker.get_ref("uuid"))->is_blocking,
         "TDB2 dependency remains blocking");
    t.notok(context.tdb2.find_pending(first.get_ref("uuid"))->is_blocked,
            "TDB2 first dependent is unblocked");
    t.ok(context.tdb2.find_pending(second.get_ref("uuid"))->is_blocked,
         "TDB2 second dependent remains blocked");
    t.is((int)context.tdb2.dependency_graph().dependencies.size(), 1,
         "TDB2 dependency map correctly removed the first dependent");
    t.is((int)context.tdb2.dependency_graph().dependents.at(blocker.get_ref("uuid")).size(), 1,
         "TDB2 dependency map contains the second edge");

    second.removeDependency(blocker.get_ref("uuid"));
    context.tdb2.modify(second);

    t.notok(context.tdb2.find_pending(blocker.get_ref("uuid"))->is_blocking,
            "TDB2 dependency no longer blocks tasks");
    t.notok(context.tdb2.find_pending(second.get_ref("uuid"))->is_blocked,
            "TDB2 second dependent is unblocked");
    t.is((int)context.tdb2.pending_tasks().size(), 3,
         "TDB2 dependency changes keep correct pending number");
    t.is((int)context.tdb2.dependency_graph().dependencies.size(), 0,
         "TDB2 dependency changes invalidated the old graph - dependencies");
    t.is((int)context.tdb2.dependency_graph().dependents.size(), 0,
         "TDB2 dependency changes remove dependents");

    // Reset for reuse.
    cleardb();
    context.tdb2.open_replica(".", /*create_if_missing=*/true, /*read_write=*/true);
    // TODO complete a task
    // TODO gc
  }

  catch (const std::string& error) {
    t.diag(error);
    return -1;
  }

  catch (...) {
    t.diag("Unknown error.");
    return -2;
  }

  rmdir("./extensions");
  unlink("./pending.data");
  unlink("./completed.data");
  unlink("./undo.data");
  unlink("./backlog.data");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
