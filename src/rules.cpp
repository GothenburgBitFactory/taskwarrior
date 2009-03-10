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
#include "Config.h"
#include "Table.h"
#include "Date.h"
#include "T.h"
#include "task.h"

static std::map <std::string, Text::color> gsFg;
static std::map <std::string, Text::color> gsBg;

////////////////////////////////////////////////////////////////////////////////
// There are three supported variants:
//   1) "fg"
//   2) "bg"
//   3) "fg bg"
static void parseColorRule (
  const std::string& rule,
  Text::color& fg,
  Text::color& bg)
{
  fg = Text::nocolor;
  bg = Text::nocolor;

  std::vector <std::string> words;
  split (words, rule, ' ');

  std::vector <std::string>::iterator it;
  for (it = words.begin (); it != words.end (); ++it)
  {
    if (it->substr (0, 3) == "on_")
      bg = Text::colorCode (*it);
    else
      fg = Text::colorCode (*it);
  }
}

////////////////////////////////////////////////////////////////////////////////
void initializeColorRules (Config& conf)
{
  std::vector <std::string> ruleNames;
  conf.all (ruleNames);
  foreach (it, ruleNames)
  {
    if (it->substr (0, 6) == "color.")
    {
      Text::color fg;
      Text::color bg;
      parseColorRule (conf.get (*it), fg, bg);
      gsFg[*it] = fg;
      gsBg[*it] = bg;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void autoColorize (
  T& task,
  Text::color& fg,
  Text::color& bg,
  Config& conf)
{
  // Note: fg, bg already contain colors specifically assigned via command.
  // Note: These rules form a hierarchy - the last rule is king.

  // Colorization of the recurring.
  if (gsFg["color.recurring"] != Text::nocolor ||
      gsBg["color.recurring"] != Text::nocolor)
  {
    if (task.getAttribute ("recur") != "")
    {
      fg = gsFg["color.recurring"];
      bg = gsBg["color.recurring"];
    }
  }

  // Colorization of the tagged.
  if (gsFg["color.tagged"] != Text::nocolor ||
      gsBg["color.tagged"] != Text::nocolor)
  {
    std::vector <std::string> tags;
    task.getTags (tags);
    if (tags.size ())
    {
      fg = gsFg["color.tagged"];
      bg = gsBg["color.tagged"];
    }
  }

  // Colorization of the low priority.
  if (gsFg["color.pri.L"] != Text::nocolor ||
      gsBg["color.pri.L"] != Text::nocolor)
  {
    if (task.getAttribute ("priority") == "L")
    {
      fg = gsFg["color.pri.L"];
      bg = gsBg["color.pri.L"];
    }
  }

  // Colorization of the medium priority.
  if (gsFg["color.pri.M"] != Text::nocolor ||
      gsBg["color.pri.M"] != Text::nocolor)
  {
    if (task.getAttribute ("priority") == "M")
    {
      fg = gsFg["color.pri.M"];
      bg = gsBg["color.pri.M"];
    }
  }

  // Colorization of the high priority.
  if (gsFg["color.pri.H"] != Text::nocolor ||
      gsBg["color.pri.H"] != Text::nocolor)
  {
    if (task.getAttribute ("priority") == "H")
    {
      fg = gsFg["color.pri.H"];
      bg = gsBg["color.pri.H"];
    }
  }

  // Colorization of the priority-less.
  if (gsFg["color.pri.none"] != Text::nocolor ||
      gsBg["color.pri.none"] != Text::nocolor)
  {
    if (task.getAttribute ("priority") == "")
    {
      fg = gsFg["color.pri.none"];
      bg = gsBg["color.pri.none"];
    }
  }

  // Colorization of the active.
  if (gsFg["color.active"] != Text::nocolor ||
      gsBg["color.active"] != Text::nocolor)
  {
    if (task.getAttribute ("start") != "")
    {
      fg = gsFg["color.active"];
      bg = gsBg["color.active"];
    }
  }

  // Colorization by tag value.
  std::map <std::string, Text::color>::iterator it;
  for (it = gsFg.begin (); it != gsFg.end (); ++it)
  {
    if (it->first.substr (0, 10) == "color.tag.")
    {
      std::string value = it->first.substr (10, std::string::npos);
      if (task.hasTag (value))
      {
        fg = gsFg[it->first];
        bg = gsBg[it->first];
      }
    }
  }

  // Colorization by project name.
  for (it = gsFg.begin (); it != gsFg.end (); ++it)
  {
    if (it->first.substr (0, 14) == "color.project.")
    {
      std::string value = it->first.substr (14, std::string::npos);
      if (task.getAttribute ("project") == value)
      {
        fg = gsFg[it->first];
        bg = gsBg[it->first];
      }
    }
  }

  // Colorization by keyword.
  for (it = gsFg.begin (); it != gsFg.end (); ++it)
  {
    if (it->first.substr (0, 14) == "color.keyword.")
    {
      std::string value = lowerCase (it->first.substr (14, std::string::npos));
      std::string desc  = lowerCase (task.getDescription ());
      if (desc.find (value) != std::string::npos)
      {
        fg = gsFg[it->first];
        bg = gsBg[it->first];
      }
    }
  }

  // Colorization of the due and overdue.
  std::string due = task.getAttribute ("due");
  if (due != "")
  {
    Date dueDate (::atoi (due.c_str ()));
    Date now;
    Date then (now + conf.get ("due", 7) * 86400);

    // Overdue
    if (dueDate < now)
    {
      fg = gsFg["color.overdue"];
      bg = gsBg["color.overdue"];
    }

    // Imminent
    else if (dueDate < then)
    {
      fg = gsFg["color.due"];
      bg = gsBg["color.due"];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

