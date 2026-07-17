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

#include <Context.h>
#include <dependency.h>
#include <format.h>
#include <shared.h>

#include <iostream>
#include <stack>

#define STRING_DEPEND_BLOCKED "Task {1} is blocked by:"

////////////////////////////////////////////////////////////////////////////////
// Returns true if the supplied task adds a cycle to the dependency chain.
bool dependencyIsCircular(const Task& task) {
  // A new task has no UUID assigned yet, and therefore cannot be part of any
  // dependency chain.
  if (!task.has("uuid")) return false;

  const auto& task_uuid = task.get_ref("uuid");
  auto& tdb2 = Context::getContext().tdb2;

  std::unordered_set<std::string> visited{task_uuid};

  // Vector of dependency UUIDs. The initial set is taken from the task object,
  // because there may have been a dependency added which is not yet in the cache.
  // Subsequent searches use the _pending_tasks cache.
  // This will always terminate as we do not return any UUID twice.
  std::vector<std::string> to_visit;
  for (const auto& dep : task.getDependencyUUIDs()) {
    if (dep == task_uuid) return true;
    if (visited.insert(dep).second) to_visit.push_back(dep);
  }

  while (!to_visit.empty()) {
    std::string dep_uuid = std::move(to_visit.back());
    to_visit.pop_back();

    auto* dep_task = tdb2.find_pending(dep_uuid);
    if (!dep_task) continue;

    const auto& deps = dep_task->getDependencyUUIDs();

    for (const auto& dep : deps) {
      if (dep == task_uuid) return true;
      if (visited.insert(dep).second) to_visit.push_back(dep);
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Determine whether a dependency chain is being broken, assuming that 'task' is
// either completed or deleted.
//
//   blocked task blocking action
//   ------- ---- -------- -----------------------------
//           [1]  2        Chain broken
//                         Nag message generated
//                         Repair offered:  1 dep:-2
//
//           [1]  2        Chain broken
//                3        Nag message generated
//                         Repair offered:  1 dep:-2,-3
//
//   1       [2]           -
//
//   1,3     [2]           -
//
//   1       [2]  3        Chain broken
//                         Nag message generated
//                         Repair offered:  2 dep:-3
//                                          1 dep:-2,3
//
//   1,4     [2]  3,5      Chain broken
//                         Nag message generated
//                         Repair offered:  2 dep:-3,-5
//                                          1 dep:3,5
//                                          4 dep:3,5
//
void dependencyChainOnComplete(Task& task) {
  auto blocking = task.getDependencyTasks();

  // If the task is anything but the tail end of a dependency chain.
  if (blocking.size()) {
    auto blocked = task.getBlockedTasks();

    // Nag about broken chain.
    if (Context::getContext().config.getBoolean("dependency.reminder")) {
      std::cout << format(STRING_DEPEND_BLOCKED, task.identifier()) << '\n';

      for (const auto& b : blocking)
        std::cout << "  " << b.id << ' ' << b.get_ref("description") << '\n';
    }

    // If there are both blocking and blocked tasks, the chain is broken.
    if (blocked.size()) {
      if (Context::getContext().config.getBoolean("dependency.reminder")) {
        std::cout << "and is blocking:\n";

        for (const auto& b : blocked)
          std::cout << "  " << b.id << ' ' << b.get_ref("description") << '\n';
      }

      if (!Context::getContext().config.getBoolean("dependency.confirmation") ||
          confirm("Would you like the dependency chain fixed?")) {
        // Repair the chain - everything in blocked should now depend on
        // everything in blocking, instead of task.id.
        for (auto& left : blocked) {
          left.removeDependency(task.id);

          for (const auto& right : blocking) left.addDependency(right.id);
        }

        // Now update TDB2, now that the updates have all occurred.
        for (auto& left : blocked) Context::getContext().tdb2.modify(left);

        for (auto& right : blocking) Context::getContext().tdb2.modify(right);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void dependencyChainOnStart(Task& task) {
  if (Context::getContext().config.getBoolean("dependency.reminder")) {
    auto blocking = task.getDependencyTasks();

    // If the task is anything but the tail end of a dependency chain, nag about
    // broken chain.
    if (blocking.size()) {
      std::cout << format(STRING_DEPEND_BLOCKED, task.identifier()) << '\n';

      for (const auto& b : blocking)
        std::cout << "  " << b.id << ' ' << b.get_ref("description") << '\n';
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
