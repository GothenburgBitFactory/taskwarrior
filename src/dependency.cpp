////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <Context.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
static bool followUpstream (const Task&, const Task&, const std::vector <Task>&,
                            std::vector <std::string>&);

////////////////////////////////////////////////////////////////////////////////
// A task is blocked if it depends on tasks that are pending or waiting.
bool dependencyIsBlocked (Task& task)
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
void dependencyGetBlocked (Task& task, std::vector <Task>& blocked)
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
        blocked.push_back (*it);
  }
}


////////////////////////////////////////////////////////////////////////////////
// To be a blocking task, there must be at least one other task that depends on
// this task, that is either pending or waiting.
bool dependencyIsBlocking (Task& task)
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
void dependencyGetBlocking (Task& task, std::vector <Task>& blocking)
{
  std::string uuid = task.get ("uuid");

  const std::vector <Task>& all = context.tdb.getAllPending ();
  std::vector <Task>::const_iterator it;
  for (it = all.begin (); it != all.end (); ++it)
    if ((it->getStatus () == Task::pending  ||
         it->getStatus () == Task::waiting) &&
        it->has ("depends")                 &&
        it->get ("depends").find (uuid) != std::string::npos)
      blocking.push_back (*it);
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
bool dependencyIsCircular (Task& task)
{
  std::vector <std::string> links;
  const std::vector <Task>& all = context.tdb.getAllPending ();

  return followUpstream (task, task, all, links);
}

////////////////////////////////////////////////////////////////////////////////
// Returns true if a task is encountered twice in a chain walk, and therefore
// indicates circularity.  Recursive.
static bool followUpstream (
  const Task& task,
  const Task& original,
  const std::vector <Task>& all,
  std::vector <std::string>& links)
{
  if (task.has ("depends"))
  {
    std::vector <std::string> uuids;
    split (uuids, task.get ("depends"), ',');

    std::vector <std::string>::iterator outer;
    for (outer = uuids.begin (); outer != uuids.end (); ++outer)
    {
      // Check that link has not already been seen.

      // This is the actual circularity check - the rest of this function is
      // just chain-walking.
      std::string link = task.get ("uuid") + " -> " + *outer;
      if (std::find (links.begin (), links.end (), link) != links.end ())
        return true;

      links.push_back (link);

      // Recurse up the chain.
      std::vector <Task>::const_iterator inner;
      for (inner = all.begin (); inner != all.end (); ++inner)
      {
        if (*outer == inner->get ("uuid"))
        {
          // Use the newly modified "task", not "*inner", which is the old
          // unmodified version.
          if (*outer == original.get ("uuid"))
          {
            if (followUpstream (task, original, all, links))
              return true;
          }
          else
          {
            if (followUpstream (*inner, original, all, links))
              return true;
          }

          break;
        }
      }
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

  std::cout << "# Task " << task.id << "\n";
  foreach (t, blocking)
    std::cout << "#  blocking " << t->id << " " << t->get ("uuid") << "\n";

  // If the task is anything but the tail end of a dependency chain.
  if (blocking.size ())
  {
    std::vector <Task> blocked;
    dependencyGetBlocked (task, blocked);

    foreach (t, blocked)
      std::cout << "#  blocked by " << t->id << " " << t->get ("uuid") << "\n";

    // If there are both blocking and blocked tasks, the chain is broken.
    if (blocked.size ())
    {
      // TODO Nag about broken chain.
      std::cout << "# Chain broken - offer to repair\n";

      // TODO Confirm that the chain should be repaired.

      // Repair the chain - everything in blocked should now depend on
      // everything in blocking, instead of task.id.
      foreach (left, blocked)
      {
        left->removeDependency (task.id);

        foreach (right, blocking)
          left->addDependency (right->id);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void dependencyChainOnStart (Task& task)
{
  std::stringstream out;

  if (context.config.getBoolean ("dependency.reminder") /* &&
       TODO check that task is actually blocked */)
  {
    out << "# dependencyChainScan nag! "
        << task.id
        << " "
        << task.get ("uuid")
        << "\n";

    context.footnote (out.str ());
  }
}

////////////////////////////////////////////////////////////////////////////////
void dependencyChainOnModify (Task& before, Task& after)
{
  // TODO Iff a dependency is being removed, is there anything to do.


}

////////////////////////////////////////////////////////////////////////////////
