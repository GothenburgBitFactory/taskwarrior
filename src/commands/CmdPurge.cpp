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
#include <CmdPurge.h>
#include <Context.h>
#include <Filter.h>
#include <i18n.h>
#include <main.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdPurge::CmdPurge ()
{
  _keyword               = "purge";
  _usage                 = "task <filter> purge";
  _description           = STRING_CMD_PURGE_USAGE;
  _read_only             = false;
  _displays_id           = false;
  _needs_confirm         = true;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
// Purges the task, while taking care of:
// - dependencies on this task
// - child tasks
void CmdPurge::purgeTask (Task& task, int& count)
{
  context.tdb2.purge (task);
  handleDeps (task);
  handleChildren (task, count);
  count++;
}

////////////////////////////////////////////////////////////////////////////////
// Makes sure that any task having the dependency on the task being purged
// has that dependency removed, to preserve referential integrity.
void CmdPurge::handleDeps (Task& task)
{
   std::string uuid = task.get ("uuid");

   for (auto& blockedConst: context.tdb2.all_tasks ())
   {
     Task& blocked = const_cast<Task&>(blockedConst);
     if (blocked.has ("depends") &&
         blocked.get ("depends").find (uuid) != std::string::npos)
     {
         blocked.removeDependency (uuid);
         context.tdb2.modify (blocked);
     }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Makes sure that with any recurrence parent are all the child tasks removed
// as well. If user chooses not to, the whole command is aborted.
void CmdPurge::handleChildren (Task& task, int& count)
{
  // If this is not a recurrence parent, we have no job here
  if (!task.has ("mask"))
    return;

  std::string uuid = task.get ("uuid");
  std::vector<Task> children;

  // Find all child tasks
  for (auto& childConst: context.tdb2.all_tasks ())
  {
    Task& child = const_cast<Task&> (childConst);

    if (child.get ("parent") == uuid)
    {
      if (child.getStatus () != Task::deleted)
        // In case any child task is not deleted, bail out
        throw format (STRING_CMD_PURGE_NDEL_CHILD,
                      task.get ("description"),
                      child.identifier (true));
      else
        children.push_back (child);
    }
  }

  // If there are no children, our job is done
  if (children.empty ())
    return;

  // Ask for confirmation to purge them, if needed
  std::string question = format (STRING_CMD_PURGE_CONFIRM_R,
                                 task.get ("description"),
                                 children.size ());

  if (context.config.getBoolean ("recurrence.confirmation") ||
      (context.config.get ("recurrence.confirmation") == "prompt"
       && confirm (question)))
  {
    for (auto& child: children)
      purgeTask (child, count);
  }
  else
    throw std::string (STRING_CMD_PURGE_ABRT);
}


////////////////////////////////////////////////////////////////////////////////
int CmdPurge::execute (std::string&)
{
  int rc = 0;
  int count = 0;

  Filter filter;
  std::vector <Task> filtered;

  // Apply filter.
  filter.subset (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  for (auto& task : filtered)
  {
    // Allow purging of deleted tasks only. Hence no need to deal with:
    // - unblocked tasks notifications (deleted tasks are not blocking)
    // - project changes (deleted tasks not included in progress)
    // It also has the nice property of being explicit - users need to
    // mark tasks as deleted before purging.
    if (task.getStatus () == Task::deleted)
    {
      std::string question;
      question = format (STRING_CMD_PURGE_CONFIRM,
                         task.identifier (true),
                         task.get ("description"));

      if (permission (question, filtered.size ()))
        purgeTask (task, count);
    }
  }

  feedback_affected (count == 1 ? STRING_CMD_PURGE_1 : STRING_CMD_PURGE_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
