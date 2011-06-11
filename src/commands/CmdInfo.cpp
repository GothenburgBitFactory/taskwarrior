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
#include <stdlib.h>
#include <Context.h>
#include <Expression.h>
#include <Date.h>
#include <Duration.h>
#include <ViewText.h>
#include <main.h>
#include <text.h>
#include <CmdInfo.h>

extern Context context;
////////////////////////////////////////////////////////////////////////////////
CmdInfo::CmdInfo ()
{
  _keyword     = "information";
  _usage       = "task information <filter>";
  _description = "Shows all data and metadata for specified tasks.";
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdInfo::execute (std::string& output)
{
  int rc = 0;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Filter.
  Arguments f = context.args.extract_read_only_filter ();
  Expression e (f);

  std::vector <Task> filtered;
  std::vector <Task>::iterator task;
  for (task = tasks.begin (); task != tasks.end (); ++task)
    if (e.eval (*task))
      filtered.push_back (*task);

  // Read the undo file.
  std::vector <std::string> undo;
  if (context.config.getBoolean ("journal.info"))
  {
    Directory location (context.config.get ("data.location"));
    std::string undoFile = location.data + "/undo.data";
    File::read (undoFile, undo);
  }

  // Find the task.
  std::stringstream out;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", "Name"));
    view.add (Column::factory ("string", "Value"));

    // If an alternating row color is specified, notify the table.
    if (context.color ())
    {
      Color alternate (context.config.get ("color.alternate"));
      view.colorOdd (alternate);
      view.intraColorOdd (alternate);
    }

    Date now;

    // id
    int row = view.addRow ();
    view.set (row, 0, "ID");
    view.set (row, 1, format (task->id));

    std::string status = ucFirst (Task::statusToText (task->getStatus ()));

    // description
    Color c;
    autoColorize (*task, c);

    row = view.addRow ();
    view.set (row, 0, "Description");
    view.set (row, 1, getFullDescription (*task, "info"), c);

    // status
    row = view.addRow ();
    view.set (row, 0, "Status");
    view.set (row, 1, status);

    // project
    if (task->has ("project"))
    {
      row = view.addRow ();
      view.set (row, 0, "Project");
      view.set (row, 1, task->get ("project"));
    }

    // priority
    if (task->has ("priority"))
    {
      row = view.addRow ();
      view.set (row, 0, "Priority");
      view.set (row, 1, task->get ("priority"));
    }

    // dependencies: blocked
    {
      std::vector <Task> blocked;
      dependencyGetBlocking (*task, blocked);
      if (blocked.size ())
      {
        std::stringstream message;
        std::vector <Task>::const_iterator it;
        for (it = blocked.begin (); it != blocked.end (); ++it)
          message << it->id << " " << it->get ("description") << "\n";

        row = view.addRow ();
        view.set (row, 0, "This task blocked by");
        view.set (row, 1, message.str ());
      }
    }

    // dependencies: blocking
    {
      std::vector <Task> blocking;
      dependencyGetBlocked (*task, blocking);
      if (blocking.size ())
      {
        std::stringstream message;
        std::vector <Task>::const_iterator it;
        for (it = blocking.begin (); it != blocking.end (); ++it)
          message << it->id << " " << it->get ("description") << "\n";

        row = view.addRow ();
        view.set (row, 0, "This task is blocking");
        view.set (row, 1, message.str ());
      }
    }

    // recur
    if (task->has ("recur"))
    {
      row = view.addRow ();
      view.set (row, 0, "Recurrence");
      view.set (row, 1, task->get ("recur"));
    }

    // until
    if (task->has ("until"))
    {
      row = view.addRow ();
      view.set (row, 0, "Recur until");

      Date dt (strtol (task->get ("until").c_str (), NULL, 10));
      std::string format = context.config.get ("reportdateformat");
      if (format == "")
        format = context.config.get ("dateformat");

      std::string until = dt.toString (context.config.get ("dateformat"));
      view.set (row, 1, until);
    }

    // mask
    if (task->getStatus () == Task::recurring)
    {
      row = view.addRow ();
      view.set (row, 0, "Mask");
      view.set (row, 1, task->get ("mask"));
    }

    if (task->has ("parent"))
    {
      // parent
      row = view.addRow ();
      view.set (row, 0, "Parent task");
      view.set (row, 1, task->get ("parent"));

      // imask
      row = view.addRow ();
      view.set (row, 0, "Mask Index");
      view.set (row, 1, task->get ("imask"));
    }

    // due (colored)
    if (task->has ("due"))
    {
      row = view.addRow ();
      view.set (row, 0, "Due");

      std::string format = context.config.get ("reportdateformat");
      if (format == "")
        format = context.config.get ("dateformat");

      view.set (row, 1, getDueDate (*task, format));
    }

    // wait
    if (task->has ("wait"))
    {
      row = view.addRow ();
      view.set (row, 0, "Waiting until");
      Date dt (strtol (task->get ("wait").c_str (), NULL, 10));
      view.set (row, 1, dt.toString (context.config.get ("dateformat")));
    }

    // start
    if (task->has ("start"))
    {
      row = view.addRow ();
      view.set (row, 0, "Start");
      Date dt (strtol (task->get ("start").c_str (), NULL, 10));
      view.set (row, 1, dt.toString (context.config.get ("dateformat")));
    }

    // end
    if (task->has ("end"))
    {
      row = view.addRow ();
      view.set (row, 0, "End");
      Date dt (strtol (task->get ("end").c_str (), NULL, 10));
      view.set (row, 1, dt.toString (context.config.get ("dateformat")));
    }

    // tags ...
    std::vector <std::string> tags;
    task->getTags (tags);
    if (tags.size ())
    {
      std::string allTags;
      join (allTags, " ", tags);

      row = view.addRow ();
      view.set (row, 0, "Tags");
      view.set (row, 1, allTags);
    }

    // uuid
    row = view.addRow ();
    view.set (row, 0, "UUID");
    std::string uuid = task->get ("uuid");
    view.set (row, 1, uuid);

    // entry
    row = view.addRow ();
    view.set (row, 0, "Entered");
    Date dt (strtol (task->get ("entry").c_str (), NULL, 10));
    std::string entry = dt.toString (context.config.get ("dateformat"));

    std::string age;
    std::string created = task->get ("entry");
    if (created.length ())
    {
      Date dt (strtol (created.c_str (), NULL, 10));
      age = Duration (now - dt).format ();
    }

    view.set (row, 1, entry + " (" + age + ")");

    // fg
    std::string color = task->get ("fg");
    if (color != "")
    {
      row = view.addRow ();
      view.set (row, 0, "Foreground color");
      view.set (row, 1, color);
    }

    // bg
    color = task->get ("bg");
    if (color != "")
    {
      row = view.addRow ();
      view.set (row, 0, "Background color");
      view.set (row, 1, color);
    }

    // Task::urgency
    row = view.addRow ();
    view.set (row, 0, "Urgency");
    view.set (row, 1, trimLeft (format (task->urgency (), 4, 4)));

    // Create a second table, containing undo log change details.
    ViewText journal;

    // If an alternating row color is specified, notify the table.
    if (context.color ())
    {
      Color alternate (context.config.get ("color.alternate"));
      journal.colorOdd (alternate);
      journal.intraColorOdd (alternate);
    }

    journal.width (context.getWidth ());
    journal.add (Column::factory ("string", "Date"));
    journal.add (Column::factory ("string", "Modification"));

    if (context.config.getBoolean ("journal.info") &&
        undo.size () > 3)
    {
      // Scan the undo data for entries matching this task.
      std::string when;
      std::string previous;
      std::string current;
      unsigned int i = 0;
      long total_time = 0;
      while (i < undo.size ())
      {
        when = undo[i++];
        previous = "";
        if (undo[i].substr (0, 3) == "old")
          previous = undo[i++];

        current = undo[i++];
        i++; // Separator

        if (current.find ("uuid:\"" + uuid) != std::string::npos)
        {
          if (previous != "")
          {
            int row = journal.addRow ();

            Date timestamp (strtol (when.substr (5).c_str (), NULL, 10));
            journal.set (row, 0, timestamp.toString (context.config.get ("dateformat")));

            Task before (previous.substr (4));
            Task after (current.substr (4));
            journal.set (row, 1, taskInfoDifferences (before, after));

            // calculate the total active time
            if (before.get ("start") == ""
              && after.get ("start") != "")
            {
              // task started
              total_time -= timestamp.toEpoch ();
            }
            else if (before.get ("start") != ""
                   && after.get ("start") == "")
            {
              // task stopped
              total_time += timestamp.toEpoch ();
            }
          }
        }
      }

      // add now() if task is still active
      if (total_time < 0)
        total_time += Date ().toEpoch ();

      // print total active time
      if (total_time > 0)
      {
        row = journal.addRow ();
        journal.set (row, 0, "Total active time");
        journal.set (row, 1, Duration (total_time).format (),
                     (context.color () ? Color ("bold") : Color ()));
      }
    }

    out << optionalBlankLine ()
        << view.render ()
        << "\n";

    if (journal.rows () > 0)
      out << journal.render ()
          << "\n";
  }

  if (! filtered.size ())
  {
    out << "No matches.\n";
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
