////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

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
  _usage       = "task delete ID";
  _description = "Deletes the specified task.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDelete::execute (std::string& output)
{
  int rc = 0;
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  context.tdb.loadPending (tasks);

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Apply the command line modifications to the new task.
  Arguments modifications = context.args.extract_modifications ();

  // Determine the end date.
  char endTime[16];
  sprintf (endTime, "%u", (unsigned int) time (NULL));

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    if (task->getStatus () == Task::pending ||
        task->getStatus () == Task::waiting)
    {
      modify_task_annotate (*task, modifications);
      apply_defaults (*task);

      std::stringstream question;
      question << "Permanently delete task "
               << task->id
               << " '"
               << task->get ("description")
               << "'?";

      if (!context.config.getBoolean ("confirmation") || confirm (question.str ()))
      {
        // Check for the more complex case of a recurring task.  If this is a
        // recurring task, get confirmation to delete them all.
        std::string parent = task->get ("parent");
        if (parent != "")
        {
          if (confirm ("This is a recurring task.  Do you want to delete all pending recurrences of this same task?"))
          {
            // Scan all pending tasks for siblings of this task, and the parent
            // itself, and delete them.
            std::vector <Task>::iterator sibling;
            for (sibling = tasks.begin (); sibling != tasks.end (); ++sibling)
            {
              if (sibling->get ("parent") == parent ||
                  sibling->get ("uuid")   == parent)
              {
                sibling->setStatus (Task::deleted);

                // Don't want a 'delete' to clobber the end date that may have
                // been written by a 'done' command.
                if (! sibling->has ("end"))
                  sibling->set ("end", endTime);

                // Apply the command line modifications to the sibling.
                modify_task_annotate (*sibling, modifications);
                apply_defaults (*sibling);

                sibling->validate ();
                context.tdb.update (*sibling);
                ++count;

                if (context.config.getBoolean ("echo.command"))
                  out << "Deleting recurring task "
                      << sibling->id
                      << " '"
                      << sibling->get ("description")
                      << "'.\n";
              }
            }
          }
          else
          {
            // Update mask in parent.
            task->setStatus (Task::deleted);
            updateRecurrenceMask (tasks, *task);

            // Don't want a 'delete' to clobber the end date that may have
            // been written by a 'done' command.
            if (! task->has ("end"))
              task->set ("end", endTime);

            task->validate ();
            context.tdb.update (*task);
            ++count;

            out << "Deleting recurring task "
                << task->id
                << " '"
                << task->get ("description")
                << "'.\n";

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
            task->set ("end", endTime);

          task->validate ();
          context.tdb.update (*task);
          ++count;

          if (context.config.getBoolean ("echo.command"))
            out << "Deleting task "
                << task->id
                << " '"
                << task->get ("description")
                << "'.\n";

          dependencyChainOnComplete (*task);
          context.footnote (onProjectChange (*task));
        }
      }
      else
      {
        out << "Task not deleted.\n";
        rc  = 1;
      }
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is neither pending nor waiting.\n";
      rc = 1;
    }
  }

  if (count)
    context.tdb.commit ();

  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
