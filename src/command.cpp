////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include "TDB.h"
#include "T.h"
#include "task.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

////////////////////////////////////////////////////////////////////////////////
void handleAdd (const TDB& tdb, T& task, Config& conf)
{
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

  if (task.getDescription () == "")
    throw std::string ("Cannot add a blank task.");

  if (!tdb.addT (task))
    throw std::string ("Could not create new task.");
}

////////////////////////////////////////////////////////////////////////////////
void handleProjects (TDB& tdb, T& task, Config& conf)
{
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

    if (conf.get ("color", true))
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

    std::cout << optionalBlankLine (conf)
              << table.render ()
              << optionalBlankLine (conf)
              << unique.size ()
              << (unique.size () == 1 ? " project" : " projects")
              << std::endl;
  }
  else
    std::cout << "No projects."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void handleTags (TDB& tdb, T& task, Config& conf)
{
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
    std::cout << optionalBlankLine (conf)
              << unique.size ()
              << (unique.size () == 1 ? " tag" : " tags")
              << std::endl;
  else
    std::cout << "No tags."
              << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// If a task is deleted, but is still in the pending file, then it may be
// undeleted simply by changing it's status.
void handleUndelete (TDB& tdb, T& task, Config& conf)
{
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
          std::cout << "Task does not support 'undelete' for recurring tasks." << std::endl;
          return;
        }

        T restored (*it);
        restored.setStatus (T::pending);
        restored.removeAttribute ("end");
        tdb.modifyT (restored);

        std::cout << "Task " << id << " successfully undeleted." << std::endl;
        return;
      }
      else
      {
        std::cout << "Task " << id << " is not deleted - therefore cannot undelete." << std::endl;
        return;
      }
    }
  }

  std::cout << "Task " << id
            << " not found - tasks can only be reliably undeleted if the undelete" << std::endl
            << "command is run immediately after the errant delete command." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
// If a task is done, but is still in the pending file, then it may be undone
// simply by changing it's status.
void handleUndo (TDB& tdb, T& task, Config& conf)
{
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
        {
          std::cout << "Task does not support 'undo' for recurring tasks." << std::endl;
          return;
        }

        T restored (*it);
        restored.setStatus (T::pending);
        restored.removeAttribute ("end");
        tdb.modifyT (restored);

        std::cout << "Task " << id << " successfully undone." << std::endl;
        return;
      }
      else
      {
        std::cout << "Task " << id << " is not done - therefore cannot be undone." << std::endl;
        return;
      }
    }
  }

  std::cout << "Task " << id
            << " not found - tasks can only be reliably undone if the undo" << std::endl
            << "command is run immediately after the errant done command." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void handleVersion (Config& conf)
{
  // Determine window size, and set table accordingly.
  int width = conf.get ("defaultwidth", 80);
#ifdef HAVE_LIBNCURSES
  if (conf.get ("curses", true))
  {
    WINDOW* w = initscr ();
    width = w->_maxx + 1;
    endwin ();
  }
#endif

  // Create a table for output.
  Table table;
  table.setTableWidth (width);
  table.setDateFormat (conf.get ("dateformat", "m/d/Y"));
  table.addColumn ("Config variable");
  table.addColumn ("Value");

  if (conf.get ("color", true))
  {
    table.setColumnUnderline (0);
    table.setColumnUnderline (1);
  }

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

  std::cout << "Copyright (C) 2006 - 2008, P. Beckingham."
            << std::endl
            << PACKAGE
            << " "
            << VERSION
            << std::endl
            << std::endl
            << "Task comes with ABSOLUTELY NO WARRANTY; for details read the COPYING file"
            << std::endl
            << "included.  This is free software, and you are welcome to redistribute it"
            << std::endl
            << "under certain conditions; again, see the COPYING file for details."
            << std::endl
            << std::endl
            << table.render ()
            << std::endl
            << "See http://www.beckingham.net/task.html for the latest releases and a full tutorial."
            << std::endl
            << std::endl;

  // Verify installation.  This is mentioned in the documentation as the way to
  // ensure everything is properly installed.

  if (all.size () == 0)
    std::cout << "Configuration error: .taskrc contains no entries"
              << std::endl;
  else
  {
    if (conf.get ("data.location") == "")
      std::cout << "Configuration error: data.location not specified in .taskrc "
                   "file."
                << std::endl;

    if (access (expandPath (conf.get ("data.location")).c_str (), X_OK))
      std::cout << "Configuration error: data.location contains a directory name"
                   " that doesn't exist, or is unreadable."
                << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
void handleDelete (TDB& tdb, T& task, Config& conf)
{
  if (conf.get ("confirmation") != "yes" || confirm ("Permanently delete task?"))
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
              if (sibling->getAttribute ("parent") == parent ||
                  sibling->getUUID ()              == parent)
                tdb.deleteT (*sibling);

            return;
          }
          else
          {
            // Update mask in parent.
            t->setStatus (T::deleted);
            updateRecurrenceMask (tdb, all, *t);
            tdb.deleteT (*t);
            return;
          }
        }
        else
          tdb.deleteT (*t);

        break;  // No point continuing the loop.
      }
    }
  }
  else
    std::cout << "Task not deleted." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
void handleStart (TDB& tdb, T& task, Config& conf)
{
  std::vector <T> all;
  tdb.pendingT (all);

  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == task.getId ())
    {
      T original (*it);

      if (original.getAttribute ("start") == "")
      {
        char startTime[16];
        sprintf (startTime, "%u", (unsigned int) time (NULL));
        original.setAttribute ("start", startTime);

        original.setId (task.getId ());
        tdb.modifyT (original);

        nag (tdb, task, conf);
        return;
      }
      else
        std::cout << "Task " << task.getId () << " already started." << std::endl;
    }
  }

  throw std::string ("Task not found.");
}

////////////////////////////////////////////////////////////////////////////////
void handleDone (TDB& tdb, T& task, Config& conf)
{
  if (!tdb.completeT (task))
    throw std::string ("Could not mark task as completed.");

  // Now update mask in parent.
  std::vector <T> all;
  tdb.allPendingT (all);
  foreach (t, all)
  {
    if (t->getId () == task.getId ())
    {
      t->setStatus (T::completed);
      updateRecurrenceMask (tdb, all, *t);
      break;
    }
  }

  nag (tdb, task, conf);
}

////////////////////////////////////////////////////////////////////////////////
void handleExport (TDB& tdb, T& task, Config& conf)
{
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
          << "'status',"
          << "'tags',"
          << "'entry',"
          << "'start',"
          << "'due',"
          << "'end',"
          << "'project',"
          << "'priority',"
          << "'fg',"
          << "'bg',"
          << "'description'"
          << "\n";

      std::vector <T> all;
      tdb.allT (all);
      filter (all, task);
      foreach (t, all)
      {
        out << t->composeCSV ().c_str ();
      }
      out.close ();
    }
    else
      throw std::string ("Could not write to export file.");
  }
  else
    throw std::string ("You must specify a file to write to.");
}

////////////////////////////////////////////////////////////////////////////////
void handleModify (TDB& tdb, T& task, Config& conf)
{
  std::vector <T> all;
  tdb.pendingT (all);

  std::vector <T>::iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    if (it->getId () == task.getId ())
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
        original.setId (task.getId ());
        tdb.modifyT (original);
      }

      return;
    }
  }

  throw std::string ("Task not found.");
}

////////////////////////////////////////////////////////////////////////////////
void handleColor (Config& conf)
{
  if (conf.get ("color", true))
  {
    std::cout << optionalBlankLine (conf) << "Foreground" << std::endl
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

              <<                 optionalBlankLine (conf);
  }
  else
  {
    std::cout << "Color is currently turned off in your .taskrc file." << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
