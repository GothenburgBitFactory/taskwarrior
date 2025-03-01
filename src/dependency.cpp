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
#include <format.h>
#include <main.h>
#include <shared.h>

#include <iostream>
#include <stack>

#define STRING_DEPEND_BLOCKED "Task {1} is blocked by:"

////////////////////////////////////////////////////////////////////////////////
// Returns true if the supplied task adds a cycle to the dependency chain.
bool dependencyIsCircular(const Task& task) {
  // A new task has no UUID assigned yet, and therefore cannot be part of any
  // dependency chain.
  if (task.has("uuid")) {
    auto task_uuid = task.get("uuid");

    std::stack<Task> s;
    s.push(task);

    std::unordered_set<std::string> visited;
    visited.insert(task_uuid);

    while (!s.empty()) {
      Task& current = s.top();
      auto deps_current = current.getDependencyUUIDs();

      // This is a basic depth first search that always terminates given the
      // fact that we do not visit any task twice
      for (const auto& dep : deps_current) {
        if (Context::getContext().tdb2.get(dep, current)) {
          auto current_uuid = current.get("uuid");

          if (task_uuid == current_uuid) {
            // Cycle found, initial task reached for the second time!
            return true;
          }

          if (visited.find(current_uuid) == visited.end()) {
            // Push the task to the stack, if it has not been processed yet
            s.push(current);
            visited.insert(current_uuid);
          }
        }
      }

      s.pop();
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
        std::cout << "  " << b.id << ' ' << b.get("description") << '\n';
    }

    // If there are both blocking and blocked tasks, the chain is broken.
    if (blocked.size()) {
      if (Context::getContext().config.getBoolean("dependency.reminder")) {
        std::cout << "and is blocking:\n";

        for (const auto& b : blocked)
          std::cout << "  " << b.id << ' ' << b.get("description") << '\n';
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
        std::cout << "  " << b.id << ' ' << b.get("description") << '\n';
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
