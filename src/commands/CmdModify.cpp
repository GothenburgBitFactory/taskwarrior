////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <CmdModify.h>
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdModify::CmdModify ()
{
  _keyword               = "modify";
  _usage                 = "task <filter> modify <mods>";
  _description           = STRING_CMD_MODIFY_USAGE1;
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = true;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::execute (std::string&)
{
  int rc = 0;

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  int count = 0;
  for (auto& task : filtered)
  {
    Task before (task);
    task.modify (Task::modReplace);

    if (before.data != task.data)
    {
      // Abort if change introduces inconsistencies.
      checkConsistency(before, task);

      std::string question;
      question = format (STRING_CMD_MODIFY_CONFIRM,
                         task.identifier (true),
                         task.get ("description"));

      if (permission (taskDifferences (before, task) + question, filtered.size ()))
      {
        count += modifyAndUpdate (before, task, &projectChanges);
      }
      else
      {
        std::cout << STRING_CMD_MODIFY_NO << "\n";
        rc = 1;
        if (_permission_quit)
          break;
      }
    }
  }

  // Now list the project changes.
  for (auto& change : projectChanges)
    if (change.first != "")
      context.footnote (change.second);

  feedback_affected (count == 1 ? STRING_CMD_MODIFY_1 : STRING_CMD_MODIFY_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void CmdModify::checkConsistency (Task &before, Task &after)
{
  // Perform some logical consistency checks.
  if (after.has ("recur")  &&
      !after.has ("due")   &&
      !before.has ("due"))
    throw std::string (STRING_CMD_MODIFY_NO_DUE);

  if (before.has ("recur") &&
      before.has ("due")   &&
      (!after.has ("due")  ||
        after.get ("due") == ""))
    throw std::string (STRING_CMD_MODIFY_REM_DUE);

  if (before.has ("recur")  &&
      (!after.has ("recur") ||
        after.get ("recur") == ""))
    throw std::string (STRING_CMD_MODIFY_REC_ALWAYS);
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::modifyAndUpdate (
  Task &before, Task &after,
  std::map <std::string, std::string> *projectChanges /* = NULL */)
{
  // This task.
  int count = 1;

  updateRecurrenceMask (after);
  feedback_affected (STRING_CMD_MODIFY_TASK, after);
  feedback_unblocked (after);
  context.tdb2.modify (after);
  if (context.verbose ("project") && projectChanges)
    (*projectChanges)[after.get ("project")] = onProjectChange (before, after);

  // Task has siblings - modify them.
  if (after.has ("parent"))
    count += modifyRecurrenceSiblings (after, projectChanges);

  // Task has child tasks - modify them.
  else if (after.get ("status") == "recurring")
    count += modifyRecurrenceParent (after, projectChanges);

  return count;
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::modifyRecurrenceSiblings (
  Task &task,
  std::map <std::string, std::string> *projectChanges /* = NULL */)
{
  int count = 0;

  if ((context.config.get ("recurrence.confirmation") == "prompt"
        && confirm (STRING_CMD_MODIFY_RECUR)) ||
      context.config.getBoolean ("recurrence.confirmation"))
  {
    std::vector <Task> siblings = context.tdb2.siblings (task);
    for (auto& sibling : siblings)
    {
      Task alternate (sibling);
      sibling.modify (Task::modReplace);
      updateRecurrenceMask (sibling);
      ++count;
      feedback_affected (STRING_CMD_MODIFY_TASK_R, sibling);
      feedback_unblocked (sibling);
      context.tdb2.modify (sibling);
      if (context.verbose ("project") && projectChanges)
        (*projectChanges)[sibling.get ("project")] = onProjectChange (alternate, sibling);
    }

    // Modify the parent
    Task parent;
    context.tdb2.get (task.get ("parent"), parent);
    parent.modify (Task::modReplace);
    context.tdb2.modify (parent);
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::modifyRecurrenceParent (
  Task &task,
  std::map <std::string, std::string> *projectChanges /* = NULL */)
{
  int count = 0;

  std::vector <Task> children = context.tdb2.children (task);
  if (children.size () &&
      (! context.config.getBoolean ("recurrence.confirmation") ||
        confirm (STRING_CMD_MODIFY_RECUR)))
  {
    for (auto& child : children)
    {
      Task alternate (child);
      child.modify (Task::modReplace);
      updateRecurrenceMask (child);
      context.tdb2.modify (child);
      if (context.verbose ("project") && projectChanges)
        (*projectChanges)[child.get ("project")] = onProjectChange (alternate, child);
      ++count;
      feedback_affected (STRING_CMD_MODIFY_TASK_R, child);
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////

