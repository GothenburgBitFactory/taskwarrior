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
#include <Context.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
static bool followUpstream (const Task&, const Task&, const std::vector <Task>&, std::vector <std::string>&);

////////////////////////////////////////////////////////////////////////////////
// All it takes to be blocked is to depend on another task.
bool dependencyIsBlocked (Task& task)
{
  return task.has ("depends");
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
bool dependencyChainBroken (Task& task)
{
  if (task.has ("depends"))
  {
    std::cout << "# chain broken\n";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Generate a nag message if a dependency chain is being violated.
void dependencyNag (Task& task)
{
  std::cout << "# dependencyNag "
            << task.id
            << " "
            << task.get ("uuid")
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
