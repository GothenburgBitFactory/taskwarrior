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
, subst ()
, task ()
, tdb ()
, stringtable ()
, program ("")
, cmd ()
{
  // Set up randomness.
#ifdef HAVE_SRANDOM
  srandom (time (NULL));
#else
  srand (time (NULL));
#endif
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
    if (i == 0)
      program = argv[i];
    else
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
  std::cout << "[1;32m--- start 1.8.0 ---[0m" << std::endl;
  try
  {
    parse ();    // Parse command line.
    // TODO tdb.load (Filter);
    dispatch (); // Dispatch to command handlers.
    // TODO Auto gc.
    shadow ();   // Auto shadow update.
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

  std::cout << "[1;32m--- end 1.8.0 ---[0m" << std::endl;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Context::dispatch ()
{
/*
  // If argc == 1 and there is a default.command, use it.  Otherwise use
  // argc/argv.
  std::string defaultCommand = context.config.get ("default.command");
  if (args.size () == 0 || defaultCommand != "")
  {
    // Stuff the command line.
    args.clear ();
    split (args, defaultCommand, ' ');
    std::cout << "[task " << defaultCommand << "]" << std::endl;
  }

  loadCustomReports ();

  std::string command;
  T task;
  parse (args, command, task);

  bool gcMod  = false; // Change occurred by way of gc.
  bool cmdMod = false; // Change occurred by way of command type.
  std::string out;

  // Read-only commands with no side effects.
       if (command == "export")             { out = handleExport          (tdb, task); }
  else if (command == "projects")           { out = handleProjects        (tdb, task); }
  else if (command == "tags")               { out = handleTags            (tdb, task); }
  else if (command == "info")               { out = handleInfo            (tdb, task); }
  else if (command == "stats")              { out = handleReportStats     (tdb, task); }
  else if (command == "history")            { out = handleReportHistory   (tdb, task); }
  else if (command == "ghistory")           { out = handleReportGHistory  (tdb, task); }
  else if (command == "calendar")           { out = handleReportCalendar  (tdb, task); }
  else if (command == "summary")            { out = handleReportSummary   (tdb, task); }
  else if (command == "timesheet")          { out = handleReportTimesheet (tdb, task); }
  else if (command == "colors")             { out = handleColor           (         ); }
  else if (command == "version")            { out = handleVersion         (         ); }
  else if (command == "help")               { out = longUsage             (         ); }

  // Commands that cause updates.
  else if (command == "" && task.getId ())  { cmdMod = true; out = handleModify    (tdb, task); }
  else if (command == "add")                { cmdMod = true; out = handleAdd       (tdb, task); }
  else if (command == "append")             { cmdMod = true; out = handleAppend    (tdb, task); }
  else if (command == "annotate")           { cmdMod = true; out = handleAnnotate  (tdb, task); }
  else if (command == "done")               { cmdMod = true; out = handleDone      (tdb, task); }
  else if (command == "undelete")           { cmdMod = true; out = handleUndelete  (tdb, task); }
  else if (command == "delete")             { cmdMod = true; out = handleDelete    (tdb, task); }
  else if (command == "start")              { cmdMod = true; out = handleStart     (tdb, task); }
  else if (command == "stop")               { cmdMod = true; out = handleStop      (tdb, task); }
  else if (command == "undo")               { cmdMod = true; out = handleUndo      (tdb, task); }
  else if (command == "import")             { cmdMod = true; out = handleImport    (tdb, task); }
  else if (command == "duplicate")          { cmdMod = true; out = handleDuplicate (tdb, task); }
  else if (command == "edit")               { cmdMod = true; out = handleEdit      (tdb, task); }

  // Command that display IDs and therefore need TDB::gc first.
  else if (command == "completed")          { if (gc) gcMod = tdb.gc (); out = handleCompleted     (tdb, task); }
  else if (command == "next")               { if (gc) gcMod = tdb.gc (); out = handleReportNext    (tdb, task); }
  else if (command == "active")             { if (gc) gcMod = tdb.gc (); out = handleReportActive  (tdb, task); }
  else if (command == "overdue")            { if (gc) gcMod = tdb.gc (); out = handleReportOverdue (tdb, task); }
  else if (isCustomReport (command))        { if (gc) gcMod = tdb.gc (); out = handleCustomReport  (tdb, task, command); }

  // If the command is not recognized, display usage.
  else                                      { out = shortUsage (); }

  // Only update the shadow file if such an update was not suppressed (shadow),
  // and if an actual change occurred (gcMod || cmdMod).
  if (shadow && (gcMod || cmdMod))
    updateShadowFile (tdb);

  return out;
*/
}

////////////////////////////////////////////////////////////////////////////////
void Context::shadow ()
{
  throw std::string ("unimplemented Context::shadow");
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

      // No need to handle it again.
      args.erase (arg);
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

      // No need to handle it again.
      args.erase (arg);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::parse ()
{
  std::string descCandidate        = "";
  bool terminated                  = false;
  bool foundSequence               = false;
  bool foundSomethingAfterSequence = false;

  foreach (arg, args)
  {
    if (!terminated)
    {
      // The '--' argument shuts off all parsing - everything is an argument.
      if (*arg == "--")
{
std::cout << "# parse terminator '" << *arg << "'" << std::endl;
        terminated = true;
}

      // Sequence
      // Note: "add" doesn't require an ID
      else if (cmd.command != "add"          &&
               ! foundSomethingAfterSequence &&
               sequence.valid (*arg))
      {
std::cout << "# parse sequence '" << *arg << "'" << std::endl;
        sequence.parse (*arg);
        foundSequence = true;
      }

/*
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
*/
/*
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
*/

      // Substitution of description and/or annotation text.
      else if (subst.valid (*arg))
      {
        if (foundSequence)
          foundSomethingAfterSequence = true;

std::cout << "# parse subst '" << *arg << "'" << std::endl;
        subst.parse (*arg);
      }

      // It might be a command if one has not already been found.
      else if (cmd.command == "" &&
               cmd.valid (*arg))
      {
std::cout << "# parse cmd '" << *arg << "'" << std::endl;
        cmd.parse (*arg);

        if (foundSequence)
          foundSomethingAfterSequence = true;
      }

      // Anything else is just considered description.
      else
      {
std::cout << "# parse description '" << *arg << "'" << std::endl;
        if (foundSequence)
          foundSomethingAfterSequence = true;

        if (descCandidate.length ())
          descCandidate += " ";
        descCandidate += *arg;
      }
    }

    // terminated, therefore everything subsequently is a description.
    else
    {
std::cout << "# parse post-termination description '" << *arg << "'" << std::endl;
      if (foundSequence)
        foundSomethingAfterSequence = true;

      if (descCandidate.length ())
        descCandidate += " ";
      descCandidate += *arg;
    }
  }

  if (validDescription (descCandidate))
    task.set ("description", descCandidate);
}

////////////////////////////////////////////////////////////////////////////////
