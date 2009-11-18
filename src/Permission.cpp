////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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

#include <iostream>
#include "Permission.h"
#include "Context.h"
#include "util.h"
#include "i18n.h"

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Permission::Permission ()
: needConfirmation (false)
, allConfirmed (false)
{
  // Turning confirmations off is the same as entering "all".
  if (context.config.get ("confirmation", true) == false)
    allConfirmed = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Permission::confirmed (const Task& task, const std::string& question)
{
  if (!needConfirmation)
    return true;

  if (allConfirmed)
    return true;

  std::cout << std::endl
            << "Task "
            << task.id
            << " \""
            << task.get ("description")
            << "\"";

  if (task.getStatus () == Task::recurring ||
      task.has ("parent"))
  {
    std::cout << " (Recurring)";
  }

  std::cout << std::endl;

  int answer = confirm3 (question);
  if (answer == 2)
    allConfirmed = true;

  if (answer > 0)
    return true;


  return false;
}

////////////////////////////////////////////////////////////////////////////////
