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

#include "Permission.h"
#include "Directory.h"
#include "Nibbler.h"
#include "text.h"
#include "util.h"
#include "main.h"
#include "../auto.h"
#include "Transport.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
int handleAdd (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-add-command"))
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

    // Tasks with a wait: date get a special status.
    else if (context.task.has ("wait"))
      context.task.setStatus (Task::waiting);

    // By default, tasks are pending.
    else
      context.task.setStatus (Task::pending);

    // Override with default.project, if not specified.
    if (context.task.get ("project") == "")
      context.task.set ("project", context.config.get ("default.project"));

    // Override with default.priority, if not specified.
    if (context.task.get ("priority") == "")
    {
      std::string defaultPriority = context.config.get ("default.priority");
      if (Att::validNameValue ("priority", "", defaultPriority))
        context.task.set ("priority", defaultPriority);
    }

    // Override with default.due, if not specified.
    if (context.task.get ("due") == "")
    {
      std::string defaultDue = context.config.get ("default.due");
      if (defaultDue != "" &&
          Att::validNameValue ("due", "", defaultDue))
        context.task.set ("due", defaultDue);
    }

    // Include tags.
    foreach (tag, context.tagAdditions)
      context.task.addTag (*tag);

    // Must load pending to resolve dependencies, and to provide a new ID.
    context.tdb.lock (context.config.getBoolean ("locking"));

    std::vector <Task> all;
    Filter none;
    context.tdb.loadPending (all, none);

    // Resolve dependencies.
    if (context.task.has ("depends"))
    {
      // Convert ID to UUID.
      std::vector <std::string> deps;
      split (deps, context.task.get ("depends"), ',');

      // Eliminate the ID-based set.
      context.task.set ("depends", "");

      std::vector <std::string>::iterator i;
      for (i = deps.begin (); i != deps.end (); i++)
      {
        int id = atoi (i->c_str ());
        if (id < 0)
          context.task.removeDependency (-id);
        else
          context.task.addDependency (id);
      }
    }

    // Only valid tasks can be added.
    context.task.validate ();

    context.tdb.add (context.task);

#ifdef FEATURE_NEW_ID
    out << "Created task " << context.tdb.nextId () << ".\n";
#endif

    context.footnote (onProjectChange (context.task));

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-add-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleLog (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-log-command"))
  {
    std::stringstream out;

    context.task.setStatus (Task::completed);
    context.task.set ("uuid", uuid ());
    context.task.setEntry ();

    // Add an end date.
    char entryTime[16];
    sprintf (entryTime, "%u", (unsigned int) time (NULL));
    context.task.set ("end", entryTime);

    // Recurring tasks get a special status.
    if (context.task.has ("recur"))
      throw std::string ("You cannot log recurring tasks.");

    if (context.task.has ("wait"))
      throw std::string ("You cannot log waiting tasks.");

    // It makes no sense to add dependencies to an already-completed task.
    if (context.task.get ("depends") != "")
      throw std::string ("You cannot specify dependencies on a completed task.");

    // Override with default.project, if not specified.
    if (context.task.get ("project") == "")
      context.task.set ("project", context.config.get ("default.project"));

    // Override with default.priority, if not specified.
    if (context.task.get ("priority") == "")
    {
      std::string defaultPriority = context.config.get ("default.priority");
      if (Att::validNameValue ("priority", "", defaultPriority))
        context.task.set ("priority", defaultPriority);
    }

    // Override with default.due, if not specified.
    if (context.task.get ("due") == "")
    {
      std::string defaultDue = context.config.get ("default.due");
      if (defaultDue != "" &&
          Att::validNameValue ("due", "", defaultDue))
        context.task.set ("due", defaultDue);
    }

    // Include tags.
    foreach (tag, context.tagAdditions)
      context.task.addTag (*tag);

    // Only valid tasks can be added.
    context.task.validate ();

    context.tdb.lock (context.config.getBoolean ("locking"));
    context.tdb.add (context.task);
    context.tdb.commit ();

    if (context.config.getBoolean ("echo.command"))
      out << "Logged task.\n";

    context.footnote (onProjectChange (context.task));
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-log-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleProjects (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-projects-command"))
  {
    std::stringstream out;

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    int quantity;
    if (context.config.getBoolean ("list.all.projects"))
      quantity = context.tdb.load (tasks, context.filter);
    else
      quantity = context.tdb.loadPending (tasks, context.filter);

    context.tdb.commit ();
    context.tdb.unlock ();

    // Scan all the tasks for their project name, building a map using project
    // names as keys.
    std::map <std::string, int> unique;
    std::map <std::string, int> high;
    std::map <std::string, int> medium;
    std::map <std::string, int> low;
    std::map <std::string, int> none;
    bool no_project = false;
    std::string project;
    std::string priority;
    foreach (t, tasks)
    {
      project = t->get ("project");
      priority = t->get ("priority");

      unique[project] += 1;
      if (project == "")
        no_project = true;

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

      if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
          context.config.getBoolean ("fontunderline"))
      {
        table.setColumnUnderline (0);
        table.setColumnUnderline (1);
        table.setColumnUnderline (2);
        table.setColumnUnderline (3);
        table.setColumnUnderline (4);
        table.setColumnUnderline (5);
      }
      else
        table.setTableDashedUnderline ();

      table.setColumnJustification (1, Table::right);
      table.setColumnJustification (2, Table::right);
      table.setColumnJustification (3, Table::right);
      table.setColumnJustification (4, Table::right);
      table.setColumnJustification (5, Table::right);

      foreach (i, unique)
      {
        int row = table.addRow ();
        table.addCell (row, 0, (i->first == "" ? "(none)" : i->first));
        table.addCell (row, 1, i->second);
        table.addCell (row, 2, none[i->first]);
        table.addCell (row, 3, low[i->first]);
        table.addCell (row, 4, medium[i->first]);
        table.addCell (row, 5, high[i->first]);
      }

      int number_projects = unique.size ();
      if (no_project)
        --number_projects;

      out << optionalBlankLine ()
          << table.render ()
          << optionalBlankLine ()
          << number_projects
          << (number_projects == 1 ? " project" : " projects")
          << " (" << quantity << (quantity == 1 ? " task" : " tasks") << ")\n";
    }
    else
    {
      out << "No projects.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-projects-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleCompletionProjects (std::string& outs)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));

  Filter filter;
  if (context.config.getBoolean ("complete.all.projects"))
    context.tdb.load (tasks, filter);
  else
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
      out << project->first << "\n";

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int handleTags (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-tags-command"))
  {
    std::stringstream out;

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    int quantity = 0;
    if (context.config.getBoolean ("list.all.tags"))
      quantity += context.tdb.load (tasks, context.filter);
    else
      quantity += context.tdb.loadPending (tasks, context.filter);

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

    bool use_color = context.config.getBoolean ("color") ||
                     context.config.getBoolean ("_forcecolor");

    if (unique.size ())
    {
      // Render a list of tags names from the map.
      Table table;
      table.addColumn ("Tag");
      table.addColumn ("Count");

      if (use_color)
      {
        table.setColumnUnderline (0);
        table.setColumnUnderline (1);
      }

      table.setColumnJustification (1, Table::right);

      Color bold ("bold");
      foreach (i, unique)
      {
        int row = table.addRow ();
        table.addCell (row, 0, i->first);
        table.addCell (row, 1, i->second);

        // Highlight the special tags.
        if (use_color && (i->first == "nocolor" ||
                          i->first == "nonag"))
        {
          table.setRowColor (row, bold);
        }
      }

      out << optionalBlankLine ()
          << table.render ()
          << optionalBlankLine ()
          << unique.size ()
          << (unique.size () == 1 ? " tag" : " tags")
          << " (" << quantity << (quantity == 1 ? " task" : " tasks") << ")\n";
    }
    else
    {
      out << "No tags.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-tags-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleCompletionTags (std::string& outs)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));

  Filter filter;
  if (context.config.getBoolean ("complete.all.tags"))
    context.tdb.load (tasks, filter);
  else
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
    out << tag->first << "\n";

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int handleCompletionCommands (std::string& outs)
{
  // Get a list of all commands.
  std::vector <std::string> commands;
  context.cmd.allCommands (commands);

  // Sort alphabetically.
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  foreach (command, commands)
    out << *command << "\n";

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int handleCompletionConfig (std::string& outs)
{
  std::vector <std::string> configs;
  context.config.all (configs);
  std::sort (configs.begin (), configs.end ());

  std::stringstream out;
  foreach (config, configs)
    out << *config << "\n";

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// A simple version display for use by completion scripts and the task-update
// script.
int handleCompletionVersion (std::string& outs)
{
  outs = VERSION;
  outs += "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Temporary command to display urgency for a task.
int handleUrgency (std::string& outs)
{
  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.loadPending (tasks, context.filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  // Filter sequence.
  context.filter.applySequence (tasks, context.sequence);

  // Find the task(s).
  std::stringstream out;
  foreach (task, tasks)
  {
    out << "task "
        << task->id
        << " urgency "
        << task->urgency ()
        << "\n";
  }

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int handleQuery (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-query-command"))
  {
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

    // Note: "limit:" feature not supported.

    // Compose output.
    outs = "{";
    std::vector <Task>::iterator t;
    for (t = tasks.begin (); t != tasks.end (); ++t)
    {
      if (t != tasks.begin ())
        outs += ",\n";

      outs += t->composeJSON ();
    }

    outs += "}\n";

    context.hooks.trigger ("post-query-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleCompletionIDs (std::string& outs)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  std::vector <int> ids;
  foreach (task, tasks)
    if (task->getStatus () != Task::deleted &&
        task->getStatus () != Task::completed)
      ids.push_back (task->id);

  std::sort (ids.begin (), ids.end ());

  std::stringstream out;
  foreach (id, ids)
    out << *id << "\n";

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int handleZshCompletionIDs (std::string& outs)
{
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);
  context.tdb.commit ();
  context.tdb.unlock ();

  std::stringstream out;
  foreach (task, tasks) {
    if (task->getStatus () != Task::deleted &&
        task->getStatus () != Task::completed) {
      out << task->id << ":" << task->get("description") << "\n";
    }
  }

  outs = out.str ();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
int handleZshCompletionCommands (std::string& outs)
{
  // Get a list of all commands.
  std::vector <std::string> commands;
  context.cmd.allCommands (commands);

  // Sort alphabetically.
  std::sort (commands.begin (), commands.end ());

  std::stringstream out;
  foreach (command, commands) {
    out << *command << ":";

    if (*command == "add") {
      out << "Adds a new task.";
    }
    else if (*command == "log") {
      out << "Adds a new task that is already completed.";
    }
    else if (*command == "append") {
      out << "Appends more description to an existing task.";
    }
    else if (*command == "prepend") {
      out << "Prepends more description to an existing task.";
    }
    else if (*command == "annotate") {
      out << "Adds an annotation to an existing task.";
    }
    else if (*command == "denotate") {
      out << "Deletes an annotation of an existing task.";
    }
    else if (*command == "edit") {
      out << "Launches an editor to let you modify a task directly.";
    }
    else if (*command == "undo") {
      out << "Reverts the most recent action.";
    }
    else if (*command == "shell") {
      out << "Launches an interactive shell.";
    }
    else if (*command == "duplicate") {
      out <<  "Duplicates the specified task, and allows modifications.";
    }
    else if (*command == "delete") {
      out << "Deletes the specified task.";
    }
    else if (*command == "info") {
      out << "Shows all data, metadata for specified task.";
    }
    else if (*command == "start") {
      out << "Marks specified task as started.";
    }
    else if (*command == "stop") {
      out << "Removes the 'start' time from a task.";
    }
    else if (*command == "done") {
      out << "Marks the specified task as completed.";
    }
    else if (*command == "projects") {
      out << "Shows a list of all project names used.";
    }
    else if (*command == "tags") {
      out << "Shows a list of all tags used.";
    }
    else if (*command == "summary") {
      out << "Shows a report of task status by project.";
    }
    else if (*command == "timesheet") {
      out << "Shows a weekly report of tasks completed and started.";
    }
    else if (*command == "history") {
      out << "Alias to history.monthly.";
    }
    else if (*command == "history.monthly") {
      out << "Shows a report of task history, by month.";
    }
    else if (*command == "history.annual") {
      out << "Shows a report of task history, by year.";
    }
    else if (*command == "ghistory") {
      out <<  "Alias to ghistory.monthly.";
    }
    else if (*command == "ghistory.monthly") {
      out <<  "Shows a graphical report of task history, by month.";
    }
    else if (*command == "ghistory.annual") {
      out << "Shows a graphical report of task history, by year.";
    }
    else if (*command == "burndown") {
      out << "Alias to burndown.weekly.";
    }
    else if (*command == "burndown.daily") {
      out << "Shows a graphical burndown chart, by day.";
    }
    else if (*command == "burndown.weekly") {
      out << "Shows a graphical burndown chart, by week.";
    }
    else if (*command == "burndown.monthly") {
      out << "Shows a graphical burndown chart, by month.";
    }
    else if (*command == "calendar") {
      out << "Shows a calendar, with due tasks marked.";
    }
    else if (*command == "stats") {
      out << "Shows task database statistics.";
    }
    else if (*command == "import") {
      out << "Imports tasks from a variety of formats.";
    }
    else if (*command == "export") {
      out << "Alias to export.csv.";
    }
    else if (*command == "export.csv") {
      out << "Lists all tasks in CSV format.";
    }
    else if (*command == "export.ical") {
      out << "Lists all tasks in iCalendar format.";
    }
    else if (*command == "export.vcalendar") {
      out << "Lists all tasks in vCalendar format.";
    }
    else if (*command == "export.yaml") {
      out << "Lists all tasks in YAML format.";
    }
    else if (*command == "merge") {
      out << "Merges the specified database with the local database.";
    }
    else if (*command == "push") {
      out << "Pushed the local database to the specified URI.";
    }
    else if (*command == "pull") {
      out << "Overwrites the local database with that found at the URI.";
    }
    else if (*command == "colors") {
      out << "Displays all possible colors or a named sample [legend | sample].";
    }
    else if (*command == "count") {
      out << "Shows only the number of matching tasks.";
    }
    else if (*command == "version") {
      out << "Shows the task version number.";
    }
    else if (*command == "show") {
      out << "Shows the entire task configuration variables.";
    }
    else if (*command == "config") {
      out << "Add, modify and remove settings in the task configuration.";
    }
    else if (*command == "diagnostics") {
      out << "Information needed when reporting a problem.";
    }
    else if (*command == "help") {
      out << "Shows the long usage text.";
    }
    else if (*command == "rm") {
      out << "Alias to delete.";
    }
    else {
      // try to interpret this as custom report
      out << context.config.get (std::string ("report." + *command + ".description"));
    }

    out << "\n";
  }

  outs = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void handleUndo ()
{
  if (context.hooks.trigger ("pre-undo-command"))
  {
    context.disallowModification ();

    context.tdb.lock (context.config.getBoolean ("locking"));
    context.tdb.undo ();
    context.tdb.unlock ();

    context.hooks.trigger ("post-undo-command");
  }
}

////////////////////////////////////////////////////////////////////////////////
void handleMerge (std::string& outs)
{
  if (context.hooks.trigger ("pre-merge-command"))
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

      context.hooks.trigger ("post-merge-command");

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
}

////////////////////////////////////////////////////////////////////////////////
// Transfers the local data (from rc.location.data) to the remote path.  Because
// this is potentially on another machine, no checking can be performed.
void handlePush (std::string& outs)
{
  if (context.hooks.trigger ("pre-push-command"))
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

      context.hooks.trigger ("post-push-command");
    }
    else
      throw std::string ("No uri was specified for the push.  Either specify "
                         "the uri of a remote .task directory, or create a "
                         "'push.default.uri' entry in your .taskrc file.");
  }
}

////////////////////////////////////////////////////////////////////////////////
void handlePull (std::string& outs)
{
  if (context.hooks.trigger ("pre-pull-command"))
  {
    std::string file = trim (context.task.get ("description"));

    Uri uri (file, "pull");
    uri.parse ();

    if (uri.data.length ())
    {
			Directory location (context.config.get ("data.location"));

      if (! uri.append ("{pending,undo,completed}.data"))
        throw std::string ("The uri '") + uri.path + "' is not a local directory.";

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
//          if (confirm ("xxxxxxxxxxxxx"))
//          {
            std::ofstream ofile1 ((location.data + "/undo.data").c_str(), std::ios_base::binary);
            std::ifstream ifile1 (path1.data.c_str()                    , std::ios_base::binary);
            ofile1 << ifile1.rdbuf();

            std::ofstream ofile2 ((location.data + "/pending.data").c_str(), std::ios_base::binary);
            std::ifstream ifile2 (path2.data.c_str()                    , std::ios_base::binary);
            ofile2 << ifile2.rdbuf();

            std::ofstream ofile3 ((location.data + "/completed.data").c_str(), std::ios_base::binary);
            std::ifstream ifile3 (path3.data.c_str()                    , std::ios_base::binary);
            ofile3 << ifile3.rdbuf();
//          }
        }
        else
        {
          throw std::string ("At least one of the database files in '" + uri.path + "' is not present.");
        }
			}

      std::cout << "Tasks transferred from " << uri.data << "\n";

      context.hooks.trigger ("post-pull-command");
    }
    else
      throw std::string ("No uri was specified for the pull.  Either specify "
                         "the uri of a remote .task directory, or create a "
                         "'pull.default.uri' entry in your .taskrc file.");
  }
}

////////////////////////////////////////////////////////////////////////////////
int handleVersion (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-version-command"))
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
        "Taskwarrior may be copied only under the terms of the GNU General Public "
        "License, which may be found in the taskwarrior source kit.");

    // Create a table for the URL.
    Table link;
    link.setTableWidth (width);
    link.addColumn (" ");
    link.setColumnWidth (0, Table::flexible);
    link.setColumnJustification (0, Table::left);
    link.addCell (link.addRow (), 0,
      "Documentation for taskwarrior can be found using 'man task', 'man taskrc', "
      "'man task-tutorial', 'man task-color', 'man task-sync', 'man task-faq' or at "
      "http://taskwarrior.org");

    Color bold ("bold");

    out << "\n"
        << ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
             ? bold.colorize (PACKAGE)
             : PACKAGE)
        << " "
        << ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
             ? bold.colorize (VERSION)
             : VERSION)
        << " built for "

#if defined (DARWIN)
        << "darwin"
#elif defined (SOLARIS)
        << "solaris"
#elif defined (CYGWIN)
        << "cygwin"
#elif defined (OPENBSD)
        << "openbsd"
#elif defined (HAIKU)
        << "haiku"
#elif defined (FREEBSD)
        << "freebsd"
#elif defined (LINUX)
        << "linux"
#else
        << "unknown"
#endif

#ifdef HAVE_LIBREADLINE
        << "-readline"
#endif

#ifdef HAVE_LIBLUA
        << "-lua"
#endif

        << "\n"
        << "Copyright (C) 2006 - 2011 P. Beckingham, F. Hernandez.\n"
#ifdef HAVE_LIBLUA
        << "Portions of this software Copyright (C) 1994 – 2008 Lua.org, PUC-Rio.\n"
#endif
        << disclaimer.render ()
        << link.render ()
        << "\n";

    outs = out.str ();
    context.hooks.trigger ("post-version-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleShow (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-config-command"))
  {
    std::stringstream out;

    // Obtain the arguments from the description.  That way, things like '--'
    // have already been handled.
    std::vector <std::string> args;
    split (args, context.task.get ("description"), ' ');

    if (args.size () > 1)
      throw std::string ("The show command takes zero or one option.");

    int width = context.getWidth ();

    std::vector <std::string> all;
    context.config.all (all);

    // Complain about configuration variables that are not recognized.
    // These are the regular configuration variables.
    // Note that there is a leading and trailing space, to make it easier to
    // search for whole words.
    std::string recognized =
      " annotations blanklines bulk burndown.bias calendar.details calendar.details.report "
      "calendar.holidays calendar.legend color calendar.offset calendar.offset.value "
      "color.active color.due color.due.today color.blocked color.burndown.done "
      "color.burndown.pending color.burndown.started color.overdue color.pri.H "
      "color.pri.L color.pri.M color.pri.none color.recurring color.tagged "
      "color.footnote color.header color.debug color.alternate color.calendar.today "
      "color.calendar.due color.calendar.due.today color.calendar.overdue regex "
      "color.calendar.weekend color.calendar.holiday color.calendar.weeknumber "
      "color.summary.background color.summary.bar color.history.add "
      "color.history.done color.history.delete color.undo.before "
      "color.sync.added color.sync.changed color.sync.rejected "
      "color.undo.after confirmation curses data.location dateformat "
      "dateformat.holiday dateformat.report dateformat.annotation debug "
      "default.command default.due default.priority default.project defaultwidth due "
      "dependency.confirmation dependency.reminder locale displayweeknumber "
      "export.ical.class echo.command fontunderline gc locking monthsperline "
      "nag next journal.time journal.time.start.annotation journal.info "
      "journal.time.stop.annotation project shadow.command shadow.file "
      "shadow.notify weekstart editor edit.verbose import.synonym.id import.synonym.uuid "
      "complete.all.projects complete.all.tags search.case.sensitive hooks "
      "active.indicator tag.indicator recurrence.indicator recurrence.limit "
      "list.all.projects list.all.tags undo.style verbose rule.precedence.color "
      "merge.autopush merge.default.uri pull.default.uri push.default.uri "
#ifdef FEATURE_SHELL
      "shell.prompt "
#endif
      "import.synonym.status import.synonym.tags import.synonym.entry "
      "import.synonym.start import.synonym.due import.synonym.recur "
      "import.synonym.end import.synonym.project import.synonym.priority "
      "import.synonym.fg import.synonym.bg import.synonym.description "

      "urgency.next.coefficient urgency.blocking.coefficient "
      "urgency.blocked.coefficient urgency.due.coefficient "
      "urgency.priority.coefficient urgency.waiting.coefficient "
      "urgency.active.coefficient urgency.project.coefficient "
      "urgency.tags.coefficient urgency.annotations.coefficient ";

    // This configuration variable is supported, but not documented.  It exists
    // so that unit tests can force color to be on even when the output from task
    // is redirected to a file, or stdout is not a tty.
    recognized += "_forcecolor ";

    std::vector <std::string> unrecognized;
    foreach (i, all)
    {
      // Disallow partial matches by tacking a leading and trailing space on each
      // variable name.
      std::string pattern = " " + *i + " ";
      if (recognized.find (pattern) == std::string::npos)
      {
        // These are special configuration variables, because their name is
        // dynamic.
        if (i->substr (0, 14) != "color.keyword."        &&
            i->substr (0, 14) != "color.project."        &&
            i->substr (0, 10) != "color.tag."            &&
            i->substr (0,  8) != "holiday."              &&
            i->substr (0,  7) != "report."               &&
            i->substr (0,  6) != "alias."                &&
            i->substr (0,  5) != "hook."                 &&
            i->substr (0, 21) != "urgency.user.project." &&
            i->substr (0, 17) != "urgency.user.tag.")
        {
          unrecognized.push_back (*i);
        }
      }
    }

    // Find all the values that match the defaults, for highlighting.
    std::vector <std::string> default_values;
    Config default_config;
    default_config.setDefaults ();

    foreach (i, all)
      if (context.config.get (*i) != default_config.get (*i))
        default_values.push_back (*i);

    // Create a table for output.
    Table table;
    table.setTableWidth (width);
    table.setDateFormat (context.config.get ("dateformat"));
    table.addColumn ("Config variable");
    table.addColumn ("Value");

    if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
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

    Color error ("bold white on red");
    Color warning ("black on yellow");

    std::string section;

    if (args.size () == 1)
      section = args[0];

    if (section == "all")
      section = "";

    foreach (i, all)
    {
      std::string::size_type loc = i->find (section, 0);

      if (loc != std::string::npos)
      {
        int row = table.addRow ();
        table.addCell (row, 0, *i);
        table.addCell (row, 1, context.config.get (*i));

        // Look for unrecognized.
        if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        {
          if (std::find (unrecognized.begin (), unrecognized.end (), *i) != unrecognized.end ())
            table.setRowColor (row, error);

          else if (std::find (default_values.begin (), default_values.end (), *i) != default_values.end ())
            table.setRowColor (row, warning);
        }
      }
    }

    out << "\n"
        << table.render ()
        << (table.rowCount () == 0 ? "No matching configuration variables.\n\n" : "\n");

    // Display the unrecognized variables.
    if (unrecognized.size ())
    {
      out << "Your .taskrc file contains these unrecognized variables:\n";

      foreach (i, unrecognized)
        out << "  " << *i << "\n";

      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        out << "\n  These are highlighted in " << error.colorize ("color") << " above.";

      out << "\n\n";
    }

    if (default_values.size ())
    {
      out << "Some of your .taskrc variables differ from the default values.";

      if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
        out << "  These are highlighted in " << warning.colorize ("color") << " above.";
    }

    out << context.config.checkForDeprecatedColor ();
    out << context.config.checkForDeprecatedColumns ();
    // TODO Check for referenced but missing theme files.
    // TODO Check for referenced but missing string files.
    // TODO Check for referenced but missing tips files.

    // Check for referenced but missing hook scripts.
#ifdef HAVE_LIBLUA
    std::vector <std::string> missing_scripts;
    foreach (i, all)
    {
      if (i->substr (0, 5) == "hook.")
      {
        std::string value = context.config.get (*i);
        Nibbler n (value);

        // <path>:<function> [, ...]
        while (!n.depleted ())
        {
          std::string file;
          std::string function;
          if (n.getUntil (':', file) &&
              n.skip (':')           &&
              n.getUntil (',', function))
          {
            Path script (file);
            if (!script.exists () || !script.readable ())
              missing_scripts.push_back (file);

            (void) n.skip (',');
          }
        }
      }
    }

    if (missing_scripts.size ())
    {
      out << "Your .taskrc file contains these missing or unreadable hook scripts:\n";

      foreach (i, missing_scripts)
        out << "  " << *i << "\n";

      out << "\n";
    }
#endif

    // Check for bad values in rc.annotations.
    std::string annotations = context.config.get ("annotations");
    if (annotations != "full"   &&
        annotations != "sparse" &&
        annotations != "none")
      out << "Configuration error: annotations contains an unrecognized value '"
          << annotations
          << "'.\n";

    // Check for bad values in rc.calendar.details.
    std::string calendardetails = context.config.get ("calendar.details");
    if (calendardetails != "full"   &&
        calendardetails != "sparse" &&
        calendardetails != "none")
      out << "Configuration error: calendar.details contains an unrecognized value '"
          << calendardetails
          << "'.\n";

    // Check for bad values in rc.calendar.holidays.
    std::string calendarholidays = context.config.get ("calendar.holidays");
    if (calendarholidays != "full"   &&
        calendarholidays != "sparse" &&
        calendarholidays != "none")
      out << "Configuration error: calendar.holidays contains an unrecognized value '"
          << calendarholidays
          << "'.\n";

    // Check for bad values in rc.default.priority.
    std::string defaultPriority = context.config.get ("default.priority");
    if (defaultPriority != "H" &&
        defaultPriority != "M" &&
        defaultPriority != "L" &&
        defaultPriority != "")
      out << "Configuration error: default.priority contains an unrecognized value '"
          << defaultPriority
          << "'.\n";

    // Verify installation.  This is mentioned in the documentation as the way
    // to ensure everything is properly installed.

    if (all.size () == 0)
    {
      out << "Configuration error: .taskrc contains no entries.\n";
      rc = 1;
    }
    else
    {
      Directory location (context.config.get ("data.location"));

      if (location.data == "")
        out << "Configuration error: data.location not specified in .taskrc "
               "file.\n";

      if (! location.exists ())
        out << "Configuration error: data.location contains a directory name"
               " that doesn't exist, or is unreadable.\n";
    }

    outs = out.str ();
    context.hooks.trigger ("post-config-command");
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleConfig (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-config-command"))
  {
    std::stringstream out;

    // Obtain the arguments from the description.  That way, things like '--'
    // have already been handled.
    std::vector <std::string> args;
    split (args, context.task.get ("description"), ' ');

    // Support:
    //   task config name value    # set name to value
    //   task config name ""       # set name to blank
    //   task config name          # remove name
    if (args.size () > 0)
    {
      std::string name = args[0];
      std::string value = "";

      if (args.size () > 1)
      {
        for (unsigned int i = 1; i < args.size (); ++i)
        {
          if (i > 1)
            value += " ";

          value += args[i];
        }
      }

      if (name != "")
      {
        bool change = false;

        // Read .taskrc (or equivalent)
        std::string contents;
        File::read (context.config.original_file, contents);

        // task config name value
        // task config name ""
        if (args.size () > 1 ||
            context.args[context.args.size () - 1] == "")
        {
          // Find existing entry & overwrite
          std::string::size_type pos = contents.find (name + "=");
          if (pos != std::string::npos)
          {
            std::string::size_type eol = contents.find_first_of ("\r\f\n", pos);
            if (eol == std::string::npos)
              throw std::string ("Cannot find EOL after entry '") + name + "'.";

            if (confirm (std::string ("Are you sure you want to change the value of '")
                           + name
                           + "' from '"
                           + context.config.get(name)
                           + "' to '"
                           + value + "'?"))
            {
              contents = contents.substr (0, pos)
                       + name + "=" + value
                       + contents.substr (eol);
              change = true;
            }
          }

          // Not found, so append instead.
          else
          {
            if (confirm (std::string ("Are you sure you want to add '") + name + "' with a value of '" + value + "'?"))
            {
              contents = contents
                       + "\n"
                       + name + "=" + value
                       + "\n";
              change = true;
            }
          }
        }

        // task config name
        else
        {
          // Remove name
          std::string::size_type pos = contents.find (name + "=");
          if (pos == std::string::npos)
            throw std::string ("No entry named '") + name + "' found.";

          std::string::size_type eol = contents.find_first_of ("\r\f\n", pos);
          if (eol == std::string::npos)
            throw std::string ("Cannot find EOL after entry '") + name + "'.";

          if (confirm (std::string ("Are you sure you want to remove '") + name + "'?"))
          {
            contents = contents.substr (0, pos) + contents.substr (eol + 1);
            change = true;
          }
        }

        // Write .taskrc (or equivalent)
        if (change)
        {
          File::write (context.config.original_file, contents);
          out << "Config file "
              << context.config.original_file.data
              << " modified.\n";
        }
        else
          out << "No changes made.\n";
      }
      else
        throw std::string ("Specify the name of a config variable to modify.");
      outs = out.str ();
      context.hooks.trigger ("post-config-command");
    }
    else
      throw std::string ("Specify the name of a config variable to modify.");
  }
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleDelete (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-delete-command"))
  {
    std::stringstream out;

    context.disallowModification ();

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    Filter filter;
    context.tdb.loadPending (tasks, filter);

    // Filter sequence.
    std::vector <Task> all = tasks;
    context.filter.applySequence (tasks, context.sequence);

    // Determine the end date.
    char endTime[16];
    sprintf (endTime, "%u", (unsigned int) time (NULL));

    foreach (task, tasks)
    {
      if (context.hooks.trigger ("pre-delete", *task))
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
        else {
          out << "Task not deleted.\n";
          rc  = 1;
        }

        context.hooks.trigger ("post-delete", *task);
      }
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-delete-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleStart (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-start-command"))
  {
    std::stringstream out;

    context.disallowModification ();

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    Filter filter;
    context.tdb.loadPending (tasks, filter);

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

        if (context.config.getBoolean ("journal.time"))
          task->addAnnotation (context.config.get ("journal.time.start.annotation"));

        context.tdb.update (*task);

        if (context.config.getBoolean ("echo.command"))
          out << "Started "
              << task->id
              << " '"
              << task->get ("description")
              << "'.\n";
        if (!nagged)
          nagged = nag (*task);

        dependencyChainOnStart (*task);
      }
      else
      {
        out << "Task "
            << task->id
            << " '"
            << task->get ("description")
            << "' already started.\n";
        rc = 1;
      }
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-start-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleStop (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-stop-command"))
  {
    std::stringstream out;

    context.disallowModification ();

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    Filter filter;
    context.tdb.loadPending (tasks, filter);

    // Filter sequence.
    context.filter.applySequence (tasks, context.sequence);

    foreach (task, tasks)
    {
      if (task->has ("start"))
      {
        task->remove ("start");

        if (context.config.getBoolean ("journal.time"))
          task->addAnnotation (context.config.get ("journal.time.stop.annotation"));

        context.tdb.update (*task);

        if (context.config.getBoolean ("echo.command"))
          out << "Stopped "
              << task->id
              << " '"
              << task->get ("description")
              << "'.\n";
      }
      else
      {
        out << "Task "
            << task->id
            << " '"
            << task->get ("description")
            << "' not started.\n";
        rc = 1;
      }
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-stop-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleDone (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-done-command"))
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

        // Only allow valid tasks.
        task->validate ();

        if (taskDiff (before, *task))
        {
          if (context.hooks.trigger ("pre-completed", *task))
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
              context.hooks.trigger ("post-completed", *task);
            }
          }
          else
            continue;
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
            << "' is neither pending nor waiting.\n";
        rc = 1;
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
    context.hooks.trigger ("post-done-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleModify (std::string& outs)
{
  context.hooks.trigger ("pre-modify-command");

  int count = 0;
  std::stringstream out;

  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  Filter filter;
  context.tdb.loadPending (tasks, filter);

  // Filter sequence.
  std::vector <Task> all = tasks;
  context.filter.applySequence (tasks, context.sequence);

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

    // This looks unnecessarily complex, but isn't.  "due" and "wait" are
    // independent and may exist without the other, but if both exist then wait
    // must be before due.
    if ((task->has ("wait")       &&
         context.task.has ("due") &&
         Date (context.task.get ("due")) < Date (task->get ("wait")))
        ||
        (context.task.has ("wait") &&
         !context.task.has ("due") &&
         task->has ("due")         &&
         Date (task->get ("due")) < Date (context.task.get ("wait")))
        ||
        (context.task.has ("wait") &&
         context.task.has ("due")  &&
         Date (context.task.get ("due")) < Date (context.task.get ("wait")))
        ||
        (task->has ("wait") &&
         task->has ("due")  &&
         Date (task->get ("due")) < Date (task->get ("wait"))))
    {
      context.footnote ("Warning: the wait date falls after the due date.");
    }

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
  context.hooks.trigger ("post-modify-command");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int handleAppend (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-append-command"))
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

    Permission permission;
    if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
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
          changes += deltaSubstitutions (*other);

          if (taskDiff (before, *other))
          {
            // Only allow valid tasks.
            other->validate ();

            if (changes && permission.confirmed (before, taskDifferences (before, *other) + "Proceed with change?"))
            {
              context.tdb.update (*other);

              if (context.config.getBoolean ("echo.command"))
                out << "Appended '"
                    << context.task.get ("description")
                    << "' to task "
                    << other->id
                    << ".\n";

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
      out << "Appended " << count << " task" << (count == 1 ? ".\n" : "s.\n");

    outs = out.str ();
    context.hooks.trigger ("post-append-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handlePrepend (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-prepend-command"))
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

    Permission permission;
    if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
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
          changes += deltaPrepend (*other);
          changes += deltaTags (*other);
          changes += deltaAttributes (*other);
          changes += deltaSubstitutions (*other);

          if (taskDiff (before, *other))
          {
            // Only allow valid tasks.
            other->validate ();

            if (changes && permission.confirmed (before, taskDifferences (before, *other) + "Are you sure?"))
            {
              context.tdb.update (*other);

              if (context.config.getBoolean ("echo.command"))
                out << "Prepended '"
                    << context.task.get ("description")
                    << "' to task "
                    << other->id
                    << ".\n";

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
      out << "Prepended " << count << " task" << (count == 1 ? ".\n" : "s.\n");

    outs = out.str ();
    context.hooks.trigger ("post-prepend-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleDuplicate (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-duplicate-command"))
  {
    std::stringstream out;
    int count = 0;

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    Filter filter;
    context.tdb.loadPending (tasks, filter);

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
            << " was a recurring task.  The new task is not.\n";
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

      // Only allow valid tasks.
      dup.validate ();

      context.tdb.add (dup);

      if (context.config.getBoolean ("echo.command"))
        out << "Duplicated "
            << task->id
            << " '"
            << task->get ("description")
            << "'.\n";

      context.footnote (onProjectChange (dup));

      ++count;
    }

    if (tasks.size () == 0)
    {
      out << "No matches.\n";
      rc = 1;
    }
    else if (context.config.getBoolean ("echo.command"))
    {
#ifdef FEATURE_NEW_ID
      // All this, just for an id number.
      std::vector <Task> all;
      Filter none;
      context.tdb.loadPending (all, none);
      out << "Created task " << context.tdb.nextId () << ".\n";
#endif
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-duplicate-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleCount (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-count-command"))
  {
    // Scan the pending tasks, applying any filter.
    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    context.tdb.load (tasks, context.filter);
    context.tdb.commit ();
    context.tdb.unlock ();

    // Find number of matching tasks.  Skip recurring parent tasks.
    int count = 0;
    std::vector <Task>::iterator it;
    for (it = tasks.begin (); it != tasks.end (); ++it)
      if (it->getStatus () != Task::recurring)
        ++count;

    std::stringstream out;
    out << count << "\n";
    outs = out.str ();
    context.hooks.trigger ("post-count-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef FEATURE_SHELL
void handleShell ()
{
  if (context.hooks.trigger ("pre-shell-command"))
  {
    // Display some kind of welcome message.
    Color bold (Color::nocolor, Color::nocolor, false, true, false);
    std::cout << ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
                   ? bold.colorize (PACKAGE_STRING)
                   : PACKAGE_STRING)
              << " shell\n\n"
              << "Enter any task command (such as 'list'), or hit 'Enter'.\n"
              << "There is no need to include the 'task' command itself.\n"
              << "Enter 'quit' (or 'bye', 'exit') to end the session.\n\n";

    // Make a copy because context.clear will delete them.
    std::string permanentOverrides = " " + context.file_override
                                   + " " + context.var_overrides;

    std::vector <std::string> quit_commands;
    quit_commands.push_back ("quit");
    quit_commands.push_back ("exit");
    quit_commands.push_back ("bye");

    std::string command;
    bool keepGoing = true;

    do
    {
      std::string prompt = context.config.get ("shell.prompt");
      if (context.hooks.trigger ("pre-shell-prompt"))
      {
        context.hooks.trigger ("format-prompt", "prompt", prompt);
        std::cout << prompt << " ";
      }
      context.hooks.trigger ("post-shell-prompt");

      command = "";
      std::getline (std::cin, command);
      std::string decoratedCommand = trim (command + permanentOverrides);

      // When looking for the 'quit' command, use 'command', not
      // 'decoratedCommand'.
      if (std::find (quit_commands.begin (), quit_commands.end (), lowerCase (command)) != quit_commands.end ())
      {
        keepGoing = false;
      }
      else
      {
        try
        {
          context.clear ();

          std::vector <std::string> args;
          split (args, decoratedCommand, ' ');
          foreach (arg, args)    context.args.push_back (*arg);

          context.initialize ();
          context.run ();
        }

        catch (std::string& error)
        {
          std::cout << error << "\n";
        }

        catch (...)
        {
          std::cerr << context.stringtable.get (100, "Unknown error.") << "\n";
        }
      }
    }
    while (keepGoing && !std::cin.eof ());

    // No need to repeat any overrides after the shell quits.
    context.clearMessages ();
    context.hooks.trigger ("post-shell-command");
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////
int handleColor (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-color-command"))
  {
    std::stringstream out;

    if (context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor"))
    {
      // If the description contains 'legend', show all the colors currently in
      // use.
      std::string description = context.task.get ("description");
      if (description.find ("legend") != std::string::npos)
      {
        out << "\nHere are the colors currently in use:\n";

        std::vector <std::string> all;
        context.config.all (all);

        Table table;
        table.addColumn ("Color");
        table.addColumn ("Definition");

        if ((context.config.getBoolean ("color") || context.config.getBoolean ("_forcecolor")) &&
            context.config.getBoolean ("fontunderline"))
        {
          table.setColumnUnderline (0);
          table.setColumnUnderline (1);
        }
        else
          table.setTableDashedUnderline ();

        foreach (item, all)
        {
          // Skip items with 'color' in their name, that are not referring to
          // actual colors.
          if (*item != "_forcecolor" &&
              *item != "color"       &&
              item->find ("color") == 0)
          {
            int row = table.addRow ();
            table.addCell (row, 0, *item);
            table.addCell (row, 1, context.config.get (*item));
            table.setRowColor (row, context.config.get (*item));
          }
        }

        out << optionalBlankLine ()
            << table.render ()
            << optionalBlankLine ()
            << "\n";
      }

      // If there is something in the description, then assume that is a color,
      // and display it as a sample.
      else if (description != "")
      {
        Color one    ("black on bright yellow");
        Color two    ("underline cyan on bright blue");
        Color three  ("color214 on color202");
        Color four   ("rgb150 on rgb020");
        Color five   ("underline grey10 on grey3");
        Color six    ("red on color173");
        Color sample (description);

        out << "\n"
            << "Use this command to see how colors are displayed by your terminal.\n\n"
            << "\n"
            << "16-color usage (supports underline, bold text, bright background):\n"
            << "  " << one.colorize ("task color black on bright yellow")            << "\n"
            << "  " << two.colorize ("task color underline cyan on bright blue")     << "\n"
            << "\n"
            << "256-color usage (supports underline):\n"
            << "  " << three.colorize ("task color color214 on color202")            << "\n"
            << "  " << four.colorize ("task color rgb150 on rgb020")                 << "\n"
            << "  " << five.colorize ("task color underline grey10 on grey3")        << "\n"
            << "  " << six.colorize ("task color red on color173")                   << "\n"
            << "\n"
            << "Your sample:"                                                        << "\n"
            << "  " << sample.colorize ("task color " + description)                 << "\n\n";
      }

      // Show all supported colors.  Possibly show some unsupported ones too.
      else
      {
        out << "\n"
            << "Basic colors"
            << "\n"
            << " " << Color::colorize (" black ",   "black")
            << " " << Color::colorize (" red ",     "red")
            << " " << Color::colorize (" blue ",    "blue")
            << " " << Color::colorize (" green ",   "green")
            << " " << Color::colorize (" magenta ", "magenta")
            << " " << Color::colorize (" cyan ",    "cyan")
            << " " << Color::colorize (" yellow ",  "yellow")
            << " " << Color::colorize (" white ",   "white")
            << "\n"
            << " " << Color::colorize (" black ",   "white on black")
            << " " << Color::colorize (" red ",     "white on red")
            << " " << Color::colorize (" blue ",    "white on blue")
            << " " << Color::colorize (" green ",   "black on green")
            << " " << Color::colorize (" magenta ", "black on magenta")
            << " " << Color::colorize (" cyan ",    "black on cyan")
            << " " << Color::colorize (" yellow ",  "black on yellow")
            << " " << Color::colorize (" white ",   "black on white")
            << "\n\n";

        out << "Effects"
            << "\n"
            << " " << Color::colorize (" red ",               "red")
            << " " << Color::colorize (" bold red ",          "bold red")
            << " " << Color::colorize (" underline on blue ", "underline on blue")
            << " " << Color::colorize (" on green ",          "black on green")
            << " " << Color::colorize (" on bright green ",   "black on bright green")
            << "\n\n";

        // 16 system colors.
        out << "color0 - color15"
            << "\n"
            << "  0 1 2 . . .\n";
        for (int r = 0; r < 2; ++r)
        {
          out << "  ";
          for (int c = 0; c < 8; ++c)
          {
            std::stringstream s;
            s << "on color" << (r*8 + c);
            out << Color::colorize ("  ", s.str ());
          }

          out << "\n";
        }

        out << "          . . . 15\n\n";

        // Color cube.
        out << "Color cube rgb"
            << Color::colorize ("0", "bold red")
            << Color::colorize ("0", "bold green")
            << Color::colorize ("0", "bold blue")
            << " - rgb"
            << Color::colorize ("5", "bold red")
            << Color::colorize ("5", "bold green")
            << Color::colorize ("5", "bold blue")
            << " (also color16 - color231)"
            << "\n"
            << "  " << Color::colorize ("0            "
                                        "1            "
                                        "2            "
                                        "3            "
                                        "4            "
                                        "5", "bold red")
            << "\n"
            << "  " << Color::colorize ("0 1 2 3 4 5  "
                                        "0 1 2 3 4 5  "
                                        "0 1 2 3 4 5  "
                                        "0 1 2 3 4 5  "
                                        "0 1 2 3 4 5  "
                                        "0 1 2 3 4 5", "bold blue")
            << "\n";

        char label [12];
        for (int g = 0; g < 6; ++g)
        {
          sprintf (label, " %d", g);
          out << Color::colorize (label, "bold green");
          for (int r = 0; r < 6; ++r)
          {
            for (int b = 0; b < 6; ++b)
            {
              std::stringstream s;
              s << "on rgb" << r << g << b;
              out << Color::colorize ("  ", s.str ());
            }

            out << " ";
          }

          out << "\n";
        }

        out << "\n";

        // Grey ramp.
        out << "Gray ramp gray0 - gray23 (also color232 - color255)\n"
            << "  0 1 2 . . .                             . . . 23\n"
            << "  ";
        for (int g = 0; g < 24; ++g)
        {
          std::stringstream s;
          s << "on gray" << g;
          out << Color::colorize ("  ", s.str ());
        }

        out << "\n\nTry running 'task color white on red'.\n\n";
      }
    }
    else
    {
      out << "Color is currently turned off in your .taskrc file.  To enable "
             "color, remove the line 'color=off', or change the 'off' to 'on'.\n";
      rc = 1;
    }

    outs = out.str ();
    context.hooks.trigger ("post-color-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleAnnotate (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-annotate-command"))
  {
    if (!context.task.has ("description"))
      throw std::string ("Cannot apply a blank annotation.");

    if (context.sequence.size () == 0)
      throw std::string ("ID needed to apply an annotation.");

    std::stringstream out;

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    Filter filter;
    context.tdb.loadPending (tasks, filter);

    // Filter sequence.
    context.filter.applySequence (tasks, context.sequence);

    Permission permission;
    if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
      permission.bigSequence ();

    foreach (task, tasks)
    {
      Task before (*task);
      task->addAnnotation (context.task.get ("description"));

      if (taskDiff (before, *task))
      {
        // Only allow valid tasks.
        task->validate ();

        if (permission.confirmed (before, taskDifferences (before, *task) + "Proceed with change?"))
        {
          context.tdb.update (*task);

          if (context.config.getBoolean ("echo.command"))
            out << "Annotated "
                << task->id
                << " with '"
                << context.task.get ("description")
                << "'.\n";
        }
      }
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-annotate-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int handleDenotate (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-denotate-command"))
  {
    if (!context.task.has ("description"))
      throw std::string ("Description needed to delete an annotation.");

    if (context.sequence.size () == 0)
      throw std::string ("A task ID is needed to delete an annotation.");

    bool sensitive = context.config.getBoolean ("search.case.sensitive");

    std::stringstream out;

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    Filter filter;
    context.tdb.loadPending (tasks, filter);

    context.filter.applySequence (tasks, context.sequence);

    Permission permission;
    if (context.sequence.size () > (size_t) context.config.getInteger ("bulk"))
      permission.bigSequence ();

    foreach (task, tasks)
    {
      Task before (*task);
      std::string desc = context.task.get ("description");
      std::vector <Att> annotations;
      task->getAnnotations (annotations);

      if (annotations.size () == 0)
        throw std::string ("The specified task has no annotations that can be deleted.");

      std::vector <Att>::iterator i;
      std::string anno;
      bool match = false;;
      for (i = annotations.begin (); i != annotations.end (); ++i)
      {
        anno = i->value ();
        if (anno == desc)
        {
          match = true;
          annotations.erase (i);
          task->setAnnotations (annotations);
          break;
        }
      }
      if (!match)
      {
        for (i = annotations.begin (); i != annotations.end (); ++i)
        {
          anno = i->value ();
          std::string::size_type loc = find (anno, desc, sensitive);

          if (loc != std::string::npos)
          {
            match = true;
            annotations.erase (i);
            task->setAnnotations (annotations);
            break;
          }
        }
      }

      if (taskDiff (before, *task))
      {
        // Only allow valid tasks.
        task->validate ();

        if (permission.confirmed (before, taskDifferences (before, *task) + "Proceed with change?"))
        {
          context.tdb.update (*task);
          if (context.config.getBoolean ("echo.command"))
            out << "Found annotation '"
                << anno
                << "' and deleted it.\n";
        }
      }
      else
        out << "Did not find any matching annotation to be deleted for '"
            << desc
            << "'.\n";
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-denotate-command");
  }

  return rc;
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
    if (context.hooks.trigger ("pre-tag", task))
    {
      task.addTag (*tag);
      ++changes;
      context.hooks.trigger ("post-tag", task);
    }
  }

  foreach (tag, context.tagRemovals)
  {
    if (context.hooks.trigger ("pre-detag", task))
    {
      task.removeTag (*tag);
      ++changes;
      context.hooks.trigger ("post-detag", task);
    }
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
