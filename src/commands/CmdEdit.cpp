////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#define L10N                                           // Localization complete.

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <Duration.h>
#include <Context.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>
#include <CmdEdit.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdEdit::CmdEdit ()
{
  _keyword     = "edit";
  _usage       = "task <filter> edit";
  _description = STRING_CMD_EDIT_USAGE;
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

  // Filter the tasks.
  handleRecurrence ();
  std::vector <Task> filtered;
  filter (filtered);

  // Find number of matching tasks.  Skip recurring parent tasks.
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    if (editFile (*task))
      context.tdb2.modify (*task);

  context.tdb2.commit ();
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
  bool verbose = context.verbose ("edit") ||
                 context.config.getBoolean ("edit.verbose"); // Deprecated 2.0

  if (verbose)
    before << "# " << STRING_EDIT_HEADER_1 << "\n"
           << "# " << STRING_EDIT_HEADER_2 << "\n"
           << "# " << STRING_EDIT_HEADER_3 << "\n"
           << "# " << STRING_EDIT_HEADER_4 << "\n"
           << "# " << STRING_EDIT_HEADER_5 << "\n"
           << "# " << STRING_EDIT_HEADER_6 << "\n"
           << "#\n"
           << "# " << STRING_EDIT_HEADER_7 << "\n"
           << "# " << STRING_EDIT_HEADER_8 << "\n"
           << "# " << STRING_EDIT_HEADER_9 << "\n"
           << "#\n"
           << "# " << STRING_EDIT_HEADER_10 << "\n"
           << "# " << STRING_EDIT_HEADER_11 << "\n"
           << "# " << STRING_EDIT_HEADER_12 << "\n"
           << "#\n";

  before << "# " << STRING_EDIT_TABLE_HEADER_1 << "\n"
         << "# " << STRING_EDIT_TABLE_HEADER_2 << "\n"
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
    before << "# " << STRING_EDIT_TAG_SEP << "\n";

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
    before << "# " << STRING_EDIT_HEADER_13 << "\n"
           << "# " << STRING_EDIT_HEADER_14 << "\n"
           << "# " << STRING_EDIT_HEADER_15 << "\n";

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
    before << "# " << STRING_EDIT_DEP_SEP << "\n";

  before << "  Dependencies:      " << allDeps << "\n";

  before << "# " << STRING_EDIT_END << "\n";
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
      context.footnote (STRING_EDIT_PROJECT_MOD);
      task.set ("project", value);
    }
    else
    {
      context.footnote (STRING_EDIT_PROJECT_DEL);
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
        context.footnote (STRING_EDIT_PRIORITY_MOD);
        task.set ("priority", value);
      }
    }
    else
    {
      context.footnote (STRING_EDIT_PRIORITY_DEL);
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
      context.footnote (STRING_EDIT_DESC_MOD);
      task.set ("description", value);
    }
    else
      throw std::string (STRING_EDIT_DESC_REMOVE_ERR);
  }

  // entry
  value = findDate (after, "\n  Created:");
  if (value != "")
  {
    Date edited (strtol (value.c_str (), NULL, 10));

    Date original (task.get_date ("entry"));
    if (!original.sameDay (edited))
    {
      context.footnote (STRING_EDIT_ENTRY_MOD);
      task.set ("entry", value);
    }
  }
  else
    throw std::string (STRING_EDIT_ENTRY_REMOVE_ERR);

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
        context.footnote (STRING_EDIT_START_MOD);
        task.set ("start", value);
      }
    }
    else
    {
      context.footnote (STRING_EDIT_START_MOD);
      task.set ("start", value);
    }
  }
  else
  {
    if (task.get ("start") != "")
    {
      context.footnote (STRING_EDIT_START_DEL);
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
        context.footnote (STRING_EDIT_END_MOD);
        task.set ("end", value);
      }
    }
    else if (task.getStatus () != Task::deleted)
      throw std::string (STRING_EDIT_END_SET_ERR);
  }
  else
  {
    if (task.get ("end") != "")
    {
      context.footnote (STRING_EDIT_END_DEL);
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
        context.footnote (STRING_EDIT_DUE_MOD);
        task.set ("due", value);
      }
    }
    else
    {
      context.footnote (STRING_EDIT_DUE_MOD);
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
        context.footnote (STRING_EDIT_DUE_DEL_ERR);
      }
      else
      {
        context.footnote (STRING_EDIT_DUE_DEL);
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
        context.footnote (STRING_EDIT_UNTIL_MOD);
        task.set ("until", value);
      }
    }
    else
    {
      context.footnote (STRING_EDIT_UNTIL_MOD);
      task.set ("until", value);
    }
  }
  else
  {
    if (task.get ("until") != "")
    {
      context.footnote (STRING_EDIT_UNTIL_DEL);
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
        context.footnote (STRING_EDIT_RECUR_MOD);
        if (task.get ("due") != "")
        {
          task.set ("recur", value);
          task.setStatus (Task::recurring);
        }
        else
          throw std::string (STRING_EDIT_RECUR_DUE_ERR);
      }
      else
        throw std::string (STRING_EDIT_RECUR_ERR);
    }
    else
    {
      context.footnote (STRING_EDIT_RECUR_DEL);
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
        context.footnote (STRING_EDIT_WAIT_MOD);
        task.set ("wait", value);
        task.setStatus (Task::waiting);
      }
    }
    else
    {
      context.footnote (STRING_EDIT_WAIT_MOD);
      task.set ("wait", value);
      task.setStatus (Task::waiting);
    }
  }
  else
  {
    if (task.get ("wait") != "")
    {
      context.footnote (STRING_EDIT_WAIT_DEL);
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
      context.footnote (STRING_EDIT_PARENT_MOD);
      task.set ("parent", value);
    }
    else
    {
      context.footnote (STRING_EDIT_PARENT_DEL);
      task.remove ("parent");
    }
  }

  // fg
  value = findValue (after, "\n  Foreground color:");
  if (value != task.get ("fg"))
  {
    if (value != "")
    {
      context.footnote (STRING_EDIT_FG_MOD);
      task.set ("fg", value);
    }
    else
    {
      context.footnote (STRING_EDIT_FG_DEL);
      task.remove ("fg");
    }
  }

  // bg
  value = findValue (after, "\n  Background color:");
  if (value != task.get ("bg"))
  {
    if (value != "")
    {
      context.footnote (STRING_EDIT_BG_MOD);
      task.set ("bg", value);
    }
    else
    {
      context.footnote (STRING_EDIT_BG_DEL);
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
    throw std::string (STRING_EDIT_UNWRITABLE);

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
  std::cout << format (STRING_EDIT_LAUNCHING, editor) << "\n";
  if (-1 == system (editor.c_str ()))
    std::cout << STRING_EDIT_NO_EDITS << "\n";
  else
    std::cout << STRING_EDIT_COMPLETE << "\n";

  // Slurp file.
  std::string after;
  File::read (file.str (), after);

  // Update task based on what can be parsed back out of the file, but only
  // if changes were made.
  if (before != after)
  {
    std::cout << STRING_EDIT_CHANGES << "\n";
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
      std::cout << STRING_ERROR_PREFIX << problem << "\n";

      // Preserve the edits.
      before = after;
      File::write (file.str (), before);

      if (confirm (STRING_EDIT_UNPARSEABLE))
        goto ARE_THESE_REALLY_HARMFUL;
    }
    else
      changes = true;
  }
  else
  {
    std::cout << STRING_EDIT_NO_CHANGES << "\n";
    changes = false;
  }

  // Cleanup.
  File::remove (file.str ());
  return changes;
}

////////////////////////////////////////////////////////////////////////////////
