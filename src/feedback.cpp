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
#include <Datetime.h>
#include <Duration.h>
#include <Lexer.h>
#include <feedback.h>
#include <format.h>
#include <shared.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static void countTasks(const std::vector<Task>&, const std::string&, int&, int&);

////////////////////////////////////////////////////////////////////////////////
std::string renderAttribute(const std::string& name, const std::string& value,
                            const std::string& format /* = "" */) {
  if (Context::getContext().columns.find(name) != Context::getContext().columns.end()) {
    Column* col = Context::getContext().columns[name];
    if (col && col->type() == "date" && value != "") {
      int64_t epoch = strtoll(value.c_str(), nullptr, 10);
      // Do not try to render an un-parseable value.
      if (epoch == 0) {
        return value;
      }
      Datetime d((time_t)epoch);
      if (format == "") return d.toString(Context::getContext().config.get("dateformat"));

      return d.toString(format);
    }
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
// Implements:
//    <string>
void feedback_affected(const std::string& effect) {
  if (Context::getContext().verbose("affected")) std::cout << effect << "\n";
}

////////////////////////////////////////////////////////////////////////////////
// Implements:
//    Deleted 3 tasks
//
// The 'effect' string should contain:
//    {1}    Quantity
void feedback_affected(const std::string& effect, int quantity) {
  if (Context::getContext().verbose("affected")) std::cout << format(effect, quantity) << "\n";
}

////////////////////////////////////////////////////////////////////////////////
// Implements:
//    Deleting task 123 'This is a test'
//
// The 'effect' string should contain:
//    {1}    ID
//    {2}    Description
void feedback_affected(const std::string& effect, const Task& task) {
  if (Context::getContext().verbose("affected")) {
    std::cout << format(effect, task.identifier(true), task.get("description")) << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Implements feedback and error when adding a reserved tag name.
void feedback_reserved_tags(const std::string& tag) {
  // Note: This list must match that in Task::hasTag.
  // Note: This list must match that in CmdInfo::execute.
  if (tag == "ACTIVE" || tag == "ANNOTATED" || tag == "BLOCKED" || tag == "BLOCKING" ||
      tag == "CHILD" ||  // Deprecated 2.6.0
      tag == "COMPLETED" || tag == "DELETED" || tag == "DUE" || tag == "DUETODAY" ||
      tag == "INSTANCE" || tag == "LATEST" || tag == "MONTH" || tag == "ORPHAN" ||
      tag == "OVERDUE" || tag == "PARENT" ||  // Deprecated 2.6.0
      tag == "PENDING" || tag == "PRIORITY" || tag == "PROJECT" || tag == "QUARTER" ||
      tag == "READY" || tag == "SCHEDULED" || tag == "TAGGED" || tag == "TEMPLATE" ||
      tag == "TODAY" || tag == "TOMORROW" || tag == "UDA" || tag == "UNBLOCKED" || tag == "UNTIL" ||
      tag == "WAITING" || tag == "WEEK" || tag == "YEAR" || tag == "YESTERDAY") {
    throw format("Virtual tags (including '{1}') are reserved and may not be added or removed.",
                 tag);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Implements feedback when adding special tags to a task.
void feedback_special_tags(const Task& task, const std::string& tag) {
  if (Context::getContext().verbose("special")) {
    std::string msg;
    if (tag == "nocolor")
      msg = "The 'nocolor' special tag will disable color rules for this task.";
    else if (tag == "nonag")
      msg = "The 'nonag' special tag will prevent nagging when this task is modified.";
    else if (tag == "nocal")
      msg = "The 'nocal' special tag will keep this task off the 'calendar' report.";
    else if (tag == "next")
      msg =
          "The 'next' special tag will boost the urgency of this task so it appears on the 'next' "
          "report.";

    if (msg.length()) {
      std::cout << format(msg, task.identifier()) << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Called on completion, deletion and update.  If this task is blocking another
// task, then if this was the *only* blocking task, that other task is now
// unblocked.  Mention it.
//
// Implements:
//    Unblocked <id> '<description>'
void feedback_unblocked(const Task& task) {
  if (Context::getContext().verbose("affected")) {
    // Get a list of tasks that depended on this task.
    auto blocked = task.getBlockedTasks();

    // Scan all the tasks that were blocked by this task
    for (auto& i : blocked) {
      auto blocking = i.getDependencyTasks();
      if (blocking.size() == 0) {
        if (i.id)
          std::cout << format("Unblocked {1} '{2}'.", i.id, i.get("description")) << "\n";
        else {
          std::string uuid = i.get("uuid");
          std::cout << format("Unblocked {1} '{2}'.", i.get("uuid"), i.get("description")) << "\n";
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
void feedback_backlog() {
  // If non-local sync is not set up, do not provide this feedback.
  if (Context::getContext().config.get("sync.encryption_secret") == "") {
    return;
  }

  if (Context::getContext().verbose("sync")) {
    int count = Context::getContext().tdb2.num_local_changes();
    if (count)
      Context::getContext().footnote(format(count > 1
                                                ? "There are {1} local changes.  Sync required."
                                                : "There is {1} local change.  Sync required.",
                                            count));
  }
}

///////////////////////////////////////////////////////////////////////////////
std::string onProjectChange(Task& task, bool scope /* = true */) {
  std::stringstream msg;
  std::string project = task.get("project");

  if (project != "") {
    if (scope) msg << format("The project '{1}' has changed.", project) << "  ";

    // Count pending and done tasks, for this project.
    int count_pending = 0;
    int count_done = 0;
    std::vector<Task> all = Context::getContext().tdb2.all_tasks();
    countTasks(all, project, count_pending, count_done);

    // count_done  count_pending  percentage
    // ----------  -------------  ----------
    //          0              0          0%
    //         >0              0        100%
    //          0             >0          0%
    //         >0             >0  calculated
    int percentage = 0;
    if (count_done == 0)
      percentage = 0;
    else if (count_pending == 0)
      percentage = 100;
    else
      percentage = (count_done * 100 / (count_done + count_pending));

    msg << format("Project '{1}' is {2}% complete", project, percentage) << ' ';

    if (count_pending == 1 && count_done == 0)
      msg << format("({1} task remaining).", count_pending);
    else
      msg << format("({1} of {2} tasks remaining).", count_pending, count_pending + count_done);
  }

  return msg.str();
}

///////////////////////////////////////////////////////////////////////////////
std::string onProjectChange(Task& task1, Task& task2) {
  if (task1.get("project") == task2.get("project")) return onProjectChange(task1, false);

  std::string messages1 = onProjectChange(task1);
  std::string messages2 = onProjectChange(task2);

  if (messages1.length() && messages2.length()) return messages1 + '\n' + messages2;

  return messages1 + messages2;
}

///////////////////////////////////////////////////////////////////////////////
std::string onExpiration(Task& task) {
  std::stringstream msg;

  if (Context::getContext().verbose("affected"))
    msg << format("Task {1} '{2}' expired and was deleted.", task.identifier(true),
                  task.get("description"));

  return msg.str();
}

///////////////////////////////////////////////////////////////////////////////
static void countTasks(const std::vector<Task>& all, const std::string& project, int& count_pending,
                       int& count_done) {
  for (auto& it : all) {
    if (it.get("project") == project) {
      switch (it.getStatus()) {
        case Task::pending:
        case Task::waiting:
          ++count_pending;
          break;

        case Task::completed:
          ++count_done;
          break;

        case Task::deleted:
        case Task::recurring:
        default:
          break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
