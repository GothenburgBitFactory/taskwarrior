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
#include <CmdDenotate.h>
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include <main.h>

#define STRING_CMD_DENO_NO           "Task not denotated."
#define STRING_CMD_DENO_1            "Denotated {1} task."
#define STRING_CMD_DENO_N            "Denotated {1} tasks."

////////////////////////////////////////////////////////////////////////////////
CmdDenotate::CmdDenotate ()
{
  _keyword               = "denotate";
  _usage                 = "task <filter> denotate <pattern>";
  _description           = "Deletes an annotation";
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDenotate::execute (std::string&)
{
  auto rc = 0;
  auto count = 0;
  auto sensitive = Context::getContext ().config.getBoolean ("search.case.sensitive");

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  if (filtered.size () == 0)
  {
    Context::getContext ().footnote ("No tasks specified.");
    return 1;
  }

  // Extract all the ORIGINAL MODIFICATION args as simple text patterns.
  std::string pattern = "";
  for (auto& a : Context::getContext ().cli2._args)
  {
    if (a.hasTag ("MISCELLANEOUS"))
    {
      if (pattern != "")
        pattern += ' ';

      pattern += a.attribute ("raw");
    }
  }

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  if(filtered.size() > 1) {
    feedback_affected("This command will alter {1} tasks.", filtered.size());
  }
  for (auto& task : filtered)
  {
    Task before (task);

    auto annotations = task.getAnnotations ();

    if (annotations.size () == 0)
      throw std::string ("The specified task has no annotations that can be deleted.");

    std::string anno;
    auto match = false;
    for (auto i = annotations.begin (); i != annotations.end (); ++i)
    {
      if (i->second == pattern)
      {
        match = true;
        anno = i->second;
        annotations.erase (i);
        task.setAnnotations (annotations);
        break;
      }
    }

    if (! match)
    {
      for (auto i = annotations.begin (); i != annotations.end (); ++i)
      {
        auto loc = find (i->second, pattern, sensitive);
        if (loc != std::string::npos)
        {
          anno = i->second;
          annotations.erase (i);
          task.setAnnotations (annotations);
          break;
        }
      }
    }

    if (before.getAnnotations () != task.getAnnotations ())
    {
      auto question = format ("Denotate task {1} '{2}'?",
                              task.identifier (true),
                              task.get ("description"));

      if (permission (before.diff (task) + question, filtered.size ()))
      {
        ++count;
        Context::getContext ().tdb2.modify (task);
        feedback_affected (format ("Found annotation '{1}' and deleted it.", anno));
        if (Context::getContext ().verbose ("project"))
          projectChanges[task.get ("project")] = onProjectChange (task, false);
      }
      else
      {
        std::cout << STRING_CMD_DENO_NO << '\n';
        rc = 1;
        if (_permission_quit)
          break;
      }
    }
    else
    {
      std::cout << format ("Did not find any matching annotation to be deleted for '{1}'.\n", pattern);
      rc = 1;
    }
  }

  // Now list the project changes.
  for (const auto& change : projectChanges)
    if (change.first != "")
      Context::getContext ().footnote (change.second);

  feedback_affected (count == 1 ? STRING_CMD_DENO_1 : STRING_CMD_DENO_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
