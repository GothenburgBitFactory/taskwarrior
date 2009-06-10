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

#include "T.h"
#include "TDB.h"
#include "text.h"
#include "util.h"
#include "task.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
std::string handleAdd (TDB& tdb, T& task)
{
  std::stringstream out;
/*
  char entryTime[16];
  sprintf (entryTime, "%u", (unsigned int) time (NULL));
  task.setAttribute ("entry", entryTime);

  std::map <std::string, std::string> atts;
  task.getAttributes (atts);
  foreach (i, atts)
    if (i->second == "")
      task.removeAttribute (i->first);

  // Recurring tasks get a special status.
  if (task.getAttribute ("due")   != "" &&
      task.getAttribute ("recur") != "")
  {
    task.setStatus (T::recurring);
    task.setAttribute ("mask", "");
  }

  // Override with default.project, if not specified.
  if (task.getAttribute ("project") == "")
    task.setAttribute ("project", context.config.get ("default.project", ""));

  // Override with default.priority, if not specified.
  if (task.getAttribute ("priority") == "")
  {
    std::string defaultPriority = context.config.get ("default.priority", "");
    if (validPriority (defaultPriority))
      task.setAttribute ("priority", defaultPriority);
  }

  // Disallow blank descriptions.
  if (task.getDescription () == "")
    throw std::string ("Cannot add a task that is blank, or contains <CR> or <LF> characters.");

  if (!tdb.addT (task))
    throw std::string ("Could not create new task.");
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleProjects ()
{
  std::stringstream out;

  context.filter.push_back (Att ("status", "pending"));

  std::vector <T2> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  int quantity = context.tdb.load (tasks, context.filter);
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

  std::vector <T2> tasks;
  context.tdb.lock (context.config.get ("locking", true));
  int quantity = context.tdb.load (tasks, context.filter);
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
  std::cout << optionalBlankLine ();
  foreach (i, unique)
    std::cout << i->first << std::endl;

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
std::string handleUndelete (TDB& tdb, T& task)
{
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);
  filterSequence (all, task);

  foreach (t, all)
  {
    if (t->getStatus () == T::deleted)
    {
      if (t->getAttribute ("recur") != "")
        out << "Task does not support 'undo' for recurring tasks.\n";

      t->setStatus (T::pending);
      t->removeAttribute ("end");
      tdb.modifyT (*t);

      out << "Task " << t->getId () << " '" << t->getDescription () << "' successfully undeleted.\n";
    }
    else
    {
      out << "Task " << t->getId () << " '" << t->getDescription () << "' is not deleted - therefore cannot be undeleted.\n";
    }
  }

  out << "\n"
      << "Please note that tasks can only be reliably undeleted if the undelete "
      << "command is run immediately after the errant delete command."
      << std::endl;
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// If a task is done, but is still in the pending file, then it may be undone
// simply by changing it's status.
std::string handleUndo (TDB& tdb, T& task)
{
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);
  filterSequence (all, task);

  foreach (t, all)
  {
    if (t->getStatus () == T::completed)
    {
      if (t->getAttribute ("recur") != "")
        out << "Task does not support 'undo' for recurring tasks.\n";

      t->setStatus (T::pending);
      t->removeAttribute ("end");
      tdb.modifyT (*t);

      out << "Task " << t->getId () << " '" << t->getDescription () << "' successfully undone." << std::endl;
    }
    else
    {
      out << "Task " << t->getId () << " '" << t->getDescription () << "' is not done - therefore cannot be undone." << std::endl;
    }
  }

  out << std::endl
      << "Please note that tasks can only be reliably undone if the undo "
      << "command is run immediately after the errant done command."
      << std::endl;
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleVersion ()
{
  std::stringstream out;
/*
  // Determine window size, and set table accordingly.
  int width = context.config.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (context.config.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Create a table for the disclaimer.
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
    "and lively discussion.  New releases containing fixes and enhancements are "
    "made frequently.");

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
  // Note that there is a leading and trailing space.
  std::string recognized =
    " blanklines color color.active color.due color.overdue color.pri.H "
    "color.pri.L color.pri.M color.pri.none color.recurring color.tagged "
    "confirmation curses data.location dateformat default.command "
    "default.priority defaultwidth displayweeknumber due echo.command locale "
    "locking monthsperline nag next project shadow.command shadow.file "
    "shadow.notify weekstart editor import.synonym.id import.synonym.uuid "
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
      if (i->find ("color.keyword.") == std::string::npos &&
          i->find ("color.project.") == std::string::npos &&
          i->find ("color.tag.")     == std::string::npos &&
          i->find ("report.")        == std::string::npos)
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
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDelete (TDB& tdb, T& task)
{
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);
  filterSequence (all, task);

  // Determine the end date.
  char endTime[16];
  sprintf (endTime, "%u", (unsigned int) time (NULL));

  foreach (t, all)
  {
    std::stringstream question;
    question << "Permanently delete task "
             << t->getId ()
             << " '"
             << t->getDescription ()
             << "'?";

    if (!context.config.get (std::string ("confirmation"), false) || confirm (question.str ()))
    {
      // Check for the more complex case of a recurring task.  If this is a
      // recurring task, get confirmation to delete them all.
      std::string parent = t->getAttribute ("parent");
      if (parent != "")
      {
        if (confirm ("This is a recurring task.  Do you want to delete all pending recurrences of this same task?"))
        {
          // Scan all pending tasks for siblings of this task, and the parent
          // itself, and delete them.
          foreach (sibling, all)
          {
            if (sibling->getAttribute ("parent") == parent ||
                sibling->getUUID ()              == parent)
            {
              sibling->setStatus (T::deleted);
              sibling->setAttribute ("end", endTime);
              tdb.modifyT (*sibling);

              if (context.config.get ("echo.command", true))
                out << "Deleting recurring task "
                    << sibling->getId ()
                    << " '"
                    << sibling->getDescription ()
                    << "'"
                    << std::endl;
            }
          }
        }
        else
        {
          // Update mask in parent.
          t->setStatus (T::deleted);
          updateRecurrenceMask (tdb, all, *t);

          t->setAttribute ("end", endTime);
          tdb.modifyT (*t);

          out << "Deleting recurring task "
              << t->getId ()
              << " '"
              << t->getDescription ()
              << "'"
              << std::endl;
        }
      }
      else
      {
        t->setStatus (T::deleted);
        t->setAttribute ("end", endTime);
        tdb.modifyT (*t);

        if (context.config.get ("echo.command", true))
          out << "Deleting task "
              << t->getId ()
              << " '"
              << t->getDescription ()
              << "'"
              << std::endl;
      }
    }
    else
      out << "Task not deleted." << std::endl;
  }
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleStart (TDB& tdb, T& task)
{
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.pendingT (all);
  filterSequence (all, task);

  foreach (t, all)
  {
    if (t->getAttribute ("start") == "")
    {
      char startTime[16];
      sprintf (startTime, "%u", (unsigned int) time (NULL));
      t->setAttribute ("start", startTime);

      tdb.modifyT (*t);

      if (context.config.get ("echo.command", true))
        out << "Started "
            << t->getId ()
            << " '"
            << t->getDescription ()
            << "'"
            << std::endl;
      nag (tdb, task);
    }
    else
    {
      out << "Task " << t->getId () << " '" << t->getDescription () << "' already started." << std::endl;
    }
  }
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleStop (TDB& tdb, T& task)
{
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.pendingT (all);
  filterSequence (all, task);

  foreach (t, all)
  {
    if (t->getAttribute ("start") != "")
    {
      t->removeAttribute ("start");
      tdb.modifyT (*t);

      if (context.config.get ("echo.command", true))
        out << "Stopped " << t->getId () << " '" << t->getDescription () << "'" << std::endl;
    }
    else
    {
      out << "Task " << t->getId () << " '" << t->getDescription () << "' not started." << std::endl;
    }
  }
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDone (TDB& tdb, T& task)
{
/*
  int count = 0;
*/
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);

  std::vector <T> filtered = all;
  filterSequence (filtered, task);
  foreach (seq, filtered)
  {
    if (seq->getStatus () == T::pending)
    {
      // Apply deltas.
      deltaDescription   (*seq, task);
      deltaTags          (*seq, task);
      deltaAttributes    (*seq, task);
      deltaSubstitutions (*seq, task);

      // Add an end date.
      char entryTime[16];
      sprintf (entryTime, "%u", (unsigned int) time (NULL));
      seq->setAttribute ("end", entryTime);

      // Change status.
      seq->setStatus (T::completed);

      if (!tdb.modifyT (*seq))
        throw std::string ("Could not mark task as completed.");

      if (context.config.get ("echo.command", true))
        out << "Completed "
            << seq->getId ()
            << " '"
            << seq->getDescription ()
            << "'"
            << std::endl;

      updateRecurrenceMask (tdb, all, *seq);
      nag (tdb, *seq);

      ++count;
    }
    else
      out << "Task "
          << seq->getId ()
          << " '"
          << seq->getDescription ()
          << "' is not pending"
          << std::endl;
  }

  if (context.config.get ("echo.command", true))
    out << "Marked "
        << count
        << " task"
        << (count == 1 ? "" : "s")
        << " as done"
        << std::endl;
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleExport (TDB& tdb, T& task)
{
  std::stringstream output;
/*
  // Use the description as a file name, then clobber the description so the
  // file name isn't used for filtering.
  std::string file = trim (task.getDescription ());
  task.setDescription ("");

  if (file.length () > 0)
  {
    std::ofstream out (file.c_str ());
    if (out.good ())
    {
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
      std::vector <T> all;
      tdb.allPendingT (all);
      filter (all, task);
      foreach (t, all)
      {
        if (t->getStatus () != T::recurring &&
            t->getStatus () != T::deleted)
        {
          out << t->composeCSV ().c_str ();
          ++count;
        }
      }
      out.close ();

      output << count << " tasks exported to '" << file << "'" << std::endl;
    }
    else
      throw std::string ("Could not write to export file.");
  }
  else
    throw std::string ("You must specify a file to write to.");
*/
  return output.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleModify (TDB& tdb, T& task)
{
/*
  int count = 0;
*/
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);

  std::vector <T> filtered = all;
  filterSequence (filtered, task);
  foreach (seq, filtered)
  {
    // Perform some logical consistency checks.
    if (task.getAttribute ("recur") != "" &&
        task.getAttribute ("due")   == "" &&
        seq->getAttribute ("due")   == "")
      throw std::string ("You cannot specify a recurring task without a due date.");

    if (task.getAttribute ("until") != "" &&
        task.getAttribute ("recur") == "" &&
        seq->getAttribute ("recur") == "")
      throw std::string ("You cannot specify an until date for a non-recurring task.");

    // Make all changes.
    foreach (other, all)
    {
      if (other->getId ()               == seq->getId ()                   || // Self
          (seq->getAttribute ("parent") != "" &&
           seq->getAttribute ("parent") == other->getAttribute ("parent")) || // Sibling
          other->getUUID ()             == seq->getAttribute ("parent"))      // Parent
      {
        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        changes += deltaDescription   (*other, task);
        changes += deltaTags          (*other, task);
        changes += deltaAttributes    (*other, task);
        changes += deltaSubstitutions (*other, task);

        if (changes)
          tdb.modifyT (*other);

        ++count;
      }
    }
  }

  if (context.config.get ("echo.command", true))
    out << "Modified " << count << " task" << (count == 1 ? "" : "s") << std::endl;
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleAppend (TDB& tdb, T& task)
{
/*
  int count = 0;
*/
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);

  std::vector <T> filtered = all;
  filterSequence (filtered, task);
  foreach (seq, filtered)
  {
    foreach (other, all)
    {
      if (other->getId ()               == seq->getId ()                   || // Self
          (seq->getAttribute ("parent") != "" &&
           seq->getAttribute ("parent") == other->getAttribute ("parent")) || // Sibling
          other->getUUID ()             == seq->getAttribute ("parent"))      // Parent
      {
        // A non-zero value forces a file write.
        int changes = 0;

        // Apply other deltas.
        changes += deltaAppend     (*other, task);
        changes += deltaTags       (*other, task);
        changes += deltaAttributes (*other, task);

        if (changes)
        {
          tdb.modifyT (*other);

          if (context.config.get ("echo.command", true))
            out << "Appended '"
                << task.getDescription ()
                << "' to task "
                << other->getId ()
                << std::endl;
        }

        ++count;
      }
    }
  }

  if (context.config.get ("echo.command", true))
    out << "Appended " << count << " task" << (count == 1 ? "" : "s") << std::endl;
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDuplicate (TDB& tdb, T& task)
{
/*
  int count = 0;
*/
  std::stringstream out;
/*
  std::vector <T> all;
  tdb.allPendingT (all);

  std::vector <T> filtered = all;
  filterSequence (filtered, task);
  foreach (seq, filtered)
  {
    if (seq->getStatus () != T::recurring && seq->getAttribute ("parent") == "")
    {
      T dup (*seq);
      dup.setUUID (uuid ());  // Needs a new UUID.

      // Apply deltas.
      deltaDescription   (dup, task);
      deltaTags          (dup, task);
      deltaAttributes    (dup, task);
      deltaSubstitutions (dup, task);

      // A New task needs a new entry time.
      char entryTime[16];
      sprintf (entryTime, "%u", (unsigned int) time (NULL));
      dup.setAttribute ("entry", entryTime);

      if (!tdb.addT (dup))
        throw std::string ("Could not create new task.");

      if (context.config.get ("echo.command", true))
        out << "Duplicated "
            << seq->getId ()
            << " '"
            << seq->getDescription ()
            << "'"
            << std::endl;
      ++count;
    }
    else
      out << "Task "
          << seq->getId ()
          << " '"
          << seq->getDescription ()
          << "' is a recurring task, and cannot be duplicated."
          << std::endl;
  }

  if (context.config.get ("echo.command", true))
    out << "Duplicated " << count << " task" << (count == 1 ? "" : "s") << std::endl;
*/
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
std::string handleAnnotate (TDB& tdb, T& task)
{
/*
  if (task.getDescription () == "")
    throw std::string ("Cannot apply a blank annotation.");
*/

  std::stringstream out;
/*
  std::vector <T> all;
  tdb.pendingT (all);
  filterSequence (all, task);

  foreach (t, all)
  {
    t->addAnnotation (task.getDescription ());
    tdb.modifyT (*t);

    if (context.config.get ("echo.command", true))
      out << "Annotated "
          << t->getId ()
          << " with '"
          << t->getDescription ()
          << "'"
          << std::endl;
  }
*/
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
int deltaAppend (T& task, T& delta)
{
  if (delta.getDescription () != "")
  {
    task.setDescription (
      task.getDescription () +
      " " +
      delta.getDescription ());

    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaDescription (T& task, T& delta)
{
  if (delta.getDescription () != "")
  {
    task.setDescription (delta.getDescription ());
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaTags (T& task, T& delta)
{
  int changes = 0;

  // Apply or remove tags, if any.
  std::vector <std::string> tags;
  delta.getTags (tags);
  for (unsigned int i = 0; i < tags.size (); ++i)
  {
    if (tags[i][0] == '+')
      task.addTag (tags[i].substr (1, std::string::npos));
    else
      task.addTag (tags[i]);

    ++changes;
  }

  delta.getRemoveTags (tags);
  for (unsigned int i = 0; i < tags.size (); ++i)
  {
    if (tags[i][0] == '-')
      task.removeTag (tags[i].substr (1, std::string::npos));
    else
      task.removeTag (tags[i]);

    ++changes;
  }

  return changes;
}

////////////////////////////////////////////////////////////////////////////////
int deltaAttributes (T& task, T& delta)
{
  int changes = 0;

  std::map <std::string, std::string> attributes;
  delta.getAttributes (attributes);
  foreach (i, attributes)
  {
    if (i->second == "")
      task.removeAttribute (i->first);
    else
      task.setAttribute (i->first, i->second);

    ++changes;
  }

  return changes;
}

////////////////////////////////////////////////////////////////////////////////
int deltaSubstitutions (T& task, T& delta)
{
  int changes = 0;
  std::string from;
  std::string to;
  bool global;
  delta.getSubstitution (from, to, global);

  if (from != "")
  {
    std::string description = task.getDescription ();
    size_t pattern;

    if (global)
    {
      // Perform all subs on description.
      while ((pattern = description.find (from)) != std::string::npos)
      {
        description.replace (pattern, from.length (), to);
        ++changes;
      }

      task.setDescription (description);

      // Perform all subs on annotations.
      std::map <time_t, std::string> annotations;
      task.getAnnotations (annotations);
      std::map <time_t, std::string>::iterator it;
      for (it = annotations.begin (); it != annotations.end (); ++it)
      {
        while ((pattern = it->second.find (from)) != std::string::npos)
        {
          it->second.replace (pattern, from.length (), to);
          ++changes;
        }
      }

      task.setAnnotations (annotations);
    }
    else
    {
      // Perform first description substitution.
      if ((pattern = description.find (from)) != std::string::npos)
      {
        description.replace (pattern, from.length (), to);
        task.setDescription (description);
        ++changes;
      }
      // Failing that, perform the first annotation substitution.
      else
      {
        std::map <time_t, std::string> annotations;
        task.getAnnotations (annotations);

        std::map <time_t, std::string>::iterator it;
        for (it = annotations.begin (); it != annotations.end (); ++it)
        {
          if ((pattern = it->second.find (from)) != std::string::npos)
          {
            it->second.replace (pattern, from.length (), to);
            ++changes;
            break;
          }
        }

        task.setAnnotations (annotations);
      }
    }
  }
  return changes;
}

////////////////////////////////////////////////////////////////////////////////
