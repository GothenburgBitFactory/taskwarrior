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
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stack>
#include <Context.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
void dependencyGetBlocked (const Task& task, std::vector <Task>& blocked)
{
  std::string uuid = task.get ("uuid");

  auto all = context.tdb2.pending.get_tasks ();
  for (auto& it : all)
    if (it.getStatus () != Task::completed &&
        it.getStatus () != Task::deleted   &&
        it.has ("depends")                 &&
        it.get ("depends").find (uuid) != std::string::npos)
      blocked.push_back (it);
}

////////////////////////////////////////////////////////////////////////////////
void dependencyGetBlocking (const Task& task, std::vector <Task>& blocking)
{
  std::string depends = task.get ("depends");
  if (depends != "")
    for (auto& it : context.tdb2.pending.get_tasks ())
      if (it.getStatus () != Task::completed &&
          it.getStatus () != Task::deleted   &&
          depends.find (it.get ("uuid")) != std::string::npos)
        blocking.push_back (it);
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if the supplied task adds a cycle to the dependency chain.
bool dependencyIsCircular (const Task& task)
{

  // A new task has no UUID assigned yet, and therefore cannot be part of any
  // dependency chain.
  if (task.has ("uuid"))
  {
    auto task_uuid = task.get ("uuid");

    std::stack <Task> s;
    s.push (task);
    while (! s.empty ())
    {
      Task& current = s.top ();
      std::vector <std::string> deps_current;
      current.getDependencies (deps_current);

      // This is a basic depth first search that always terminates given the
      // assumption that any cycles in the dependency graph must have been
      // introduced by the task that is being checked.
      // Since any previous cycles would have been prevented by this very
      // function, this is a reasonable assumption.
      for (unsigned int i = 0; i < deps_current.size (); i++)
      {
        if (context.tdb2.get (deps_current[i], current))
        {
          if (task_uuid == current.get ("uuid"))
          {
            // Cycle found, initial task reached for the second time!
            return true;
          }

          s.push (current);
        }
      }

      s.pop ();
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Determine whether a dependency chain is being broken, assuming that 'task' is
// either completed or deleted.
//
//   blocked task blocking action
//   ------- ---- -------- -----------------------------
//           [1]  2        Chain broken
//                         Nag message generated
//                         Repair offered:  1 dep:-2
//
//           [1]  2        Chain broken
//                3        Nag message generated
//                         Repair offered:  1 dep:-2,-3
//
//   1       [2]           -
//
//   1,3     [2]           -
//
//   1       [2]  3        Chain broken
//                         Nag message generated
//                         Repair offered:  2 dep:-3
//                                          1 dep:-2,3
//
//   1,4     [2]  3,5      Chain broken
//                         Nag message generated
//                         Repair offered:  2 dep:-3,-5
//                                          1 dep:3,5
//                                          4 dep:3,5
//
void dependencyChainOnComplete (Task& task)
{
  std::vector <Task> blocking;
  dependencyGetBlocking (task, blocking);

  // If the task is anything but the tail end of a dependency chain.
  if (blocking.size ())
  {
    std::vector <Task> blocked;
    dependencyGetBlocked (task, blocked);

    // Nag about broken chain.
    if (context.config.getBoolean ("dependency.reminder"))
    {
      std::cout << format (STRING_DEPEND_BLOCKED, task.identifier ())
                << "\n";

      for (auto& b : blocking)
        std::cout << "  " << b.id << " " << b.get ("description") << "\n";
    }

    // If there are both blocking and blocked tasks, the chain is broken.
    if (blocked.size ())
    {
      if (context.config.getBoolean ("dependency.reminder"))
      {
        std::cout << STRING_DEPEND_BLOCKING
                  << "\n";

        for (auto& b : blocked)
          std::cout << "  " << b.id << " " << b.get ("description") << "\n";
      }

      if (!context.config.getBoolean ("dependency.confirmation") ||
          confirm (STRING_DEPEND_FIX_CHAIN))
      {
        // Repair the chain - everything in blocked should now depend on
        // everything in blocking, instead of task.id.
        for (auto& left : blocked)
        {
          left.removeDependency (task.id);

          for (auto& right : blocking)
            left.addDependency (right.id);
        }

        // Now update TDB2, now that the updates have all occurred.
        for (auto& left : blocked)
          context.tdb2.modify (left);

        for (auto& right : blocking)
          context.tdb2.modify (right);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void dependencyChainOnStart (Task& task)
{
  if (context.config.getBoolean ("dependency.reminder"))
  {
    std::vector <Task> blocking;
    dependencyGetBlocking (task, blocking);

    // If the task is anything but the tail end of a dependency chain, nag about
    // broken chain.
    if (blocking.size ())
    {
      std::cout << format (STRING_DEPEND_BLOCKED, task.identifier ())
                << "\n";

      for (auto& b : blocking)
        std::cout << "  " << b.id << " " << b.get ("description") << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
