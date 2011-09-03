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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include <Context.h>
#include <Directory.h>
#include <File.h>
#include <Date.h>
#include <Duration.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <main.h>

static void countTasks (const std::vector <Task>&, const std::string&, const std::vector <Task>&, int&, int&);

extern Context context;

///////////////////////////////////////////////////////////////////////////////
std::string getFullDescription (Task& task, const std::string& report)
{
  std::string desc = task.get ("description");
  std::string annotationDetails;

  std::map <std::string, std::string> annotations;
  task.getAnnotations (annotations);

  if (annotations.size () != 0)
  {
    std::string annotationDetails = context.config.get ("report." + report + ".annotations");
    if (annotationDetails == "")
      annotationDetails = context.config.get ("annotations");
    if (report == "info")
      annotationDetails = "full";

    if (annotationDetails == "none")
    {
      desc = "+" + desc;
    }
    else if (annotationDetails == "sparse")
    {
      if (annotations.size () > 1)
        desc = "+" + desc;

      std::map <std::string, std::string>::iterator i = annotations.begin ();

      Date dt (strtol (i->first.substr (11).c_str (), NULL, 10));
      std::string format = context.config.get ("dateformat.annotation");
      if (format == "")
        format = context.config.get ("dateformat");
      std::string when = dt.toString (format);
      desc += "\n" + when + " " + i->second;
    }
    else
    {
      std::map <std::string, std::string>::iterator anno;
      for (anno = annotations.begin (); anno != annotations.end (); ++anno)
      {
        Date dt (strtol (anno->first.substr (11).c_str (), NULL, 10));
        std::string format = context.config.get ("dateformat.annotation");
        if (format == "")
          format = context.config.get ("dateformat");
        std::string when = dt.toString (format);
        desc += "\n" + when + " " + anno->second;
      }
    }
  }

  return desc;
}

///////////////////////////////////////////////////////////////////////////////
std::string getDueDate (Task& task, const std::string& format)
{
  std::string due = task.get ("due");
  if (due.length ())
  {
    Date d (atoi (due.c_str ()));
    due = d.toString (format);
  }

  return due;
}

///////////////////////////////////////////////////////////////////////////////
std::string onProjectChange (Task& task, bool scope /* = true */)
{
  std::stringstream msg;
  std::string project = task.get ("project");

  if (project != "")
  {
    if (scope)
      msg << "The project '"
          << project
          << "' has changed.  ";

    // Count pending and done tasks, for this project.
    int count_pending = 0;
    int count_done = 0;

    std::vector <Task> all;
    std::vector <Task> none;
// TODO Fix.
//    context.tdb.load (all);
//
//    countTasks (all,                           project, context.tdb.getAllModified (), count_pending, count_done);
//    countTasks (context.tdb.getAllModified (), project, none,                          count_pending, count_done);

    // count_done  count_pending  percentage
    // ----------  -------------  ----------
    //          0              0          0%
    //         >0              0        100%
    //          0             >0          0%
    //         >0             >0  calculated
    int percentage = 0;
    if (count_done == 0)
      percentage = 0;
    else if (count_pending == 0)
      percentage = 100;
    else
      percentage = (count_done * 100 / (count_done + count_pending));

    msg << "Project '"
        << project
        << "' is "
        << percentage
        << "% complete ("
        << count_pending
        << " of "
        << (count_pending + count_done)
        << " tasks remaining).\n";
  }

  return msg.str ();
}

///////////////////////////////////////////////////////////////////////////////
std::string onProjectChange (Task& task1, Task& task2)
{
  std::string messages = onProjectChange (task1);
  messages            += onProjectChange (task2);

  return messages;
}

///////////////////////////////////////////////////////////////////////////////
static void countTasks (
  const std::vector <Task>& all,
  const std::string& project,
  const std::vector <Task>& skipTasks,
  int& count_pending,
  int& count_done)
{
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
  {
    bool skip = 0;

    if (it->get ("project") == project)
    {
      std::vector <Task>::const_iterator itSkipTasks;
      for (itSkipTasks = skipTasks.begin (); itSkipTasks != skipTasks.end (); ++itSkipTasks)
      {
        if (it->get("uuid") == itSkipTasks->get("uuid"))
        {
          skip = 1;
          break;
        }
      }

      if (skip == 0)
      {
        switch (it->getStatus ())
        {
        case Task::pending:
        case Task::waiting:
          ++count_pending;
          break;

        case Task::completed:
          ++count_done;
          break;

        default:
          break;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
