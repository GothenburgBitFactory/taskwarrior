////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#include <cmake.h>
#include <CmdEdit.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <cerrno>
#include <ISO8601.h>
#include <Context.h>
#include <Lexer.h>
#include <Filter.h>
#include <Nibbler.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>
#include <JSON.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdEdit::CmdEdit ()
{
  _keyword               = "edit";
  _usage                 = "task <filter> edit";
  _description           = STRING_CMD_EDIT_USAGE;
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
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  if (! filtered.size ())
  {
    context.footnote (STRING_FEEDBACK_NO_MATCH);
    return 1;
  }

  // Find number of matching tasks.
  for (auto& task : filtered)
  {
    auto result = editFile (task);
    if (result == CmdEdit::editResult::error)
      break;
    else if (result == CmdEdit::editResult::changes)
      context.tdb2.modify (task);
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
    auto eol = text.find ("\n", found + 1);
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
      auto eol = text.find ("\n", found + 1);
      if (eol != std::string::npos)
      {
        std::string value = text.substr (
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
  std::string value = task.get (attribute);
  if (value.length ())
  {
    ISO8601d dt (value);
    value = dt.toString (dateformat);
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::formatDuration (
  Task& task,
  const std::string& attribute)
{
  std::string value = task.get (attribute);
  if (value.length ())
  {
    ISO8601p dur (value);
    value = dur.format ();
  }

  return value;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdEdit::formatTask (Task task, const std::string& dateformat)
{
  std::stringstream before;
  bool verbose = context.verbose ("edit");

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
         << "# ID:                " << task.id                                                 << "\n"
         << "# UUID:              " << task.get ("uuid")                                       << "\n"
         << "# Status:            " << Lexer::ucFirst (Task::statusToText (task.getStatus ())) << "\n"
         << "# Mask:              " << task.get ("mask")                                       << "\n"
         << "# iMask:             " << task.get ("imask")                                      << "\n"
         << "  Project:           " << task.get ("project")                                    << "\n";

  std::vector <std::string> tags;
  task.getTags (tags);
  std::string allTags;
  join (allTags, " ", tags);

  if (verbose)
    before << "# " << STRING_EDIT_TAG_SEP << "\n";

  before << "  Tags:              " << allTags                                          << "\n"
         << "  Description:       " << task.get ("description")                         << "\n"
         << "  Created:           " << formatDate (task, "entry", dateformat)           << "\n"
         << "  Started:           " << formatDate (task, "start", dateformat)           << "\n"
         << "  Ended:             " << formatDate (task, "end", dateformat)             << "\n"
         << "  Scheduled:         " << formatDate (task, "scheduled", dateformat)       << "\n"
         << "  Due:               " << formatDate (task, "due", dateformat)             << "\n"
         << "  Until:             " << formatDate (task, "until", dateformat)           << "\n"
         << "  Recur:             " << task.get ("recur")                               << "\n"
         << "  Wait until:        " << formatDate (task, "wait", dateformat)            << "\n"
         << "# Modified:          " << formatDate (task, "modified", dateformat)        << "\n"
         << "  Parent:            " << task.get ("parent")                              << "\n";

  if (verbose)
    before << "# " << STRING_EDIT_HEADER_13 << "\n"
           << "# " << STRING_EDIT_HEADER_14 << "\n"
           << "# " << STRING_EDIT_HEADER_15 << "\n";

  std::map <std::string, std::string> annotations;
  task.getAnnotations (annotations);
  for (auto& anno : annotations)
  {
    ISO8601d dt (strtol (anno.first.substr (11).c_str (), NULL, 10));
    before << "  Annotation:        " << dt.toString (dateformat)
           << " -- "                  << json::encode (anno.second) << "\n";
  }

  ISO8601d now;
  before << "  Annotation:        " << now.toString (dateformat) << " -- \n";

  // Add dependencies here.
  std::vector <std::string> dependencies;
  task.getDependencies (dependencies);
  std::stringstream allDeps;
  for (unsigned int i = 0; i < dependencies.size (); ++i)
  {
    if (i)
      allDeps << ",";

    Task t;
    context.tdb2.get (dependencies[i], t);
    if (t.getStatus () == Task::pending ||
        t.getStatus () == Task::waiting)
      allDeps << t.id;
    else
      allDeps << dependencies[i];
  }

  if (verbose)
    before << "# " << STRING_EDIT_DEP_SEP << "\n";

  before << "  Dependencies:      " << allDeps.str () << "\n";

  // UDAs
  std::vector <std::string> udas;
  for (auto& col : context.columns)
    if (context.config.get ("uda." + col.first + ".type") != "")
      udas.push_back (col.first);

  if (udas.size ())
  {
    before << "# " << STRING_EDIT_UDA_SEP << "\n";
    std::sort (udas.begin (), udas.end ());
    for (auto& uda : udas)
    {
      int pad = 13 - uda.length ();
      std::string padding = "";
      if (pad > 0)
        padding = std::string (pad, ' ');

      std::string type = context.config.get ("uda." + uda + ".type");
      if (type == "string" || type == "numeric")    
        before << "  UDA " << uda << ": " << padding << task.get (uda) << "\n";
      else if (type == "date")
        before << "  UDA " << uda << ": " << padding << formatDate (task, uda, dateformat) << "\n";
      else if (type == "duration")
        before << "  UDA " << uda << ": " << padding << formatDuration (task, uda) << "\n";
    }
  }

  // UDA orphans
  std::vector <std::string> orphans;
  task.getUDAOrphans (orphans);

  if (orphans.size ())
  {
    before << "# " << STRING_EDIT_UDA_ORPHAN_SEP << "\n";
    std::sort (orphans.begin (), orphans.end ());
    for (auto& orphan : orphans)
    {
      int pad = 6 - orphan.length ();
      std::string padding = "";
      if (pad > 0)
        padding = std::string (pad, ' ');

      before << "  UDA Orphan " << orphan << ": " << padding << task.get (orphan) << "\n";
    }
  }

  before << "# " << STRING_EDIT_END << "\n";
  return before.str ();
}

////////////////////////////////////////////////////////////////////////////////
void CmdEdit::parseTask (Task& task, const std::string& after, const std::string& dateformat)
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

  // tags
  value = findValue (after, "\n  Tags:");
  std::vector <std::string> tags;
  split (tags, value, ' ');
  task.remove ("tags");
  task.addTags (tags);

  // description.
  value = findMultilineValue (after, "\n  Description:", "\n  Created:");
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
  value = findValue (after, "\n  Created:");
  if (value != "")
  {
    std::string formatted = formatDate (task, "entry", dateformat);

    if (formatted != value)
    {
      context.footnote (STRING_EDIT_ENTRY_MOD);
      task.set ("entry", ISO8601d (value, dateformat).toEpochString ());
    }
  }
  else
    throw std::string (STRING_EDIT_ENTRY_REMOVE_ERR);

  // start
  value = findValue (after, "\n  Started:");
  if (value != "")
  {
    if (task.get ("start") != "")
    {
      std::string formatted = formatDate (task, "start", dateformat);

      if (formatted != value)
      {
        context.footnote (STRING_EDIT_START_MOD);
        task.set ("start", ISO8601d (value, dateformat).toEpochString ());
      }
    }
    else
    {
      context.footnote (STRING_EDIT_START_MOD);
      task.set ("start", ISO8601d (value, dateformat).toEpochString ());
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
  value = findValue (after, "\n  Ended:");
  if (value != "")
  {
    if (task.get ("end") != "")
    {
      std::string formatted = formatDate (task, "end", dateformat);

      if (formatted != value)
      {
        context.footnote (STRING_EDIT_END_MOD);
        task.set ("end", ISO8601d (value, dateformat).toEpochString ());
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

  // scheduled
  value = findValue (after, "\n  Scheduled:");
  if (value != "")
  {
    if (task.get ("scheduled") != "")
    {
      std::string formatted = formatDate (task, "scheduled", dateformat);

      if (formatted != value)
      {
        context.footnote (STRING_EDIT_SCHED_MOD);
        task.set ("scheduled", ISO8601d (value, dateformat).toEpochString ());
      }
    }
    else
    {
      context.footnote (STRING_EDIT_SCHED_MOD);
      task.set ("scheduled", ISO8601d (value, dateformat).toEpochString ());
    }
  }
  else
  {
    if (task.get ("scheduled") != "")
    {
      context.footnote (STRING_EDIT_SCHED_DEL);
      task.setStatus (Task::pending);
      task.remove ("scheduled");
    }
  }

  // due
  value = findValue (after, "\n  Due:");
  if (value != "")
  {
    if (task.get ("due") != "")
    {
      std::string formatted = formatDate (task, "due", dateformat);

      if (formatted != value)
      {
        context.footnote (STRING_EDIT_DUE_MOD);
        task.set ("due", ISO8601d (value, dateformat).toEpochString ());
      }
    }
    else
    {
      context.footnote (STRING_EDIT_DUE_MOD);
      task.set ("due", ISO8601d (value, dateformat).toEpochString ());
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
  value = findValue (after, "\n  Until:");
  if (value != "")
  {
    if (task.get ("until") != "")
    {
      std::string formatted = formatDate (task, "until", dateformat);

      if (formatted != value)
      {
        context.footnote (STRING_EDIT_UNTIL_MOD);
        task.set ("until", ISO8601d (value, dateformat).toEpochString ());
      }
    }
    else
    {
      context.footnote (STRING_EDIT_UNTIL_MOD);
      task.set ("until", ISO8601d (value, dateformat).toEpochString ());
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
      ISO8601p p;
      std::string::size_type idx = 0;
      if (p.parse (value, idx))
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
  value = findValue (after, "\n  Wait until:");
  if (value != "")
  {
    if (task.get ("wait") != "")
    {
      std::string formatted = formatDate (task, "wait", dateformat);

      if (formatted != value)
      {
        context.footnote (STRING_EDIT_WAIT_MOD);
        task.set ("wait", ISO8601d (value, dateformat).toEpochString ());
        task.setStatus (Task::waiting);
      }
    }
    else
    {
      context.footnote (STRING_EDIT_WAIT_MOD);
      task.set ("wait", ISO8601d (value, dateformat).toEpochString ());
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

  // Annotations
  std::map <std::string, std::string> annotations;
  std::string::size_type found = 0;
  while ((found = after.find ("\n  Annotation:", found)) != std::string::npos)
  {
    found += 14;  // Length of "\n  Annotation:".

    auto eol = after.find ("\n", found + 1);
    if (eol != std::string::npos)
    {
      std::string value = Lexer::trim (after.substr (
        found,
        eol - found), "\t ");

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
        ISO8601d when (value.substr (0, gap), dateformat);

        // If the map already contains a annotation for a given timestamp
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

        std::string text = Lexer::trim (value.substr (gap + 4), "\t ");
        annotations.insert (std::make_pair (name.str (), json::decode (text)));
      }
    }
  }

  task.setAnnotations (annotations);

  // Dependencies
  value = findValue (after, "\n  Dependencies:");
  std::vector <std::string> dependencies;
  split (dependencies, value, ",");

  task.remove ("depends");
  for (auto& dep : dependencies)
  {
    if (dep.length () >= 7)
      task.addDependency (dep);
    else
      task.addDependency ((int) strtol (dep.c_str (), NULL, 10));
  }

  // UDAs
  for (auto& col : context.columns)
  {
    std::string type = context.config.get ("uda." + col.first + ".type");
    if (type != "")
    {
      std::string value = findValue (after, "\n  UDA " + col.first + ":");
      if ((task.get (col.first) != value) && (type != "date" ||
           (task.get (col.first) != ISO8601d (value, dateformat).toEpochString ())) &&
           (type != "duration" ||
           (task.get (col.first) != (std::string) ISO8601p (value))))
      {
        if (value != "")
        {
          context.footnote (format (STRING_EDIT_UDA_MOD, col.first));

          if (type == "string")
          {
            task.set (col.first, value);
          }
          else if (type == "numeric")
          {
            Nibbler n (value);
            double d;
            if (n.getNumber (d) &&
                n.depleted ())
              task.set (col.first, value);
            else
              throw format (STRING_UDA_NUMERIC, value);
          }
          else if (type == "date")
          {
            task.set (col.first, ISO8601d (value, dateformat).toEpochString ());
          }
          else if (type == "duration")
          {
            task.set (col.first, (time_t) ISO8601p (value));
          }
        }
        else
        {
          context.footnote (format (STRING_EDIT_UDA_DEL, col.first));
          task.remove (col.first);
        }
      }
    }
  }

  // UDA orphans
  std::vector <std::string> orphanValues = findValues (after, "\n  UDA Orphan ");
  for (auto& orphan : orphanValues)
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
  Directory location (context.config.get ("data.location"));
  if (! location.writable ())
    throw std::string (STRING_EDIT_UNWRITABLE);

  // Create a temp file name in data.location.
  std::stringstream file;
  file << "task." << task.get ("uuid").substr (0, 8) << ".task";

  // Determine the output date format, which uses a hierarchy of definitions.
  //   rc.dateformat.edit
  //   rc.dateformat
  std::string dateformat = context.config.get ("dateformat.edit");
  if (dateformat == "")
    dateformat = context.config.get ("dateformat");

  // Change directory for the editor
  std::string current_dir = Directory::cwd ();
  int ignored = chdir (location._data.c_str ());
  ++ignored; // Keep compiler quiet.

  // Check if the file already exists, if so, bail out
  Path filepath = Path (file.str ());
  if (filepath.exists ())
    throw std::string (STRING_EDIT_IN_PROGRESS);

  // Format the contents, T -> text, write to a file.
  std::string before = formatTask (task, dateformat);
  std::string before_orig = before;
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
  int exitcode = system (editor.c_str ());
  if (0 == exitcode)
    std::cout << STRING_EDIT_COMPLETE << "\n";
  else
  {
    std::cout << format (STRING_EDIT_FAILED, exitcode) << "\n";
    if (-1 == exitcode)
      std::cout << std::strerror (errno) << "\n";
    return CmdEdit::editResult::error;
  }

  // Slurp file.
  std::string after;
  File::read (file.str (), after);

  // Update task based on what can be parsed back out of the file, but only
  // if changes were made.
  if (before_orig != after)
  {
    std::cout << STRING_EDIT_CHANGES << "\n";
    std::string problem = "";
    bool oops = false;

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
      std::cerr << STRING_ERROR_PREFIX << problem << "\n";

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
  ignored = chdir (current_dir.c_str ());
  return changes
         ? CmdEdit::editResult::changes
         : CmdEdit::editResult::nochanges;
}

////////////////////////////////////////////////////////////////////////////////
