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

#include <iostream>
#include <Permission.h>
#include <Context.h>
#include <util.h>
#include <text.h>
#include <i18n.h>

#define L10N                                           // Localization complete.

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Permission::Permission ()
: needConfirmation (false)
, allConfirmed (false)
, quit (false)
{
  // Turning confirmations off is the same as entering "all".
  if (context.config.getBoolean ("confirmation") == false)
    allConfirmed = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Permission::confirmed (const Task& task, const std::string& question)
{
  if (quit)
    return false;

  if (!needConfirmation)
    return true;

  if (allConfirmed)
    return true;

  std::cout << "\n"
            << format (STRING_PERM_TASK_LINE, task.id, task.get ("description"));

  if (task.getStatus () == Task::recurring ||
      task.has ("parent"))
  {
    std::cout << " "
              << STRING_PERM_RECURRING;
  }

  std::cout << std::endl;  // Flush.

  int answer = confirm4 (question);
  std::cout << "\n";       // #499

  if (answer == 2)
    allConfirmed = true;

  if (answer == 1 || answer == 2)
    return true;

  if (answer == 3)
    quit = true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
