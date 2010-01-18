////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include "Hooks.h"

////////////////////////////////////////////////////////////////////////////////
Hooks::Hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
Hooks::~Hooks ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Hooks::initialize ()
{
  api.initialize ();
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::trigger (const std::string& event)
{
#ifdef HAVE_LIBLUA
  // TODO Look up scripts/functions hooking this event.
  // TODO Load the scripts if necessary.

  // TODO Call each function.
  std::string type;
  if (eventType (event, type))
  {
         if (type == "program") return triggerProgramEvent (event);
    else if (type == "list")    return triggerListEvent    (event);
    else if (type == "task")    return triggerTaskEvent    (event);
    else if (type == "field")   return triggerFieldEvent   (event);
  }
  else
    throw std::string ("Unrecognized hook event '") + event + "'";
#endif

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::eventType (const std::string& event, std::string& type)
{
  if (event == "post-start" ||
      event == "pre-exit")
  {
    type = "program";
    return true;
  }
  else if (event == "?")
  {
    type = "list";
    return true;
  }
  else if (event == "?")
  {
    type = "task";
    return true;
  }
  else if (event == "?")
  {
    type = "field";
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_LIBLUA

bool Hooks::triggerProgramEvent (const std::string& event)
{
  std::cout << "Hooks::triggerProgramEvent " << event << std::endl;

  // TODO Is this event hooked?
  // TODO Is the associated script loaded?
  // TODO Call the function
  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::triggerListEvent (const std::string& event)
{
  std::cout << "Hooks::triggerListEvent " << event << std::endl;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::triggerTaskEvent (const std::string& event)
{
  std::cout << "Hooks::triggerTaskEvent " << event << std::endl;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Hooks::triggerFieldEvent (const std::string& event)
{
  std::cout << "Hooks::triggerFieldEvent " << event << std::endl;
  return true;
}

#endif
////////////////////////////////////////////////////////////////////////////////
