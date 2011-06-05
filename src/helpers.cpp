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

  std::vector <Att> annotations;
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
      Att anno (annotations.back());
      Date dt (atoi (anno.name ().substr (11).c_str ()));
      std::string format = context.config.get ("dateformat.annotation");
      if (format == "")
        format = context.config.get ("dateformat");
      std::string when = dt.toString (format);
      desc += "\n" + when + " " + anno.value ();
    }
    else
    {
      std::vector <Att>::iterator anno;
      for (anno = annotations.begin (); anno != annotations.end (); ++anno)
      {
        Date dt (atoi (anno->name ().substr (11).c_str ()));
        std::string format = context.config.get ("dateformat.annotation");
        if (format == "")
          format = context.config.get ("dateformat");
        std::string when = dt.toString (format);
        desc += "\n" + when + " " + anno->value ();
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
    Filter filter;
    context.tdb.load (all, filter);

    countTasks (all,                           project, context.tdb.getAllModified (), count_pending, count_done);
    countTasks (context.tdb.getAllModified (), project, (std::vector <Task>) NULL,     count_pending, count_done);

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
int deltaAppend (Task& task)
{
/*
  if (context.task.has ("description"))
  {
    task.set ("description",
              task.get ("description") + " " + context.task.get ("description"));
    return 1;
  }
*/
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaPrepend (Task& task)
{
/*
  if (context.task.has ("description"))
  {
    task.set ("description",
              context.task.get ("description") + " " + task.get ("description"));
    return 1;
  }
*/

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaDescription (Task& task)
{
/*
  if (context.task.has ("description"))
  {
    task.set ("description", context.task.get ("description"));
    return 1;
  }
*/

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int deltaTags (Task& task)
{
  int changes = 0;

/*
  // Apply or remove tags, if any.
  std::vector <std::string> tags;
  context.task.getTags (tags);
  std::vector <std::string>::iterator tag;
  for (tag = tags.begin (); tag != tags.end (); ++tag)
  {
    task.addTag (*tag);
    ++changes;
  }

  for (tag = context.tagRemovals.begin (); tag != context.tagRemovals.end (); ++tag)
  {
    task.removeTag (*tag);
    ++changes;
  }
*/

  return changes;
}

////////////////////////////////////////////////////////////////////////////////
int deltaAttributes (Task& task)
{
  int changes = 0;

/*
  std::map <std::string, Att>::iterator att;
  for (att = context.task.begin (); att != context.task.end (); ++att)
  {
    if (att->second.name () != "uuid"        &&
        att->second.name () != "description" &&
        att->second.name () != "tags")
    {
      // Some things don't propagate to the parent task.
      if (att->second.name () == "wait" &&
          task.getStatus () == Task::recurring)
      {
        // NOP
      }

      // Modifying "wait" changes status, but not for recurring parent tasks.
      else if (att->second.name () == "wait")
      {
        if (att->second.value () == "")
        {
          task.remove (att->first);
          task.setStatus (Task::pending);
        }
        else
        {
          task.set (att->first, att->second.value ());
          task.setStatus (Task::waiting);
        }
      }

      // Modifying dependencies requires adding/removing uuids.
      else if (att->second.name () == "depends")
      {
        std::vector <std::string> deps;
        split (deps, att->second.value (), ',');

        std::vector <std::string>::iterator i;
        for (i = deps.begin (); i != deps.end (); i++)
        {
          int id = atoi (i->c_str ());
          if (id < 0)
            task.removeDependency (-id);
          else
            task.addDependency (id);
        }
      }

      // Now the generalized handling.
      else if (att->second.value () == "")
        task.remove (att->second.name ());
      else
        // One of the few places where the compound attribute name is used.
        task.set (att->first, att->second.value ());

      ++changes;
    }
  }
*/

  return changes;
}

////////////////////////////////////////////////////////////////////////////////
int deltaSubstitutions (Task& task)
{
/*
  std::string description = task.get ("description");
  std::vector <Att> annotations;
  task.getAnnotations (annotations);

  apply_subst (description, annotations);

  task.set ("description", description);
  task.setAnnotations (annotations);
*/

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
/*
void apply_subst (
  std::string& description,
  std::vector <Att>& annotations) const
{
  std::string::size_type pattern;
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  if (mFrom != "")
  {
#ifdef FEATURE_REGEX
    if (context.config.getBoolean ("regex"))
    {
      // Insert capturing parentheses, if necessary.
      std::string pattern;
      if (mFrom.find ('(') != std::string::npos)
        pattern = mFrom;
      else
        pattern = "(" + mFrom + ")";

      std::vector <int> start;
      std::vector <int> end;

      // Perform all subs on description.
      int counter = 0;
      if (regexMatch (start, end, description, pattern, sensitive))
      {
        for (unsigned int i = 0; i < start.size (); ++i)
        {
          description.replace (start[i], end[i] - start[i], mTo);
          if (!mGlobal)
            break;

          if (++counter > 1000)
            throw ("Terminated substitution because more than a thousand changes were made - infinite loop protection.");
        }
      }

      // Perform all subs on annotations.
      counter = 0;
      std::vector <Att>::iterator i;
      for (i = annotations.begin (); i != annotations.end (); ++i)
      {
        std::string annotation = i->value ();
        start.clear ();
        end.clear ();

        if (regexMatch (start, end, annotation, pattern, sensitive))
        {
          for (unsigned int match = 0; match < start.size (); ++match)
          {
            annotation.replace (start[match], end[match] - start[match], mTo);
            i->value (annotation);
            if (!mGlobal)
              break;

            if (++counter > 1000)
              throw ("Terminated substitution because more than a thousand changes were made - infinite loop protection.");
          }
        }
      }
    }
    else
    {
#endif
      if (mGlobal)
      {
        // Perform all subs on description.
        int counter = 0;
        pattern = 0;

        while ((pattern = find (description, mFrom, pattern, sensitive)) != std::string::npos)
        {
          description.replace (pattern, mFrom.length (), mTo);
          pattern += mTo.length ();

          if (++counter > 1000)
            throw ("Terminated substitution because more than a thousand changes were made - infinite loop protection.");
        }

        // Perform all subs on annotations.
        counter = 0;
        std::vector <Att>::iterator i;
        for (i = annotations.begin (); i != annotations.end (); ++i)
        {
          pattern = 0;
          std::string annotation = i->value ();
          while ((pattern = find (annotation, mFrom, pattern, sensitive)) != std::string::npos)
          {
            annotation.replace (pattern, mFrom.length (), mTo);
            pattern += mTo.length ();

            i->value (annotation);

            if (++counter > 1000)
              throw ("Terminated substitution because more than a thousand changes were made - infinite loop protection.");
          }
        }
      }
      else
      {
        // Perform first description substitution.
        if ((pattern = find (description, mFrom, sensitive)) != std::string::npos)
          description.replace (pattern, mFrom.length (), mTo);

        // Failing that, perform the first annotation substitution.
        else
        {
          std::vector <Att>::iterator i;
          for (i = annotations.begin (); i != annotations.end (); ++i)
          {
            std::string annotation = i->value ();
            if ((pattern = find (annotation, mFrom, sensitive)) != std::string::npos)
            {
              annotation.replace (pattern, mFrom.length (), mTo);
              i->value (annotation);
              break;
            }
          }
        }
      }
#ifdef FEATURE_REGEX
    }
#endif
  }
}
*/

////////////////////////////////////////////////////////////////////////////////
