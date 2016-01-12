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
#include <CmdDuplicate.h>
#include <iostream>
#include <Context.h>
#include <Filter.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDuplicate::CmdDuplicate ()
{
  _keyword               = "duplicate";
  _usage                 = "task <filter> duplicate <mods>";
  _description           = STRING_CMD_DUPLICATE_USAGE;
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = true;
  _accepts_filter        = true;
  _accepts_modifications = true;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdDuplicate::execute (std::string&)
{
  int rc = 0;
  int count = 0;

  // Apply filter.
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  if (filtered.size () == 0)
  {
    context.footnote (STRING_FEEDBACK_NO_TASKS_SP);
    return 1;
  }

  // Accumulated project change notifications.
  std::map <std::string, std::string> projectChanges;

  for (auto& task : filtered)
  {
    // Duplicate the specified task.
    Task dup (task);
    dup.id = 0;                    // Reset, and TDB2::add will set.
    dup.set ("uuid", uuid ());     // Needs a new UUID.
    dup.remove ("start");          // Does not inherit start date.
    dup.remove ("end");            // Does not inherit end date.
    dup.remove ("entry");          // Does not inherit entry date.

    // When duplicating a child task, downgrade it to a plain task.
    if (dup.has ("parent"))
    {
      dup.remove ("parent");
      dup.remove ("recur");
      dup.remove ("until");
      dup.remove ("imask");
      std::cout << format (STRING_CMD_DUPLICATE_NON_REC, task.identifier ())
          << "\n";
    }

    // When duplicating a parent task, create a new parent task.
    else if (dup.getStatus () == Task::recurring)
    {
      dup.remove ("mask");
      std::cout << format (STRING_CMD_DUPLICATE_REC, task.identifier ())
          << "\n";
    }

    dup.setStatus (Task::pending); // Does not inherit status.
                                   // Must occur after Task::recurring check.

    dup.modify (Task::modAnnotate);

    if (permission (format (STRING_CMD_DUPLICATE_CONFIRM,
                            task.identifier (true),
                            task.get ("description")),
                    filtered.size ()))
    {
      context.tdb2.add (dup);
      ++count;
      feedback_affected (STRING_CMD_DUPLICATE_TASK, task);

      auto status = dup.getStatus ();
      if (context.verbose ("new-id") &&
          (status == Task::pending ||
           status == Task::waiting))
        std::cout << format (STRING_CMD_ADD_FEEDBACK, dup.id) + "\n";

      else if (context.verbose ("new-uuid") &&
               status != Task::recurring)
        std::cout << format (STRING_CMD_ADD_FEEDBACK, dup.get ("uuid")) + "\n";

      if (context.verbose ("project"))
        projectChanges[task.get ("project")] = onProjectChange (task);
    }
    else
    {
      std::cout << STRING_CMD_DUPLICATE_NO << "\n";
      rc = 1;
      if (_permission_quit)
        break;
    }
  }

  // Now list the project changes.
  for (auto& change : projectChanges)
    if (change.first != "")
      context.footnote (change.second);

  feedback_affected (count == 1 ? STRING_CMD_DUPLICATE_1 : STRING_CMD_DUPLICATE_N, count);
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
