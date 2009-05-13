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
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "task.h"

////////////////////////////////////////////////////////////////////////////////
static std::string findSimpleValue (
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

      std::cout << "value '" << value << "'" << std::endl;
      return trim (value, "\t ");
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
static time_t findSimpleDate (
  Config& conf,
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

      std::cout << "value '" << value << "'" << std::endl;
      Date dt (trim (value, "\t "), conf.get ("dateformat", "m/d/Y"));
      return dt.toEpoch ();
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
static std::string formatStatus (T& task)
{
  switch (task.getStatus ())
  {
  case T::pending:   return "Pending";   break;
  case T::completed: return "Completed"; break;
  case T::deleted:   return "Deleted";   break;
  case T::recurring: return "Recurring"; break;
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
static std::string formatDate (
  Config& conf,
  T& task,
  const std::string& attribute)
{
  std::string value = task.getAttribute (attribute);
  if (value.length ())
  {
    Date dt (::atoi (value.c_str ()));
    value = dt.toString (conf.get ("dateformat", "m/d/Y"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
static std::string formatTask (Config& conf, T task)
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
         << "# ID:                " << task.getId ()                                    << std::endl
         << "# UUID:              " << task.getUUID ()                                  << std::endl
         << "# Status:            " << formatStatus (task)                              << std::endl
         << "# Mask:              " << task.getAttribute ("mask")                       << std::endl
         << "# iMask:             " << task.getAttribute ("imask")                      << std::endl
         << "  Project:           " << task.getAttribute ("project")                    << std::endl
         << "  Priority:          " << task.getAttribute ("priority")                   << std::endl;

  std::vector <std::string> tags;
  task.getTags (tags);
  std::string allTags;
  join (allTags, " ", tags);
  before << "# Separate the tags with spaces, like this:   tag1 tag2"                   << std::endl
         << "  Tags:              " << allTags                                          << std::endl
         << "# The description field is allowed to wrap and use multiple lines.  Task"  << std::endl
         << "# will combine them."                                                      << std::endl
         << "  Description:       " << task.getDescription ()                           << std::endl
         << "  Created:           " << formatDate (conf, task, "entry")                 << std::endl
         << "  Started:           " << formatDate (conf, task, "start")                 << std::endl
         << "  Ended:             " << formatDate (conf, task, "end")                   << std::endl
         << "  Due:               " << formatDate (conf, task, "due")                   << std::endl
         << "  Until:             " << formatDate (conf, task, "until")                 << std::endl
         << "  Recur:             " << task.getAttribute ("recur")                      << std::endl
         << "  Foreground color:  " << task.getAttribute ("fg")                         << std::endl
         << "  Background color:  " << task.getAttribute ("bg")                         << std::endl
         << "# Annotations look like this: <date> <text>, and there can be any number"  << std::endl
         << "# of them."                                                                << std::endl;

  std::map <time_t, std::string> annotations;
  task.getAnnotations (annotations);
  foreach (anno, annotations)
    before << "  Annotation:        " << anno->first << " " << anno->second             << std::endl;

  before << "  Annotation:        "                                                     << std::endl
         << "  Annotation:        "                                                     << std::endl
         << "# End"                                                                     << std::endl;
  return before.str ();
}

////////////////////////////////////////////////////////////////////////////////
static void parseTask (Config& conf, T& task, const std::string& after)
{
  std::string value;

  // Project
  value = findSimpleValue (after, "Project:");
  if (value != "" &&
     task.getAttribute ("project") != value)
  {
    std::cout << "Project modified." << std::endl;
    task.setAttribute ("project", value);
  }

  // Priority
  value = findSimpleValue (after, "Priority:");
  if (value != "" &&
      validPriority (value) &&
      task.getAttribute ("priority") != value)
  {
    std::cout << "Priority modified." << std::endl;
    task.setAttribute ("priority", value);
  }

  // Tags
/*
  value = findSimpleValue (after, "tags");
  std::vector <std::string> tags;
  split (tags, " ", value);
*/

  // TODO Disallow blank description.
//  value = findMultilineValue (after, "Description: ");
//  if (value != "")
//  {
//    std::cout << "Description modified." << std::endl;
//    task.setDescription (value);
//  }

  // start
  time_t dt = findSimpleDate (conf, after, "Created:");
  if (dt != 0)
  {
    std::cout << "Creation date modified." << std::endl;
    char epoch[16];
    sprintf (epoch, "%d", (int) dt);
    task.setAttribute ("entry", epoch);
  }
  else
    std::cout << "Cannot remove creation date" << std::endl;

  // end
  dt = findSimpleDate (conf, after, "Ended:");
  if (dt != 0)
  {
    std::cout << "Done date modified." << std::endl;
    char epoch[16];
    sprintf (epoch, "%d", (int) dt);
    task.setAttribute ("end", epoch);
  }
  else
  {
    std::cout << "Done date removed." << std::endl;
    task.removeAttribute ("end");
  }

  // due
  dt = findSimpleDate (conf, after, "Due:");
  if (dt != 0)
  {
    std::cout << "Due date modified." << std::endl;
    char epoch[16];
    sprintf (epoch, "%d", (int) dt);
    task.setAttribute ("due", epoch);
  }
  else
  {
    if (task.getStatus () == T::recurring ||
        task.getAttribute ("parent") != "")
    {
      std::cout << "Cannot remove a due date from a recurring task." << std::endl;
    }
    else
    {
      std::cout << "Due date removed." << std::endl;
      task.removeAttribute ("due");
    }
  }

  // until
  dt = findSimpleDate (conf, after, "Until:");
  if (dt != 0)
  {
    std::cout << "Until date modified." << std::endl;
    char epoch[16];
    sprintf (epoch, "%d", (int) dt);
    task.setAttribute ("until", epoch);
  }
  else
  {
    std::cout << "Until date removed." << std::endl;
    task.removeAttribute ("until");
  }

  // TODO recur
  // TODO parent
  // TODO mask
  // TODO imask
  // TODO fg
  // TODO bg
  // TODO Annotations
}

////////////////////////////////////////////////////////////////////////////////
// Introducing the Silver Bullet.  This feature is the catch-all fixative for
// various other ills.  This is like opening up the hood and going in with a
// wrench.  To be used sparingly.
std::string handleEdit (TDB& tdb, T& task, Config& conf)
{
  std::stringstream out;
  std::vector <T> all;
  tdb.pendingT (all);

  filterSequence (all, task);
  foreach (seq, all)
  {
    // Check for file permissions.
    std::string dataLocation = expandPath (conf.get ("data.location"));
    if (access (dataLocation.c_str (), X_OK))
      throw std::string ("Your data.location directory is not writable.");

    // Create a temp file name in data.location.
    std::stringstream pattern;
    pattern << dataLocation << "/task." << seq->getId () << ".XXXXXX";
    char cpattern [PATH_MAX];
    strcpy (cpattern, pattern.str ().c_str ());
    char* file = mktemp (cpattern);

    // Format the contents, T -> text, write to a file.
    std::string before = formatTask (conf, *seq);
    spit (file, before);

    // Determine correct editor: .taskrc:editor > $VISUAL > $EDITOR > vi
    std::string editor = conf.get ("editor", "");
    if (editor == "") editor = getenv ("VISUAL");
    if (editor == "") editor = getenv ("EDITOR");
    if (editor == "") editor = "vi";

ARE_THESE_REALLY_HARMFUL:
    // Launch the editor.
    std::cout << "Launching '" << editor << "' now..." << std::endl;
    editor += " ";
    editor += file;
    system (editor.c_str ());
    std::cout << "Editing complete." << std::endl;

    // Slurp file.
    std::string after;
    slurp (file, after, false);

    // TODO Remove this debugging code.
    //spit ("./before", before);
    //spit ("./after",  after);

    // Update seq based on what can be parsed back out of the file, but only
    // if changes were made.
    if (before != after)
    {
      std::cout << "Edits were detected." << std::endl;
      std::string problem = "";
      bool oops = false;

      try
      {
        parseTask (conf, *seq, after);
        tdb.modifyT (*seq);
      }

      catch (std::string& e)
      {
        problem = e;
        oops = true;
      }

      if (oops)
      {
        std::cout << problem << std::endl;

        // Preserve the edits.
        before = after;
        spit (file, before);

        if (confirm ("Task couldn't handle your edits.  Would you like to try again?"))
          goto ARE_THESE_REALLY_HARMFUL;
      }
    }
    else
      std::cout << "No edits were detected." << std::endl;

    // Cleanup.
    unlink (file);
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
