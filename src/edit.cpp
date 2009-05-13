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
static std::string findSimpleValue (const std::string& text, const std::string& name)
{
  // Look for /^\s+name:\s+(.*)$/
  // Extract
  // Trim
  // Join
  return "";
}

////////////////////////////////////////////////////////////////////////////////
static std::string formatTask (T task)
{
  std::stringstream before;
  before << "# The 'task edit <id>' command allows you to modify all aspects of a task" << std::endl
         << "# using a text editor.  What is shown below is a representation of the"    << std::endl
         << "# task in all it's detail.  Modify what you wish, and if you save and"     << std::endl
         << "# quit your editor, task will read this file and try to make sense of"     << std::endl
         << "# what changed, and apply those changes.  If you quit your editor without" << std::endl
         << "# saving or making any modifications, task will do nothing."               << std::endl
         << "#"                                                                         << std::endl
         << "# Lines that begin with # are comments, and will be ignored by task."      << std::endl
         << "# All other edits will be ignored."                                        << std::endl
         << "#"                                                                         << std::endl
         << "# Things you probably want to edit"                                        << std::endl
         << "  ID:          " << task.getId ()                                          << std::endl
         << "  Status:      " << task.getStatus ()                                      << std::endl
         << "  Project:     " << task.getAttribute ("project")                          << std::endl
         << "  Priority:    " << task.getAttribute ("priority")                         << std::endl;

  std::vector <std::string> tags;
  task.getTags (tags);
  std::string allTags;
  join (allTags, " ", tags);
  before << "  Tags:        " << allTags                                                << std::endl;

  std::map <time_t, std::string> annotations;
  task.getAnnotations (annotations);
  foreach (anno, annotations)
    before << "  Annotation:  " << anno->first << " " << anno->second                   << std::endl;

  before << "  Description: " << task.getDescription ()                                 << std::endl
         << "#"                                                                         << std::endl
         << "# Things you should be very careful about editing"                         << std::endl
         << "  Start:       " << task.getAttribute ("start")                            << std::endl
         << "  End:         " << task.getAttribute ("end")                              << std::endl
         << "  Due:         " << task.getAttribute ("due")                              << std::endl
         << "#"                                                                         << std::endl
         << "# Things you should not edit, but can"                                     << std::endl
         << "  Recur:       " << task.getAttribute ("recur")                            << std::endl
         << "  Mask:        " << task.getAttribute ("mask")                             << std::endl
         << "  iMask:       " << task.getAttribute ("imask")                            << std::endl;

  return before.str ();
}

////////////////////////////////////////////////////////////////////////////////
static void parseTask (T& task, const std::string& before, const std::string& after)
{
  task.setAttribute   ("Project",  findSimpleValue (after, "Project"));
  task.setAttribute   ("Priority", findSimpleValue (after, "Priority"));
  task.setDescription (            findSimpleValue (after, "Description"));
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
    std::string before = formatTask (*seq);
    spit (file, before);

    // Determine correct editor: .taskrc:editor > $VISUAL > $EDITOR > vi
    std::string editor = conf.get ("editor", "");
    if (editor == "") editor = getenv ("VISUAL");
    if (editor == "") editor = getenv ("EDITOR");
    if (editor == "") editor = "vi";

    // Launch the editor.
    editor += " ";
    editor += file;
    system (editor.c_str ());

    // Slurp file.
    std::string after;
    slurp (file, after, true);

    // Update seq based on what can be parsed back out of the file.
    parseTask (*seq, before, after);

    // Modify task.
    tdb.modifyT (*seq);

    // Cleanup.
    unlink (file);
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
