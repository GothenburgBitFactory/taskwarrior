////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
        char epoch [16];
        sprintf (epoch, "%d", (int)dt.toEpoch ());
        return std::string (epoch);
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
  before << "# The 'task edit <id>' command allows you to modify all aspects of a task" << std::endl
         << "# using a text editor.  What is shown below is a representation of the"    << std::endl
         << "# task in all it's detail.  Modify what you wish, and if you save and"     << std::endl
         << "# quit your editor, task will read this file and try to make sense of"     << std::endl
         << "# what changed, and apply those changes.  If you quit your editor without" << std::endl
         << "# saving or making any modifications, task will do nothing."               << std::endl
         << "#"                                                                         << std::endl
         << "# Lines that begin with # represent data you cannot change, like ID."      << std::endl
         << "# If you get too 'creative' with your editing, task will dump you back "   << std::endl
         << "# into the editor to try again."                                           << std::endl
         << "#"                                                                         << std::endl
         << "# Should you find yourself in an endless Groundhog Day loop, editing and"  << std::endl
         << "# editing the same file, just quit the editor without making any changes." << std::endl
         << "# Task will notice this and stop the editing."                             << std::endl
         << "#"                                                                         << std::endl
         << "# Name               Editable details"                                     << std::endl
         << "# -----------------  ----------------------------------------------------" << std::endl
         << "# ID:                " << task.id                                          << std::endl
         << "# UUID:              " << task.get ("uuid")                                << std::endl
         << "# Status:            " << ucFirst (Task::statusToText (task.getStatus ())) << std::endl
         << "# Mask:              " << task.get ("mask")                                << std::endl
         << "# iMask:             " << task.get ("imask")                               << std::endl
         << "  Project:           " << task.get ("project")                             << std::endl
         << "  Priority:          " << task.get ("priority")                            << std::endl;

  std::vector <std::string> tags;
  task.getTags (tags);
  std::string allTags;
  join (allTags, " ", tags);
  before << "# Separate the tags with spaces, like this: tag1 tag2"                     << std::endl
         << "  Tags:              " << allTags                                          << std::endl
         << "# The description field is allowed to wrap and use multiple lines.  Task"  << std::endl
         << "# will combine them."                                                      << std::endl
         << "  Description:       " << task.get ("description")                         << std::endl
         << "  Created:           " << formatDate (task, "entry")                       << std::endl
         << "  Started:           " << formatDate (task, "start")                       << std::endl
         << "  Ended:             " << formatDate (task, "end")                         << std::endl
         << "  Due:               " << formatDate (task, "due")                         << std::endl
         << "  Until:             " << formatDate (task, "until")                       << std::endl
         << "  Recur:             " << task.get ("recur")                               << std::endl
         << "  Wait until:        " << formatDate (task, "wait")                        << std::endl
         << "  Parent:            " << task.get ("parent")                              << std::endl
         << "  Foreground color:  " << task.get ("fg")                                  << std::endl
         << "  Background color:  " << task.get ("bg")                                  << std::endl
         << "# Annotations look like this: <date> -- <text> and there can be any number of them"  << std::endl
         << "# ' -- ' is the separator between the date and text field. It should not be removed" << std::endl;

  std::vector <Att> annotations;
  task.getAnnotations (annotations);
  foreach (anno, annotations)
  {
    Date dt (::atoi (anno->name ().substr (11).c_str ()));
    before << "  Annotation:        " << dt.toString (context.config.get ("dateformat.annotation"))
           << " -- "                  << anno->value ()                                 << std::endl;
  }

  Date now;
  before << "  Annotation:        " << now.toString (context.config.get ("dateformat.annotation")) << " -- " << std::endl
         << "# End"                                                                     << std::endl;

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
      std::cout << "Project modified." << std::endl;
      task.set ("project", value);
    }
    else
    {
      std::cout << "Project deleted." << std::endl;
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
        std::cout << "Priority modified." << std::endl;
        task.set ("priority", value);
      }
    }
    else
    {
      std::cout << "Priority deleted." << std::endl;
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
      std::cout << "Description modified." << std::endl;
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
      std::cout << "Creation date modified." << std::endl;
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
        std::cout << "Start date modified." << std::endl;
        task.set ("start", value);
      }
    }
    else
    {
      std::cout << "Start date modified." << std::endl;
      task.set ("start", value);
    }
  }
  else
  {
    if (task.get ("start") != "")
    {
      std::cout << "Start date removed." << std::endl;
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
        std::cout << "Done date modified." << std::endl;
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
      std::cout << "Done date removed." << std::endl;
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
        std::cout << "Due date modified." << std::endl;
        task.set ("due", value);
      }
    }
    else
    {
      std::cout << "Due date modified." << std::endl;
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
        std::cout << "Cannot remove a due date from a recurring task." << std::endl;
      }
      else
      {
        std::cout << "Due date removed." << std::endl;
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
        std::cout << "Until date modified." << std::endl;
        task.set ("until", value);
      }
    }
    else
    {
      std::cout << "Until date modified." << std::endl;
      task.set ("until", value);
    }
  }
  else
  {
    if (task.get ("until") != "")
    {
      std::cout << "Until date removed." << std::endl;
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
        std::cout << "Recurrence modified." << std::endl;
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
      std::cout << "Recurrence removed." << std::endl;
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
        std::cout << "Wait date modified." << std::endl;
        task.set ("wait", value);
      }
    }
    else
    {
      std::cout << "Wait date modified." << std::endl;
      task.set ("wait", value);
    }
  }
  else
  {
    if (task.get ("wait") != "")
    {
      std::cout << "Wait date removed." << std::endl;
      task.remove ("wait");
    }
  }

  // parent
  value = findValue (after, "Parent:");
  if (value != task.get ("parent"))
  {
    if (value != "")
    {
      std::cout << "Parent UUID modified." << std::endl;
      task.set ("parent", value);
    }
    else
    {
      std::cout << "Parent UUID removed." << std::endl;
      task.remove ("parent");
    }
  }

  // fg
  value = findValue (after, "Foreground color:");
  if (value != task.get ("fg"))
  {
    if (value != "")
    {
      std::cout << "Foreground color modified." << std::endl;
      task.set ("fg", value);
    }
    else
    {
      std::cout << "Foreground color removed." << std::endl;
      task.remove ("fg");
    }
  }

  // bg
  value = findValue (after, "Background color:");
  if (value != task.get ("bg"))
  {
    if (value != "")
    {
      std::cout << "Background color modified." << std::endl;
      task.set ("bg", value);
    }
    else
    {
      std::cout << "Background color removed." << std::endl;
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
  std::cout << "Launching '" << editor << "' now..." << std::endl;
  if (-1 == system (editor.c_str ()))
    std::cout << "No editing performed." << std::endl;
  else
    std::cout << "Editing complete." << std::endl;

  // Slurp file.
  std::string after;
  File::read (file.str (), after);

  // Update task based on what can be parsed back out of the file, but only
  // if changes were made.
  if (before != after)
  {
    std::cout << "Edits were detected." << std::endl;
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
      std::cout << "Error: " << problem << std::endl;

      // Preserve the edits.
      before = after;
      File::write (file.str (), before);

      if (confirm ("Task couldn't handle your edits.  Would you like to try again?"))
        goto ARE_THESE_REALLY_HARMFUL;
    }
  }
  else
    std::cout << "No edits were detected." << std::endl;

  // Cleanup.
  File::remove (file.str ());
}

////////////////////////////////////////////////////////////////////////////////
// Introducing the Silver Bullet.  This feature is the catch-all fixative for
// various other ills.  This is like opening up the hood and going in with a
// wrench.  To be used sparingly.
int handleEdit (std::string &outs)
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

    foreach (task, tasks)
    {
      editFile (*task);
      context.tdb.update (*task);
/*
      foreach (other, all)
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
