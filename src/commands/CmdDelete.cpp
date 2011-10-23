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
#include <util.h>
#include <text.h>
#include <i18n.h>
#include <main.h>
#include <CmdDelete.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDelete::CmdDelete ()
{
  _keyword       = "delete";
  _usage         = "task <filter> delete <mods>";
  _description   = STRING_CMD_DELETE_USAGE;
  _read_only     = false;
  _displays_id   = false;
  _needs_confirm = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDelete::execute (std::string& output)
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

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);

    if (task->getStatus () == Task::pending   ||
        task->getStatus () == Task::completed ||
        task->getStatus () == Task::waiting)
    {
      // Delete the specified task.
      std::string question;
      if (task->id)
        question = format (STRING_CMD_DELETE_CONFIRM,
                           task->id,
                           task->get ("description"));
      else
        question = format (STRING_CMD_DELETE_CONFIRM,
                           task->get ("uuid"),
                           task->get ("description"));

      modify_task_annotate (*task, modifications);
      task->setStatus (Task::deleted);
      if (! task->has ("end"))
        task->setEnd ();

      if (permission (*task, question, filtered.size ()))
      {
        updateRecurrenceMask (*task);
        context.tdb2.modify (*task);
        ++count;
        feedback_affected (STRING_CMD_DELETE_TASK, *task);
        dependencyChainOnComplete (*task);
        context.footnote (onProjectChange (*task, true));

        // Delete siblings.
        if (task->has ("parent"))
        {
          std::vector <Task> siblings = context.tdb2.siblings (*task);
          if (siblings.size () &&
              confirm (STRING_CMD_DELETE_CONFIRM_R))
          {
            std::vector <Task>::iterator sibling;
            for (sibling = siblings.begin (); sibling != siblings.end (); ++sibling)
            {
              modify_task_annotate (*sibling, modifications);
              sibling->setStatus (Task::deleted);
              if (! sibling->has ("end"))
                sibling->setEnd ();

              updateRecurrenceMask (*sibling);
              context.tdb2.modify (*sibling);
              ++count;
              feedback_affected (STRING_CMD_DELETE_TASK_R, *sibling);
            }

            // Delete the parent
            Task parent;
            context.tdb2.get (task->get ("parent"), parent);
            parent.setStatus (Task::deleted);
            if (! parent.has ("end"))
              parent.setEnd ();

            context.tdb2.modify (parent);
          }
        }
      }
      else
      {
        std::cout << STRING_CMD_DELETE_NO << "\n";
        rc  = 1;
      }
    }
    else
    {
      std::cout << format (STRING_CMD_DELETE_NOT_DEL,
                           task->id,
                           task->get ("description"))
          << "\n";
      rc = 1;
    }
  }

  context.tdb2.commit ();
  feedback_affected (count == 1 ? STRING_CMD_DELETE_1 : STRING_CMD_DELETE_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
