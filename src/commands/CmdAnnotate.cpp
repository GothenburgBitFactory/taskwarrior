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
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <CmdAnnotate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdAnnotate::CmdAnnotate ()
{
  _keyword     = "annotate";
  _usage       = "task <filter> annotate <mods>";
  _description = STRING_CMD_ANNO_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAnnotate::execute (std::string& output)
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
  if (!modifications.size ())
    throw std::string (STRING_CMD_XPEND_NEED_TEXT);

  Permission permission;
  if (filtered.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);

    // Annotate the specified task.
    std::string question = format (STRING_CMD_ANNO_QUESTION,
                                   task->id,
                                   task->get ("description"));

    modify_task_annotate (*task, modifications);

    if (permission.confirmed (*task, taskDifferences (before, *task) + question))
    {
      context.tdb2.modify (*task);
      ++count;

      if (context.verbose ("affected") ||
          context.config.getBoolean ("echo.command")) // Deprecated 2.0
        out << format (task->has ("parent")
                         ? STRING_CMD_ANNO_RECURRING
                         : STRING_CMD_ANNO_DELETING,
                       task->id,
                       task->get ("description"))
            << "\n";

      dependencyChainOnComplete (*task);
      context.footnote (onProjectChange (*task, true));

      // Annotate siblings.
      if (task->has ("parent"))
      {
        std::vector <Task> siblings = context.tdb2.siblings (*task);
        if (siblings.size () &&
            confirm (STRING_CMD_ANNO_CONF_RECUR))
        {
          std::vector <Task>::iterator sibling;
          for (sibling = siblings.begin (); sibling != siblings.end (); ++sibling)
          {
            modify_task_annotate (*sibling, modifications);
            context.tdb2.modify (*sibling);
            ++count;

            if (context.verbose ("affected") ||
                context.config.getBoolean ("echo.command")) // Deprecated 2.0
              out << format (STRING_CMD_ANNO_RECURRING,
                             sibling->id,
                             sibling->get ("description"))
                  << "\n";
          }

          // Annotate the parent
          Task parent;
          context.tdb2.get (task->get ("parent"), parent);
          modify_task_annotate (parent, modifications);
          context.tdb2.modify (parent);
        }
      }
    }
    else
    {
      out << STRING_CMD_DELETE_NOT << "\n";
      rc  = 1;
    }
  }

  context.tdb2.commit ();

  if (context.verbose ("affected") ||
      context.config.getBoolean ("echo.command")) // Deprecated 2.0
    out << format ((count == 1
                      ? STRING_CMD_ANNO_SUMMARY
                      : STRING_CMD_ANNO_SUMMARY_N),
                   count)
        << "\n";

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
