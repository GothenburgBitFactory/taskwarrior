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
#include <CmdStop.h>
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdStop::CmdStop ()
{
  _keyword               = "stop";
  _usage                 = "task <filter> stop <mods>";
  _description           = STRING_CMD_STOP_USAGE;
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
int CmdStop::execute (std::string&)
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

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  for (auto& task : filtered)
  {
    if (task.has ("start"))
    {
      Task before (task);

      // Stop the specified task.
      std::string question = format (STRING_CMD_STOP_CONFIRM,
                                     task.identifier (true),
                                     task.get ("description"));

      task.modify (Task::modAnnotate);
      task.remove ("start");

      if (context.config.getBoolean ("journal.time"))
        task.addAnnotation (context.config.get ("journal.time.stop.annotation"));

      if (permission (taskDifferences (before, task) + question, filtered.size ()))
      {
        updateRecurrenceMask (task);
        context.tdb2.modify (task);
        ++count;
        feedback_affected (STRING_CMD_STOP_TASK, task);
        dependencyChainOnStart (task);
        if (context.verbose ("project"))
          projectChanges[task.get ("project")] = onProjectChange (task, false);
      }
      else
      {
        std::cout << STRING_CMD_STOP_NO << "\n";
        rc = 1;
        if (_permission_quit)
          break;
      }
    }
    else
    {
      std::cout << format (STRING_CMD_STOP_ALREADY,
                           task.identifier (true),
                           task.get ("description"))
                << "\n";
      rc = 1;
    }
  }

  // Now list the project changes.
  for (auto& change : projectChanges)
    if (change.first != "")
      context.footnote (change.second);

  feedback_affected (count == 1 ? STRING_CMD_STOP_1 : STRING_CMD_STOP_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
