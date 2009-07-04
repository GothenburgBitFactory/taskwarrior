////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Permission.h"
#include "text.h"
#include "util.h"
#include "main.h"
#include "../auto.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
std::string handleAdd ()
{
  std::stringstream out;

  context.task.set ("uuid", uuid ());
  context.task.setEntry ();

  // Recurring tasks get a special status.
  if (context.task.has ("due") &&
      context.task.has ("recur"))
  {
    context.task.setStatus (Task::recurring);
    context.task.set ("mask", "");
  }
  else if (context.task.has ("wait"))
    context.task.setStatus (Task::waiting);
  else
    context.task.setStatus (Task::pending);

  // Override with default.project, if not specified.
  if (context.task.get ("project") == "")
    context.task.set ("project", context.config.get ("default.project", ""));

  // Override with default.priority, if not specified.
  if (context.task.get ("priority") == "")
  {
    std::string defaultPriority = context.config.get ("default.priority", "");
    if (Att::validNameValue ("priority", "", defaultPriority))
      context.task.set ("priority", defaultPriority);
  }

  // Include tags.
  foreach (tag, context.tagAdditions)
    context.task.addTag (*tag);

  // Only valid tasks can be added.
  context.task.validate ();

  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.add (context.task);

#ifdef FEATURE_NEW_ID
  // All this, just for an id number.
  std::vector <Task> all;
  Filter none;
  context.tdb.loadPending (all, none);
  out << "Created task " << context.tdb.nextId () << std::endl;
#endif

  context.tdb.commit ();
  context.tdb.unlock ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleProjects ()
{
  std::stringstream out;

  context.filter.push_back (Att ("status", "pending"));

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  int quantity = context.tdb.loadPending (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  std::map <std::string, int> high;
  std::map <std::string, int> medium;
  std::map <std::string, int> low;
  std::map <std::string, int> none;
  std::string project;
  std::string priority;
  foreach (t, tasks)
  {
     project = t->get ("project");
    priority = t->get ("priority");

    unique[project] += 1;

         if (priority == "H") high[project]   += 1;
    else if (priority == "M") medium[project] += 1;
    else if (priority == "L") low[project]    += 1;
    else                      none[project]   += 1;
  }

  if (unique.size ())
  {
    // Render a list of project names from the map.
    Table table;
    table.addColumn ("Project");
    table.addColumn ("Tasks");
    table.addColumn ("Pri:None");
    table.addColumn ("Pri:L");
    table.addColumn ("Pri:M");
    table.addColumn ("Pri:H");

    if (context.config.get ("color", true) ||
        context.config.get (std::string ("_forcecolor"), false))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
      table.setColumnUnderline (2);
      table.setColumnUnderline (3);
      table.setColumnUnderline (4);
      table.setColumnUnderline (5);
    }

    table.setColumnJustification (1, Table::right);
    table.setColumnJustification (2, Table::right);
    table.setColumnJustification (3, Table::right);
    table.setColumnJustification (4, Table::right);
    table.setColumnJustification (5, Table::right);

    foreach (i, unique)
    {
      int row = table.addRow ();
      table.addCell (row, 0, i->first);
      table.addCell (row, 1, i->second);
      table.addCell (row, 2, none[i->first]);
      table.addCell (row, 3, low[i->first]);
      table.addCell (row, 4, medium[i->first]);
      table.addCell (row, 5, high[i->first]);
    }

    out << optionalBlankLine ()
        << table.render ()
        << optionalBlankLine ()
        << unique.size ()
        << (unique.size () == 1 ? " project" : " projects")
        << " (" << quantity << (quantity == 1 ? " task" : " tasks") << ")"
        << std::endl;
  }
  else
    out << "No projects."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleCompletionProjects ()
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  foreach (t, tasks)
    unique[t->get ("project")] = 0;

  std::stringstream out;
  foreach (project, unique)
    if (project->first.length ())
      out << project->first << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleTags ()
{
  std::stringstream out;

  context.filter.push_back (Att ("status", "pending"));

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  int quantity = context.tdb.loadPending (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  foreach (t, tasks)
  {
    std::vector <std::string> tags;
    t->getTags (tags);

    foreach (tag, tags)
      if (unique.find (*tag) != unique.end ())
        unique[*tag]++;
      else
        unique[*tag] = 1;
  }

  if (unique.size ())
  {
    // Render a list of tags names from the map.
    Table table;
    table.addColumn ("Tag");
    table.addColumn ("Count");

    if (context.config.get ("color", true) ||
        context.config.get (std::string ("_forcecolor"), false))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }

    table.setColumnJustification (1, Table::right);

    foreach (i, unique)
    {
      int row = table.addRow ();
      table.addCell (row, 0, i->first);
      table.addCell (row, 1, i->second);
    }

    out << optionalBlankLine ()
        << table.render ()
        << optionalBlankLine ()
        << unique.size ()
        << (unique.size () == 1 ? " tag" : " tags")
        << " (" << quantity << (quantity == 1 ? " task" : " tasks") << ")"
        << std::endl;
  }
  else
    out << "No tags."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleCompletionTags ()
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  foreach (t, tasks)
  {
    std::vector <std::string> tags;
    t->getTags (tags);

    foreach (tag, tags)
      unique[*tag] = 0;
  }

  std::stringstream out;
  foreach (tag, unique)
    out << tag->first << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleCompletionCommands ()
{
  std::vector <std::string> commands;
  context.cmd.allCommands (commands);
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  foreach (command, commands)
    out << *command << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
void handleUndo ()
{
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.undo ();
  context.tdb.unlock ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleVersion ()
{
  std::stringstream out;

  // Create a table for the disclaimer.
  int width = context.getWidth ();
  Table disclaimer;
  disclaimer.setTableWidth (width);
  disclaimer.addColumn (" ");
  disclaimer.setColumnWidth (0, Table::flexible);
  disclaimer.setColumnJustification (0, Table::left);
  disclaimer.addCell (disclaimer.addRow (), 0,
    "Task comes with ABSOLUTELY NO WARRANTY; for details read the COPYING file "
    "included.  This is free software, and you are welcome to redistribute it "
    "under certain conditions; again, see the COPYING file for details.");

  // Create a table for the URL.
  Table link;
  link.setTableWidth (width);
  link.addColumn (" ");
  link.setColumnWidth (0, Table::flexible);
  link.setColumnJustification (0, Table::left);
  link.addCell (link.addRow (), 0,
    "See http://taskwarrior.org for the latest releases, online documentation "
    "and lively discussion.  New releases containing fixes and enhancements "
    "are made frequently.");

  std::vector <std::string> all;
  context.config.all (all);

  // Create a table for output.
  Table table;
  if (context.config.get ("longversion", true))
  {
    table.setTableWidth (width);
    table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));
    table.addColumn ("Config variable");
    table.addColumn ("Value");

    if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }
    else
      table.setTableDashedUnderline ();

    table.setColumnWidth (0, Table::minimum);
    table.setColumnWidth (1, Table::flexible);
    table.setColumnJustification (0, Table::left);
    table.setColumnJustification (1, Table::left);
    table.sortOn (0, Table::ascendingCharacter);

    foreach (i, all)
    {
      std::string value = context.config.get (*i);
      if (value != "")
      {
        int row = table.addRow ();
        table.addCell (row, 0, *i);
        table.addCell (row, 1, value);
      }
    }
  }

  out << "Copyright (C) 2006 - 2009, P. Beckingham."
      << std::endl
      << ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
           ? Text::colorize (Text::bold, Text::nocolor, PACKAGE)
           : PACKAGE)
      << " "
      << ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
           ? Text::colorize (Text::bold, Text::nocolor, VERSION)
           : VERSION)
      << std::endl
      << disclaimer.render ()
      << std::endl
      << (context.config.get ("longversion", true) ? table.render () : "")
      << link.render ()
      << std::endl;

  // Complain about configuration variables that are not recognized.
  // These are the regular configuration variables.
  // Note that there is a leading and trailing space, to make searching easier.
  std::string recognized =
    " blanklines bulk color color.active color.due color.overdue color.pri.H "
    "color.pri.L color.pri.M color.pri.none color.recurring color.tagged "
    "color.footnote color.header color.debug confirmation curses data.location "
    "dateformat debug default.command default.priority defaultwidth due locale "
    "displayweeknumber echo.command locking monthsperline nag next project "
    "shadow.command shadow.file shadow.notify weekstart editor import.synonym.id "
    "import.synonym.uuid longversion "
#ifdef FEATURE_SHELL
    "shell.prompt "
#endif
    "import.synonym.status import.synonym.tags import.synonym.entry "
    "import.synonym.start import.synonym.due import.synonym.recur "
    "import.synonym.end import.synonym.project import.synonym.priority "
    "import.synonym.fg import.synonym.bg import.synonym.description ";

  // This configuration variable is supported, but not documented.  It exists
  // so that unit tests can force color to be on even when the output from task
  // is redirected to a file, or stdout is not a tty.
  recognized += "_forcecolor ";

  std::vector <std::string> unrecognized;
  foreach (i, all)
  {
    // Disallow partial matches by tacking a leading an trailing space on each
    // variable name.
    std::string pattern = " " + *i + " ";
    if (recognized.find (pattern) == std::string::npos)
    {
      // These are special configuration variables, because their name is
      // dynamic.
      if (i->substr (0, 14) != "color.keyword." &&
          i->substr (0, 14) != "color.project." &&
          i->substr (0, 10) != "color.tag."     &&
          i->substr (0,  7) != "report."        &&
          i->substr (0,  6) != "alias.")
      {
        unrecognized.push_back (*i);
      }
    }
  }

  if (unrecognized.size ())
  {
    out << "Your .taskrc file contains these unrecognized variables:"
        << std::endl;

    foreach (i, unrecognized)
      out << "  " << *i << std::endl;

    out << std::endl;
  }

  // Verify installation.  This is mentioned in the documentation as the way to
  // ensure everything is properly installed.

  if (all.size () == 0)
    out << "Configuration error: .taskrc contains no entries"
        << std::endl;
  else
  {
    if (context.config.get ("data.location") == "")
      out << "Configuration error: data.location not specified in .taskrc "
             "file."
          << std::endl;

    if (access (expandPath (context.config.get ("data.location")).c_str (), X_OK))
      out << "Configuration error: data.location contains a directory name"
             " that doesn't exist, or is unreadable."
          << std::endl;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDelete ()
{
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  // Determine the end date.
  char endTime[16];
  sprintf (endTime, "%u", (unsigned int) time (NULL));

  foreach (task, tasks)
  {
    std::stringstream question;
    question << "Permanently delete task "
             << task->id
             << " '"
             << task->get ("description")
             << "'?";

    if (!context.config.get (std::string ("confirmation"), false) || confirm (question.str ()))
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
          foreach (sibling, tasks)
          {
            if (sibling->get ("parent") == parent ||
                sibling->get ("uuid")   == parent)
            {
              sibling->setStatus (Task::deleted);
              sibling->set ("end", endTime);
              context.tdb.update (*sibling);

              if (context.config.get ("echo.command", true))
                out << "Deleting recurring task "
                    << sibling->id
                    << " '"
                    << sibling->get ("description")
                    << "'"
                    << std::endl;
            }
          }
        }
        else
        {
          // Update mask in parent.
          task->setStatus (Task::deleted);
          updateRecurrenceMask (tasks, *task);

          task->set ("end", endTime);
          context.tdb.update (*task);

          out << "Deleting recurring task "
              << task->id
              << " '"
              << task->get ("description")
              << "'"
              << std::endl;
        }
      }
      else
      {
        task->setStatus (Task::deleted);
        task->set ("end", endTime);
        context.tdb.update (*task);

        if (context.config.get ("echo.command", true))
          out << "Deleting task "
              << task->id
              << " '"
              << task->get ("description")
              << "'"
              << std::endl;
      }
    }
    else
      out << "Task not deleted." << std::endl;
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleStart ()
{
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  bool nagged = false;
  foreach (task, tasks)
  {
    if (! task->has ("start"))
    {
      char startTime[16];
      sprintf (startTime, "%u", (unsigned int) time (NULL));
      task->set ("start", startTime);

      context.tdb.update (*task);

      if (context.config.get ("echo.command", true))
        out << "Started "
            << task->id
            << " '"
            << task->get ("description")
            << "'"
            << std::endl;
      if (!nagged)
        nagged = nag (*task);
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' already started."
          << std::endl;
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleStop ()
{
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  foreach (task, tasks)
  {
    if (task->has ("start"))
    {
      task->remove ("start");
      context.tdb.update (*task);

      if (context.config.get ("echo.command", true))
        out << "Stopped "
            << task->id
            << " '"
            << task->get ("description")
            << "'"
            << std::endl;
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' not started."
          << std::endl;
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDone ()
{
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.get ("bulk", 2))
    permission.bigSequence ();

  bool nagged = false;
  foreach (task, tasks)
  {
    if (task->getStatus () == Task::pending)
    {
      Task before (*task);

      // Apply other deltas.
      if (deltaDescription (*task))
        permission.bigChange ();

      deltaTags (*task);
      deltaAttributes (*task);
      deltaSubstitutions (*task);

      // Add an end date.
      char entryTime[16];
      sprintf (entryTime, "%u", (unsigned int) time (NULL));
      task->set ("end", entryTime);

      // Change status.
      task->setStatus (Task::completed);

      if (taskDiff (before, *task))
      {
        std::string question = taskDifferences (before, *task) + "Are you sure?";
        if (permission.confirmed (question))
        {
          context.tdb.update (*task);

          if (context.config.get ("echo.command", true))
            out << "Completed "
                << task->id
                << " '"
                << task->get ("description")
                << "'"
                << std::endl;

          ++count;
        }
      }

      updateRecurrenceMask (all, *task);
      if (!nagged)
        nagged = nag (*task);
    }
    else
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is not pending"
          << std::endl;
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  if (context.config.get ("echo.command", true))
    out << "Marked "
        << count
        << " task"
        << (count == 1 ? "" : "s")
        << " as done"
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleExport ()
{
  std::stringstream out;

  out << "'id',"
      << "'uuid',"
      << "'status',"
      << "'tags',"
      << "'entry',"
      << "'start',"
      << "'due',"
      << "'recur',"
      << "'end',"
      << "'project',"
      << "'priority',"
      << "'fg',"
      << "'bg',"
      << "'description'"
      << "\n";

  int count = 0;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  foreach (task, tasks)
  {
    if (task->getStatus () != Task::recurring)
    {
      out << task->composeCSV ().c_str ();
      ++count;
    }
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleModify ()
{
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.get ("bulk", 2))
    permission.bigSequence ();

  foreach (task, tasks)
  {
    // Perform some logical consistency checks.
    if (context.task.has ("recur") &&
        !context.task.has ("due")  &&
        !task->has ("due"))
      throw std::string ("You cannot specify a recurring task without a due date.");

    if (context.task.has ("until")  &&
        !context.task.has ("recur") &&
        !task->has ("recur"))
      throw std::string ("You cannot specify an until date for a non-recurring task.");

    // Make all changes.
    foreach (other, all)
    {
      if (other->id             == task->id               || // Self
          (task->has ("parent") &&
           task->get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")   == task->get ("parent"))     // Parent
      {
        Task before (*other);

        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        if (deltaDescription (*other))
        {
          permission.bigChange ();
          ++changes;
        }

        changes += deltaTags (*other);
        changes += deltaAttributes (*other);
        changes += deltaSubstitutions (*other);

        if (taskDiff (before, *other))
        {
          std::string question = taskDifferences (before, *other) + "Are you sure?";
          if (changes && permission.confirmed (question))
          {
            context.tdb.update (*other);
            ++count;
          }
        }
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  if (context.config.get ("echo.command", true))
    out << "Modified " << count << " task" << (count == 1 ? "" : "s") << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleAppend ()
{
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.get ("bulk", 2))
    permission.bigSequence ();

  foreach (task, tasks)
  {
    foreach (other, all)
    {
      if (other->id             == task->id               || // Self
          (task->has ("parent") &&
           task->get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")   == task->get ("parent"))     // Parent
      {
        Task before (*other);

        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        changes += deltaAppend (*other);
        changes += deltaTags (*other);
        changes += deltaAttributes (*other);

        if (taskDiff (before, *other))
        {
          std::string question = taskDifferences (before, *other) + "Are you sure?";
          if (changes && permission.confirmed (question))
          {
            context.tdb.update (*other);

            if (context.config.get ("echo.command", true))
              out << "Appended '"
                  << context.task.get ("description")
                  << "' to task "
                  << other->id
                  << std::endl;

            ++count;
          }
        }
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  if (context.config.get ("echo.command", true))
    out << "Appended " << count << " task" << (count == 1 ? "" : "s") << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDuplicate ()
{
  std::stringstream out;
  int count = 0;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  foreach (task, tasks)
  {
    Task dup (*task);
    dup.set ("uuid", uuid ());  // Needs a new UUID.
    dup.setStatus (Task::pending);
    dup.remove ("start");   // Does not inherit start date.
    dup.remove ("end");     // Does not inherit end date.

    // Recurring tasks are duplicated and downgraded to regular tasks.
    if (task->getStatus () == Task::recurring)
    {
      dup.remove ("parent");
      dup.remove ("recur");
      dup.remove ("until");
      dup.remove ("imak");
      dup.remove ("imask");

      out << "Note: task "
          << task->id
          << " was a recurring task.  The new task is not."
          << std::endl;
    }

    // Apply deltas.
    deltaDescription (dup);
    deltaTags (dup);
    deltaAttributes (dup);
    deltaSubstitutions (dup);

    // A New task needs a new entry time.
    char entryTime[16];
    sprintf (entryTime, "%u", (unsigned int) time (NULL));
    dup.set ("entry", entryTime);

    context.tdb.add (dup);

    if (context.config.get ("echo.command", true))
      out << "Duplicated "
          << task->id
          << " '"
          << task->get ("description")
          << "'"
          << std::endl;
    ++count;
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  if (context.config.get ("echo.command", true))
    out << "Duplicated " << count << " task" << (count == 1 ? "" : "s") << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
#ifdef FEATURE_SHELL
void handleShell ()
{
  // Display some kind of welcome message.
  std::cout << ((context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
                 ? Text::colorize (Text::bold, Text::nocolor, PACKAGE_STRING)
                 : PACKAGE_STRING)
            << " shell"
            << std::endl
            << std::endl
            << "Enter any task command (such as 'list'), or hit 'Enter'."
            << std::endl
            << "There is no need to include the 'task' command itself."
            << std::endl
            << "Enter 'quit' to end the session."
            << std::endl
            << std::endl;

  // Preserve any special override arguments, and reapply them for each
  // shell command.
  std::vector <std::string> special;
  foreach (arg, context.args)
    if (arg->substr (0, 3) == "rc." ||
        arg->substr (0, 3) == "rc:")
      special.push_back (*arg);

  std::string quit = "quit"; // TODO i18n
  std::string command;
  bool keepGoing = true;

  do
  {
    std::cout << context.config.get ("shell.prompt", "task>") << " ";

    command = "";
    std::getline (std::cin, command);
    command = lowerCase (trim (command));

    if (command.length () > 0               &&
        command.length () <= quit.length () &&
        command == quit.substr (0, command.length ()))
    {
      keepGoing = false;
    }
    else
    {
      try
      {
        context.clear ();

        std::vector <std::string> args;
        split (args, command, ' ');
        foreach (arg, special) context.args.push_back (*arg);
        foreach (arg, args)    context.args.push_back (*arg);

        context.initialize ();
        context.run ();
      }

      catch (std::string& error)
      {
        std::cout << error << std::endl;
      }

      catch (...)
      {
        std::cerr << context.stringtable.get (100, "Unknown error.") << std::endl;
      }
    }
  }
  while (keepGoing);
}
#endif

////////////////////////////////////////////////////////////////////////////////
std::string handleColor ()
{
  std::stringstream out;
  if (context.config.get ("color", true) || context.config.get (std::string ("_forcecolor"), false))
  {
    out << optionalBlankLine () << "Foreground" << std::endl
        << "           "
                << Text::colorize (Text::bold,                   Text::nocolor, "bold")                   << "          "
                << Text::colorize (Text::underline,              Text::nocolor, "underline")              << "          "
                << Text::colorize (Text::bold_underline,         Text::nocolor, "bold_underline")         << std::endl

        << "  " << Text::colorize (Text::black,                  Text::nocolor, "black")                  << "    "
                << Text::colorize (Text::bold_black,             Text::nocolor, "bold_black")             << "    "
                << Text::colorize (Text::underline_black,        Text::nocolor, "underline_black")        << "    "
                << Text::colorize (Text::bold_underline_black,   Text::nocolor, "bold_underline_black")   << std::endl

        << "  " << Text::colorize (Text::red,                    Text::nocolor, "red")                    << "      "
                << Text::colorize (Text::bold_red,               Text::nocolor, "bold_red")               << "      "
                << Text::colorize (Text::underline_red,          Text::nocolor, "underline_red")          << "      "
                << Text::colorize (Text::bold_underline_red,     Text::nocolor, "bold_underline_red")     << std::endl

        << "  " << Text::colorize (Text::green,                  Text::nocolor, "green")                  << "    "
                << Text::colorize (Text::bold_green,             Text::nocolor, "bold_green")             << "    "
                << Text::colorize (Text::underline_green,        Text::nocolor, "underline_green")        << "    "
                << Text::colorize (Text::bold_underline_green,   Text::nocolor, "bold_underline_green")   << std::endl

        << "  " << Text::colorize (Text::yellow,                 Text::nocolor, "yellow")                 << "   "
                << Text::colorize (Text::bold_yellow,            Text::nocolor, "bold_yellow")            << "   "
                << Text::colorize (Text::underline_yellow,       Text::nocolor, "underline_yellow")       << "   "
                << Text::colorize (Text::bold_underline_yellow,  Text::nocolor, "bold_underline_yellow")  << std::endl

        << "  " << Text::colorize (Text::blue,                   Text::nocolor, "blue")                   << "     "
                << Text::colorize (Text::bold_blue,              Text::nocolor, "bold_blue")              << "     "
                << Text::colorize (Text::underline_blue,         Text::nocolor, "underline_blue")         << "     "
                << Text::colorize (Text::bold_underline_blue,    Text::nocolor, "bold_underline_blue")    << std::endl

        << "  " << Text::colorize (Text::magenta,                Text::nocolor, "magenta")                << "  "
                << Text::colorize (Text::bold_magenta,           Text::nocolor, "bold_magenta")           << "  "
                << Text::colorize (Text::underline_magenta,      Text::nocolor, "underline_magenta")      << "  "
                << Text::colorize (Text::bold_underline_magenta, Text::nocolor, "bold_underline_magenta") << std::endl

        << "  " << Text::colorize (Text::cyan,                   Text::nocolor, "cyan")                   << "     "
                << Text::colorize (Text::bold_cyan,              Text::nocolor, "bold_cyan")              << "     "
                << Text::colorize (Text::underline_cyan,         Text::nocolor, "underline_cyan")         << "     "
                << Text::colorize (Text::bold_underline_cyan,    Text::nocolor, "bold_underline_cyan")    << std::endl

        << "  " << Text::colorize (Text::white,                  Text::nocolor, "white")                  << "    "
                << Text::colorize (Text::bold_white,             Text::nocolor, "bold_white")             << "    "
                << Text::colorize (Text::underline_white,        Text::nocolor, "underline_white")        << "    "
                << Text::colorize (Text::bold_underline_white,   Text::nocolor, "bold_underline_white")   << std::endl

        << std::endl << "Background" << std::endl
        << "  " << Text::colorize (Text::nocolor, Text::on_black,          "on_black")          << "    "
                << Text::colorize (Text::nocolor, Text::on_bright_black,   "on_bright_black")   << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_red,            "on_red")            << "      "
                << Text::colorize (Text::nocolor, Text::on_bright_red,     "on_bright_red")     << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_green,          "on_green")          << "    "
                << Text::colorize (Text::nocolor, Text::on_bright_green,   "on_bright_green")   << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_yellow,         "on_yellow")         << "   "
                << Text::colorize (Text::nocolor, Text::on_bright_yellow,  "on_bright_yellow")  << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_blue,           "on_blue")           << "     "
                << Text::colorize (Text::nocolor, Text::on_bright_blue,    "on_bright_blue")    << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_magenta,        "on_magenta")        << "  "
                << Text::colorize (Text::nocolor, Text::on_bright_magenta, "on_bright_magenta") << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_cyan,           "on_cyan")           << "     "
                << Text::colorize (Text::nocolor, Text::on_bright_cyan,    "on_bright_cyan")    << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_white,          "on_white")          << "    "
                << Text::colorize (Text::nocolor, Text::on_bright_white,   "on_bright_white")   << std::endl

        << optionalBlankLine ();
  }
  else
  {
    out << "Color is currently turned off in your .taskrc file.  "
           "To enable color, create the entry 'color=on'."
        << std::endl;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleAnnotate ()
{
  if (!context.task.has ("description"))
    throw std::string ("Cannot apply a blank annotation.");

  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.get ("bulk", 2))
    permission.bigSequence ();

  foreach (task, tasks)
  {
    Task before (*task);
    task->addAnnotation (context.task.get ("description"));

    if (taskDiff (before, *task))
    {
      std::string question = taskDifferences (before, *task) + "Are you sure?";
      if (permission.confirmed (question))
      {
        context.tdb.update (*task);

        if (context.config.get ("echo.command", true))
          out << "Annotated "
              << task->id
              << " with '"
              << context.task.get ("description")
              << "'"
              << std::endl;
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
int deltaAppend (Task& task)
{
  if (context.task.has ("description"))
  {
    task.set ("description",
              task.get ("description") + " " + context.task.get ("description"));
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaDescription (Task& task)
{
  if (context.task.has ("description"))
  {
    task.set ("description", context.task.get ("description"));
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaTags (Task& task)
{
  int changes = 0;

  // Apply or remove tags, if any.
  std::vector <std::string> tags;
  context.task.getTags (tags);
  foreach (tag, tags)
  {
    task.addTag (*tag);
    ++changes;
  }

  foreach (tag, context.tagRemovals)
  {
    task.removeTag (*tag);
    ++changes;
  }

  return changes;
}

////////////////////////////////////////////////////////////////////////////////
int deltaAttributes (Task& task)
{
  int changes = 0;

  foreach (att, context.task)
  {
    if (att->first != "uuid"        &&
        att->first != "description" &&
        att->first != "tags")
    {
      // Modifying "wait" changes status.
      if (att->first == "wait")
      {
        if (att->second.value () == "")
          task.setStatus (Task::pending);
        else
          task.setStatus (Task::waiting);
      }

      if (att->second.value () == "")
        task.remove (att->first);
      else
        task.set (att->first, att->second.value ());

      ++changes;
    }
  }

  return changes;
}

////////////////////////////////////////////////////////////////////////////////
int deltaSubstitutions (Task& task)
{
  std::string description = task.get ("description");
  std::vector <Att> annotations;
  task.getAnnotations (annotations);

  context.subst.apply (description, annotations);

  task.set ("description", description);
  task.setAnnotations (annotations);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
