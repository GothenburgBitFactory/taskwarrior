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
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Config.h"
#include "Date.h"
#include "Table.h"
#include "color.h"
#include "TDB.h"
#include "T.h"
#include "task.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

////////////////////////////////////////////////////////////////////////////////
std::string handleAdd (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

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
    task.setAttribute ("project", conf.get ("default.project", ""));

  // Override with default.priority, if not specified.
  if (task.getAttribute ("priority") == "")
  {
    std::string defaultPriority = conf.get ("default.priority", "");
    if (validPriority (defaultPriority))
      task.setAttribute ("priority", defaultPriority);
  }

  // Disallow blank descriptions.
  if (task.getDescription () == "")
    throw std::string ("Cannot add a task that is blank, or contains <CR> or <LF> characters.");

  if (!tdb.addT (task))
    throw std::string ("Could not create new task.");

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleProjects (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Get all the tasks, including deleted ones.
  std::vector <T> tasks;
  tdb.pendingT (tasks);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T task (tasks[i]);
    unique[task.getAttribute ("project")] += 1;
  }

  if (unique.size ())
  {
    // Render a list of project names from the map.
    Table table;
    table.addColumn ("Project");
    table.addColumn ("Tasks");

    if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
    {
      table.setColumnUnderline (0);
      table.setColumnUnderline (1);
    }

    table.setColumnJustification (1, Table::right);
    table.setDateFormat (conf.get ("dateformat", "m/d/Y"));

    foreach (i, unique)
    {
      int row = table.addRow ();
      table.addCell (row, 0, i->first);
      table.addCell (row, 1, i->second);
    }

    out << optionalBlankLine (conf)
        << table.render ()
        << optionalBlankLine (conf)
        << unique.size ()
        << (unique.size () == 1 ? " project" : " projects")
        << std::endl;
  }
  else
    out << "No projects."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleTags (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  // Get all the tasks.
  std::vector <T> tasks;
  tdb.pendingT (tasks);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, std::string> unique;
  for (unsigned int i = 0; i < tasks.size (); ++i)
  {
    T task (tasks[i]);

    std::vector <std::string> tags;
    task.getTags (tags);

    for (unsigned int t = 0; t < tags.size (); ++t)
      unique[tags[t]] = "";
  }

  // Render a list of tag names from the map.
  std::cout << optionalBlankLine (conf);
  foreach (i, unique)
    std::cout << i->first << std::endl;

  if (unique.size ())
    out << optionalBlankLine (conf)
        << unique.size ()
        << (unique.size () == 1 ? " tag" : " tags")
        << std::endl;
  else
    out << "No tags."
        << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// If a task is deleted, but is still in the pending file, then it may be
// undeleted simply by changing it's status.
std::string handleUndelete (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;
  std::vector <T> all;
  tdb.allPendingT (all);

  int id = task.getId ();
  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == id)
    {
      if (it->getStatus () == T::deleted)
      {
        if (it->getAttribute ("recur") != "")
        {
          out << "Task does not support 'undelete' for recurring tasks." << std::endl;
          return out.str ();
        }

        T restored (*it);
        restored.setStatus (T::pending);
        restored.removeAttribute ("end");
        tdb.modifyT (restored);

        out << "Task " << id << " successfully undeleted." << std::endl;
        return out.str ();
      }
      else
      {
        out << "Task " << id << " is not deleted - therefore cannot undelete." << std::endl;
        return out.str ();
      }
    }
  }

  out << "Task " << id
      << " not found - tasks can only be reliably undeleted if the undelete" << std::endl
      << "command is run immediately after the errant delete command." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// If a task is done, but is still in the pending file, then it may be undone
// simply by changing it's status.
std::string handleUndo (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  std::vector <T> all;
  tdb.allPendingT (all);

  int id = task.getId ();
  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == id)
    {
      if (it->getStatus () == T::completed)
      {
        if (it->getAttribute ("recur") != "")
          return std::string ("Task does not support 'undo' for recurring tasks.\n");

        T restored (*it);
        restored.setStatus (T::pending);
        restored.removeAttribute ("end");
        tdb.modifyT (restored);

        out << "Task " << id << " successfully undone." << std::endl;
        return out.str ();
      }
      else
      {
        out << "Task " << id << " is not done - therefore cannot be undone." << std::endl;
        return out.str ();
      }
    }
  }

  out << "Task " << id
      << " not found - tasks can only be reliably undone if the undo" << std::endl
      << "command is run immediately after the errant done command." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleVersion (Config& conf)
{
  std::stringstream out;

  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", (int) 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
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
    "See http://www.beckingham.net/task.html for the latest releases and a "
    "full tutorial.  New releases containing fixes and enhancements are "
    "made frequently.");

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("Config variable");
  table.addColumn ("Value");

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
  conf.all (all);
  foreach (i, all)
  {
    std::string value = conf.get (*i);
    if (value != "")
    {
      int row = table.addRow ();
      table.addCell (row, 0, *i);
      table.addCell (row, 1, value);
    }
  }

  out << "Copyright (C) 2006 - 2009, P. Beckingham."
      << std::endl
      << ((conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
           ? Text::colorize (Text::bold, Text::nocolor, PACKAGE)
           : PACKAGE)
      << " "
      << ((conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
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
  std::string recognized =
    "blanklines color color.active color.due color.overdue color.pri.H "
    "color.pri.L color.pri.M color.pri.none color.recurring color.tagged "
    "confirmation curses data.location dateformat default.command "
    "default.priority defaultwidth due echo.command locking monthsperline nag "
    "next project shadow.command shadow.file shadow.notify";

  // This configuration variable is supported, but not documented.  It exists
  // so that unit tests can force color to be on even when the output from task
  // is redirected to a file, or stdout is not a tty.
  recognized += " _forcecolor";

  std::vector <std::string> unrecognized;
  foreach (i, all)
  {
    if (recognized.find (*i) == std::string::npos)
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
    if (conf.get ("data.location") == "")
      out << "Configuration error: data.location not specified in .taskrc "
             "file."
          << std::endl;

    if (access (expandPath (conf.get ("data.location")).c_str (), X_OK))
      out << "Configuration error: data.location contains a directory name"
             " that doesn't exist, or is unreadable."
          << std::endl;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDelete (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  if (!conf.get (std::string ("confirmation"), false) || confirm ("Permanently delete task?"))
  {
    std::vector <T> all;
    tdb.allPendingT (all);
    foreach (t, all)
    {
      if (t->getId () == task.getId ())
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
                tdb.deleteT (*sibling);
                if (conf.get ("echo.command", true))
                  out << "Deleting recurring task "
                      << sibling->getId ()
                      << " "
                      << sibling->getDescription ()
                      << std::endl;
              }
            }
          }
          else
          {
            // Update mask in parent.
            t->setStatus (T::deleted);
            updateRecurrenceMask (tdb, all, *t);
            tdb.deleteT (*t);
            out << "Deleting recurring task "
                << t->getId ()
                << " "
                << t->getDescription ()
                << std::endl;
          }
        }
        else
        {
          tdb.deleteT (*t);
          if (conf.get ("echo.command", true))
            out << "Deleting task "
                << t->getId ()
                << " "
                << t->getDescription ()
                << std::endl;
        }

        break;  // No point continuing the loop.
      }
    }
  }
  else
    out << "Task not deleted." << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleStart (TDB& tdb, T& task, Config& conf)
{
  std::vector <T> all;
  tdb.pendingT (all);

  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == task.getId ())
    {
      T original (*it);
      std::stringstream out;

      if (original.getAttribute ("start") == "")
      {
        char startTime[16];
        sprintf (startTime, "%u", (unsigned int) time (NULL));
        original.setAttribute ("start", startTime);

        original.setId (task.getId ());
        tdb.modifyT (original);

        if (conf.get ("echo.command", true))
          out << "Started "
              << original.getId ()
              << " "
              << original.getDescription ()
              << std::endl;
        nag (tdb, task, conf);
      }
      else
      {
        out << "Task " << task.getId () << " already started." << std::endl;
      }

      return out.str ();
    }
  }

  throw std::string ("Task not found.");
  return std::string (""); // To satisfy gcc.
}

////////////////////////////////////////////////////////////////////////////////
std::string handleStop (TDB& tdb, T& task, Config& conf)
{
  std::vector <T> all;
  tdb.pendingT (all);

  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == task.getId ())
    {
      T original (*it);
      std::stringstream out;

      if (original.getAttribute ("start") != "")
      {
        original.removeAttribute ("start");
        original.setId (task.getId ());
        tdb.modifyT (original);

        if (conf.get ("echo.command", true))
          out << "Stopped " << original.getId () << " " << original.getDescription () << std::endl;
      }
      else
      {
        out << "Task " << task.getId () << " not started." << std::endl;
      }

      return out.str ();
    }
  }

  throw std::string ("Task not found.");
  return std::string (""); // To satisfy gcc.
}

////////////////////////////////////////////////////////////////////////////////
std::string handleDone (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;

  if (!tdb.completeT (task))
    throw std::string ("Could not mark task as completed.");

  // Now update mask in parent.
  std::vector <T> all;
  tdb.allPendingT (all);
  foreach (t, all)
  {
    if (t->getId () == task.getId ())
    {
      if (conf.get ("echo.command", true))
        out << "Completed "
            << t->getId ()
            << " "
            << t->getDescription ()
            << std::endl;

      t->setStatus (T::completed);
      updateRecurrenceMask (tdb, all, *t);
      break;
    }
  }

  nag (tdb, task, conf);
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleExport (TDB& tdb, T& task, Config& conf)
{
  std::stringstream output;

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

  return output.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleModify (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;
  std::vector <T> all;
  tdb.allPendingT (all);

  // Lookup the complete task.
  T complete = findT (task.getId (), all);

  // Perform some logical consistency checks.
  if (task.getAttribute ("recur")   != "" &&
      task.getAttribute ("due")     == "" &&
      complete.getAttribute ("due") == "")
    throw std::string ("You cannot specify a recurring task without a due date.");

  if (task.getAttribute ("until")     != "" &&
      task.getAttribute ("recur")     == "" &&
      complete.getAttribute ("recur") == "")
    throw std::string ("You cannot specify an until date for a non-recurring task.");

  int count = 0;
  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId ()                == complete.getId ()                 || // Self
        (complete.getAttribute ("parent") != "" &&
        it->getAttribute ("parent") == complete.getAttribute ("parent")) || // Sibling
        it->getUUID ()              == complete.getAttribute ("parent"))    // Parent
    {
      T original (*it);

      // A non-zero value forces a file write.
      int changes = 0;

      // Apply a new description, if any.
      if (task.getDescription () != "")
      {
        original.setDescription (task.getDescription ());
        ++changes;
      }

      // Apply or remove tags, if any.
      std::vector <std::string> tags;
      task.getTags (tags);
      for (unsigned int i = 0; i < tags.size (); ++i)
      {
        if (tags[i][0] == '+')
          original.addTag (tags[i].substr (1, std::string::npos));
        else
          original.addTag (tags[i]);

        ++changes;
      }

      task.getRemoveTags (tags);
      for (unsigned int i = 0; i < tags.size (); ++i)
      {
        if (tags[i][0] == '-')
          original.removeTag (tags[i].substr (1, std::string::npos));
        else
          original.removeTag (tags[i]);

        ++changes;
      }

      // Apply or remove attributes, if any.
      std::map <std::string, std::string> attributes;
      task.getAttributes (attributes);
      foreach (i, attributes)
      {
        if (i->second == "")
          original.removeAttribute (i->first);
        else
        {
          original.setAttribute (i->first, i->second);

          // If a "recur" attribute is added, upgrade to a recurring task.
          if (i->first == "recur")
            original.setStatus (T::recurring);
        }

        ++changes;
      }

      std::string from;
      std::string to;
      task.getSubstitution (from, to);
      if (from != "")
      {
        std::string description = original.getDescription ();
        size_t pattern = description.find (from);
        if (pattern != std::string::npos)
        {
          description = description.substr (0, pattern) +
                        to                              +
                        description.substr (pattern + from.length (), std::string::npos);
          original.setDescription (description);
          ++changes;
        }
      }

      if (changes)
        tdb.modifyT (original);

      ++count;
    }
  }

  if (count == 0)
    throw std::string ("Task not found.");

  if (conf.get ("echo.command", true))
    out << "Modified " << count << " task" << (count == 1 ? "" : "s") << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleAppend (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;
  std::vector <T> all;
  tdb.allPendingT (all);

  // Lookup the complete task.
  T complete = findT (task.getId (), all);

  int count = 0;
  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId ()                == complete.getId ()                 || // Self
        (complete.getAttribute ("parent") != "" &&
        it->getAttribute ("parent") == complete.getAttribute ("parent")) || // Sibling
        it->getUUID ()              == complete.getAttribute ("parent"))    // Parent
    {
      T original (*it);

      // A non-zero value forces a file write.
      int changes = 0;

      // Apply a new description, if any.
      if (task.getDescription () != "")
      {
        original.setDescription (original.getDescription () +
                                 " " +
                                task.getDescription ());
        ++changes;
      }

      // Apply or remove tags, if any.
      std::vector <std::string> tags;
      task.getTags (tags);
      for (unsigned int i = 0; i < tags.size (); ++i)
      {
        if (tags[i][0] == '+')
          original.addTag (tags[i].substr (1, std::string::npos));
        else
          original.addTag (tags[i]);

        ++changes;
      }

      task.getRemoveTags (tags);
      for (unsigned int i = 0; i < tags.size (); ++i)
      {
        if (tags[i][0] == '-')
          original.removeTag (tags[i].substr (1, std::string::npos));
        else
          original.removeTag (tags[i]);

        ++changes;
      }

      // Apply or remove attributes, if any.
      std::map <std::string, std::string> attributes;
      task.getAttributes (attributes);
      foreach (i, attributes)
      {
        if (i->second == "")
          original.removeAttribute (i->first);
        else
          original.setAttribute (i->first, i->second);

        ++changes;
      }

      std::string from;
      std::string to;
      task.getSubstitution (from, to);
      if (from != "")
      {
        std::string description = original.getDescription ();
        size_t pattern = description.find (from);
        if (pattern != std::string::npos)
        {
          description = description.substr (0, pattern) +
                        to                              +
                        description.substr (pattern + from.length (), std::string::npos);
          original.setDescription (description);
          ++changes;
        }
      }

      if (changes)
      {
        tdb.modifyT (original);

        if (conf.get ("echo.command", true))
          out << "Appended '"
              << task.getDescription ()
              << "' to task "
              << original.getId ()
              << std::endl;
      }

      ++count;
    }
  }

  if (count == 0)
    throw std::string ("Task not found.");

  if (conf.get ("echo.command", true))
    out << "Modified " << count << " task" << (count == 1 ? "" : "s") << std::endl;

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleColor (Config& conf)
{
  std::stringstream out;

  if (conf.get ("color", true) || conf.get (std::string ("_forcecolor"), false))
  {
    out << optionalBlankLine (conf) << "Foreground" << std::endl
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
        << "  " << Text::colorize (Text::nocolor, Text::on_black,          "on_black")               << "    "
                << Text::colorize (Text::nocolor, Text::on_bright_black,   "on_bright_black")        << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_red,            "on_red")                 << "      "
                << Text::colorize (Text::nocolor, Text::on_bright_red,     "on_bright_red")          << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_green,          "on_green")               << "    "
                << Text::colorize (Text::nocolor, Text::on_bright_green,   "on_bright_green")        << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_yellow,         "on_yellow")              << "   "
                << Text::colorize (Text::nocolor, Text::on_bright_yellow,  "on_bright_yellow")       << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_blue,           "on_blue")                << "     "
                << Text::colorize (Text::nocolor, Text::on_bright_blue,    "on_bright_blue")         << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_magenta,        "on_magenta")             << "  "
                << Text::colorize (Text::nocolor, Text::on_bright_magenta, "on_bright_magenta")      << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_cyan,           "on_cyan")                << "     "
                << Text::colorize (Text::nocolor, Text::on_bright_cyan,    "on_bright_cyan")         << std::endl

        << "  " << Text::colorize (Text::nocolor, Text::on_white,          "on_white")               << "    "
                << Text::colorize (Text::nocolor, Text::on_bright_white,   "on_bright_white")        << std::endl

        << optionalBlankLine (conf);
  }
  else
  {
    out << "Color is currently turned off in your .taskrc file." << std::endl;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string handleAnnotate (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;
  std::vector <T> all;
  tdb.pendingT (all);

  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == task.getId ())
    {
      it->addAnnotation (task.getDescription ());
      tdb.modifyT (*it);

      if (conf.get ("echo.command", true))
        out << "Annotated "
            << task.getId ()
            << " with '"
            << task.getDescription ()
            << "'"
            << std::endl;

      return out.str ();
    }
  }

  throw std::string ("Task not found.");
  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
T findT (int id, const std::vector <T>& all)
{
  std::vector <T>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    if (id == it->getId ())
      return *it;

  return T ();
}

////////////////////////////////////////////////////////////////////////////////
