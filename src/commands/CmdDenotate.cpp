////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>
#include <CmdDenotate.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDenotate::CmdDenotate ()
{
  _keyword     = "denotate";
  _usage       = "task <filter> denotate <pattern>";
  _description = STRING_CMD_DENO_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDenotate::execute (std::string& output)
{
  int rc = 0;
  int count = 0;
  bool sensitive = context.config.getBoolean ("search.case.sensitive");

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Extract all the ORIGINAL MODIFICATION args as simple text patterns.
  std::string pattern = "";
  std::vector <A>::iterator a;
  for (a = context.cli._args.begin (); a != context.cli._args.end (); ++a)
  {
    if (a->hasTag ("ORIGINAL") &&
        a->hasTag ("MODIFICATION"))
    {
      if (pattern != "")
        pattern += ' ';

      pattern += a->attribute ("raw");
    }
  }

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
  {
    Task before (*task);

    std::map <std::string, std::string> annotations;
    task->getAnnotations (annotations);

    if (annotations.size () == 0)
      throw std::string (STRING_CMD_DENO_NONE);

    std::map <std::string, std::string>::iterator i;
    std::string anno;
    bool match = false;
    for (i = annotations.begin (); i != annotations.end (); ++i)
    {
      if (i->second == pattern)
      {
        match = true;
        anno = i->second;
        annotations.erase (i);
        task->setAnnotations (annotations);
        break;
      }
    }
    if (! match)
    {
      for (i = annotations.begin (); i != annotations.end (); ++i)
      {
        std::string::size_type loc = find (i->second, pattern, sensitive);
        if (loc != std::string::npos)
        {
          anno = i->second;
          annotations.erase (i);
          task->setAnnotations (annotations);
          break;
        }
      }
    }

    if (taskDiff (before, *task))
    {
      std::string question = format (STRING_CMD_DENO_CONFIRM,
                                     task->id,
                                     task->get ("description"));

      if (permission (*task, taskDifferences (before, *task) + question, filtered.size ()))
      {
        ++count;
        context.tdb2.modify (*task);
        feedback_affected (format (STRING_CMD_DENO_FOUND, anno));
        if (context.verbose ("project"))
          projectChanges[task->get ("project")] = onProjectChange (*task, false);
      }
      else
      {
        std::cout << STRING_CMD_DENO_NO << "\n";
        rc = 1;
        if (_permission_quit)
          break;
      }
    }
    else
    {
      std::cout << format (STRING_CMD_DENO_NOMATCH, pattern) << "\n";
      rc = 1;
    }
  }

  // Now list the project changes.
  std::map <std::string, std::string>::iterator i;
  for (i = projectChanges.begin (); i != projectChanges.end (); ++i)
    if (i->first != "")
      context.footnote (i->second);

  feedback_affected (count == 1 ? STRING_CMD_DENO_1 : STRING_CMD_DENO_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
