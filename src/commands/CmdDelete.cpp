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

#include <sstream>
#include <Context.h>
#include <Permission.h>
#include <util.h>
#include <text.h>
#include <i18n.h>
#include <main.h>
#include <CmdDelete.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDelete::CmdDelete ()
{
  _keyword     = "delete";
  _usage       = "task <filter> delete <mods>";
  _description = STRING_CMD_DELETE_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDelete::execute (std::string& output)
{
  int rc = 0;
  int count = 0;
  std::stringstream out;

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
    if (task->getStatus () == Task::pending ||
        task->getStatus () == Task::waiting)
    {
      modify_task_annotate (*task, modifications);

      std::string question = format (STRING_CMD_DELETE_QUESTION,
                                     task->id,
                                     task->get ("description"));

      if (!context.config.getBoolean ("confirmation") || confirm (question))
      {
        // Check for the more complex case of a recurring task.  If this is a
        // recurring task, get confirmation to delete them all.
        std::string parent = task->get ("parent");
        if (parent != "")
        {
          std::vector <Task> siblings;

          if (context.config.getBoolean ("confirmation") &&
              confirm (STRING_CMD_DELETE_CONF_RECUR))
          {
            // Scan all pending tasks for siblings of this task, and the parent
            // itself, and delete them.
            siblings = context.tdb2.siblings (*task);
            std::vector <Task>::iterator sibling;
            for (sibling = siblings.begin (); sibling != siblings.end (); ++sibling)
            {
              if (sibling->get ("parent") == parent ||
                  sibling->get ("uuid")   == parent)
              {
                sibling->setStatus (Task::deleted);

                // Don't want a 'delete' to clobber the end date that may have
                // been written by a 'done' command.
                if (! sibling->has ("end"))
                  sibling->setEnd ();

                // Apply the command line modifications to the sibling.
                modify_task_annotate (*sibling, modifications);
                context.tdb2.modify (*sibling);
                ++count;

                if (context.verbose ("affected") ||
                    context.config.getBoolean ("echo.command")) // Deprecated 2.0
                  out << format (STRING_CMD_DELETE_RECURRING,
                                 sibling->id,
                                 sibling->get ("description"))
                      << "\n";
              }
            }
          }
          else
          {
            // Update mask in parent.
            task->setStatus (Task::deleted);
            updateRecurrenceMask (*task);

            // Don't want a 'delete' to clobber the end date that may have
            // been written by a 'done' command.
            if (! task->has ("end"))
              task->setEnd ();

            context.tdb2.modify (*task);
            ++count;

            out << format (STRING_CMD_DELETE_RECURRING,
                           task->id,
                           task->get ("description"))
                << "\n";

            dependencyChainOnComplete (*task);
            context.footnote (onProjectChange (*task));
          }
        }
        else
        {
          task->setStatus (Task::deleted);

          // Don't want a 'delete' to clobber the end date that may have
          // been written by a 'done' command.
          if (! task->has ("end"))
            task->setEnd ();

          context.tdb2.modify (*task);
          ++count;

          if (context.verbose ("affected") ||
              context.config.getBoolean ("echo.command")) // Deprecated 2.0
            out << format (STRING_CMD_DELETE_DELETING,
                           task->id,
                           task->get ("description"))
                << "\n";

          dependencyChainOnComplete (*task);
          context.footnote (onProjectChange (*task));
        }
      }
      else
      {
        out << STRING_CMD_DELETE_NOT << "\n";
        rc  = 1;
      }
    }
    else
    {
      out << format (STRING_CMD_DELETE_NOTPEND,
                     task->id,
                     task->get ("description"))
          << "\n";
      rc = 1;
    }
  }

  context.tdb2.commit ();
  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
