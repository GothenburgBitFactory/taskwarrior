////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <CmdEdit.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <cerrno>
#include <Datetime.h>
#include <Duration.h>
#include <Context.h>
#include <Lexer.h>
#include <Filter.h>
#include <Pig.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include <main.h>
#include <JSON.h>

#define STRING_EDIT_START_MOD        "Start date modified."
#define STRING_EDIT_SCHED_MOD        "Scheduled date modified."
#define STRING_EDIT_DUE_MOD          "Due date modified."
#define STRING_EDIT_UNTIL_MOD        "Until date modified."
#define STRING_EDIT_WAIT_MOD         "Wait date modified."

const std::string CmdEdit::ANNOTATION_EDIT_MARKER = "\n                     ";

////////////////////////////////////////////////////////////////////////////////
CmdEdit::CmdEdit ()
{
  _keyword               = "edit";
  _usage                 = "task <filter> edit";
  _description           = "Launches an editor to modify a task directly";
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
// Introducing the Silver Bullet.  This feature is the catch-all fixative for
// various other ills.  This is like opening up the hood and going in with a
// wrench.  To be used sparingly.
int CmdEdit::execute (std::string&)
{
  // Filter the tasks.
  handleUntil ();
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  if (! filtered.size ())
  {
    Context::getContext ().footnote ("No matches.");
    return 1;
  }

  unsigned int bulk = Context::getContext ().config.getInteger ("bulk");

  // If we are editing more than "bulk" tasks, ask for confirmation.
  // Bulk = 0 denotes infinite bulk.
  if ((filtered.size () > bulk) && (bulk != 0))
    if (! confirm (format ("Do you wish to manually edit {1} tasks?", filtered.size ())))
      return 2;

  // Find number of matching tasks.
  for (auto& task : filtered)
  {
    auto result = editFile (task);
    if (result == CmdEdit::editResult::error)
      break;
    else if (result == CmdEdit::editResult::changes)
      Context::getContext ().tdb2.modify (task);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::findValue (
  const std::string& text,
  const std::string& name)
{
  auto found = text.find (name);
  if (found != std::string::npos)
  {
    auto eol = text.find ('\n', found + 1);
    if (eol != std::string::npos)
    {
      std::string value = text.substr (
        found + name.length (),
        eol - (found + name.length ()));

      return Lexer::trim (value, "\t ");
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::findMultilineValue (
  const std::string& text,
  const std::string& startMarker,
  const std::string& endMarker)
{
  auto start = text.find (startMarker);
  if (start != std::string::npos)
  {
    auto end = text.find (endMarker, start);
    if (end != std::string::npos)
    {
      std::string value = text.substr (start + startMarker.length (),
                                       end - (start + startMarker.length ()));
      return Lexer::trim (value, "\\\t ");
    }
  }
  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> CmdEdit::findValues (
  const std::string& text,
  const std::string& name)
{
  std::vector <std::string> results;
  std::string::size_type found = 0;

  while (found != std::string::npos)
  {
    found = text.find (name, found + 1);
    if (found != std::string::npos)
    {
      auto eol = text.find ('\n', found + 1);
      if (eol != std::string::npos)
      {
        auto value = text.substr (
          found + name.length (),
          eol - (found + name.length ()));

        found = eol - 1;
        results.push_back (Lexer::trim (value, "\t "));
      }
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::formatDate (
  Task& task,
  const std::string& attribute,
  const std::string& dateformat)
{
  auto value = task.get (attribute);
  if (value.length ())
    value = Datetime (value).toString (dateformat);

  return value;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::formatDuration (
  Task& task,
  const std::string& attribute)
{
  auto value = task.get (attribute);
  if (value.length ())
    value = Duration (value).formatISO ();

  return value;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::formatTask (Task task, const std::string& dateformat)
{
  std::stringstream before;
  auto verbose = Context::getContext ().verbose ("edit");

  if (verbose)
    before << "# The 'task <id> edit' command allows you to modify all aspects of a task\n"
              "# using a text editor.  Below is a representation of all the task details.\n"
              "# Modify what you wish, and when you save and quit your editor,\n"
              "# Taskwarrior will read this file, determine what changed, and apply\n"
              "# those changes.  If you exit your editor without saving or making\n"
              "# modifications, Taskwarrior will do nothing.\n"
              "#\n"
              "# Lines that begin with # represent data you cannot change, like ID.\n"
              "# If you get too creative with your editing, Taskwarrior will send you\n"
              "# back to the editor to try again.\n"
              "#\n"
              "# Should you find yourself in an endless loop, re-editing the same file,\n"
              "# just quit the editor without making any changes.  Taskwarrior will\n"
              "# notice this and stop the editing.\n"
              "#\n";

  before << "# Name               Editable details\n"
         << "# -----------------  ----------------------------------------------------\n"
         << "# ID:                " << task.id                                                 << '\n'
         << "# UUID:              " << task.get ("uuid")                                       << '\n'
         << "# Status:            " << Lexer::ucFirst (Task::statusToText (task.getStatus ())) << '\n'
         << "# Mask:              " << task.get ("mask")                                       << '\n'
         << "# iMask:             " << task.get ("imask")                                      << '\n'
         << "  Project:           " << task.get ("project")                                    << '\n';

  if (verbose)
    before << "# Separate the tags with spaces, like this: tag1 tag2\n";

  before << "  Tags:              " << join (" ", task.getTags ())                      << '\n'
         << "  Description:       " << task.get ("description")                         << '\n'
         << "  Created:           " << formatDate (task, "entry", dateformat)           << '\n'
         << "  Started:           " << formatDate (task, "start", dateformat)           << '\n'
         << "  Ended:             " << formatDate (task, "end", dateformat)             << '\n'
         << "  Scheduled:         " << formatDate (task, "scheduled", dateformat)       << '\n'
         << "  Due:               " << formatDate (task, "due", dateformat)             << '\n'
         << "  Until:             " << formatDate (task, "until", dateformat)           << '\n'
         << "  Recur:             " << task.get ("recur")                               << '\n'
         << "  Wait until:        " << formatDate (task, "wait", dateformat)            << '\n'
         << "# Modified:          " << formatDate (task, "modified", dateformat)        << '\n'
         << "  Parent:            " << task.get ("parent")                              << '\n';

  if (verbose)
    before << "# Annotations look like this: <date> -- <text> and there can be any number of them.\n"
              "# The ' -- ' separator between the date and text field should not be removed.\n"
              "# Multiline annotations need to be indented up to <date> (" << ANNOTATION_EDIT_MARKER.length () - 1 << " spaces).\n"
              "# A \"blank slot\" for adding an annotation follows for your convenience.\n";

  for (auto& anno : task.getAnnotations ())
  {
    Datetime dt (strtol (anno.first.substr (11).c_str (), nullptr, 10));
    before << "  Annotation:        " << dt.toString (dateformat)
           << " -- "                  << str_replace (anno.second, "\n", ANNOTATION_EDIT_MARKER) << '\n';
  }

  Datetime now;
  before << "  Annotation:        " << now.toString (dateformat) << " -- \n";

  // Add dependencies here.
  auto dependencies = task.getDependencyUUIDs ();
  std::stringstream allDeps;
  for (unsigned int i = 0; i < dependencies.size (); ++i)
  {
    if (i)
      allDeps << ",";

    Task t;
    Context::getContext ().tdb2.get (dependencies[i], t);
    if (t.getStatus () == Task::pending ||
        t.getStatus () == Task::waiting)
      allDeps << t.id;
    else
      allDeps << dependencies[i];
  }

  if (verbose)
    before << "# Dependencies should be a comma-separated list of task IDs/UUIDs or ID ranges, with no spaces.\n";

  before << "  Dependencies:      " << allDeps.str () << '\n';

  // UDAs
  std::vector <std::string> udas;
  for (auto& col : Context::getContext ().columns)
    if (Context::getContext ().config.get ("uda." + col.first + ".type") != "")
      udas.push_back (col.first);

  if (udas.size ())
  {
    before << "# User Defined Attributes\n";
    std::sort (udas.begin (), udas.end ());
    for (auto& uda : udas)
    {
      int pad = 13 - uda.length ();
      std::string padding = "";
      if (pad > 0)
        padding = std::string (pad, ' ');

      std::string type = Context::getContext ().config.get ("uda." + uda + ".type");
      if (type == "string" || type == "numeric")
      {
        auto value = task.get (uda);
        if (type == "string")
          value = json::encode (value);
        before << "  UDA " << uda << ": " << padding << value << '\n';
      }
      else if (type == "date")
        before << "  UDA " << uda << ": " << padding << formatDate (task, uda, dateformat) << '\n';
      else if (type == "duration")
        before << "  UDA " << uda << ": " << padding << formatDuration (task, uda) << '\n';
    }
  }

  // UDA orphans
  auto orphans = task.getUDAOrphans ();
  if (orphans.size ())
  {
    before << "# User Defined Attribute Orphans\n";
    std::sort (orphans.begin (), orphans.end ());
    for (auto& orphan : orphans)
    {
      int pad = 6 - orphan.length ();
      std::string padding = "";
      if (pad > 0)
        padding = std::string (pad, ' ');

      before << "  UDA Orphan " << orphan << ": " << padding << task.get (orphan) << '\n';
    }
  }

  before << "# End\n";
  return before.str ();
}

////////////////////////////////////////////////////////////////////////////////
void CmdEdit::parseTask (Task& task, const std::string& after, const std::string& dateformat)
{
  // project
  auto value = findValue (after, "\n  Project:");
  if (task.get ("project") != value)
  {
    if (value != "")
    {
      Context::getContext ().footnote ("Project modified.");
      task.set ("project", value);
    }
    else
    {
      Context::getContext ().footnote ("Project deleted.");
      task.remove ("project");
    }
  }

  // tags
  value = findValue (after, "\n  Tags:");
  task.remove ("tags");
  task.setTags (split (value, ' '));

  // description.
  value = findMultilineValue (after, "\n  Description:", "\n  Created:");
  if (task.get ("description") != value)
  {
    if (value != "")
    {
      Context::getContext ().footnote ("Description modified.");
      task.set ("description", value);
    }
    else
      throw std::string ("Cannot remove description.");
  }

  // entry
  value = findValue (after, "\n  Created:");
  if (value != "")
  {
    if (value != formatDate (task, "entry", dateformat))
    {
      Context::getContext ().footnote ("Creation date modified.");
      task.set ("entry", Datetime (value, dateformat).toEpochString ());
    }
  }
  else
    throw std::string ("Cannot remove creation date.");

  // start
  value = findValue (after, "\n  Started:");
  if (value != "")
  {
    if (task.get ("start") != "")
    {
      if (value != formatDate (task, "start", dateformat))
      {
        Context::getContext ().footnote (STRING_EDIT_START_MOD);
        task.set ("start", Datetime (value, dateformat).toEpochString ());
      }
    }
    else
    {
      Context::getContext ().footnote (STRING_EDIT_START_MOD);
      task.set ("start", Datetime (value, dateformat).toEpochString ());
    }
  }
  else
  {
    if (task.get ("start") != "")
    {
      Context::getContext ().footnote ("Start date removed.");
      task.remove ("start");
    }
  }

  // end
  value = findValue (after, "\n  Ended:");
  if (value != "")
  {
    if (task.get ("end") != "")
    {
      if (value != formatDate (task, "end", dateformat))
      {
        Context::getContext ().footnote ("End date modified.");
        task.set ("end", Datetime (value, dateformat).toEpochString ());
      }
    }
    else if (task.getStatus () != Task::deleted)
      throw std::string ("Cannot set a done date on a pending task.");
  }
  else
  {
    if (task.get ("end") != "")
    {
      Context::getContext ().footnote ("End date removed.");
      task.setStatus (Task::pending);
      task.remove ("end");
    }
  }

  // scheduled
  value = findValue (after, "\n  Scheduled:");
  if (value != "")
  {
    if (task.get ("scheduled") != "")
    {
      if (value != formatDate (task, "scheduled", dateformat))
      {
        Context::getContext ().footnote (STRING_EDIT_SCHED_MOD);
        task.set ("scheduled", Datetime (value, dateformat).toEpochString ());
      }
    }
    else
    {
      Context::getContext ().footnote (STRING_EDIT_SCHED_MOD);
      task.set ("scheduled", Datetime (value, dateformat).toEpochString ());
    }
  }
  else
  {
    if (task.get ("scheduled") != "")
    {
      Context::getContext ().footnote ("Scheduled date removed.");
      task.remove ("scheduled");
    }
  }

  // due
  value = findValue (after, "\n  Due:");
  if (value != "")
  {
    if (task.get ("due") != "")
    {
      if (value != formatDate (task, "due", dateformat))
      {
        Context::getContext ().footnote (STRING_EDIT_DUE_MOD);
        task.set ("due", Datetime (value, dateformat).toEpochString ());
      }
    }
    else
    {
      Context::getContext ().footnote (STRING_EDIT_DUE_MOD);
      task.set ("due", Datetime (value, dateformat).toEpochString ());
    }
  }
  else
  {
    if (task.get ("due") != "")
    {
      if (task.getStatus () == Task::recurring ||
          task.get ("parent") != "")
      {
        Context::getContext ().footnote ("Cannot remove a due date from a recurring task.");
      }
      else
      {
        Context::getContext ().footnote ("Due date removed.");
        task.remove ("due");
      }
    }
  }

  // until
  value = findValue (after, "\n  Until:");
  if (value != "")
  {
    if (task.get ("until") != "")
    {
      if (value != formatDate (task, "until", dateformat))
      {
        Context::getContext ().footnote (STRING_EDIT_UNTIL_MOD);
        task.set ("until", Datetime (value, dateformat).toEpochString ());
      }
    }
    else
    {
      Context::getContext ().footnote (STRING_EDIT_UNTIL_MOD);
      task.set ("until", Datetime (value, dateformat).toEpochString ());
    }
  }
  else
  {
    if (task.get ("until") != "")
    {
      Context::getContext ().footnote ("Until date removed.");
      task.remove ("until");
    }
  }

  // recur
  value = findValue (after, "\n  Recur:");
  if (value != task.get ("recur"))
  {
    if (value != "")
    {
      Duration p;
      std::string::size_type idx = 0;
      if (p.parse (value, idx))
      {
        Context::getContext ().footnote ("Recurrence modified.");
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
      Context::getContext ().footnote ("Recurrence removed.");
      task.setStatus (Task::pending);
      task.remove ("recur");
      task.remove ("until");
      task.remove ("mask");
      task.remove ("imask");
    }
  }

  // wait
  value = findValue (after, "\n  Wait until:");
  if (value != "")
  {
    if (task.get ("wait") != "")
    {
      if (value != formatDate (task, "wait", dateformat))
      {
        Context::getContext ().footnote (STRING_EDIT_WAIT_MOD);
        task.set ("wait", Datetime (value, dateformat).toEpochString ());
        task.setStatus (Task::waiting);
      }
    }
    else
    {
      Context::getContext ().footnote (STRING_EDIT_WAIT_MOD);
      task.set ("wait", Datetime (value, dateformat).toEpochString ());
      task.setStatus (Task::waiting);
    }
  }
  else
  {
    if (task.get ("wait") != "")
    {
      Context::getContext ().footnote ("Wait date removed.");
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
      Context::getContext ().footnote ("Parent UUID modified.");
      task.set ("parent", value);
    }
    else
    {
      Context::getContext ().footnote ("Parent UUID removed.");
      task.remove ("parent");
    }
  }

  // Annotations
  std::map <std::string, std::string> annotations;
  std::string::size_type found = 0;
  while ((found = after.find ("\n  Annotation:", found)) != std::string::npos)
  {
    found += 14;  // Length of "\n  Annotation:".

    auto eol = found;
    while ((eol = after.find ('\n', ++eol)) != std::string::npos)
      if (after.substr (eol, ANNOTATION_EDIT_MARKER.length ()) != ANNOTATION_EDIT_MARKER)
        break;

    if (eol != std::string::npos)
    {
      auto value = Lexer::trim (str_replace (after.substr (found, eol - found), ANNOTATION_EDIT_MARKER, "\n"), "\t ");
      auto gap = value.find (" -- ");
      if (gap != std::string::npos)
      {
        // TODO keeping the initial dates even if dateformat approximates them
        // is complex as finding the correspondence between each original line
        // and edited line may be impossible (bug #705). It would be simpler if
        // each annotation was put on a line with a distinguishable id (then
        // for each line: if the annotation is the same, then it is copied; if
        // the annotation is modified, then its original date may be kept; and
        // if there is no corresponding id, then a new unique date is created).
        Datetime when (value.substr (0, gap), dateformat);

        // If the map already contains an annotation for a given timestamp
        // we need to increment until we find an unused key
        int timestamp = (int) when.toEpoch ();

        std::stringstream name;

        do
        {
          name.str ("");  // Clear
          name << "annotation_" << timestamp;
          timestamp++;
        }
        while (annotations.find (name.str ()) != annotations.end ());

        auto text = Lexer::trim (value.substr (gap + 4), "\t ");
        annotations.emplace (name.str (), text);
      }
    }
  }

  task.setAnnotations (annotations);

  // Dependencies
  value = findValue (after, "\n  Dependencies:");
  auto dependencies = split (value, ',');

  for (auto& dep : task.getDependencyUUIDs ())
    task.removeDependency (dep);
  for (auto& dep : dependencies)
  {
    if (dep.length () >= 7)
      task.addDependency (dep);
    else
      task.addDependency ((int) strtol (dep.c_str (), nullptr, 10));
  }

  // UDAs
  for (auto& col : Context::getContext ().columns)
  {
    auto type = Context::getContext ().config.get ("uda." + col.first + ".type");
    if (type != "")
    {
      auto value = findValue (after, "\n  UDA " + col.first + ":");
      if (type == "string")
        value = json::decode (value);
      if ((task.get (col.first) != value) && (type != "date" ||
           (task.get (col.first) != Datetime (value, dateformat).toEpochString ())) &&
           (type != "duration" ||
           (task.get (col.first) != Duration (value).toString ())))
      {
        if (value != "")
        {
          Context::getContext ().footnote (format ("UDA {1} modified.", col.first));

          if (type == "string")
          {
            task.set (col.first, value);
          }
          else if (type == "numeric")
          {
            Pig pig (value);
            double d;
            if (pig.getNumber (d) &&
                pig.eos ())
              task.set (col.first, value);
            else
              throw format ("The value '{1}' is not a valid numeric value.", value);
          }
          else if (type == "date")
          {
            task.set (col.first, Datetime (value, dateformat).toEpochString ());
          }
          else if (type == "duration")
          {
            task.set (col.first, Duration (value).toTime_t ());
          }
        }
        else
        {
          Context::getContext ().footnote (format ("UDA {1} deleted.", col.first));
          task.remove (col.first);
        }
      }
    }
  }

  // UDA orphans
  for (auto& orphan : findValues (after, "\n  UDA Orphan "))
  {
    auto colon = orphan.find (':');
    if (colon != std::string::npos)
    {
      std::string name  = Lexer::trim (orphan.substr (0, colon),  "\t ");
      std::string value = Lexer::trim (orphan.substr (colon + 1), "\t ");

      if (value != "")
        task.set (name, value);
      else
        task.remove (name);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
CmdEdit::editResult CmdEdit::editFile (Task& task)
{
  // Check for file permissions.
  Directory location (Context::getContext ().config.get ("data.location"));
  if (! location.writable ())
    throw std::string ("Your data.location directory is not writable.");

  // Create a temp file name in data.location.
  std::stringstream file;
  file << "task." << task.get ("uuid").substr (0, 8) << ".task";

  // Determine the output date format, which uses a hierarchy of definitions.
  //   rc.dateformat.edit
  //   rc.dateformat
  auto dateformat = Context::getContext ().config.get ("dateformat.edit");
  if (dateformat == "")
    dateformat = Context::getContext ().config.get ("dateformat");

  // Change directory for the editor
  auto current_dir = Directory::cwd ();
  int ignored = chdir (location._data.c_str ());
  ++ignored; // Keep compiler quiet.

  // Check if the file already exists, if so, bail out
  Path filepath = Path (file.str ());
  if (filepath.exists ())
    throw std::string ("Task is already being edited.");

  // Format the contents, T -> text, write to a file.
  auto before = formatTask (task, dateformat);
  auto before_orig = before;
  File::write (file.str (), before);

  // Determine correct editor: .taskrc:editor > $VISUAL > $EDITOR > vi
  auto editor = Context::getContext ().config.get ("editor");
  char* peditor = getenv ("VISUAL");
  if (editor == "" && peditor) editor = std::string (peditor);
  peditor = getenv ("EDITOR");
  if (editor == "" && peditor) editor = std::string (peditor);
  if (editor == "") editor = "vi";

  // Complete the command line.
  editor += ' ';
  editor += '"' + file.str () + '"';

ARE_THESE_REALLY_HARMFUL:
  bool changes = false; // No changes made.

  // Launch the editor.
  std::cout << format ("Launching '{1}' now...\n", editor);
  int exitcode = system (editor.c_str ());
  auto captured_errno = errno;
  if (0 == exitcode)
    std::cout << "Editing complete.\n";
  else
  {
    std::cout << format ("Editing failed with exit code {1}.\n", exitcode);
    if (-1 == exitcode)
      std::cout << std::strerror (captured_errno) << '\n';
    File::remove (file.str ());
    return CmdEdit::editResult::error;
  }

  // Slurp file.
  std::string after;
  File::read (file.str (), after);

  // Update task based on what can be parsed back out of the file, but only
  // if changes were made.
  if (before_orig != after)
  {
    std::cout << "Edits were detected.\n";
    std::string problem = "";
    auto oops = false;

    try
    {
      parseTask (task, after, dateformat);
    }

    catch (const std::string& e)
    {
      problem = e;
      oops = true;
    }

    if (oops)
    {
      std::cerr << "Error: " << problem << '\n';

      File::remove (file.str());

      if (confirm ("Taskwarrior couldn't handle your edits.  Would you like to try again?"))
      {
        // Preserve the edits.
        before = after;
        File::write (file.str (), before);

        goto ARE_THESE_REALLY_HARMFUL;
      }
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
  ignored = chdir (current_dir.c_str ());
  return changes
         ? CmdEdit::editResult::changes
         : CmdEdit::editResult::nochanges;
}

////////////////////////////////////////////////////////////////////////////////
