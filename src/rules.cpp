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
#include <stdlib.h>
#include "Context.h"
#include "Table.h"
#include "Date.h"
#include "text.h"
#include "util.h"
#include "main.h"

extern Context context;

static std::map <std::string, Color> gsColor;

////////////////////////////////////////////////////////////////////////////////
void initializeColorRules ()
{
  std::vector <std::string> ruleNames;
  context.config.all (ruleNames);
  foreach (it, ruleNames)
  {
    if (it->substr (0, 6) == "color.")
    {
      Color c (context.config.get (*it));
      gsColor[*it] = c;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void autoColorize (Task& task, Color& c)
{
  // Note: fg, bg already contain colors specifically assigned via command.
  // Note: These rules form a hierarchy - the last rule is King.

  // Colorization of the tagged.
  if (gsColor["color.tagged"].nontrivial ())
    if (task.getTagCount ())
      c.blend (gsColor["color.tagged"]);

  // Colorization of the low priority.
  if (gsColor["color.pri.L"].nontrivial ())
    if (task.get ("priority") == "L")
      c.blend (gsColor["color.pri.L"]);

  // Colorization of the medium priority.
  if (gsColor["color.pri.M"].nontrivial ())
    if (task.get ("priority") == "M")
      c.blend (gsColor["color.pri.M"]);

  // Colorization of the high priority.
  if (gsColor["color.pri.H"].nontrivial ())
    if (task.get ("priority") == "H")
      c.blend (gsColor["color.pri.H"]);

  // Colorization of the priority-less.
  if (gsColor["color.pri.none"].nontrivial ())
    if (task.get ("priority") == "")
      c.blend (gsColor["color.pri.none"]);

  // Colorization of the active.
  if (gsColor["color.active"].nontrivial ())
    if (task.has ("start"))
      c.blend (gsColor["color.active"]);

  // Colorization by tag value.
  std::map <std::string, Color>::iterator it;
  for (it = gsColor.begin (); it != gsColor.end (); ++it)
  {
    if (it->first.substr (0, 10) == "color.tag.")
    {
      std::string value = it->first.substr (10, std::string::npos);
      if (task.hasTag (value))
        c.blend (it->second);
    }
  }

  // Colorization by project name.
  for (it = gsColor.begin (); it != gsColor.end (); ++it)
  {
    if (it->first.substr (0, 14) == "color.project.")
    {
      std::string value = it->first.substr (14, std::string::npos);
      if (task.get ("project") == value)
        c.blend (it->second);
    }
  }

  // Colorization by keyword.
  for (it = gsColor.begin (); it != gsColor.end (); ++it)
  {
    if (it->first.substr (0, 14) == "color.keyword.")
    {
      std::string value = lowerCase (it->first.substr (14, std::string::npos));
      std::string desc  = lowerCase (task.get ("description"));
      if (desc.find (value) != std::string::npos)
        c.blend (it->second);
    }
  }

  // Colorization of the due and overdue.
  if (task.has ("due"))
  {
    std::string due = task.get ("due");
    switch (getDueState (due))
    {
    case 1: // imminent
      c.blend (gsColor["color.due"]);
      break;

    case 2: // overdue
      c.blend (gsColor["color.overdue"]);
      break;

    case 0: // not due at all
    default:
      break;
    }
  }

  // Colorization of the recurring.
  if (gsColor["color.recurring"].nontrivial ())
    if (task.has ("recur"))
      c.blend (gsColor["color.recurring"]);
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeHeader (const std::string& input)
{
  if (gsColor["color.header"].nontrivial ())
    return gsColor["color.header"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeMessage (const std::string& input)
{
  if (gsColor["color.message"].nontrivial ())
    return gsColor["color.message"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeFootnote (const std::string& input)
{
  if (gsColor["color.footnote"].nontrivial ())
    return gsColor["color.footnote"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeDebug (const std::string& input)
{
  if (gsColor["color.debug"].nontrivial ())
    return gsColor["color.debug"].colorize (input);

  return input;
}

////////////////////////////////////////////////////////////////////////////////

