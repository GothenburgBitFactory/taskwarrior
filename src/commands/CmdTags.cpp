////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <Context.h>
#include <Filter.h>
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
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (tasks, filtered);

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
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

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
