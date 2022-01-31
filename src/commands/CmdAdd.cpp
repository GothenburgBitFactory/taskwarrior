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
#include <CmdAdd.h>
#include <Context.h>
#include <format.h>
#include <main.h>

////////////////////////////////////////////////////////////////////////////////
CmdAdd::CmdAdd ()
{
  _keyword               = "add";
  _usage                 = "task          add <mods>";
  _description           = "Adds a new task";
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = false;
  _accepts_modifications = true;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdAdd::execute (std::string& output)
{
  // Apply the command line modifications to the new task.
  Task task;

  // the task is empty, but DOM references can refer to earlier parts of the
  // command line, e.g., `task add due:20110101 wait:due`.
  task.modify (Task::modReplace, true);
  Context::getContext ().tdb2.add (task);

  // Do not display ID 0, users cannot query by that
  auto status = task.getStatus ();

  // We may have a situation where both new-id and new-uuid config
  // variables are set. In that case, we'll show the new-uuid, as
  // it's enduring and never changes, and it's unlikely the caller
  // asked for this if they just wanted a human-friendly number.

  if (Context::getContext ().verbose ("new-uuid") &&
           status == Task::recurring)
    output += format ("Created task {1} (recurrence template).\n", task.get ("uuid"));

  else if (Context::getContext ().verbose ("new-uuid") ||
          (Context::getContext ().verbose ("new-id") &&
            (status == Task::completed ||
             status == Task::deleted)))
    output += format ("Created task {1}.\n", task.get ("uuid"));

  else if (Context::getContext ().verbose ("new-id") &&
      (status == Task::pending ||
       status == Task::waiting))
    output += format ("Created task {1}.\n", task.id);

  else if (Context::getContext ().verbose ("new-id") &&
           status == Task::recurring)
    output += format ("Created task {1} (recurrence template).\n", task.id);

  if (Context::getContext ().verbose ("project"))
    Context::getContext ().footnote (onProjectChange (task));

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
