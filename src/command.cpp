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

#include "text.h"
#include "util.h"
#include "main.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
std::string handleAdd ()
{
  std::stringstream out;

  context.task.setEntry ();

  // Recurring tasks get a special status.
  if (context.task.has ("due") &&
      context.task.has ("recur"))
  {
    context.task.setStatus (Task::recurring);
    context.task.set ("mask", "");
  }
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
  int quantity = context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  foreach (t, tasks)
    unique[t->get ("project")] += 1;

  if (unique.size ())
  {
    // Render a list of project names from the map.
    Table table;
    table.addColumn ("Project");
    table.addColumn ("Tasks");

    if (context.config.get ("color", true) ||
        context.config.get (std::string ("_forcecolor"), false))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }

    table.setColumnJustification (1, Table::right);
    table.setDateFormat (context.config.get ("dateformat", "m/d/Y"));

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
std::string handleTags ()
{
  std::stringstream out;

  context.filter.push_back (Att ("status", "pending"));

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  int quantity = context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, std::string> unique;
  foreach (t, tasks)
  {
    std::vector <std::string> tags;
    t->getTags (tags);

    foreach (tag, tags)
      unique[*tag] = "";
  }

  // Render a list of tag names from the map.
  out << optionalBlankLine ();
  foreach (i, unique)
    out << i->first << std::endl;

  if (unique.size ())
    out << optionalBlankLine ()
        << unique.size ()
        << (unique.size () == 1 ? " tag" : " tags")
        << " (" << quantity << (quantity == 1 ? " task" : " tasks") << ")"
        << std::endl;
  else
    out << "No tags."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// If a task is deleted, but is still in the pending file, then it may be
// undeleted simply by changing it's status.
std::string handleUndelete ()
{
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  foreach (task, tasks)
  {
    if (task->getStatus () == Task::deleted)
    {
      if (task->has ("recur"))
        out << "Task does not support 'undelete' for recurring tasks.\n";

      task->setStatus (Task::pending);
      task->remove ("end");
      context.tdb.update (*task);

      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' successfully undeleted.\n";
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is not deleted - therefore cannot be undeleted.\n";
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  out << "\n"
      << "Please note that tasks can only be reliably undeleted if the undelete "
      << "command is run immediately after the errant delete command."
      << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// If a task is done, but is still in the pending file, then it may be undone
// simply by changing it's status.
std::string handleUndo ()
{
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  context.tdb.loadPending (tasks, context.filter);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  foreach (task, tasks)
  {
    if (task->getStatus () == Task::completed)
    {
      if (task->has ("recur"))
        out << "Task does not support 'undo' for recurring tasks.\n";

      task->setStatus (Task::pending);
      task->remove ("end");
      context.tdb.update (*task);

      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' successfully undone.\n";
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is not completed - therefore cannot be undone.\n";
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  out << "\n"
      << "Please note that tasks can only be reliably undone if the undo "
      << "command is run immediately after the errant done command."
      << std::endl;

  return out.str ();
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

  // Create a table for output.
  Table table;
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

  std::vector <std::string> all;
  context.config.all (all);
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
      << table.render ()
      << link.render ()
      << std::endl;

  // Complain about configuration variables that are not recognized.
  // These are the regular configuration variables.
  // Note that there is a leading and trailing space, to make searching easier.
  std::string recognized =
    " blanklines color color.active color.due color.overdue color.pri.H "
    "color.pri.L color.pri.M color.pri.none color.recurring color.tagged "
    "color.footnote color.message confirmation curses data.location dateformat "
    "default.command default.priority defaultwidth displayweeknumber due "
    "echo.command locale locking monthsperline nag next project shadow.command "
    "shadow.file shadow.notify weekstart editor import.synonym.id "
    "import.synonym.uuid import.synonym.status import.synonym.tags "
    "import.synonym.entry import.synonym.start import.synonym.due "
    "import.synonym.recur import.synonym.end import.synonym.project "
    "import.synonym.priority import.synonym.fg import.synonym.bg "
    "import.synonym.description ";

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
          i->substr (0,  7) != "report.")
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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);

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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

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
      nag (*task);
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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);

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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

  foreach (task, tasks)
  {
    if (task->getStatus () == Task::pending)
    {
      // Apply deltas.
      deltaDescription (*task);
      deltaTags (*task);
      deltaAttributes (*task);
      deltaSubstitutions (*task);

      // Add an end date.
      char entryTime[16];
      sprintf (entryTime, "%u", (unsigned int) time (NULL));
      task->set ("end", entryTime);

      // Change status.
      task->setStatus (Task::completed);
      context.tdb.update (*task);

      if (context.config.get ("echo.command", true))
        out << "Completed "
            << task->id
            << " '"
            << task->get ("description")
            << "'"
            << std::endl;

      updateRecurrenceMask (all, *task);
      nag (*task);

      ++count;
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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);
  context.tdb.commit ();
  context.tdb.unlock ();

  foreach (task, tasks)
  {
    if (task->getStatus () != Task::recurring &&
        task->getStatus () != Task::deleted)
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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

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
        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        changes += deltaDescription (*other);
        changes += deltaTags (*other);
        changes += deltaAttributes (*other);
        changes += deltaSubstitutions (*other);

        if (changes)
          context.tdb.update (*other);

        ++count;
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
  context.tdb.loadPending (tasks, context.filter);
  handleRecurrence (tasks);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

  foreach (task, tasks)
  {
    foreach (other, all)
    {
      if (other->id             == task->id               || // Self
          (task->has ("parent") &&
           task->get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")   == task->get ("parent"))     // Parent
      {
        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        changes += deltaAppend (*other);
        changes += deltaTags (*other);
        changes += deltaAttributes (*other);

        if (changes)
        {
          context.tdb.update (*other);

          if (context.config.get ("echo.command", true))
            out << "Appended '"
                << context.task.get ("description")
                << "' to task "
                << other->id
                << std::endl;
        }

        ++count;
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

  foreach (task, tasks)
  {
    task->addAnnotation (context.task.get ("description"));
    context.tdb.update (*task);

    if (context.config.get ("echo.command", true))
      out << "Annotated "
          << task->id
          << " with '"
          << task->get ("description")
          << "'"
          << std::endl;
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
