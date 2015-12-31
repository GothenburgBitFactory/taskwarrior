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
#include <CmdPrepend.h>
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <util.h>
#include <text.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdPrepend::CmdPrepend ()
{
  _keyword               = "prepend";
  _usage                 = "task <filter> prepend <mods>";
  _description           = STRING_CMD_PREPEND_USAGE;
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = true;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdPrepend::execute (std::string&)
{
  int rc = 0;
  int count = 0;

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // TODO Complain when no modifications are specified.

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  for (auto& task : filtered)
  {
    Task before (task);

    // Prepend to the specified task.
    std::string question = format (STRING_CMD_PREPEND_CONFIRM,
                                   task.identifier (true),
                                   task.get ("description"));

    task.modify (Task::modPrepend, true);

    if (permission (taskDifferences (before, task) + question, filtered.size ()))
    {
      context.tdb2.modify (task);
      ++count;
      feedback_affected (STRING_CMD_PREPEND_TASK, task);
      if (context.verbose ("project"))
        projectChanges[task.get ("project")] = onProjectChange (task, false);

      // Prepend to siblings.
      if (task.has ("parent"))
      {
        if ((context.config.get ("recurrence.confirmation") == "prompt"
             && confirm (STRING_CMD_PREPEND_CONFIRM_R)) ||
            context.config.getBoolean ("recurrence.confirmation"))
        {
          std::vector <Task> siblings = context.tdb2.siblings (task);
          for (auto& sibling : siblings)
          {
            sibling.modify (Task::modPrepend, true);
            context.tdb2.modify (sibling);
            ++count;
            feedback_affected (STRING_CMD_PREPEND_TASK_R, sibling);
          }

          // Prepend to the parent
          Task parent;
          context.tdb2.get (task.get ("parent"), parent);
          parent.modify (Task::modPrepend, true);
          context.tdb2.modify (parent);
        }
      }
    }
    else
    {
      std::cout << STRING_CMD_PREPEND_NO << "\n";
      rc = 1;
      if (_permission_quit)
        break;
    }
  }

  // Now list the project changes.
  for (auto& change : projectChanges)
    if (change.first != "")
      context.footnote (change.second);

  feedback_affected (count == 1 ? STRING_CMD_PREPEND_1 : STRING_CMD_PREPEND_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
