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
#include <text.h>
#include <main.h>
#include <CmdDenotate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDenotate::CmdDenotate ()
{
  _keyword     = "denotate";
  _usage       = "task denotate ID desc...";
  _description = "Deletes an annotation from an existing task.";
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDenotate::execute (const std::string&, std::string& output)
{
  int rc = 0;

  if (!context.task.has ("description"))
    throw std::string ("Description needed to delete an annotation.");

  if (context.sequence.size () == 0)
    throw std::string ("A task ID is needed to delete an annotation.");

  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    context.footnote ("No tasks specified.");
    return 1;
  }

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
  {
    Task before (*task);
    std::string desc = context.task.get ("description");
    std::vector <Att> annotations;
    task->getAnnotations (annotations);

    if (annotations.size () == 0)
      throw std::string ("The specified task has no annotations that can be deleted.");

    std::vector <Att>::iterator i;
    std::string anno;
    bool match = false;;
    for (i = annotations.begin (); i != annotations.end (); ++i)
    {
      anno = i->value ();
      if (anno == desc)
      {
        match = true;
        annotations.erase (i);
        task->setAnnotations (annotations);
        break;
      }
    }
    if (!match)
    {
      for (i = annotations.begin (); i != annotations.end (); ++i)
      {
        anno = i->value ();
        std::string::size_type loc = find (anno, desc, sensitive);

        if (loc != std::string::npos)
        {
          match = true;
          annotations.erase (i);
          task->setAnnotations (annotations);
          break;
        }
      }
    }

    if (taskDiff (before, *task))
    {
      // Only allow valid tasks.
      task->validate ();

      if (permission.confirmed (before, taskDifferences (before, *task) + "Proceed with change?"))
      {
        context.tdb.update (*task);
        if (context.config.getBoolean ("echo.command"))
          out << "Found annotation '"
              << anno
              << "' and deleted it.\n";
      }
    }
    else
      out << "Did not find any matching annotation to be deleted for '"
          << desc
          << "'.\n";
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
