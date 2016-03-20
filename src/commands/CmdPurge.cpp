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
    std::string uuid = task.get ("uuid");

    if (task.getStatus () == Task::deleted)
    {
      std::string question;
      question = format (STRING_CMD_PURGE_CONFIRM,
                         task.identifier (true),
                         task.get ("description"));

      if (permission (question, filtered.size ()))
      {
        context.tdb2.purge (task);
        handleDeps(task);
        count++;
      }
    }
  }

  feedback_affected (count == 1 ? STRING_CMD_PURGE_1 : STRING_CMD_PURGE_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
