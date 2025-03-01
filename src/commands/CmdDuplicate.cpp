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

#include <CmdDuplicate.h>
#include <Context.h>
#include <Filter.h>
#include <feedback.h>
#include <format.h>
#include <util.h>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
CmdDuplicate::CmdDuplicate() {
  _keyword = "duplicate";
  _usage = "task <filter> duplicate <mods>";
  _description = "Duplicates the specified tasks";
  _read_only = false;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = true;
  _accepts_filter = true;
  _accepts_modifications = true;
  _accepts_miscellaneous = false;
  _category = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDuplicate::execute(std::string&) {
  auto rc = 0;
  auto count = 0;

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);
  if (filtered.size() == 0) {
    Context::getContext().footnote("No tasks specified.");
    return 1;
  }

  // Accumulated project change notifications.
  std::map<std::string, std::string> projectChanges;

  for (auto& task : filtered) {
    // Duplicate the specified task.
    Task dup(task);
    dup.id = 0;               // Reset, and TDB2::add will set.
    dup.set("uuid", uuid());  // Needs a new UUID.
    dup.remove("start");      // Does not inherit start date.
    dup.remove("end");        // Does not inherit end date.
    dup.remove("entry");      // Does not inherit entry date.

    // When duplicating a child task, downgrade it to a plain task.
    if (dup.has("parent")) {
      dup.remove("parent");
      dup.remove("recur");
      dup.remove("until");
      dup.remove("imask");
      std::cout << format("Note: task {1} was a recurring task.  The duplicated task is not.",
                          task.identifier())
                << '\n';
    }

    // When duplicating a parent task, create a new parent task.
    else if (dup.getStatus() == Task::recurring) {
      dup.remove("mask");
      std::cout << format(
                       "Note: task {1} was a parent recurring task.  The duplicated task is too.",
                       task.identifier())
                << '\n';
    }

    dup.setStatus(Task::pending);  // Does not inherit status.
                                   // Must occur after Task::recurring check.

    dup.modify(Task::modAnnotate);

    if (permission(
            format("Duplicate task {1} '{2}'?", task.identifier(true), task.get("description")),
            filtered.size())) {
      Context::getContext().tdb2.add(dup);
      ++count;
      feedback_affected("Duplicated task {1} '{2}'.", task);

      auto status = dup.getStatus();
      if (Context::getContext().verbose("new-id") &&
          (status == Task::pending || status == Task::waiting))
        std::cout << format("Created task {1}.\n", dup.id);

      else if (Context::getContext().verbose("new-uuid") && status != Task::recurring)
        std::cout << format("Created task {1}.\n", dup.get("uuid"));

      if (Context::getContext().verbose("project"))
        projectChanges[task.get("project")] = onProjectChange(task);
    } else {
      std::cout << "Task not duplicated.\n";
      rc = 1;
      if (_permission_quit) break;
    }
  }

  // Now list the project changes.
  for (const auto& change : projectChanges)
    if (change.first != "") Context::getContext().footnote(change.second);

  feedback_affected(count == 1 ? "Duplicated {1} task." : "Duplicated {1} tasks.", count);

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
