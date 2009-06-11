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
#include "main.h"
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

  // TODO Handle "--version, -v" right here?

  // init TDB.
  std::vector <std::string> all;
  split (all, location, ',');
  foreach (path, all)
    tdb.location (expandPath (*path));
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  std::cout << "[1;32m--- start 1.8.0 ---[0m" << std::endl;
  try
  {
    parse ();    // Parse command line.
    dispatch (); // Dispatch to command handlers.
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
*/

  bool gcMod  = false; // Change occurred by way of gc.
  bool cmdMod = false; // Change occurred by way of command type.
  std::string out;
/*
  // Read-only commands with no side effects.
       if (command == "export")             { out = handleExport ();   }
*/
       if (cmd.command == "projects")       { out = handleProjects (); }
  else if (cmd.command == "tags")           { out = handleTags ();     }
  else if (cmd.command == "colors")         { out = handleColor ();    }
  else if (cmd.command == "version")        { out = handleVersion ();  }
  else if (cmd.command == "help")           { out = longUsage ();      }
/*
  else if (command == "info")               { out = handleInfo            (); }
  else if (command == "stats")              { out = handleReportStats     (); }
  else if (command == "history")            { out = handleReportHistory   (); }
  else if (command == "ghistory")           { out = handleReportGHistory  (); }
  else if (command == "calendar")           { out = handleReportCalendar  (); }
  else if (command == "summary")            { out = handleReportSummary   (); }
  else if (command == "timesheet")          { out = handleReportTimesheet (); }

  // Commands that cause updates.
  else if (command == "" && task.getId ())  { cmdMod = true; out = handleModify    (); }
  else if (command == "add")                { cmdMod = true; out = handleAdd       (); }
  else if (command == "append")             { cmdMod = true; out = handleAppend    (); }
  else if (command == "annotate")           { cmdMod = true; out = handleAnnotate  (); }
  else if (command == "done")               { cmdMod = true; out = handleDone      (); }
  else if (command == "undelete")           { cmdMod = true; out = handleUndelete  (); }
  else if (command == "delete")             { cmdMod = true; out = handleDelete    (); }
  else if (command == "start")              { cmdMod = true; out = handleStart     (); }
  else if (command == "stop")               { cmdMod = true; out = handleStop      (); }
  else if (command == "undo")               { cmdMod = true; out = handleUndo      (); }
  else if (command == "import")             { cmdMod = true; out = handleImport    (); }
  else if (command == "duplicate")          { cmdMod = true; out = handleDuplicate (); }
  else if (command == "edit")               { cmdMod = true; out = handleEdit      (); }

  // Command that display IDs and therefore need TDB::gc first.
  else if (command == "completed")          { if (gc) gcMod = tdb.gc (); out = handleCompleted     (); }
  else if (command == "next")               { if (gc) gcMod = tdb.gc (); out = handleReportNext    (); }
  else if (command == "active")             { if (gc) gcMod = tdb.gc (); out = handleReportActive  (); }
  else if (command == "overdue")            { if (gc) gcMod = tdb.gc (); out = handleReportOverdue (); }
  else if (isCustomReport (command))        { if (gc) gcMod = tdb.gc (); out = handleCustomReport  (command); }

  // If the command is not recognized, display usage.
*/
  else                                      { out = shortUsage (); }

/*
  // Only update the shadow file if such an update was not suppressed (shadow),
  // and if an actual change occurred (gcMod || cmdMod).
  if (shadow && (gcMod || cmdMod))
    updateShadowFile (tdb);
*/

  std::cout << out;
}

////////////////////////////////////////////////////////////////////////////////
void Context::shadow ()
{
/*
  try
  {
    // Determine if shadow file is enabled.
    std::string shadowFile = expandPath (context.config.get ("shadow.file"));
    if (shadowFile != "")
    {
      std::string oldCurses = context.config.get ("curses");
      std::string oldColor = context.config.get ("color");
      context.config.set ("curses", "off");
      context.config.set ("color",  "off");

      // Run report.  Use shadow.command, using default.command as a fallback
      // with "list" as a default.
      std::string command = context.config.get ("shadow.command",
                              context.config.get ("default.command", "list"));
      std::vector <std::string> args;
      split (args, command, ' ');
      std::string result = runTaskCommand (args, tdb);

      std::ofstream out (shadowFile.c_str ());
      if (out.good ())
      {
        out << result;
        out.close ();
      }
      else
        throw std::string ("Could not write file '") + shadowFile + "'";

      context.config.set ("curses", oldCurses);
      context.config.set ("color",  oldColor);
    }

    // Optionally display a notification that the shadow file was updated.
    if (context.config.get (std::string ("shadow.notify"), false))
      std::cout << "[Shadow file '" << shadowFile << "' updated]" << std::endl;
  }

  catch (std::string& error)
  {
    std::cerr << error << std::endl;
  }

  catch (...)
  {
    std::cerr << "Unknown error." << std::endl;
  }
*/
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
  Att attribute;
  tagAdditions.clear ();
  tagRemovals.clear ();
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

      // Tags to include begin with '+'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '+')
      {
std::cout << "# parse tag addition '" << *arg << "'" << std::endl;
        if (foundSequence)
          foundSomethingAfterSequence = true;

        tagAdditions.push_back (arg->substr (1, std::string::npos));
      }

      // Tags to remove begin with '-'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '-')
      {
std::cout << "# parse tag removal '" << *arg << "'" << std::endl;
        if (foundSequence)
          foundSomethingAfterSequence = true;

        if (arg->find (',') != std::string::npos)
          throw stringtable.get (TAGS_NO_COMMA,
                                 "Tags are not permitted to contain commas.");

        tagRemovals.push_back (arg->substr (1, std::string::npos));
      }

      else if (attribute.valid (*arg))
      {
std::cout << "# parse attribute '" << *arg << "'" << std::endl;
        if (foundSequence)
          foundSomethingAfterSequence = true;

        attribute.parse (*arg);
        task[attribute.name ()] = attribute;
      }

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

  constructFilter ();
}

////////////////////////////////////////////////////////////////////////////////
// Add all the attributes in the task to the filter.  All except uuid.
void Context::constructFilter ()
{
  foreach (att, task)
    if (att->first != "uuid")
      filter.push_back (att->second);
}

////////////////////////////////////////////////////////////////////////////////
