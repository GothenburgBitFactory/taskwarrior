////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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
#include <i18n.h>
#include <CmdStop.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStop::CmdStop ()
{
  _keyword     = "stop";
  _usage       = "task <filter> stop <mods>";
  _description = STRING_CMD_STOP_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdStop::execute (std::string& output)
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

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    if (task->has ("start"))
    {
      Task before (*task);

      // Stop the specified task.
      std::string question = format (STRING_CMD_STOP_CONFIRM,
                                     task->id,
                                     task->get ("description"));

      modify_task_annotate (*task, modifications);
      task->remove ("start");

      if (context.config.getBoolean ("journal.time"))
        task->addAnnotation (context.config.get ("journal.time.stop.annotation"));

      if (permission (*task, taskDifferences (before, *task) + question, filtered.size ()))
      {
        updateRecurrenceMask (*task);
        context.tdb2.modify (*task);
        ++count;
        feedback_affected (STRING_CMD_STOP_TASK, *task);
        dependencyChainOnStart (*task);
        projectChanges[task->get ("project")] = onProjectChange (*task, false);
      }
      else
      {
        std::cout << STRING_CMD_STOP_NO << "\n";
        rc  = 1;
      }
    }
    else
    {
      std::cout << format (STRING_CMD_STOP_ALREADY,
                           task->id,
                           task->get ("description"))
                << "\n";
      rc = 1;
    }
  }

  // Now list the project changes.
  std::map <std::string, std::string>::iterator i;
  for (i = projectChanges.begin (); i != projectChanges.end (); ++i)
    if (i->first != "")
      context.footnote (i->second);

  context.tdb2.commit ();
  feedback_affected (count == 1 ? STRING_CMD_STOP_1 : STRING_CMD_STOP_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
