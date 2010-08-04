////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
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
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// void dependencyCheckDangling ();
// bool dependencyRepairNeeded ();
// void dependencyRepairChain ();
// bool dependencyRepairConfirm ();
// void dependencyNag ();

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
// Follow each of the given task's dependencies to the end of the chain, and if
// any duplicates show up, or the chain length exceeds N, stop.
bool dependencyCheckCircular (Task& task)
{
  int maximum = 100;
  std::vector <std::string> all;

  // Must include self if checking for circular.
  all.push_back (task.get ("uuid"));

  // TODO foreach dependency
    // TODO is uuid in all?
      // TODO y: circular!
      // TODO n: add uuid to all.

  return false;
}

////////////////////////////////////////////////////////////////////////////////
