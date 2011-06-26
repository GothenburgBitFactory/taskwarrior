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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <Context.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
static bool followUpstream (const Task&, const Task&, std::vector <std::string>&);

////////////////////////////////////////////////////////////////////////////////
// A task is blocked if it depends on tasks that are pending or waiting.
bool dependencyIsBlocked (const Task& task)
{
  if (task.has ("depends"))
  {
    std::string depends = task.get ("depends");
    const std::vector <Task>& all = context.tdb.getAllPending ();
    std::vector <Task>::const_iterator it;
    for (it = all.begin (); it != all.end (); ++it)
      if ((it->getStatus () == Task::pending  ||
           it->getStatus () == Task::waiting) &&
          depends.find (it->get ("uuid")) != std::string::npos)
        return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void dependencyGetBlocked (const Task& task, std::vector <Task>& blocked)
{
  std::string uuid = task.get ("uuid");

  const std::vector <Task>& all = context.tdb.getAllPending ();
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    if ((it->getStatus () == Task::pending  ||
         it->getStatus () == Task::waiting) &&
        it->has ("depends")                 &&
        it->get ("depends").find (uuid) != std::string::npos)
      blocked.push_back (*it);
}


////////////////////////////////////////////////////////////////////////////////
// To be a blocking task, there must be at least one other task that depends on
// this task, that is either pending or waiting.
bool dependencyIsBlocking (const Task& task)
{
  std::string uuid = task.get ("uuid");

  const std::vector <Task>& all = context.tdb.getAllPending ();
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    if ((it->getStatus () == Task::pending  ||
         it->getStatus () == Task::waiting) &&
        it->has ("depends")                 &&
        it->get ("depends").find (uuid) != std::string::npos)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void dependencyGetBlocking (const Task& task, std::vector <Task>& blocking)
{
  std::string depends = task.get ("depends");
  if (depends != "")
  {
    const std::vector <Task>& all = context.tdb.getAllPending ();
    std::vector <Task>::const_iterator it;
    for (it = all.begin (); it != all.end (); ++it)
      if ((it->getStatus () == Task::pending  ||
           it->getStatus () == Task::waiting) &&
          depends.find (it->get ("uuid")) != std::string::npos)
        blocking.push_back (*it);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Terminology:
//   -->    if a depends on b, then it can be said that a --> b
//   Head   if a --> b, then b is the head
//   Tail   if a --> b, then a is the tail
//
// Algorithm:
//   Keep walking the chain, recording the links (a --> b, b --> c, ...) until
//   either the end of the chain is found (therefore not circular), or the chain
//   loops and a repeat link is spotted (therefore circular).
bool dependencyIsCircular (const Task& task)
{
  std::vector <std::string> seen;
  return followUpstream (task, task, seen);
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if a task is encountered twice in a chain walk, and therefore
// indicates circularity.  Recursive.
static bool followUpstream (
  const Task& task,
  const Task& original,
  std::vector <std::string>& seen)
{
  std::vector <Task> blocking;
  dependencyGetBlocking (task, blocking);
  std::vector <Task>::iterator b;
  for (b = blocking.begin (); b != blocking.end (); ++b)
  {
    std::string link = task.get ("uuid") + " -> " + b->get ("uuid");

    // Have we seen this link before?  If so, circularity has been detected.
    if (std::find (seen.begin (), seen.end (), link) != seen.end ())
      return true;

    seen.push_back (link);

    // Use 'original' over '*b' if they both refer to the same task, otherwise
    // '*b' is from TDB's committed list, and lacks recent modifications.
    if (followUpstream (
          (b->get ("uuid") == original.get ("uuid") ? original : *b),
          original,
          seen))
      return true;
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
      std::cout << format (STRING_DEPEND_BLOCKED, task.id)
                << "\n";

      std::vector <Task>::iterator b;
      for (b = blocking.begin (); b != blocking.end (); ++b)
        std::cout << "  " << b->id << " " << b->get ("description") << "\n";
    }

    // If there are both blocking and blocked tasks, the chain is broken.
    if (blocked.size ())
    {
      if (context.config.getBoolean ("dependency.reminder"))
      {
        std::cout << STRING_DEPEND_BLOCKING
                  << "\n";

        std::vector <Task>::iterator b;
        for (b = blocked.begin (); b != blocked.end (); ++b)
          std::cout << "  " << b->id << " " << b->get ("description") << "\n";
      }

      if (!context.config.getBoolean ("dependency.confirmation") ||
          confirm (STRING_DEPEND_FIX_CHAIN))
      {
        // Repair the chain - everything in blocked should now depend on
        // everything in blocking, instead of task.id.
        std::vector <Task>::iterator left;
        std::vector <Task>::iterator right;
        for (left = blocked.begin (); left != blocked.end (); ++left)
        {
          left->removeDependency (task.id);

          for (right = blocking.begin (); right != blocking.end (); ++right)
            left->addDependency (right->id);
        }

        // Now update TDB, now that the updates have all occurred.
        for (left = blocked.begin (); left != blocked.end (); ++left)
          context.tdb.update (*left);

        for (right = blocking.begin (); right != blocking.end (); ++right)
          context.tdb.update (*right);
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
      std::cout << format (STRING_DEPEND_BLOCKED, task.id)
                << "\n";

      std::vector <Task>::iterator b;
      for (b = blocking.begin (); b != blocking.end (); ++b)
        std::cout << "  " << b->id << " " << b->get ("description") << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Iff a dependency is being removed, is there something to do.
void dependencyChainOnModify (Task& before, Task& after)
{
  // TODO It is not clear that this should even happen.  TBD.
/*
  // Get the dependencies from before.
  std::string depends = before.get ("depends");
  std::vector <std::string> before_depends;
  split (before_depends, depends, ',');
  std::cout << "# dependencyChainOnModify before has " << before_depends.size () << "\n";

  // Get the dependencies from after.
  depends = after.get ("depends");
  std::vector <std::string> after_depends;
  split (after_depends, depends, ',');
  std::cout << "# dependencyChainOnModify after has " << after_depends.size () << "\n";

  // listDiff
  std::vector <std::string> before_only;
  std::vector <std::string> after_only;
  listDiff (before_depends, after_depends, before_only, after_only);

  // Any dependencies in before_only indicates that a dependency was removed.
  if (before_only.size ())
  {
    std::cout << "# dependencyChainOnModify detected a dependency removal\n";

    // before   dep:2,3
    // after    dep:2
    // 
    // any tasks blocked by after might should be repaired to depend on 3.

    std::vector <Task> blocked;
    dependencyGetBlocked (after, blocked);

    std::vector <Task>::iterator b;
    for (b = blocked.begin (); b != blocked.end (); ++b)
    {
      std::cout << "# dependencyChainOnModify\n";
    }
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
