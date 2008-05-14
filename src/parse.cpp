////////////////////////////////////////////////////////////////////////////////
// Copyright 2006 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Date.h"
#include "task.h"
#include "T.h"

////////////////////////////////////////////////////////////////////////////////
static char* colors[] =
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

static char* attributes[] =
{
  "project",
  "priority",
  "fg",
  "bg",
  "due",
  "entry",
  "start",
  "end",
  "",
};

static char* commands[] =
{
  "active",
  "add",
  "calendar",
  "completed",
  "delete",
  "done",
  "export",
  "history",
  "info",
  "list",
  "long",
  "ls",
  "next",
  "overdue",
  "projects",
  "start",
  "stats",
  "summary",
  "tags",
  "usage",
  "version",
  "",
};

void guess (const std::string& type, char** list, std::string& candidate)
{
  std::vector <std::string> options;
  for (int i = 0; list[i][0]; ++i)
    options.push_back (list[i]);

  std::vector <std::string> matches;
  autoComplete (candidate, options, matches);
  if (1 == matches.size ())
    candidate = matches[0];

  else if (0 == matches.size ())
    throw std::string ("Unrecognized ") + type + " '" + candidate + "'";

  else
  {
    std::string error = "Ambiguous ";
    error += type;
    error += " '";
    error += candidate;
    error += "' - could be either of ";
    for (unsigned int i = 0; i < matches.size (); ++i)
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
bool validDate (std::string& date)
{
  unsigned int firstSlash  = date.find ("/");
  unsigned int secondSlash = date.find ("/", firstSlash + 1);
  if (firstSlash != std::string::npos &&
      secondSlash != std::string::npos)
  {
    int m = ::atoi (date.substr (0,               firstSlash              ).c_str ());
    int d = ::atoi (date.substr (firstSlash  + 1, secondSlash - firstSlash).c_str ());
    int y = ::atoi (date.substr (secondSlash + 1, std::string::npos       ).c_str ());
    if (!Date::valid (m, d, y))
      throw std::string ("\"") + date + "\" is not a valid date.";

    // Convert to epoch form.
    Date dt (m, d, y);
    time_t t;
    dt.toEpoch (t);
    char converted[12];
    sprintf (converted, "%u", (unsigned int) t);
    date = converted;
  }
  else
    throw std::string ("Badly formed date - use the MM/DD/YYYY format");

  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validPriority (std::string& input)
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
static bool validAttribute (std::string& name, std::string& value)
{
  guess ("attribute", attributes, name);

  if ((name == "fg" || name == "bg") && value != "")
    guess ("color", colors, value);

  else if (name == "due" && value != "")
    validDate (value);

  else if (name == "priority")
  {
    for (std::string::iterator i = value.begin (); i != value.end (); ++i)
      *i = ::toupper (*i);

    return validPriority (value);
  }

  else if (name == "entry" ||
           name == "start" ||
           name == "end")
    throw std::string ("\"") +
          name              +
          "\" is not an attribute you may modify directly.";

  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validId (const std::string& input)
{
  for (unsigned int i = 0; i < input.length (); ++i)
    if (!::isdigit (input[i]))
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validTag (std::string& input)
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
  guess ("command", commands, input);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
static bool validSubstitution (
  std::string& input,
  std::string& from,
  std::string& to)
{
  unsigned int first = input.find ('/');
  if (first != std::string::npos)
  {
    unsigned int second = input.find ('/', first + 1);
    if (second != std::string::npos)
    {
      unsigned int third = input.find ('/', second + 1);
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
  T& task)
{
  command = "";

  std::string descCandidate = "";
  for (unsigned int i = 0; i < args.size (); ++i)
  {
    std::string arg (args[i]);
    unsigned int colon;               // Pointer to colon in argument.
    std::string from;
    std::string to;

    // An id is the first argument found that contains all digits.
    if (command != "add"   && // "add" doesn't require an ID
        task.getId () == 0 &&
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
      std::string name  = arg.substr (0, colon);
      std::string value = arg.substr (colon + 1, std::string::npos);

      if (validAttribute (name, value))
        task.setAttribute (name, value);
    }

    // Substitution of description text.
    else if (validSubstitution (arg, from, to))
    {
      task.setSubstitution (from, to);
    }

    // Command.
    else if (command == "")
    {
      if (!isCommand (arg))
        descCandidate += std::string (arg) + " ";
      else if (validCommand (arg))
        command = arg;
    }

    // Anything else is just considered description.
    else
      descCandidate += std::string (arg) + " ";
  }

  if (validDescription (descCandidate))
    task.setDescription (descCandidate);
}

////////////////////////////////////////////////////////////////////////////////

