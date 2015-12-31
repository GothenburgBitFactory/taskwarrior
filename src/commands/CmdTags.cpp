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
#include <CmdTags.h>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <Context.h>
#include <Filter.h>
#include <ViewText.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdTags::CmdTags ()
{
  _keyword               = "tags";
  _usage                 = "task <filter> tags";
  _description           = STRING_CMD_TAGS_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdTags::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Get all the tasks.
  auto tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("list.all.tags"))
    for (auto& task : context.tdb2.completed.get_tasks ())
      tasks.push_back (task);

  int quantity = tasks.size ();

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (tasks, filtered);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  for (auto& task : filtered)
  {
    std::vector <std::string> tags;
    task.getTags (tags);

    for (auto& tag : tags)
      if (unique.find (tag) != unique.end ())
        unique[tag]++;
      else
        unique[tag] = 1;
  }

  if (unique.size ())
  {
    // Render a list of tags names from the map.
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_TAG));
    view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_COUNT));

    Color bold;
    if (context.color ())
    {
      bold = Color ("bold");

      Color label (context.config.get ("color.label"));
      view.colorHeader (label);
    }

    bool special = false;
    for (auto& i : unique)
    {
      // Highlight the special tags.
      special = (context.color () &&
                 (i.first == "nocolor" ||
                  i.first == "nonag"   ||
                  i.first == "nocal"   ||
                  i.first == "next")) ? true : false;

      int row = view.addRow ();
      view.set (row, 0, i.first,  special ? bold : Color ());
      view.set (row, 1, i.second, special ? bold : Color ());
    }

    out << optionalBlankLine ()
        << view.render ()
        << optionalBlankLine ();

    if (unique.size () == 1)
      context.footnote (STRING_CMD_TAGS_SINGLE);
    else
      context.footnote (format (STRING_CMD_TAGS_PLURAL, unique.size ()));

    if (quantity == 1)
      context.footnote (STRING_FEEDBACK_TASKS_SINGLE);
    else
      context.footnote (format (STRING_FEEDBACK_TASKS_PLURAL, quantity));

    out << "\n";
  }
  else
  {
    context.footnote (STRING_CMD_TAGS_NO_TAGS);
    rc = 1;
  }

  output = out.str ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionTags::CmdCompletionTags ()
{
  _keyword               = "_tags";
  _usage                 = "task <filter> _tags";
  _description           = STRING_CMD_COMTAGS_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionTags::execute (std::string& output)
{
  // Get all the tasks.
  auto tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("complete.all.tags"))
    for (auto& task : context.tdb2.completed.get_tasks ())
      tasks.push_back (task);

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Scan all the tasks for their tags, building a map using tag
  // names as keys.
  std::map <std::string, int> unique;
  for (auto& task : filtered)
  {
    std::vector <std::string> tags;
    task.getTags (tags);

    for (auto& tag : tags)
      unique[tag] = 0;
  }

  // Add built-in tags to map.
  unique["nocolor"]   = 0;
  unique["nonag"]     = 0;
  unique["nocal"]     = 0;
  unique["next"]      = 0;
  unique["ACTIVE"]    = 0;
  unique["ANNOTATED"] = 0;
  unique["BLOCKED"]   = 0;
  unique["BLOCKING"]  = 0;
  unique["CHILD"]     = 0;
  unique["COMPLETED"] = 0;
  unique["DELETED"]   = 0;
  unique["DUE"]       = 0;
  unique["DUETODAY"]  = 0;
  unique["MONTH"]     = 0;
  unique["ORPHAN"]    = 0;
  unique["OVERDUE"]   = 0;
  unique["PARENT"]    = 0;
  unique["PENDING"]   = 0;
  unique["READY"]     = 0;
  unique["SCHEDULED"] = 0;
  unique["TAGGED"]    = 0;
  unique["TODAY"]     = 0;
  unique["TOMORROW"]  = 0;
  unique["UDA"]       = 0;
  unique["UNBLOCKED"] = 0;
  unique["UNTIL"]     = 0;
  unique["WAITING"]   = 0;
  unique["WEEK"]      = 0;
  unique["YEAR"]      = 0;
  unique["YESTERDAY"] = 0;
  // If you update the above list, update src/commands/CmdInfo.cpp and src/commands/CmdTags.cpp as well.

  std::stringstream out;
  for (auto& it : unique)
    out << it.first << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
