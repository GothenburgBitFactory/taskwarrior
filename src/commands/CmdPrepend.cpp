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

#include <CmdPrepend.h>
#include <Context.h>
#include <Filter.h>
#include <feedback.h>
#include <format.h>
#include <shared.h>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
CmdPrepend::CmdPrepend() {
  _keyword = "prepend";
  _usage = "task <filter> prepend <mods>";
  _description = "Prepends text to an existing task description";
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
int CmdPrepend::execute(std::string&) {
  int rc = 0;
  int count = 0;

  // Apply filter.
  Filter filter;
  std::vector<Task> filtered;
  filter.subset(filtered);
  if (filtered.size() == 0) {
    Context::getContext().footnote("No tasks specified.");
    return 1;
  }

  // TODO Complain when no modifications are specified.

  // Accumulated project change notifications.
  std::map<std::string, std::string> projectChanges;

  if (filtered.size() > 1) {
    feedback_affected("This command will alter {1} tasks.", filtered.size());
  }
  for (auto& task : filtered) {
    Task before(task);

    // Prepend to the specified task.
    std::string question =
        format("Prepend to task {1} '{2}'?", task.identifier(true), task.get("description"));

    task.modify(Task::modPrepend, true);

    if (permission(before.diff(task) + question, filtered.size())) {
      Context::getContext().tdb2.modify(task);
      ++count;
      feedback_affected("Prepending to task {1} '{2}'.", task);
      if (Context::getContext().verbose("project"))
        projectChanges[task.get("project")] = onProjectChange(task, false);

      // Prepend to siblings.
      if (task.has("parent")) {
        if ((Context::getContext().config.get("recurrence.confirmation") == "prompt" &&
             confirm("This is a recurring task.  Do you want to prepend to all pending recurrences "
                     "of this same task?")) ||
            Context::getContext().config.getBoolean("recurrence.confirmation")) {
          std::vector<Task> siblings = Context::getContext().tdb2.siblings(task);
          for (auto& sibling : siblings) {
            sibling.modify(Task::modPrepend, true);
            Context::getContext().tdb2.modify(sibling);
            ++count;
            feedback_affected("Prepending to recurring task {1} '{2}'.", sibling);
          }

          // Prepend to the parent
          Task parent;
          Context::getContext().tdb2.get(task.get("parent"), parent);
          parent.modify(Task::modPrepend, true);
          Context::getContext().tdb2.modify(parent);
        }
      }
    } else {
      std::cout << "Task not prepended.\n";
      rc = 1;
      if (_permission_quit) break;
    }
  }

  // Now list the project changes.
  for (auto& change : projectChanges)
    if (change.first != "") Context::getContext().footnote(change.second);

  feedback_affected(count == 1 ? "Prepended {1} task." : "Prepended {1} tasks.", count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
