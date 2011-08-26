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
#include <stdlib.h>
#include <Duration.h>
#include <Context.h>
#include <text.h>
#include <util.h>
#include <main.h>
#include <CmdEdit.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdEdit::CmdEdit ()
{
  _keyword     = "edit";
  _usage       = "task edit <ID>";
  _description = "Launches an editor to let you modify all aspects of a task "
                 "directly, therefore it is to be used carefully.";
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
// Introducing the Silver Bullet.  This feature is the catch-all fixative for
// various other ills.  This is like opening up the hood and going in with a
// wrench.  To be used sparingly.
int CmdEdit::execute (std::string& output)
{
  int rc = 0;

  // Get all the tasks.
  std::vector <Task> tasks;
  context.tdb.lock (context.config.getBoolean ("locking"));
  handleRecurrence ();
  context.tdb.load (tasks);

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Find number of matching tasks.  Skip recurring parent tasks.
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    if (editFile (*task))
      context.tdb.update (*task);

  context.tdb.commit ();
  context.tdb.unlock ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::findValue (
  const std::string& text,
  const std::string& name)
{
  std::string::size_type found = text.find (name);
  if (found != std::string::npos)
  {
    std::string::size_type eol = text.find ("\n", found + 1);
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
std::string CmdEdit::findDate (
  const std::string& text,
  const std::string& name)
{
  std::string::size_type found = text.find (name);
  if (found != std::string::npos)
  {
    std::string::size_type eol = text.find ("\n", found + 1);
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
std::string CmdEdit::formatDate (
  Task& task,
  const std::string& attribute)
{
  std::string value = task.get (attribute);
  if (value.length ())
  {
    Date dt (strtol (value.c_str (), NULL, 10));
    value = dt.toString (context.config.get ("dateformat"));
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::formatTask (Task task)
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

  before << "  Tags:              " << allTags                                          << "\n"
         << "  Description:       " << task.get ("description")                         << "\n"
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

  std::map <std::string, std::string> annotations;
  task.getAnnotations (annotations);
  std::map <std::string, std::string>::iterator anno;
  for (anno = annotations.begin (); anno != annotations.end (); ++anno)
  {
    Date dt (strtol (anno->first.substr (11).c_str (), NULL, 10));
    before << "  Annotation:        " << dt.toString (context.config.get ("dateformat.annotation"))
           << " -- "                  << anno->second << "\n";
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
void CmdEdit::parseTask (Task& task, const std::string& after)
{
  // project
  std::string value = findValue (after, "\n  Project:");
  if (task.get ("project") != value)
  {
    if (value != "")
    {
      context.footnote ("Project modified.");
      task.set ("project", value);
    }
    else
    {
      context.footnote ("Project deleted.");
      task.remove ("project");
    }
  }

  // priority
  value = findValue (after, "\n  Priority:");
  if (task.get ("priority") != value)
  {
    if (value != "")
    {
      if (context.columns["priority"]->validate (value))
      {
        context.footnote ("Priority modified.");
        task.set ("priority", value);
      }
    }
    else
    {
      context.footnote ("Priority deleted.");
      task.remove ("priority");
    }
  }

  // tags
  value = findValue (after, "\n  Tags:");
  std::vector <std::string> tags;
  split (tags, value, ' ');
  task.remove ("tags");
  task.addTags (tags);

  // description.
  value = findValue (after, "\n  Description:");
  if (task.get ("description") != value)
  {
    if (value != "")
    {
      context.footnote ("Description modified.");
      task.set ("description", value);
    }
    else
      throw std::string ("Cannot remove description.");
  }

  // entry
  value = findDate (after, "\n  Created:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    Date original (task.get_date ("entry"));
    if (!original.sameDay (edited))
    {
      context.footnote ("Creation date modified.");
      task.set ("entry", value);
    }
  }
  else
    throw std::string ("Cannot remove creation date.");

  // start
  value = findDate (after, "\n  Started:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    if (task.get ("start") != "")
    {
      Date original (task.get_date ("start"));
      if (!original.sameDay (edited))
      {
        context.footnote ("Start date modified.");
        task.set ("start", value);
      }
    }
    else
    {
      context.footnote ("Start date modified.");
      task.set ("start", value);
    }
  }
  else
  {
    if (task.get ("start") != "")
    {
      context.footnote ("Start date removed.");
      task.remove ("start");
    }
  }

  // end
  value = findDate (after, "\n  Ended:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    if (task.get ("end") != "")
    {
      Date original (task.get_date ("end"));
      if (!original.sameDay (edited))
      {
        context.footnote ("Done date modified.");
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
      context.footnote ("Done date removed.");
      task.setStatus (Task::pending);
      task.remove ("end");
    }
  }

  // due
  value = findDate (after, "\n  Due:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    if (task.get ("due") != "")
    {
      Date original (task.get_date ("due"));
      if (!original.sameDay (edited))
      {
        context.footnote ("Due date modified.");
        task.set ("due", value);
      }
    }
    else
    {
      context.footnote ("Due date modified.");
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
        context.footnote ("Cannot remove a due date from a recurring task.");
      }
      else
      {
        context.footnote ("Due date removed.");
        task.remove ("due");
      }
    }
  }

  // until
  value = findDate (after, "\n  Until:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    if (task.get ("until") != "")
    {
      Date original (task.get_date ("until"));
      if (!original.sameDay (edited))
      {
        context.footnote ("Until date modified.");
        task.set ("until", value);
      }
    }
    else
    {
      context.footnote ("Until date modified.");
      task.set ("until", value);
    }
  }
  else
  {
    if (task.get ("until") != "")
    {
      context.footnote ("Until date removed.");
      task.remove ("until");
    }
  }

  // recur
  value = findValue (after, "\n  Recur:");
  if (value != task.get ("recur"))
  {
    if (value != "")
    {
      Duration d;
      if (d.valid (value))
      {
        context.footnote ("Recurrence modified.");
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
      context.footnote ("Recurrence removed.");
      task.setStatus (Task::pending);
      task.remove ("recur");
      task.remove ("until");
      task.remove ("mask");
      task.remove ("imask");
    }
  }

  // wait
  value = findDate (after, "\n  Wait until:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    if (task.get ("wait") != "")
    {
      Date original (task.get_date ("wait"));
      if (!original.sameDay (edited))
      {
        context.footnote ("Wait date modified.");
        task.set ("wait", value);
        task.setStatus (Task::waiting);
      }
    }
    else
    {
      context.footnote ("Wait date modified.");
      task.set ("wait", value);
      task.setStatus (Task::waiting);
    }
  }
  else
  {
    if (task.get ("wait") != "")
    {
      context.footnote ("Wait date removed.");
      task.remove ("wait");
      task.setStatus (Task::pending);
    }
  }

  // parent
  value = findValue (after, "\n  Parent:");
  if (value != task.get ("parent"))
  {
    if (value != "")
    {
      context.footnote ("Parent UUID modified.");
      task.set ("parent", value);
    }
    else
    {
      context.footnote ("Parent UUID removed.");
      task.remove ("parent");
    }
  }

  // fg
  value = findValue (after, "\n  Foreground color:");
  if (value != task.get ("fg"))
  {
    if (value != "")
    {
      context.footnote ("Foreground color modified.");
      task.set ("fg", value);
    }
    else
    {
      context.footnote ("Foreground color removed.");
      task.remove ("fg");
    }
  }

  // bg
  value = findValue (after, "\n  Background color:");
  if (value != task.get ("bg"))
  {
    if (value != "")
    {
      context.footnote ("Background color modified.");
      task.set ("bg", value);
    }
    else
    {
      context.footnote ("Background color removed.");
      task.remove ("bg");
    }
  }

  // Annotations
  std::map <std::string, std::string> annotations;
  std::string::size_type found = 0;
  while ((found = after.find ("\n  Annotation:", found)) != std::string::npos)
  {
    found += 14;  // Length of "\n  Annotation:".

    std::string::size_type eol = after.find ("\n", found + 1);
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
        annotations.insert (std::make_pair (name.str (), text));
      }
    }
  }

  task.setAnnotations (annotations);

  // Dependencies
  value = findValue (after, "\n  Dependencies:");
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
bool CmdEdit::editFile (Task& task)
{
  // Check for file permissions.
  Directory location (context.config.get ("data.location"));
  if (! location.writable ())
    throw std::string ("Your data.location directory is not writable.");

  // Create a temp file name in data.location.
  std::stringstream file;
  file << "task." << getpid () << "." << task.id << ".task";
  std::string path = location._data + "/" + file.str ();

  // Format the contents, T -> text, write to a file.
  std::string before = formatTask (task);
  int ignored = chdir (location._data.c_str ());
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
  bool changes = false; // No changes made.

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
    else
      changes = true;
  }
  else
  {
    std::cout << "No edits were detected.\n";
    changes = false;
  }

  // Cleanup.
  File::remove (file.str ());
  return changes;
}

////////////////////////////////////////////////////////////////////////////////
