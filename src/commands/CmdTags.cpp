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

#define L10N                                           // Localization complete.

#include <sstream>
#include <vector>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <CmdTags.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdTags::CmdTags ()
{
  _keyword     = "tags";
  _usage       = "task <filter> tags";
  _description = STRING_CMD_TAGS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdTags::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Get all the tasks.
  std::vector <Task> tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("list.all.tags"))
  {
    std::vector <Task> extra = context.tdb2.completed.get_tasks ();
    std::vector <Task>::iterator task;
    for (task = extra.begin (); task != extra.end (); ++task)
      tasks.push_back (*task);
  }

  int quantity = tasks.size ();

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Scan all the tasks for their project name, building a map using project
  // names as keys.
  std::map <std::string, int> unique;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    std::vector <std::string> tags;
    task->getTags (tags);

    std::vector <std::string>::iterator tag;
    for (tag = tags.begin (); tag != tags.end (); ++tag)
      if (unique.find (*tag) != unique.end ())
        unique[*tag]++;
      else
        unique[*tag] = 1;
  }

  if (unique.size ())
  {
    // Render a list of tags names from the map.
    ViewText view;
    view.width (context.getWidth ());
    view.add (Column::factory ("string", STRING_COLUMN_LABEL_TAG));
    view.add (Column::factory ("string.right", STRING_COLUMN_LABEL_COUNT));

    Color bold ("bold");
    bool special = false;
    std::map <std::string, int>::iterator i;
    for (i = unique.begin (); i != unique.end (); ++i)
    {
      // Highlight the special tags.
      special = (context.color () &&
                 (i->first == "nocolor" ||
                  i->first == "nonag"   ||
                  i->first == "nocal"   ||
                  i->first == "next")) ? true : false;

      int row = view.addRow ();
      view.set (row, 0, i->first,  special ? bold : Color ());
      view.set (row, 1, i->second, special ? bold : Color ());
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
  _keyword     = "_tags";
  _usage       = "task <filter> _tags";
  _description = STRING_CMD_COMTAGS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionTags::execute (std::string& output)
{
  // Get all the tasks.
  std::vector <Task> tasks = context.tdb2.pending.get_tasks ();

  if (context.config.getBoolean ("complete.all.tags"))
  {
    std::vector <Task> extra = context.tdb2.completed.get_tasks ();
    std::vector <Task>::iterator task;
    for (task = extra.begin (); task != extra.end (); ++task)
      tasks.push_back (*task);
  }

  // Apply filter.
  std::vector <Task> filtered;
  filter (tasks, filtered);

  // Scan all the tasks for their tags, building a map using tag
  // names as keys.
  std::map <std::string, int> unique;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    std::vector <std::string> tags;
    task->getTags (tags);

    std::vector <std::string>::iterator tag;
    for (tag = tags.begin (); tag != tags.end (); ++tag)
      unique[*tag] = 0;
  }

  // add built-in tags to map
  unique["nocolor"] = 0;
  unique["nonag"]   = 0;
  unique["nocal"]   = 0;
  unique["next"]    = 0;

  std::stringstream out;
  std::map <std::string, int>::iterator it;
  for (it = unique.begin (); it != unique.end (); ++it)
    out << it->first << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
