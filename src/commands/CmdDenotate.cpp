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
#include <text.h>
#include <i18n.h>
#include <main.h>
#include <CmdDenotate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDenotate::CmdDenotate ()
{
  _keyword     = "denotate";
  _usage       = "task <filter> denotate <pattern>";
  _description = STRING_CMD_DENO_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDenotate::execute (std::string& output)
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

  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  // Apply the command line modifications to the completed task.
  A3 words = context.a3.extract_modifications ();
  if (!words.size ())
    throw std::string (STRING_CMD_DENO_WORDS);

  std::string pattern = words.combine ();

  Permission permission;
  if (filtered.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);

    std::map <std::string, std::string> annotations;
    task->getAnnotations (annotations);

    if (annotations.size () == 0)
      throw std::string (STRING_CMD_DENO_NONE);

    std::map <std::string, std::string>::iterator i;
    std::string anno;
    bool match = false;
    for (i = annotations.begin (); i != annotations.end (); ++i)
    {
      if (i->second == pattern)
      {
        match = true;
        anno = i->second;
        annotations.erase (i);
        task->setAnnotations (annotations);
        break;
      }
    }
    if (!match)
    {
      for (i = annotations.begin (); i != annotations.end (); ++i)
      {
        std::string::size_type loc = find (i->second, pattern, sensitive);
        if (loc != std::string::npos)
        {
          anno = i->second;
          annotations.erase (i);
          task->setAnnotations (annotations);
          break;
        }
      }
    }

    if (taskDiff (before, *task))
    {
      if (permission.confirmed (before,
                                taskDifferences (before, *task) + STRING_CMD_DONE_PROCEED))
      {
        ++count;
        context.tdb2.modify (*task);
        if (context.verbose ("affected") ||
            context.config.getBoolean ("echo.command")) // Deprecated 2.0
          out << format (STRING_CMD_DENO_FOUND, anno)
              << "\n";
      }
    }
    else
      out << format (STRING_CMD_DENO_NOMATCH, pattern)
          << "\n";
  }

  context.tdb2.commit ();
  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
