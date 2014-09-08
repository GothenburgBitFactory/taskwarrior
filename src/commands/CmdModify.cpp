////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <CmdModify.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdModify::CmdModify ()
{
  _keyword     = "modify";
  _usage       = "task <filter> modify <mods>";
  _description = STRING_CMD_MODIFY_USAGE1;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdModify::execute (std::string& output)
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

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);
    task->modify (Task::modReplace);

    if (taskDiff (before, *task))
    {
      // Perform some logical consistency checks.
      if (task->has ("recur")  &&
          !task->has ("due")   &&
          !before.has ("due"))
        throw std::string (STRING_CMD_MODIFY_NO_DUE);

      if (before.has ("recur") &&
          before.has ("due")   &&
          (!task->has ("due")  ||
           task->get ("due") == ""))
        throw std::string (STRING_CMD_MODIFY_REM_DUE);

      if (before.has ("recur")  &&
          (!task->has ("recur") ||
           task->get ("recur") == ""))
        throw std::string (STRING_CMD_MODIFY_REC_ALWAYS);

      // Delete the specified task.
      std::string question;
      if (task->id != 0)
        question = format (STRING_CMD_MODIFY_CONFIRM,
                           task->id,
                           task->get ("description"));
      else
        question = format (STRING_CMD_MODIFY_CONFIRM,
                           task->get ("uuid"),
                           task->get ("description"));

      if (permission (*task, taskDifferences (before, *task) + question, filtered.size ()))
      {
        updateRecurrenceMask (*task);
        dependencyChainOnModify (before, *task);
        ++count;
        feedback_affected (STRING_CMD_MODIFY_TASK, *task);
        feedback_unblocked (*task);
        context.tdb2.modify (*task);
        if (context.verbose ("project"))
          projectChanges[task->get ("project")] = onProjectChange (before, *task);

        // Task potentially has siblings - modify them.
        if (task->has ("parent"))
        {
          if (confirm (STRING_CMD_MODIFY_RECUR))
          {
            std::vector <Task> siblings = context.tdb2.siblings (*task);
            std::vector <Task>::iterator sibling;
            for (sibling = siblings.begin (); sibling != siblings.end (); ++sibling)
            {
              Task alternate (*sibling);
              sibling->modify (Task::modReplace);
              updateRecurrenceMask (*sibling);
              dependencyChainOnModify (alternate, *sibling);
              ++count;
              feedback_affected (STRING_CMD_MODIFY_TASK_R, *sibling);
              feedback_unblocked (*sibling);
              context.tdb2.modify (*sibling);
              if (context.verbose ("project"))
                projectChanges[sibling->get ("project")] = onProjectChange (alternate, *sibling);
            }

            // Modify the parent
            Task parent;
            context.tdb2.get (task->get ("parent"), parent);
            parent.modify (Task::modReplace);
            context.tdb2.modify (parent);
          }
        }

        // Task potentially has child tasks - modify them.
        else if (task->get ("status") == "recurring")
        {
          std::vector <Task> children = context.tdb2.children (*task);
          if (children.size () &&
              confirm (STRING_CMD_MODIFY_RECUR))
          {
            std::vector <Task>::iterator child;
            for (child = children.begin (); child != children.end (); ++child)
            {
              Task alternate (*child);
              child->modify (Task::modReplace);
              updateRecurrenceMask (*child);
              context.tdb2.modify (*child);
              dependencyChainOnModify (alternate, *child);
              if (context.verbose ("project"))
                projectChanges[child->get ("project")] = onProjectChange (alternate, *child);
              ++count;
              feedback_affected (STRING_CMD_MODIFY_TASK_R, *child);
            }
          }
        }
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
  std::map <std::string, std::string>::iterator i;
  for (i = projectChanges.begin (); i != projectChanges.end (); ++i)
    if (i->first != "")
      context.footnote (i->second);

  feedback_affected (count == 1 ? STRING_CMD_MODIFY_1 : STRING_CMD_MODIFY_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
