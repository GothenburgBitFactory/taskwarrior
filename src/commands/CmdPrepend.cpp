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
#include <i18n.h>
#include <CmdPrepend.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdPrepend::CmdPrepend ()
{
  _keyword     = "prepend";
  _usage       = "task <filter> prepend <mods>";
  _description = STRING_CMD_PREPEND_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdPrepend::execute (std::string& output)
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

  // Apply the command line modifications to the started task.
  A3 modifications = context.a3.extract_modifications ();
  if (!modifications.size ())
    throw std::string (STRING_CMD_XPEND_NEED_TEXT);

  Permission permission;
  if (filtered.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  // A non-zero value forces a file write.
  int changes = 0;

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    modify_task_description_prepend (*task, modifications);
    context.tdb2.modify (*task);
    ++changes;

    std::vector <Task> siblings = context.tdb2.siblings (*task);
    std::vector <Task>::iterator sibling;
    for (sibling = siblings.begin (); sibling != siblings.end (); ++sibling)
    {
      Task before (*sibling);

      // Apply other deltas.
      modify_task_description_prepend (*sibling, modifications);

      if (taskDiff (before, *sibling))
      {
        if (changes && permission.confirmed (before, taskDifferences (before, *sibling) + "Proceed with change?"))
        {
          context.tdb2.modify (*sibling);
          ++changes;

          if (context.verbose ("affected") ||
              context.config.getBoolean ("echo.command")) // Deprecated 2.0
            out << format (STRING_CMD_PREPEND_DONE, sibling->id)
                << "\n";

          if (before.get ("project") != sibling->get ("project"))
            context.footnote (onProjectChange (before, *sibling));

          ++count;
        }
      }
    }
  }

  context.tdb2.commit ();

  if (context.verbose ("affected") ||
      context.config.getBoolean ("echo.command")) // Deprecated 2.0
    out << format ((count == 1
                      ? STRING_CMD_PREPEND_SUMMARY
                      : STRING_CMD_PREPEND_SUMMARY_N),
                   count)
        << "\n";

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
