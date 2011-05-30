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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include <Permission.h>
#include <Directory.h>
#include <Nibbler.h>
#include <text.h>
#include <util.h>
#include <main.h>
#include <Transport.h>
#include <ViewText.h>
#include <cmake.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
int handleQuery (std::string& outs)
{
  int rc = 0;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Filter sequence.
  if (context.sequence.size ())
    context.filter.applySequence (tasks, context.sequence);

  if (tasks.size () == 0)
  {
    std::cout << "No matches.\n";
    return 1;
  }

  // Note: "limit:" feature not supported.

  // Compose output.
  std::vector <Task>::iterator t;
  for (t = tasks.begin (); t != tasks.end (); ++t)
  {
    if (t != tasks.begin ())
      outs += ",\n";

    outs += t->composeJSON (true);
  }

  outs += "\n";
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void handleUndo ()
{
  context.disallowModification ();

  context.tdb.lock (context.config.getBoolean ("locking"));
  context.tdb.undo ();
  context.tdb.unlock ();
}

////////////////////////////////////////////////////////////////////////////////
void handleMerge (std::string&)
{
  std::string file = trim (context.task.get ("description"));
  std::string pushfile = "";
  std::string tmpfile = "";

  std::string sAutopush = lowerCase (context.config.get        ("merge.autopush"));
  bool        bAutopush =            context.config.getBoolean ("merge.autopush");

  Uri uri (file, "merge");
  uri.parse();

  if (uri.data.length ())
  {
    Directory location (context.config.get ("data.location"));

    // be sure that uri points to a file
    uri.append ("undo.data");

    Transport* transport;
    if ((transport = Transport::getTransport (uri)) != NULL )
    {
      tmpfile = location.data + "/undo_remote.data";
      transport->recv (tmpfile);
      delete transport;

      file = tmpfile;
    }
    else
      file = uri.path;

    context.tdb.lock (context.config.getBoolean ("locking"));
    context.tdb.merge (file);
    context.tdb.unlock ();

    std::cout << "Merge complete.\n";

    if (tmpfile != "")
      remove (tmpfile.c_str ());

    if ( ((sAutopush == "ask") && (confirm ("Would you like to push the merged changes to \'" + uri.data + "\'?")) )
       || (bAutopush) )
    {
      std::string out;
	  context.task.set ("description", uri.data);
      handlePush (out);
    }
  }
  else
    throw std::string ("No uri was specified for the merge.  Either specify "
                       "the uri of a remote .task directory, or create a "
                       "'merge.default.uri' entry in your .taskrc file.");
}

////////////////////////////////////////////////////////////////////////////////
// Transfers the local data (from rc.location.data) to the remote path.  Because
// this is potentially on another machine, no checking can be performed.
void handlePush (std::string&)
{
  std::string file = trim (context.task.get ("description"));

  Uri uri (file, "push");
  uri.parse ();

  if (uri.data.length ())
  {
		Directory location (context.config.get ("data.location"));

		Transport* transport;
		if ((transport = Transport::getTransport (uri)) != NULL )
		{
			transport->send (location.data + "/{pending,undo,completed}.data");
			delete transport;
		}
		else
		{
      // Verify that files are not being copied from rc.data.location to the
      // same place.
      if (Directory (uri.path) == Directory (context.config.get ("data.location")))
        throw std::string ("Cannot push files when the source and destination are the same.");

      // copy files locally
      if (! Path (uri.data).is_directory ())
        throw std::string ("The uri '") + uri.path + "' is not a local directory.";

      std::ifstream ifile1 ((location.data + "/undo.data").c_str(), std::ios_base::binary);
      std::ofstream ofile1 ((uri.path      + "/undo.data").c_str(), std::ios_base::binary);
      ofile1 << ifile1.rdbuf();

      std::ifstream ifile2 ((location.data + "/pending.data").c_str(), std::ios_base::binary);
      std::ofstream ofile2 ((uri.path      + "/pending.data").c_str(), std::ios_base::binary);
      ofile2 << ifile2.rdbuf();

      std::ifstream ifile3 ((location.data + "/completed.data").c_str(), std::ios_base::binary);
      std::ofstream ofile3 ((uri.path      + "/completed.data").c_str(), std::ios_base::binary);
      ofile3 << ifile3.rdbuf();
		}

    std::cout << "Local tasks transferred to " << uri.data << "\n";
  }
  else
    throw std::string ("No uri was specified for the push.  Either specify "
                       "the uri of a remote .task directory, or create a "
                       "'push.default.uri' entry in your .taskrc file.");
}

////////////////////////////////////////////////////////////////////////////////
void handlePull (std::string&)
{
  std::string file = trim (context.task.get ("description"));

  Uri uri (file, "pull");
  uri.parse ();

  if (uri.data.length ())
  {
		Directory location (context.config.get ("data.location"));

    if (! uri.append ("{pending,undo,completed}.data"))
      throw std::string ("The uri '") + uri.path + "' is not a directory. Did you forget a trailing '/'?";

		Transport* transport;
		if ((transport = Transport::getTransport (uri)) != NULL)
		{
			transport->recv (location.data + "/");
			delete transport;
		}
		else
		{
      // Verify that files are not being copied from rc.data.location to the
      // same place.
      if (Directory (uri.path) == Directory (context.config.get ("data.location")))
        throw std::string ("Cannot pull files when the source and destination are the same.");

      // copy files locally

      // remove {pending,undo,completed}.data
      uri.path = uri.parent();

      Path path1 (uri.path + "undo.data");
      Path path2 (uri.path + "pending.data");
      Path path3 (uri.path + "completed.data");

      if (path1.exists() && path2.exists() && path3.exists())
      {
//        if (confirm ("xxxxxxxxxxxxx"))
//        {
          std::ofstream ofile1 ((location.data + "/undo.data").c_str(), std::ios_base::binary);
          std::ifstream ifile1 (path1.data.c_str()                    , std::ios_base::binary);
          ofile1 << ifile1.rdbuf();

          std::ofstream ofile2 ((location.data + "/pending.data").c_str(), std::ios_base::binary);
          std::ifstream ifile2 (path2.data.c_str()                    , std::ios_base::binary);
          ofile2 << ifile2.rdbuf();

          std::ofstream ofile3 ((location.data + "/completed.data").c_str(), std::ios_base::binary);
          std::ifstream ifile3 (path3.data.c_str()                    , std::ios_base::binary);
          ofile3 << ifile3.rdbuf();
//        }
      }
      else
      {
        throw std::string ("At least one of the database files in '" + uri.path + "' is not present.");
      }
		}

    std::cout << "Tasks transferred from " << uri.data << "\n";
  }
  else
    throw std::string ("No uri was specified for the pull.  Either specify "
                       "the uri of a remote .task directory, or create a "
                       "'pull.default.uri' entry in your .taskrc file.");
}

////////////////////////////////////////////////////////////////////////////////
int handleDelete (std::string& outs)
{
  int rc = 0;
  std::stringstream out;

  context.disallowModification ();

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    std::cout << "No tasks specified.\n";
    return 1;
  }

  // Determine the end date.
  char endTime[16];
  sprintf (endTime, "%u", (unsigned int) time (NULL));

  foreach (task, tasks)
  {
    if (task->getStatus () == Task::pending ||
        task->getStatus () == Task::waiting)
    {
      std::stringstream question;
      question << "Permanently delete task "
               << task->id
               << " '"
               << task->get ("description")
               << "'?";

      if (!context.config.getBoolean ("confirmation") || confirm (question.str ()))
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
            foreach (sibling, all)
            {
              if (sibling->get ("parent") == parent ||
                  sibling->get ("uuid")   == parent)
              {
                sibling->setStatus (Task::deleted);

                // Don't want a 'delete' to clobber the end date that may have
                // been written by a 'done' command.
                if (! sibling->has ("end"))
                  sibling->set ("end", endTime);

                context.tdb.update (*sibling);

                if (context.config.getBoolean ("echo.command"))
                  out << "Deleting recurring task "
                      << sibling->id
                      << " '"
                      << sibling->get ("description")
                      << "'.\n";
              }
            }
          }
          else
          {
            // Update mask in parent.
            task->setStatus (Task::deleted);
            updateRecurrenceMask (all, *task);

            // Don't want a 'delete' to clobber the end date that may have
            // been written by a 'done' command.
            if (! task->has ("end"))
              task->set ("end", endTime);

            context.tdb.update (*task);

            out << "Deleting recurring task "
                << task->id
                << " '"
                << task->get ("description")
                << "'.\n";

            dependencyChainOnComplete (*task);
            context.footnote (onProjectChange (*task));
          }
        }
        else
        {
          task->setStatus (Task::deleted);

          // Don't want a 'delete' to clobber the end date that may have
          // been written by a 'done' command.
          if (! task->has ("end"))
            task->set ("end", endTime);

          context.tdb.update (*task);

          if (context.config.getBoolean ("echo.command"))
            out << "Deleting task "
                << task->id
                << " '"
                << task->get ("description")
                << "'.\n";

          dependencyChainOnComplete (*task);
          context.footnote (onProjectChange (*task));
        }
      }
      else
      {
        out << "Task not deleted.\n";
        rc  = 1;
      }
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is neither pending nor waiting.\n";
      rc = 1;
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  outs = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleDone (std::string& outs)
{
  int rc = 0;
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    std::cout << "No tasks specified.\n";
    return 1;
  }

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
    permission.bigSequence ();

  bool nagged = false;
  foreach (task, tasks)
  {
    if (task->getStatus () == Task::pending ||
        task->getStatus () == Task::waiting)
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

      // Stop the task, if started.
      if (task->has ("start") &&
          context.config.getBoolean ("journal.time"))
        task->addAnnotation (context.config.get ("journal.time.stop.annotation"));

      // Only allow valid tasks.
      task->validate ();

      if (taskDiff (before, *task))
      {
        if (permission.confirmed (before, taskDifferences (before, *task) + "Proceed with change?"))
        {
          context.tdb.update (*task);

          if (context.config.getBoolean ("echo.command"))
            out << "Completed "
                << task->id
                << " '"
                << task->get ("description")
                << "'.\n";

          dependencyChainOnComplete (*task);
          context.footnote (onProjectChange (*task, false));

          ++count;
        }
      }

      updateRecurrenceMask (all, *task);
      if (!nagged)
        nagged = nag (*task);
    }
    else
    {
      out << "Task "
          << task->id
          << " '"
          << task->get ("description")
          << "' is neither pending nor waiting.\n";
      rc = 1;
    }
  }

  if (count)
    context.tdb.commit ();

  context.tdb.unlock ();

  if (context.config.getBoolean ("echo.command"))
    out << "Marked "
        << count
        << " task"
        << (count == 1 ? "" : "s")
        << " as done.\n";

  outs = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleModify (std::string& outs)
{
  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);
  if (tasks.size () == 0)
  {
    std::cout << "No tasks specified.\n";
    return 1;
  }

  Permission permission;
  if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
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

    if (task->has ("recur")     &&
        task->has ("due")        &&
        context.task.has ("due") &&
        context.task.get ("due") == "")
      throw std::string ("You cannot remove the due date from a recurring task.");

    if (task->has ("recur")        &&
        context.task.has ("recur") &&
        context.task.get ("recur") == "")
      throw std::string ("You cannot remove the recurrence from a recurring task.");

    // Make all changes.
    bool warned = false;
    foreach (other, all)
    {
      // Skip wait: modification to a parent task, and other child tasks.  Too
      // difficult to achieve properly without losing 'waiting' as a status.
      // Soon...
      if (other->id             == task->id               || // Self
          (! context.task.has ("wait") &&                    // skip waits
           task->has ("parent") &&                           // is recurring
           task->get ("parent") == other->get ("parent")) || // Sibling
          other->get ("uuid")   == task->get ("parent"))     // Parent
      {
        if (task->has ("parent") && !warned)
        {
          warned = true;
          std::cout << "Task "
                    << task->id
                    << " is a recurring task, and all other instances of this"
                    << " task will be modified.\n";
        }

        Task before (*other);

        // A non-zero value forces a file write.
        int changes = 0;

        // If a task is being made recurring, there are other cascading
        // changes.
        if (!task->has ("recur") &&
            context.task.has ("recur"))
        {
          other->setStatus (Task::recurring);
          other->set ("mask", "");
          ++changes;

          std::cout << "Task "
                    << other->id
                    << " is now a recurring task.\n";
        }

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
          // Only allow valid tasks.
          other->validate ();

          if (changes && permission.confirmed (before, taskDifferences (before, *other) + "Proceed with change?"))
          {
            // TODO Are dependencies being explicitly removed?
            //      Either we scan context.task for negative IDs "depends:-n"
            //      or we ask deltaAttributes (above) to record dependency
            //      removal.
            dependencyChainOnModify (before, *other);

            context.tdb.update (*other);

            if (before.get ("project") != other->get ("project"))
              context.footnote (onProjectChange (before, *other));

            ++count;
          }
        }
      }
    }
  }

  context.tdb.commit ();
  context.tdb.unlock ();

  if (context.config.getBoolean ("echo.command"))
    out << "Modified " << count << " task" << (count == 1 ? ".\n" : "s.\n");

  outs = out.str ();
  return 0;
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
int deltaPrepend (Task& task)
{
  if (context.task.has ("description"))
  {
    task.set ("description",
              context.task.get ("description") + " " + task.get ("description"));
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
// vim: ts=2 sw=2 et
