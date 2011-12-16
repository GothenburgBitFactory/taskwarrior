////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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

#define L10N                                           // Localization complete.

#include <iostream>
#include <Context.h>
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
  std::vector <Task> filtered;
  filter (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Apply the command line modifications to the new task.
  A3 modifications = context.a3.extract_modifications ();
  if (!modifications.size ())
    throw std::string (STRING_CMD_MODIFY_NEED_TEXT);

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);
    modify_task_description_replace (*task, modifications);
    if (taskDiff (before, *task))
    {
      // Perform some logical consistency checks.
      if (task->has ("recur")  &&
          !task->has ("due")   &&
          !before.has ("due"))
        throw std::string (STRING_CMD_MODIFY_NO_DUE);

      if (task->has ("until")  &&
          !task->has ("recur") &&
          !before.has ("recur"))
        throw std::string (STRING_CMD_MODIFY_UNTIL);

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
      std::string question = format (STRING_CMD_MODIFY_CONFIRM,
                                     task->id,
                                     task->get ("description"));

      if (permission (*task, taskDifferences (before, *task) + question, filtered.size ()))
      {
        updateRecurrenceMask (*task);
        context.tdb2.modify (*task);
        ++count;
        feedback_affected (STRING_CMD_MODIFY_TASK, *task);
        dependencyChainOnModify (before, *task);
        context.footnote (onProjectChange (before, *task));

        // Delete siblings.
        if (task->has ("parent"))
        {
          std::vector <Task> siblings = context.tdb2.siblings (*task);
          if (siblings.size () &&
              confirm (STRING_CMD_MODIFY_RECUR))
          {
            std::vector <Task>::iterator sibling;
            for (sibling = siblings.begin (); sibling != siblings.end (); ++sibling)
            {
              Task alternate (*sibling);
              modify_task_description_replace (*sibling, modifications);
              updateRecurrenceMask (*sibling);
              context.tdb2.modify (*sibling);
              dependencyChainOnModify (alternate, *sibling);
              context.footnote (onProjectChange (alternate, *sibling));
              ++count;
              feedback_affected (STRING_CMD_MODIFY_TASK_R, *sibling);
            }
          }
        }
      }
      else
      {
        std::cout << STRING_CMD_MODIFY_NO << "\n";
        rc  = 1;
      }
    }
  }

  context.tdb2.commit ();
  feedback_affected (count == 1 ? STRING_CMD_MODIFY_1 : STRING_CMD_MODIFY_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
