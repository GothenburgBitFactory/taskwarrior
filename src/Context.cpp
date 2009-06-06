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
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include "Context.h"
#include "text.h"
#include "util.h"
#include "task.h"
#include "i18n.h"
#include "../auto.h"

////////////////////////////////////////////////////////////////////////////////
Context::Context ()
: config ()
, filter ()
, keymap ()
, sequence ()
, task ()
, tdb ()
, stringtable ()
, command ("")
{
  // Set up randomness.
#ifdef HAVE_SRANDOM
  srandom (time (NULL));
#else
  srand (time (NULL));
#endif
}

////////////////////////////////////////////////////////////////////////////////
Context::Context (const Context& other)
{
  throw std::string ("unimplemented Context::Context");
  config      = other.config;
  filter      = other.filter;
  keymap      = other.keymap;
  sequence    = other.sequence;
  task        = other.task;
  tdb         = other.tdb;
  stringtable = other.stringtable;
  args        = other.args;
  command     = other.command;
  messages    = other.messages;
  footnotes   = other.footnotes;
}

////////////////////////////////////////////////////////////////////////////////
Context& Context::operator= (const Context& other)
{
  throw std::string ("unimplemented Context::operator=");
  if (this != &other)
  {
    config      = other.config;
    filter      = other.filter;
    keymap      = other.keymap;
    sequence    = other.sequence;
    task        = other.task;
    tdb         = other.tdb;
    stringtable = other.stringtable;
    args        = other.args;
    command     = other.command;
    messages    = other.messages;
    footnotes   = other.footnotes;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Context::~Context ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Context::initialize (int argc, char** argv)
{
  // Capture the args.
  for (int i = 0; i < argc; ++i)
    args.push_back (argv[i]);

  // Load the configuration file from the home directory.  If the file cannot
  // be found, offer to create a sample one.
  loadCorrectConfigFile ();

  // When redirecting output to a file, do not use color, curses.
  if (!isatty (fileno (stdout)))
  {
    config.set ("curses", "off");

    if (! config.get (std::string ("_forcecolor"), false))
      config.set ("color",  "off");
  }

  // Load appropriate stringtable as soon after the config file as possible, to
  // allow all subsequent messages to be localizable.
  std::string location = expandPath (config.get ("data.location"));
  std::string locale = config.get ("locale");
  if (locale != "")
    stringtable.load (location + "/strings." + locale);

  // TODO Handle "--version, -v" right here.

  // init TDB.
  std::vector <std::string> all;
  split (all, location, ',');
  foreach (path, all)
    tdb.location (expandPath (*path));

  // TODO Load pending.data.
  // TODO Load completed.data.
  // TODO Load deleted.data.
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  std::cout << "--- start 1.8.0 ---" << std::endl;
  try
  {
    parse ();

    // TODO Dispatch to command handlers.
    // TODO Auto shadow update.
    // TODO Auto gc.
    // TODO tdb.load (Filter);
  }

  catch (const std::string& error)
  {
    messages.push_back (error);
  }

  catch (...)
  {
    messages.push_back (stringtable.get (100, "Unknown error."));
  }

  // Dump all messages.
  foreach (m, messages)
    std::cout << *m << std::endl;

  if (footnotes.size ())
  {
    std::cout << std::endl;
    foreach (f, footnotes)
      std::cout << *f << std::endl;
  }

  std::cout << "--- end 1.8.0 ---" << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
int Context::interactive ()
{
  // TODO init ncurses
  // TODO create worker thread
  // TODO create refresh thread

  // TODO join refresh thread
  // TODO join worker thread
  // TODO take down ncurses

  throw std::string ("unimplemented Context::interactive");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadCorrectConfigFile ()
{
  foreach (arg, args)
  {
    if (arg->substr (0, 3) == "rc:")
    {
      std::string file = arg->substr (3, std::string::npos);
      if (access (file.c_str (), F_OK))
        throw std::string ("Could not read configuration file '") + file + "'";

      messages.push_back (std::string ("Using alternate .taskrc file ") + file); // TODO i18n
      config.load (file);
    }
  }

  struct passwd* pw = getpwuid (getuid ());
  if (!pw)
    throw std::string (
      stringtable.get (SHELL_READ_PASSWD,
                       "Could not read home directory from the passwd file."));

  std::string file = pw->pw_dir;
  config.createDefault (file);

  // Apply overrides of type: "rc.name:value"
  foreach (arg, args)
  {
    if (arg->substr (0, 3) == "rc.")
    {
      std::string name;
      std::string value;
      Nibbler n (*arg);
      if (n.getUntil ('.', name) &&
          n.skip ('.')           &&
          n.getUntil (':', name) &&
          n.skip (':')           &&
          n.getUntilEOS (value))
      {
        config.set (name, value);
        messages.push_back (std::string ("Configuration override ") +  // TODO i18n
                           arg->substr (3, std::string::npos));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::parse ()
{
  command = "";

  bool terminated = false;
  bool foundSequence = false;
  bool foundSomethingAfterSequence = false;

  std::string descCandidate = "";

  foreach (arg, args)
  {
    // Ignore any argument that is "rc:...", because that is the command line
    // specified rc file.
    if (arg->substr (0, 3) != "rc:" ||
        arg->substr (0, 3) != "rc.")
    {
      if (!terminated)
      {
/*
        size_t colon;               // Pointer to colon in argument.
        std::string from;
        std::string to;
        bool global;
        std::vector <int> sequence;

        // The '--' argument shuts off all parsing - everything is an argument.
        if (arg == "--")
          terminated = true;

        // An id is the first argument found that contains all digits.
        else if (lowerCase (command) != "add"  && // "add" doesn't require an ID
            validSequence (arg, sequence) &&
            ! foundSomethingAfterSequence)
        {
          foundSequence = true;
          foreach (id, sequence)
            task.addId (*id);
        }

        // Tags begin with + or - and contain arbitrary text.
        else if (validTag (arg))
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          if (arg[0] == '+')
            task.addTag (arg->substr (1, std::string::npos));
          else if (arg[0] == '-')
            task.addRemoveTag (arg->substr (1, std::string::npos));
        }

        // Attributes contain a constant string followed by a colon, followed by a
        // value.
        else if ((colon = arg->find (":")) != std::string::npos)
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          std::string name  = lowerCase (arg->substr (0, colon));
          std::string value = arg->substr (colon + 1, std::string::npos);

          if (validAttribute (name, value))
          {
            if (name != "recur" || validDuration (value))
              task.setAttribute (name, value);
          }

          // If it is not a valid attribute, then allow the argument as part of
          // the description.
          else
          {
            if (descCandidate.length ())
              descCandidate += " ";
            descCandidate += arg;
          }
        }

        // Substitution of description text.
        else if (validSubstitution (arg, from, to, global))
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          task.setSubstitution (from, to, global);
        }

        // Command.
        else if (command == "")
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          std::string l = lowerCase (arg);
          if (isCommand (l) && validCommand (l))
            command = l;
          else
          {
            if (descCandidate.length ())
              descCandidate += " ";
            descCandidate += arg;
          }
        }

        // Anything else is just considered description.
        else
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          if (descCandidate.length ())
            descCandidate += " ";
          descCandidate += arg;
        }
*/
      }
      // terminated, therefore everything subsequently is a description.
      else
      {
/*
        if (foundSequence)
          foundSomethingAfterSequence = true;

        if (descCandidate.length ())
          descCandidate += " ";
        descCandidate += arg;
*/
      }
    }
  }

/*
  if (validDescription (descCandidate))
    task.set ("description", descCandidate);
*/

  // TODO Replace parse.cpp:parse
  throw std::string ("unimplemented Context::parse");
}

////////////////////////////////////////////////////////////////////////////////
