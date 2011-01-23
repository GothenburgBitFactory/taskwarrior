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
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "Directory.h"
#include "File.h"
#include "Date.h"
#include "Duration.h"
#include "text.h"
#include "util.h"
#include "main.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
static std::string findValue (
  const std::string& text,
  const std::string& name)
{
  std::string::size_type found = text.find (name);
  if (found != std::string::npos)
  {
    std::string::size_type eol = text.find ("\n", found);
    if (eol != std::string::npos)
    {
      std::string value = text.substr (
        found + name.length (),
        eol - (found + name.length ()));

      return trim (value, "\t ");
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
static std::string findDate (
  const std::string& text,
  const std::string& name)
{
  std::string::size_type found = text.find (name);
  if (found != std::string::npos)
  {
    std::string::size_type eol = text.find ("\n", found);
    if (eol != std::string::npos)
    {
      std::string value = trim (text.substr (
        found + name.length (),
        eol - (found + name.length ())), "\t ");

      if (value != "")
      {
        Date dt (value, context.config.get ("dateformat"));
        return format ((int) dt.toEpoch ());
      }
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
static std::string formatDate (
  Task& task,
  const std::string& attribute)
{
  std::string value = task.get (attribute);
  if (value.length ())
  {
    Date dt (::atoi (value.c_str ()));
    value = dt.toString (context.config.get ("dateformat"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
static std::string formatTask (Task task)
{
  std::stringstream before;
  bool verbose = context.config.getBoolean ("edit.verbose");

  if (verbose)
    before << "# The 'task edit <id>' command allows you to modify all aspects of a task\n"
           << "# using a text editor.  What is shown below is a representation of the\n"
           << "# task in all it's detail.  Modify what you wish, and if you save and\n"
           << "# quit your editor, taskwarrior will read this file and try to make sense\n"
           << "# of what changed, and apply those changes.  If you quit your editor\n"
           << "# without saving or making any modifications, taskwarrior will do nothing.\n"
           << "#\n"
           << "# Lines that begin with # represent data you cannot change, like ID.\n"
           << "# If you get too 'creative' with your editing, taskwarrior will dump you\n"
           << "# back into the editor to try again.\n"
           << "#\n"
           << "# Should you find yourself in an endless Groundhog Day loop, editing and\n"
           << "# editing the same file, just quit the editor without making any changes.\n"
           << "# Taskwarrior will notice this and stop the editing.\n"
           << "#\n";

  before << "# Name               Editable details\n"
         << "# -----------------  ----------------------------------------------------\n"
         << "# ID:                " << task.id                                          << "\n"
         << "# UUID:              " << task.get ("uuid")                                << "\n"
         << "# Status:            " << ucFirst (Task::statusToText (task.getStatus ())) << "\n"
         << "# Mask:              " << task.get ("mask")                                << "\n"
         << "# iMask:             " << task.get ("imask")                               << "\n"
         << "  Project:           " << task.get ("project")                             << "\n"
         << "  Priority:          " << task.get ("priority")                            << "\n";

  std::vector <std::string> tags;
  task.getTags (tags);
  std::string allTags;
  join (allTags, " ", tags);

  if (verbose)
    before << "# Separate the tags with spaces, like this: tag1 tag2\n";

  before   << "  Tags:              " << allTags                                        << "\n";

  if (verbose)
    before << "# The description field is allowed to wrap and use multiple lines.  Task\n"
           << "# will combine them.\n";

  before << "  Description:       " << task.get ("description")                         << "\n"
         << "  Created:           " << formatDate (task, "entry")                       << "\n"
         << "  Started:           " << formatDate (task, "start")                       << "\n"
         << "  Ended:             " << formatDate (task, "end")                         << "\n"
         << "  Due:               " << formatDate (task, "due")                         << "\n"
         << "  Until:             " << formatDate (task, "until")                       << "\n"
         << "  Recur:             " << task.get ("recur")                               << "\n"
         << "  Wait until:        " << formatDate (task, "wait")                        << "\n"
         << "  Parent:            " << task.get ("parent")                              << "\n"
         << "  Foreground color:  " << task.get ("fg")                                  << "\n"
         << "  Background color:  " << task.get ("bg")                                  << "\n";

  if (verbose)
    before << "# Annotations look like this: <date> -- <text> and there can be any number of them.\n"
           << "# The ' -- ' separator between the date and text field should not be removed.\n"
           << "# A \"blank slot\" for adding an annotation follows for your convenience.\n";

  std::vector <Att> annotations;
  task.getAnnotations (annotations);
  std::vector <Att>::iterator anno;
  for (anno = annotations.begin (); anno != annotations.end (); ++anno)
  {
    Date dt (::atoi (anno->name ().substr (11).c_str ()));
    before << "  Annotation:        " << dt.toString (context.config.get ("dateformat.annotation"))
           << " -- "                  << anno->value ()                                 << "\n";
  }

  Date now;
  before << "  Annotation:        " << now.toString (context.config.get ("dateformat.annotation")) << " -- \n";

  // Add dependencies here.
  std::vector <int> dependencies;
  task.getDependencies (dependencies);
  std::string allDeps;
  join (allDeps, ",", dependencies);

  if (verbose)
    before << "# Dependencies should be a comma-separated list of task IDs, with no spaces.\n";

  before << "  Dependencies:      " << allDeps << "\n";

  before << "# End\n";
  return before.str ();
}

////////////////////////////////////////////////////////////////////////////////
static void parseTask (Task& task, const std::string& after)
{
  // project
  std::string value = findValue (after, "Project:");
  if (task.get ("project") != value)
  {
    if (value != "")
    {
      std::cout << "Project modified.\n";
      task.set ("project", value);
    }
    else
    {
      std::cout << "Project deleted.\n";
      task.remove ("project");
    }
  }

  // priority
  value = findValue (after, "Priority:");
  if (task.get ("priority") != value)
  {
    if (value != "")
    {
      if (Att::validNameValue ("priority", "", value))
      {
        std::cout << "Priority modified.\n";
        task.set ("priority", value);
      }
    }
    else
    {
      std::cout << "Priority deleted.\n";
      task.remove ("priority");
    }
  }

  // tags
  value = findValue (after, "Tags:");
  std::vector <std::string> tags;
  split (tags, value, ' ');
  task.remove ("tags");
  task.addTags (tags);

  // description.
  value = findValue (after, "Description: ");
  if (task.get ("description") != value)
  {
    if (value != "")
    {
      std::cout << "Description modified.\n";
      task.set ("description", value);
    }
    else
      throw std::string ("Cannot remove description.");
  }

  // entry
  value = findDate (after, "Created:");
  if (value != "")
  {
    Date edited (::atoi (value.c_str ()));

    Date original (::atoi (task.get ("entry").c_str ()));
    if (!original.sameDay (edited))
    {
      std::cout << "Creation date modified.\n";
      task.set ("entry", value);
    }
  }
  else
    throw std::string ("Cannot remove creation date.");

  // start
  value = findDate (after, "Started:");
  if (value != "")
  {
    Date edited (::atoi (value.c_str ()));

    if (task.get ("start") != "")
    {
      Date original (::atoi (task.get ("start").c_str ()));
      if (!original.sameDay (edited))
      {
        std::cout << "Start date modified.\n";
        task.set ("start", value);
      }
    }
    else
    {
      std::cout << "Start date modified.\n";
      task.set ("start", value);
    }
  }
  else
  {
    if (task.get ("start") != "")
    {
      std::cout << "Start date removed.\n";
      task.remove ("start");
    }
  }

  // end
  value = findDate (after, "Ended:");
  if (value != "")
  {
    Date edited (::atoi (value.c_str ()));

    if (task.get ("end") != "")
    {
      Date original (::atoi (task.get ("end").c_str ()));
      if (!original.sameDay (edited))
      {
        std::cout << "Done date modified.\n";
        task.set ("end", value);
      }
    }
    else if (task.getStatus () != Task::deleted)
      throw std::string ("Cannot set a done date on a pending task.");
  }
  else
  {
    if (task.get ("end") != "")
    {
      std::cout << "Done date removed.\n";
      task.setStatus (Task::pending);
      task.remove ("end");
    }
  }

  // due
  value = findDate (after, "Due:");
  if (value != "")
  {
    Date edited (::atoi (value.c_str ()));

    if (task.get ("due") != "")
    {
      Date original (::atoi (task.get ("due").c_str ()));
      if (!original.sameDay (edited))
      {
        std::cout << "Due date modified.\n";
        task.set ("due", value);
      }
    }
    else
    {
      std::cout << "Due date modified.\n";
      task.set ("due", value);
    }
  }
  else
  {
    if (task.get ("due") != "")
    {
      if (task.getStatus () == Task::recurring ||
          task.get ("parent") != "")
      {
        std::cout << "Cannot remove a due date from a recurring task.\n";
      }
      else
      {
        std::cout << "Due date removed.\n";
        task.remove ("due");
      }
    }
  }

  // until
  value = findDate (after, "Until:");
  if (value != "")
  {
    Date edited (::atoi (value.c_str ()));

    if (task.get ("until") != "")
    {
      Date original (::atoi (task.get ("until").c_str ()));
      if (!original.sameDay (edited))
      {
        std::cout << "Until date modified.\n";
        task.set ("until", value);
      }
    }
    else
    {
      std::cout << "Until date modified.\n";
      task.set ("until", value);
    }
  }
  else
  {
    if (task.get ("until") != "")
    {
      std::cout << "Until date removed.\n";
      task.remove ("until");
    }
  }

  // recur
  value = findValue (after, "Recur:");
  if (value != task.get ("recur"))
  {
    if (value != "")
    {
      Duration d;
      if (d.valid (value))
      {
        std::cout << "Recurrence modified.\n";
        if (task.get ("due") != "")
        {
          task.set ("recur", value);
          task.setStatus (Task::recurring);
        }
        else
          throw std::string ("A recurring task must have a due date.");
      }
      else
        throw std::string ("Not a valid recurrence duration.");
    }
    else
    {
      std::cout << "Recurrence removed.\n";
      task.setStatus (Task::pending);
      task.remove ("recur");
      task.remove ("until");
      task.remove ("mask");
      task.remove ("imask");
    }
  }

  // wait
  value = findDate (after, "Wait until:");
  if (value != "")
  {
    Date edited (::atoi (value.c_str ()));

    if (task.get ("wait") != "")
    {
      Date original (::atoi (task.get ("wait").c_str ()));
      if (!original.sameDay (edited))
      {
        std::cout << "Wait date modified.\n";
        task.set ("wait", value);
        task.setStatus (Task::waiting);
      }
    }
    else
    {
      std::cout << "Wait date modified.\n";
      task.set ("wait", value);
      task.setStatus (Task::waiting);
    }
  }
  else
  {
    if (task.get ("wait") != "")
    {
      std::cout << "Wait date removed.\n";
      task.remove ("wait");
      task.setStatus (Task::pending);
    }
  }

  // parent
  value = findValue (after, "Parent:");
  if (value != task.get ("parent"))
  {
    if (value != "")
    {
      std::cout << "Parent UUID modified.\n";
      task.set ("parent", value);
    }
    else
    {
      std::cout << "Parent UUID removed.\n";
      task.remove ("parent");
    }
  }

  // fg
  value = findValue (after, "Foreground color:");
  if (value != task.get ("fg"))
  {
    if (value != "")
    {
      std::cout << "Foreground color modified.\n";
      task.set ("fg", value);
    }
    else
    {
      std::cout << "Foreground color removed.\n";
      task.remove ("fg");
    }
  }

  // bg
  value = findValue (after, "Background color:");
  if (value != task.get ("bg"))
  {
    if (value != "")
    {
      std::cout << "Background color modified.\n";
      task.set ("bg", value);
    }
    else
    {
      std::cout << "Background color removed.\n";
      task.remove ("bg");
    }
  }

  // Annotations
  std::vector <Att> annotations;
  std::string::size_type found = 0;
  while ((found = after.find ("Annotation:", found)) != std::string::npos)
  {
    found += 11;

    std::string::size_type eol = after.find ("\n", found);
    if (eol != std::string::npos)
    {
      std::string value = trim (after.substr (
        found,
        eol - found), "\t ");

      std::string::size_type gap = value.find (" -- ");
      if (gap != std::string::npos)
      {
        Date when (value.substr (0, gap), context.config.get ("dateformat.annotation"));

        // This guarantees that if more than one annotation has the same date,
        // that the seconds will be different, thus unique, thus not squashed.
        // Bug #249
        when += (const int) annotations.size ();

        std::stringstream name;
        name << "annotation_" << when.toEpoch ();
        std::string text = trim (value.substr (gap + 4), "\t ");
        annotations.push_back (Att (name.str (), text));
      }
    }
  }

  task.setAnnotations (annotations);

  // Dependencies
  value = findValue (after, "Dependencies:");
  std::vector <std::string> dependencies;
  split (dependencies, value, ",");

  task.remove ("depends");
  std::vector <std::string>::iterator dep;
  for (dep = dependencies.begin (); dep != dependencies.end (); ++dep)
  {
    int id = atoi (dep->c_str ());
    if (id)
      task.addDependency (id);
  }
}

////////////////////////////////////////////////////////////////////////////////
void editFile (Task& task)
{
  // Check for file permissions.
  Directory location (context.config.get ("data.location"));
  if (! location.writable ())
    throw std::string ("Your data.location directory is not writable.");

  // Create a temp file name in data.location.
  std::stringstream file;
  file << "task." << getpid () << "." << task.id << ".task";
  std::string path = location.data + "/" + file.str ();

  // Format the contents, T -> text, write to a file.
  std::string before = formatTask (task);
  int ignored = chdir (location.data.c_str ());
  ++ignored; // Keep compiler quiet.
  File::write (file.str (), before);

  // Determine correct editor: .taskrc:editor > $VISUAL > $EDITOR > vi
  std::string editor = context.config.get ("editor");
  char* peditor = getenv ("VISUAL");
  if (editor == "" && peditor) editor = std::string (peditor);
  peditor = getenv ("EDITOR");
  if (editor == "" && peditor) editor = std::string (peditor);
  if (editor == "") editor = "vi";

  // Complete the command line.
  editor += " ";
  editor += "\"" + file.str () + "\"";

ARE_THESE_REALLY_HARMFUL:
  // Launch the editor.
  std::cout << "Launching '" << editor << "' now...\n";
  if (-1 == system (editor.c_str ()))
    std::cout << "No editing performed.\n";
  else
    std::cout << "Editing complete.\n";

  // Slurp file.
  std::string after;
  File::read (file.str (), after);

  // Update task based on what can be parsed back out of the file, but only
  // if changes were made.
  if (before != after)
  {
    std::cout << "Edits were detected.\n";
    std::string problem = "";
    bool oops = false;

    try
    {
      parseTask (task, after);
    }

    catch (std::string& e)
    {
      problem = e;
      oops = true;
    }

    if (oops)
    {
      std::cout << "Error: " << problem << "\n";

      // Preserve the edits.
      before = after;
      File::write (file.str (), before);

      if (confirm ("Taskwarrior couldn't handle your edits.  Would you like to try again?"))
        goto ARE_THESE_REALLY_HARMFUL;
    }
  }
  else
    std::cout << "No edits were detected.\n";

  // Cleanup.
  File::remove (file.str ());
}

////////////////////////////////////////////////////////////////////////////////
// Introducing the Silver Bullet.  This feature is the catch-all fixative for
// various other ills.  This is like opening up the hood and going in with a
// wrench.  To be used sparingly.
int handleEdit (std::string& outs)
{
  int rc = 0;

  if (context.hooks.trigger ("pre-edit-command"))
  {
    std::stringstream out;

    std::vector <Task> tasks;
    context.tdb.lock (context.config.getBoolean ("locking"));
    handleRecurrence ();
    Filter filter;
    context.tdb.loadPending (tasks, filter);

    // Filter sequence.
    std::vector <Task> all = tasks;
    context.filter.applySequence (tasks, context.sequence);

    std::vector <Task>::iterator task;
    for (task = tasks.begin (); task != tasks.end (); ++task)
    {
      editFile (*task);
      context.tdb.update (*task);
/*
      TODO Figure out what this is.  I can't remember, but don't want to remove
           it until I do.

      std::vector <Task>::iterator other;
      for (other = all.begin (); other != all.end (); ++other)
      {
        if (other->id != task.id) // Don't edit the same task again.
        {
          if (task.has ("parent") && 
          if (other is parent of task)
          {
            // Transfer everything but mask, imask, uuid, parent.
          }
          else if (task is parent of other)
          {
            // Transfer everything but mask, imask, uuid, parent.
          }
          else if (task and other are siblings)
          {
            // Transfer everything but mask, imask, uuid, parent.
          }
        }
      }
*/
    }

    context.tdb.commit ();
    context.tdb.unlock ();

    outs = out.str ();
    context.hooks.trigger ("post-edit-command");
  }

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
