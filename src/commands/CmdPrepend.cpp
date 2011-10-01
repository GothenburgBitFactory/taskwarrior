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
  _usage       = "task <filter> prepend <modifications>";
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
