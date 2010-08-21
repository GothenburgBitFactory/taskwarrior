////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <stdlib.h>

#include "Att.h"
#include "text.h"
#include "util.h"
#include "main.h"
#include "../auto.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
int handleExportCSV (std::string &outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-export-command"))
  {
    std::stringstream out;

    // Deliberately no 'id'.
    out << "'uuid',"
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

    // Get all the tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      context.hooks.trigger ("pre-display", *task);

      if (task->getStatus () != Task::recurring)
        out << task->composeCSV ().c_str ();
    }

    outs = out.str ();
    context.hooks.trigger ("post-export-command");

    // Prevent messages from cluttering the export output.
    context.headers.clear ();
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// http://tools.ietf.org/html/rfc5545
//
// Note: Recurring tasks could be included in more detail.
int handleExportiCal (std::string &outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-export-command"))
  {
    std::stringstream out;

    out << "BEGIN:VCALENDAR\n"
        << "VERSION:2.0\n"
        << "PRODID:-//GBF//" << PACKAGE_STRING << "//EN\n";

    // Get all the tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      context.hooks.trigger ("pre-display", *task);

      if (task->getStatus () != Task::recurring)
      {
        out << "BEGIN:VTODO\n";

        // Required UID:20070313T123432Z-456553@example.com
        out << "UID:" << task->get ("uuid") << "\n";

        // Required DTSTAMP:20070313T123432Z
        Date entry (atoi (task->get ("entry").c_str ()));
        out << "DTSTAMP:" << entry.toISO () << "\n";

        // Optional DTSTART:20070514T110000Z
        if (task->has ("start"))
        {
          Date start (atoi (task->get ("start").c_str ()));
          out << "DTSTART:" << start.toISO () << "\n";
        }

        // Optional DUE:20070709T130000Z
        if (task->has ("due"))
        {
          Date due (atoi (task->get ("due").c_str ()));
          out << "DUE:" << due.toISO () << "\n";
        }

        // Optional COMPLETED:20070707T100000Z
        if (task->has ("end") && task->getStatus () == Task::completed)
        {
          Date end (atoi (task->get ("end").c_str ()));
          out << "COMPLETED:" << end.toISO () << "\n";
        }

        out << "SUMMARY:" << task->get ("description") << "\n";

        // Optional CLASS:PUBLIC/PRIVATE/CONFIDENTIAL
        std::string classification = context.config.get ("export.ical.class");
        if (classification == "")
          classification = "PRIVATE";
        out << "CLASS:" << classification << "\n";

        // Optional multiple CATEGORIES:FAMILY,FINANCE
        if (task->getTagCount () > 0)
        {
          std::vector <std::string> tags;
          task->getTags (tags);
          std::string all;
          join (all, ",", tags);
          out << "CATEGORIES:" << all << "\n";
        }

        // Optional PRIORITY:
        // 1-4  H
        // 5    M
        // 6-9  L
        if (task->has ("priority"))
        {
          out << "PRIORITY:";
          std::string priority = task->get ("priority");

               if (priority == "H") out << "1";
          else if (priority == "M") out << "5";
          else                      out << "9";

          out << "\n";
        }

        // Optional STATUS:NEEDS-ACTION/IN-PROCESS/COMPLETED/CANCELLED
        out << "STATUS:";
        Task::status stat = task->getStatus ();
        if (stat == Task::pending || stat == Task::waiting)
        {
          if (task->has ("start"))
            out << "IN-PROCESS";
          else
            out << "NEEDS-ACTION";
        }
        else if (stat == Task::completed)
        {
          out << "COMPLETED";
        }
        else if (stat == Task::deleted)
        {
          out << "CANCELLED";
        }
        out << "\n";

        // Optional COMMENT:annotation1
        // Optional COMMENT:annotation2
        std::vector <Att> annotations;
        task->getAnnotations (annotations);
        foreach (anno, annotations)
          out << "COMMENT:" << anno->value () << "\n";

        out << "END:VTODO\n";
      }
    }

    out << "END:VCALENDAR\n";

    outs = out.str ();
    context.hooks.trigger ("post-export-command");

    // Prevent messages from cluttering the export output.
    context.headers.clear ();
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleExportYAML (std::string &outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-export-command"))
  {
    // YAML header.
    std::stringstream out;
    out << "%YAML 1.1\n"
        << "---\n";

    // Get all the tasks.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    foreach (task, tasks)
    {
      context.hooks.trigger ("pre-display", *task);
      out << task->composeYAML ().c_str ();
    }

    out << "...\n";

    outs = out.str ();
    context.hooks.trigger ("post-export-command");

    // Prevent messages from cluttering the export output.
    context.headers.clear ();
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
