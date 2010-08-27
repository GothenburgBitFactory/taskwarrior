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
// void dependencyCheckDangling ();
// bool dependencyRepairNeeded ();
// void dependencyRepairChain ();
// bool dependencyRepairConfirm ();
// void dependencyNag ();
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
//   Find all tails, ie tasks that have dependencies, with no other tasks that
//   are dependent on them.
//
//   For each tail:
//     follow the chain, recording all linkages, ie a --> b, b --> c.  If a
//     linkage appears that has already occurred in this chain => circularity.
//
bool dependencyIsCircular (Task& task)
{
  std::vector <std::string> links;
  const std::vector <Task>& all = context.tdb.getAllPending ();

  return followUpstream (task, task, all, links);
}

////////////////////////////////////////////////////////////////////////////////
// To follow dependencies upstream, follow the heads.
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
