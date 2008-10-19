////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include <string>
#include <vector>
#include <map>

#include "Date.h"
#include "task.h"
#include "T.h"

////////////////////////////////////////////////////////////////////////////////
static const char* colors[] =
{
  "bold",
  "underline",
  "bold_underline",

  "black",
  "red",
  "green",
  "yellow",
  "blue",
  "magenta",
  "cyan",
  "white",

  "bold_black",
  "bold_red",
  "bold_green",
  "bold_yellow",
  "bold_blue",
  "bold_magenta",
  "bold_cyan",
  "bold_white",

  "underline_black",
  "underline_red",
  "underline_green",
  "underline_yellow",
  "underline_blue",
  "underline_magenta",
  "underline_cyan",
  "underline_white",

  "bold_underline_black",
  "bold_underline_red",
  "bold_underline_green",
  "bold_underline_yellow",
  "bold_underline_blue",
  "bold_underline_magenta",
  "bold_underline_cyan",
  "bold_underline_white",

  "on_black",
  "on_red",
  "on_green",
  "on_yellow",
  "on_blue",
  "on_magenta",
  "on_cyan",
  "on_white",

  "on_bright_black",
  "on_bright_red",
  "on_bright_green",
  "on_bright_yellow",
  "on_bright_blue",
  "on_bright_magenta",
  "on_bright_cyan",
  "on_bright_white",
  "",
};

static const char* attributes[] =
{
  "project",
  "priority",
  "fg",
  "bg",
  "due",
  "entry",
  "start",
  "end",
  "recur",
  "until",
  "mask",
  "imask",
  "",
};

static const char* commands[] =
{
  "active",
  "add",
  "calendar",
  "colors",
  "completed",
  "delete",
  "done",
  "export",
  "help",
  "history",
  "ghistory",
  "info",
  "list",
  "long",
  "ls",
  "newest",
  "next",
  "oldest",
  "overdue",
  "projects",
  "start",
  "stats",
  "stop",
  "summary",
  "tags",
  "undelete",
  "undo",
  "version",
  "",
};

void guess (const std::string& type, const char** list, std::string& candidate)
{
  std::vector <std::string> options;
  for (int i = 0; list[i][0]; ++i)
    options.push_back (list[i]);

  std::vector <std::string> matches;
  autoComplete (candidate, options, matches);
  if (1 == matches.size ())
    candidate = matches[0];

  else if (0 == matches.size ())
//    throw std::string ("Unrecognized ") + type + " '" + candidate + "'";
    candidate = "";

  else
  {
    std::string error = "Ambiguous ";
    error += type;
    error += " '";
    error += candidate;
    error += "' - could be either of ";
    for (size_t i = 0; i < matches.size (); ++i)
    {
      if (i)
        error += ", ";
      error += matches[i];
    }

    throw error;
  }
}

////////////////////////////////////////////////////////////////////////////////
static bool isCommand (const std::string& candidate)
{
  std::vector <std::string> options;
  for (int i = 0; commands[i][0]; ++i)
    options.push_back (commands[i]);

  std::vector <std::string> matches;
  autoComplete (candidate, options, matches);
  if (0 == matches.size ())
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool validDate (std::string& date, Config& conf)
{
  Date test (date, conf.get ("dateformat", "m/d/Y"));

  char epoch[12];
  sprintf (epoch, "%d", (int) test.toEpoch ());
  date = epoch;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool validPriority (const std::string& input)
{
  if (input != "H" &&
      input != "M" &&
      input != "L" &&
      input != "")
    throw std::string ("\"") +
          input              +
          "\" is not a valid priority.  Use H, M, L or leave blank.";

  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validAttribute (
  std::string& name,
  std::string& value,
  Config& conf)
{
  guess ("attribute", attributes, name);
  if (name != "")
  {
    if ((name == "fg" || name == "bg") && value != "")
      guess ("color", colors, value);

    else if (name == "due" && value != "")
      validDate (value, conf);

    else if (name == "until" && value != "")
      validDate (value, conf);

    else if (name == "priority")
    {
      value = upperCase (value);
      return validPriority (value);
    }

    // Some attributes are intended to be private.
    else if (name == "entry" ||
             name == "start" ||
             name == "end"   ||
             name == "mask"  ||
             name == "imask")
      throw std::string ("\"") +
            name               +
            "\" is not an attribute you may modify directly.";

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool validId (const std::string& input)
{
  for (size_t i = 0; i < input.length (); ++i)
    if (!::isdigit (input[i]))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validTag (const std::string& input)
{
  if ((input[0] == '-' || input[0] == '+') &&
       input.length () > 1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool validDescription (const std::string& input)
{
  if (input.length () > 0)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool validCommand (std::string& input)
{
  std::string copy = input;
  guess ("command", commands, copy);
  if (copy == "")
    return false;

  input = copy;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validSubstitution (
  std::string& input,
  std::string& from,
  std::string& to)
{
  size_t first = input.find ('/');
  if (first != std::string::npos)
  {
    size_t second = input.find ('/', first + 1);
    if (second != std::string::npos)
    {
      size_t third = input.find ('/', second + 1);
      if (third != std::string::npos)
      {
        if (first == 0 &&
            first < second &&
            second < third &&
            third == input.length () - 1)
        {
          from = input.substr (first  + 1, second - first  - 1);
          to   = input.substr (second + 1, third  - second - 1);
          return true;
        }
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool validDuration (std::string& input)
{
  return (convertDuration (input) != 0) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// Token        Distinguishing characteristic
// -------      -----------------------------
// command      first positional
// id           \d+
// description  default, accumulate
// substitution /\w+/\w*/
// tags         [-+]\w+
// attributes   \w+:.+
//
void parse (
  std::vector <std::string>& args,
  std::string& command,
  T& task,
  Config& conf)
{
  command = "";

  std::string descCandidate = "";
  for (size_t i = 0; i < args.size (); ++i)
  {
    std::string arg (args[i]);

    // Ignore any argument that is "rc:...", because that is the command line
    // specified rc file.
    if (arg.substr (0, 3) != "rc:")
    {
      size_t colon;               // Pointer to colon in argument.
      std::string from;
      std::string to;

      // An id is the first argument found that contains all digits.
      if (lowerCase (command) != "add" && // "add" doesn't require an ID
          task.getId () == 0           &&
          validId (arg))
        task.setId (::atoi (arg.c_str ()));

      // Tags begin with + or - and contain arbitrary text.
      else if (validTag (arg))
      {
        if (arg[0] == '+')
          task.addTag (arg.substr (1, std::string::npos));
        else if (arg[0] == '-')
          task.addRemoveTag (arg.substr (1, std::string::npos));
      }

      // Attributes contain a constant string followed by a colon, followed by a
      // value.
      else if ((colon = arg.find (":")) != std::string::npos)
      {
        std::string name  = lowerCase (arg.substr (0, colon));
        std::string value = arg.substr (colon + 1, std::string::npos);

        if (validAttribute (name, value, conf))
        {
          if (name != "recur" || validDuration (value))
            task.setAttribute (name, value);
        }

        // If it is not a valid attribute, then allow the argument as part of
        // the description.
        else
          descCandidate += arg;
      }

      // Substitution of description text.
      else if (validSubstitution (arg, from, to))
      {
        task.setSubstitution (from, to);
      }

      // Command.
      else if (command == "")
      {
        std::string l = lowerCase (arg);
        if (isCommand (l) && validCommand (l))
          command = l;
        else
          descCandidate += arg;
      }

      // Anything else is just considered description.
      else
        descCandidate += std::string (arg) + " ";
    }
  }

  if (task.getAttribute ("recur") != "" &&
      task.getAttribute ("due")   == "")
    throw std::string ("You cannot specify a recurring task without a due date.");

  if (task.getAttribute ("until") != "" &&
      task.getAttribute ("recur") == "")
    throw std::string ("You cannot specify an until date for a non-recurring task.");

  if (validDescription (descCandidate))
    task.setDescription (descCandidate);
}

////////////////////////////////////////////////////////////////////////////////

