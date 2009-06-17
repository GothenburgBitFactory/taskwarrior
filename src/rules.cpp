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
void initializeColorRules ()
{
  std::vector <std::string> ruleNames;
  context.config.all (ruleNames);
  foreach (it, ruleNames)
  {
    if (it->substr (0, 6) == "color.")
    {
      Text::color fg;
      Text::color bg;
      parseColorRule (context.config.get (*it), fg, bg);
      gsFg[*it] = fg;
      gsBg[*it] = bg;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void autoColorize (
  Task& task,
  Text::color& fg,
  Text::color& bg)
{
  // Note: fg, bg already contain colors specifically assigned via command.
  // Note: These rules form a hierarchy - the last rule is King.

  // Colorization of the tagged.
  if (gsFg["color.tagged"] != Text::nocolor ||
      gsBg["color.tagged"] != Text::nocolor)
  {
    if (task.getTagCount ())
    {
      fg = gsFg["color.tagged"];
      bg = gsBg["color.tagged"];
    }
  }

  // Colorization of the low priority.
  if (gsFg["color.pri.L"] != Text::nocolor ||
      gsBg["color.pri.L"] != Text::nocolor)
  {
    if (task.get ("priority") == "L")
    {
      fg = gsFg["color.pri.L"];
      bg = gsBg["color.pri.L"];
    }
  }

  // Colorization of the medium priority.
  if (gsFg["color.pri.M"] != Text::nocolor ||
      gsBg["color.pri.M"] != Text::nocolor)
  {
    if (task.get ("priority") == "M")
    {
      fg = gsFg["color.pri.M"];
      bg = gsBg["color.pri.M"];
    }
  }

  // Colorization of the high priority.
  if (gsFg["color.pri.H"] != Text::nocolor ||
      gsBg["color.pri.H"] != Text::nocolor)
  {
    if (task.get ("priority") == "H")
    {
      fg = gsFg["color.pri.H"];
      bg = gsBg["color.pri.H"];
    }
  }

  // Colorization of the priority-less.
  if (gsFg["color.pri.none"] != Text::nocolor ||
      gsBg["color.pri.none"] != Text::nocolor)
  {
    if (task.get ("priority") == "")
    {
      fg = gsFg["color.pri.none"];
      bg = gsBg["color.pri.none"];
    }
  }

  // Colorization of the active.
  if (gsFg["color.active"] != Text::nocolor ||
      gsBg["color.active"] != Text::nocolor)
  {
    if (task.has ("start"))
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
      if (task.get ("project") == value)
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
      std::string desc  = lowerCase (task.get ("description"));
      if (desc.find (value) != std::string::npos)
      {
        fg = gsFg[it->first];
        bg = gsBg[it->first];
      }
    }
  }

  // Colorization of the due and overdue.
  if (task.has ("due"))
  {
    std::string due = task.get ("due");
    switch (getDueState (due))
    {
    case 1: // imminent
      fg = gsFg["color.due"];
      bg = gsBg["color.due"];
      break;

    case 2: // overdue
      fg = gsFg["color.overdue"];
      bg = gsBg["color.overdue"];
      break;

    case 0: // not due at all
    default:
      break;
    }
  }

  // Colorization of the recurring.
  if (gsFg["color.recurring"] != Text::nocolor ||
      gsBg["color.recurring"] != Text::nocolor)
  {
    if (task.has ("recur"))
    {
      fg = gsFg["color.recurring"];
      bg = gsBg["color.recurring"];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeHeader (const std::string& input)
{
  if (gsFg["color.header"] != Text::nocolor ||
      gsBg["color.header"] != Text::nocolor)
  {
    return Text::colorize (
           gsFg["color.header"],
           gsBg["color.header"],
           input);
  }

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeMessage (const std::string& input)
{
  if (gsFg["color.message"] != Text::nocolor ||
      gsBg["color.message"] != Text::nocolor)
  {
    return Text::colorize (
           gsFg["color.message"],
           gsBg["color.message"],
           input);
  }

  return input;
}

////////////////////////////////////////////////////////////////////////////////
std::string colorizeFootnote (const std::string& input)
{
  if (gsFg["color.footnote"] != Text::nocolor ||
      gsBg["color.footnote"] != Text::nocolor)
  {
    return Text::colorize (
           gsFg["color.footnote"],
           gsBg["color.footnote"],
           input);
  }

  return input;
}

////////////////////////////////////////////////////////////////////////////////

