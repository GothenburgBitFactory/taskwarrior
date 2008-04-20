////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
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
//   2)    "on bg"
//   3) "fg on bg"
static void parseColorRule (
  const std::string& rule,
  Text::color& fg,
  Text::color& bg)
{
  fg = Text::nocolor;
  bg = Text::nocolor;
  bool error = false;

  std::vector <std::string> words;
  split (words, rule, ' ');
  switch (words.size ())
  {
  case 1: // "fg" - no spaces.
    fg = Text::colorCode (words[0]);
    break;

  case 2: // "on bg" - one space, "on" before.
    if (words[0] == "on")
      bg = Text::colorCode (words[1]);
    else
      error = true;
    break;

  case 3: // "fg on bg" - two spaces, "on" between them.
    if (words[1] == "on")
    {
      fg = Text::colorCode (words[0]);
      bg = Text::colorCode (words[2]);
    }
    else
      error = true;
    break;

  case 0:
  default:
    error = true;
    break;
  }

  if (error)
    std::cout << "Malformed color rule '" << rule << "'" << std::endl;
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
void autoColorize (T& task, Text::color& fg, Text::color& bg)
{
  fg = Text::nocolor;
  bg = Text::nocolor;

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

  // Colorization of the due and overdue.
  std::string due = task.getAttribute ("due");
  if (due != "")
  {
    Date dueDate (::atoi (due.c_str ()));
    Date now;
    Date then (now + 7 * 86400);

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

