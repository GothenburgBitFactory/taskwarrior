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
#include <fstream>
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
, inShadow (false)
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
  // Set up randomness.
#ifdef HAVE_SRANDOM
  srandom (time (NULL));
#else
  srand (time (NULL));
#endif

  // Capture the args.
  for (int i = 0; i < argc; ++i)
    if (i == 0)
      program = argv[i];
    else
      args.push_back (argv[i]);

  initialize ();
}

////////////////////////////////////////////////////////////////////////////////
void Context::initialize ()
{
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

  if (config.get ("color", true))
    initializeColorRules ();

  // Load appropriate stringtable as soon after the config file as possible, to
  // allow all subsequent messages to be localizable.
  std::string location = expandPath (config.get ("data.location"));
  std::string locale = config.get ("locale");

  // If there is a locale variant (en-US.<variant>), then strip it.
  std::string::size_type period = locale.find ('.');
  if (period != std::string::npos)
    locale = locale.substr (0, period);

  if (locale != "")
    stringtable.load (location + "/strings." + locale);

  // TODO Handle "--version, -v" right here?

  // init TDB.
  tdb.clear ();
  std::vector <std::string> all;
  split (all, location, ',');
  foreach (path, all)
    tdb.location (expandPath (*path));
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  std::string output;
  try
  {
    parse ();             // Parse command line.
    output = dispatch (); // Dispatch to command handlers.
  }

  catch (const std::string& error)
  {
    message (error);
  }

  catch (...)
  {
    message (stringtable.get (100, "Unknown error."));
  }

  // Dump all headers.
  foreach (h, headers)
    std::cout << colorizeHeader (*h) << std::endl;

  // Dump the report output.
  std::cout << output;

  // Dump all messages.
  foreach (m, messages)
    std::cout << colorizeMessage (*m) << std::endl;

  // Dump all footnotes.
  if (footnotes.size ())
  {
    std::cout << std::endl;
    foreach (f, footnotes)
      std::cout << colorizeFootnote (*f) << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::string Context::dispatch ()
{
  int gcMod = 0; // Change occurred by way of gc.
  std::string out;

  // TODO Just look at this thing.  It cries out for a dispatch table.
       if (cmd.command == "projects")          { out = handleProjects        (); }
  else if (cmd.command == "tags")              { out = handleTags            (); }
  else if (cmd.command == "colors")            { out = handleColor           (); }
  else if (cmd.command == "version")           { out = handleVersion         (); }
  else if (cmd.command == "help")              { out = longUsage             (); }
  else if (cmd.command == "stats")             { out = handleReportStats     (); }
  else if (cmd.command == "info")              { out = handleInfo            (); }
  else if (cmd.command == "history")           { out = handleReportHistory   (); }
  else if (cmd.command == "ghistory")          { out = handleReportGHistory  (); }
  else if (cmd.command == "summary")           { out = handleReportSummary   (); }
  else if (cmd.command == "calendar")          { out = handleReportCalendar  (); }
  else if (cmd.command == "timesheet")         { out = handleReportTimesheet (); }
  else if (cmd.command == "add")               { out = handleAdd             (); }
/*
  else if (cmd.command == "" && task.getId ()) { out = handleModify          (); }
*/
  else if (cmd.command == "append")            { out = handleAppend          (); }
/*
  else if (cmd.command == "annotate")          { out = handleAnnotate        (); }
*/
  else if (cmd.command == "done")              { out = handleDone            (); }
  else if (cmd.command == "undo")              { out = handleUndo            (); }
  else if (cmd.command == "delete")            { out = handleDelete          (); }
  else if (cmd.command == "undelete")          { out = handleUndelete        (); }
  else if (cmd.command == "start")             { out = handleStart           (); }
  else if (cmd.command == "stop")              { out = handleStop            (); }
  else if (cmd.command == "export")            { out = handleExport          (); }
/*
  else if (cmd.command == "import")            { out = handleImport          (); }
*/
  else if (cmd.command == "duplicate")         { out = handleDuplicate       (); }
/*
  else if (cmd.command == "edit")              { out = handleEdit            (); }
*/

  // Command that display IDs and therefore need TDB::gc first.
  else if (cmd.command == "next")              { if (!inShadow) gcMod = tdb.gc (); out = handleReportNext    (); }
  else if (cmd.validCustom (cmd.command))      { if (!inShadow) gcMod = tdb.gc (); out = handleCustomReport  (cmd.command); }

  // If the command is not recognized, display usage.
  else                                         { out = shortUsage (); }

  // Only update the shadow file if such an update was not suppressed (shadow),
// TODO
//  if (cmd.isWriteCommand (cmd.command) && !inShadow))
//    shadow ();

  return out;
}

////////////////////////////////////////////////////////////////////////////////
void Context::shadow ()
{
  // Determine if shadow file is enabled.
  std::string shadowFile = expandPath (config.get ("shadow.file"));
  if (shadowFile != "")
  {
    inShadow = true;  // Prevents recursion in case shadow command writes.

    // TODO Reinstate these checks.
/*
    // Check for silly shadow file settings.
    if (shadowFile == dataLocation + "/pending.data")
      throw std::string ("Configuration variable 'shadow.file' is set to "
                         "overwrite your pending tasks.  Please change it.");

    if (shadowFile == dataLocation + "/completed.data")
      throw std::string ("Configuration variable 'shadow.file' is set to "
                         "overwrite your completed tasks.  Please change it.");
*/

    std::string oldCurses = config.get ("curses");
    std::string oldColor  = config.get ("color");
    config.set ("curses", "off");
    config.set ("color",  "off");

    // Run report.  Use shadow.command, using default.command as a fallback
    // with "list" as a default.
    std::string command = config.get ("shadow.command",
                            config.get ("default.command", "list"));
    split (args, command, ' ');

    initialize ();
    parse ();
    std::string result = dispatch ();

    std::ofstream out (shadowFile.c_str ());
    if (out.good ())
    {
      out << result;
      out.close ();
    }
    else
      throw std::string ("Could not write file '") + shadowFile + "'";

    config.set ("curses", oldCurses);
    config.set ("color",  oldColor);

    // Optionally display a notification that the shadow file was updated.
    if (config.get (std::string ("shadow.notify"), false))
      footnote (std::string ("[Shadow file '") + shadowFile + "' updated]");

    inShadow = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadCorrectConfigFile ()
{
  bool needNewConfig = true;

  foreach (arg, args)
  {
    if (arg->substr (0, 3) == "rc:")
    {
      std::string file = arg->substr (3, std::string::npos);
      if (access (file.c_str (), F_OK))
        throw std::string ("Could not read configuration file '") + file + "'";

      message (std::string ("Using alternate .taskrc file ") + file); // TODO i18n
      config.clear ();       // Dump current values.
      config.setDefaults (); // Add in the custom reports.
      config.load (file);    // Load new file.
      needNewConfig = false;

      // No need to handle it again.
      args.erase (arg);
      break;
    }
  }

  if (needNewConfig)
  {
    struct passwd* pw = getpwuid (getuid ());
    if (!pw)
      throw std::string (
        stringtable.get (SHELL_READ_PASSWD,
                         "Could not read home directory from the passwd file."));

    config.createDefault (pw->pw_dir);
  }

  // Apply overrides of type: "rc.name:value"
  std::vector <std::string> filtered;
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
        message (std::string ("Configuration override ") +  // TODO i18n
                 arg->substr (3, std::string::npos));
      }
    }
    else
      filtered.push_back (*arg);
  }

  args = filtered;
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
        header ("parse terminator '" + *arg + "'");
        terminated = true;
      }

      // Sequence
      // Note: "add" doesn't require an ID
      else if (cmd.command != "add"          &&
               ! foundSomethingAfterSequence &&
               sequence.valid (*arg))
      {
        header ("parse sequence '" + *arg + "'");
        sequence.parse (*arg);
        foundSequence = true;
      }

      // Tags to include begin with '+'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '+'   &&
               validTag (*arg))
      {
        header ("parse tag addition '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        tagAdditions.push_back (arg->substr (1, std::string::npos));
        task.addTag            (arg->substr (1, std::string::npos));
      }

      // Tags to remove begin with '-'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '-'   &&
               validTag (*arg))
      {
        header ("parse tag removal '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        if (arg->find (',') != std::string::npos)
          throw stringtable.get (TAGS_NO_COMMA,
                                 "Tags are not permitted to contain commas.");

        tagRemovals.push_back (arg->substr (1, std::string::npos));
      }

      // Atributes - name[.mod]:[value]
      else if (attribute.valid (*arg))
      {
        header ("parse attribute '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        attribute.parse (*arg);

        // There has to be a better way.  And it starts with a fresh coffee.
        std::string name = attribute.name ();
        std::string mod = attribute.mod ();
        std::string value = attribute.value ();
        attribute.validNameValue (name, mod, value);
        attribute.name (name);
        attribute.mod (mod);
        attribute.value (value);

        task[attribute.name ()] = attribute;
      }

      // Substitution of description and/or annotation text.
      else if (subst.valid (*arg))
      {
        if (foundSequence)
          foundSomethingAfterSequence = true;

        header ("parse subst '" + *arg + "'");
        subst.parse (*arg);
      }

      // It might be a command if one has not already been found.
      else if (cmd.command == "" &&
               cmd.valid (*arg))
      {
        header ("parse cmd '" + *arg + "'");
        cmd.parse (*arg);

        if (foundSequence)
          foundSomethingAfterSequence = true;
      }

      // Anything else is just considered description.
      else
      {
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
      header ("parse post-termination description '" + *arg + "'");
      if (foundSequence)
        foundSomethingAfterSequence = true;

      if (descCandidate.length ())
        descCandidate += " ";
      descCandidate += *arg;
    }
  }

  if (descCandidate != "" && noVerticalSpace (descCandidate))
  {
    header ("parse description '" + descCandidate + "'");
    task.set ("description", descCandidate);
  }

  // TODO task.validate () ?

  // Read-only command (reports, status, info ...) use filters.  Write commands
  // (add, done ...) do not.
  if (cmd.isReadOnlyCommand ())
    autoFilter ();

  // If no command was specified, and there were no command line arguments
  // then invoke the default command.
  if (cmd.command == "" && args.size () == 0)
  {
    std::string defaultCommand = config.get ("default.command");
    if (defaultCommand != "")
    {
      // Stuff the command line.
      args.clear ();
      split (args, defaultCommand, ' ');
      header ("[task " + defaultCommand + "]");

      // Reinitialize the context and recurse.
      initialize ();
      parse ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Add all the attributes in the task to the filter.  All except uuid.
void Context::autoFilter ()
{
  foreach (att, task)
  {
    // Words are found in the description using the .has modifier.
    if (att->first == "description")
    {
      std::vector <std::string> words;
      split (words, att->second.value (), ' ');
      foreach (word, words)
      {
        filter.push_back (Att ("description", "has", *word));
        header ("auto filter: " + att->first + ".has:" + *word);
      }
    }

    // Projects are matched left-most.
    else if (att->first == "project")
    {
      filter.push_back (Att ("project", "startswith", att->second.value ()));
        header ("auto filter: " + att->first + ".startswith:" + att->second.value ());
    }

    // TODO Don't create a uuid for every task?
    // Every task has a unique uuid by default, and it shouldn't be included.
    // The mechanism for filtering on tags is +/-<tag>, not tags:foo which
    // means that there can only be one tag, "foo".
    else if (att->first != "uuid" &&
             att->first != "tags" &&
             att->first != "project")
    {
      filter.push_back (att->second);
      header ("auto filter: " + att->first + ":" + att->second.value ());
    }
  }

  // TODO Include Annotations as part of the description?

  // Include tagAdditions.
  foreach (tag, tagAdditions)
  {
    filter.push_back (Att ("tags", "has", *tag));
    header ("auto filter: +" + *tag);
  }

  // Include tagRemovals.
  foreach (tag, tagRemovals)
  {
    filter.push_back (Att ("tags", "hasnt", *tag));
    header ("auto filter: -" + *tag);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::header (const std::string& input)
{
  headers.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::message (const std::string& input)
{
  messages.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::footnote (const std::string& input)
{
  footnotes.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
