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
#include "Timer.h"
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
    {
      program = argv[i];
      if (program.find ("cal") != std::string::npos)
        args.push_back ("calendar");
    }
    else
      args.push_back (argv[i]);

  initialize ();
}

////////////////////////////////////////////////////////////////////////////////
void Context::initialize ()
{
  Timer t ("Context::initialize");

  // Load the configuration file from the home directory.  If the file cannot
  // be found, offer to create a sample one.
  loadCorrectConfigFile ();
  loadAliases ();

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
  Timer t ("Context::run");

  std::string output;
  try
  {
    parse ();             // Parse command line.
    output = dispatch (); // Dispatch to command handlers.
  }

  catch (const std::string& error)
  {
    footnote (error);
  }

  catch (...)
  {
    footnote (stringtable.get (100, "Unknown error."));
  }

  // Dump all debug messages.
  if (config.get (std::string ("debug"), false))
    foreach (d, debugMessages)
      std::cout << colorizeDebug (*d) << std::endl;

  // Dump all headers.
  foreach (h, headers)
    std::cout << colorizeHeader (*h) << std::endl;

  // Dump the report output.
  std::cout << output;

  // Dump all footnotes.
  foreach (f, footnotes)
    std::cout << colorizeFootnote (*f) << std::endl;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::string Context::dispatch ()
{
  Timer t ("Context::dispatch");

  // TODO Just look at this thing.  It cries out for a dispatch table.
  std::string out;
       if (cmd.command == "projects")      { out = handleProjects           (); }
  else if (cmd.command == "tags")          { out = handleTags               (); }
  else if (cmd.command == "colors")        { out = handleColor              (); }
  else if (cmd.command == "version")       { out = handleVersion            (); }
  else if (cmd.command == "help")          { out = longUsage                (); }
  else if (cmd.command == "stats")         { out = handleReportStats        (); }
  else if (cmd.command == "info")          { out = handleInfo               (); }
  else if (cmd.command == "history")       { out = handleReportHistory      (); }
  else if (cmd.command == "ghistory")      { out = handleReportGHistory     (); }
  else if (cmd.command == "summary")       { out = handleReportSummary      (); }
  else if (cmd.command == "calendar")      { out = handleReportCalendar     (); }
  else if (cmd.command == "timesheet")     { out = handleReportTimesheet    (); }
  else if (cmd.command == "add")           { out = handleAdd                (); }
  else if (cmd.command == "append")        { out = handleAppend             (); }
  else if (cmd.command == "annotate")      { out = handleAnnotate           (); }
  else if (cmd.command == "done")          { out = handleDone               (); }
  else if (cmd.command == "delete")        { out = handleDelete             (); }
  else if (cmd.command == "start")         { out = handleStart              (); }
  else if (cmd.command == "stop")          { out = handleStop               (); }
  else if (cmd.command == "export")        { out = handleExport             (); }
  else if (cmd.command == "import")        { out = handleImport             (); }
  else if (cmd.command == "duplicate")     { out = handleDuplicate          (); }
  else if (cmd.command == "edit")          { out = handleEdit               (); }
#ifdef FEATURE_SHELL
  else if (cmd.command == "shell")         {       handleShell              (); }
#endif
  else if (cmd.command == "undo")          {       handleUndo               (); }
  else if (cmd.command == "_projects")     { out = handleCompletionProjects (); }
  else if (cmd.command == "_tags")         { out = handleCompletionTags     (); }
  else if (cmd.command == "_commands")     { out = handleCompletionCommands (); }
  else if (cmd.command == "" &&
           sequence.size ())               { out = handleModify             (); }

  // Command that display IDs and therefore need TDB::gc first.
  else if (cmd.command == "next")          { if (!inShadow) tdb.gc (); out = handleReportNext    (); }
  else if (cmd.validCustom (cmd.command))  { if (!inShadow) tdb.gc (); out = handleCustomReport  (cmd.command); }

  // If the command is not recognized, display usage.
  else                                     { out = shortUsage (); }

  // Only update the shadow file if such an update was not suppressed (shadow),
  if (cmd.isWriteCommand () && !inShadow)
    shadow ();

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

    clear ();

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
// Only allows aliases 10 deep.
std::string Context::canonicalize (const std::string& input) const
{
  std::string canonical = input;

  // Follow the chain.
  int i = 10;  // Safety valve.
  std::map <std::string, std::string>::const_iterator found;
  while ((found = aliases.find (canonical)) != aliases.end () && i-- > 0)
    canonical = found->second;

 if (i < 1)
   return input;

 return canonical;
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadCorrectConfigFile ()
{
  // Set up default locations.
  struct passwd* pw = getpwuid (getuid ());
  if (!pw)
    throw std::string (
      stringtable.get (
        SHELL_READ_PASSWD,
        "Could not read home directory from the passwd file."));

  std::string home = pw->pw_dir;
  std::string rc   = home + "/.taskrc";
  std::string data = home + "/.task";

  // Is there an override for rc?
  foreach (arg, args)
    if (arg->substr (0, 3) == "rc:")
    {
      rc = arg->substr (3, std::string::npos);
      args.erase (arg);
      header ("Using alternate .taskrc file " + rc); // TODO i18n
      break;
    }

  // Load rc file.
  config.clear ();       // Dump current values.
  config.setDefaults (); // Add in the custom reports.
  config.load (rc);      // Load new file.

  if (config.get ("data.location") != "")
    data = config.get ("data.location");

  // Is there an override for data?
  foreach (arg, args)
    if (arg->substr (0, 17) == "rc.data.location:")
    {
      data = arg->substr (17, std::string::npos);
      header ("Using alternate data.location " + data); // TODO i18n
      break;
    }

  // Do we need to create a default rc?
  if (access (rc.c_str (), F_OK) &&
      confirm ("A configuration file could not be found in " // TODO i18n
             + home
             + "\n\n"
             + "Would you like a sample .taskrc created, so task can proceed?"))
  {
    config.createDefaultRC (rc, data);
  }

  // Create data location, if necessary.
  config.createDefaultData (data);

  // Load rc file.
  config.clear ();       // Dump current values.
  config.setDefaults (); // Add in the custom reports.
  config.load (rc);      // Load new file.

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
        footnote (std::string ("Configuration override ") +  // TODO i18n
                  arg->substr (3, std::string::npos));
      }
    }
    else
      filtered.push_back (*arg);
  }

  args = filtered;
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadAliases ()
{
  aliases.clear ();

  std::vector <std::string> vars;
  config.all (vars);
  foreach (var, vars)
  {
    if (var->substr (0, 6) == "alias.")
    {
      std::string alias = var->substr (6, std::string::npos);
      std::string canonical = config.get (*var);

      aliases[alias] = canonical;
      debug (std::string ("Alias ") + alias + " -> " + canonical);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::parse ()
{
  parse (args, cmd, task, sequence, subst, filter);
}

////////////////////////////////////////////////////////////////////////////////
void Context::parse (
  std::vector <std::string>& parseArgs,
  Cmd& parseCmd,
  Task& parseTask,
  Sequence& parseSequence,
  Subst& parseSubst,
  Filter& parseFilter)
{
  Timer t ("Context::parse");

  Att attribute;
  tagAdditions.clear ();
  tagRemovals.clear ();
  std::string descCandidate        = "";
  bool terminated                  = false;
  bool foundSequence               = false;
  bool foundSomethingAfterSequence = false;

  foreach (arg, parseArgs)
  {
    if (!terminated)
    {
      // The '--' argument shuts off all parsing - everything is an argument.
      if (*arg == "--")
      {
        debug ("parse terminator '" + *arg + "'");
        terminated = true;
      }

      // Sequence
      // Note: "add" doesn't require an ID
      else if (parseCmd.command != "add"          &&
               ! foundSomethingAfterSequence &&
               parseSequence.valid (*arg))
      {
        debug ("parse sequence '" + *arg + "'");
        parseSequence.parse (*arg);
        foundSequence = true;
      }

      // Tags to include begin with '+'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '+'   &&
               noSpaces (*arg))
      {
        debug ("parse tag addition '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        if (arg->find (',') != std::string::npos)
          throw stringtable.get (TAGS_NO_COMMA,
                                 "Tags are not permitted to contain commas.");

        tagAdditions.push_back (arg->substr (1, std::string::npos));
        parseTask.addTag       (arg->substr (1, std::string::npos));
      }

      // Tags to remove begin with '-'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '-'   &&
               noSpaces (*arg))
      {
        debug ("parse tag removal '" + *arg + "'");
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
        debug ("parse attribute '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        attribute.parse (*arg);

        // There has to be a better way.  And it starts with a fresh coffee.
        std::string name = attribute.name ();
        std::string mod = attribute.mod ();
        std::string value = attribute.value ();
        if (attribute.validNameValue (name, mod, value))
        {
          attribute.name (name);
          attribute.mod (mod);
          attribute.value (value);

          parseTask[attribute.name ()] = attribute;
        }

        // *arg has the appearance of an attribute (foo:bar), but isn't
        // recognized, so downgrade it to part of the description.
        else
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          if (descCandidate.length ())
            descCandidate += " ";
          descCandidate += *arg;
        }
      }

      // Substitution of description and/or annotation text.
      else if (parseSubst.valid (*arg))
      {
        if (foundSequence)
          foundSomethingAfterSequence = true;

        debug ("parse subst '" + *arg + "'");
        parseSubst.parse (*arg);
      }

      // It might be a command if one has not already been found.
      else if (parseCmd.command == "" &&
               parseCmd.valid (*arg))
      {
        debug ("parse cmd '" + *arg + "'");
        parseCmd.parse (*arg);

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
      debug ("parse post-termination description '" + *arg + "'");
      if (foundSequence)
        foundSomethingAfterSequence = true;

      if (descCandidate.length ())
        descCandidate += " ";
      descCandidate += *arg;
    }
  }

  if (descCandidate != "" && noVerticalSpace (descCandidate))
  {
    debug ("parse description '" + descCandidate + "'");
    parseTask.set ("description", descCandidate);
  }

  // At this point, either a sequence or a command should have been found.
  if (parseSequence.size () == 0 && parseCmd.command == "")
    parseCmd.parse (descCandidate);

  // Read-only command (reports, status, info ...) use filters.  Write commands
  // (add, done ...) do not.
  if (parseCmd.isReadOnlyCommand ())
    autoFilter (parseTask, parseFilter);

  // If no command was specified, and there were no command line arguments
  // then invoke the default command.
  if (parseCmd.command == "" && parseArgs.size () == 0)
  {
    std::string defaultCommand = config.get ("default.command");
    if (defaultCommand != "")
    {
      // Stuff the command line.
      parseArgs.clear ();
      split (parseArgs, defaultCommand, ' ');
      header ("[task " + defaultCommand + "]");

      // Reinitialize the context and recurse.
      initialize ();
      parse (args, cmd, task, sequence, subst, filter);
    }
    else
      throw stringtable.get (
        CMD_MISSING,
        "You must specify a command, or a task ID to modify");
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::clear ()
{
//  Config                    config;
  filter.clear ();
//  Keymap                    keymap;
  sequence.clear ();
  subst.clear ();
//  task.clear ();
  task = Task ();
  tdb.clear ();
//  stringtable.clear ();
  program = "";
  args.clear ();
  cmd.command = "";
  tagAdditions.clear ();
  tagRemovals.clear ();

  clearMessages ();
  inShadow = false;
}

////////////////////////////////////////////////////////////////////////////////
// Add all the attributes in the task to the filter.  All except uuid.
void Context::autoFilter (Task& t, Filter& f)
{
  foreach (att, t)
  {
    // Words are found in the description using the .has modifier.
    if (att->first == "description" && att->second.mod () == "")
    {
      std::vector <std::string> words;
      split (words, att->second.value (), ' ');
      foreach (word, words)
      {
        f.push_back (Att ("description", "has", *word));
        debug ("auto filter: " + att->first + ".has:" + *word);
      }
    }

    // Projects are matched left-most.
    else if (att->first == "project" && att->second.mod () == "")
    {
      f.push_back (Att ("project", "startswith", att->second.value ()));
        debug ("auto filter: " + att->first + ".startswith:" + att->second.value ());
    }

    // The limit attribute does not participate in filtering, and needs to be
    // specifically handled in handleCustomReport.
    else if (att->first == "limit")
    {
    }

    // Every task has a unique uuid by default, and it shouldn't be included,
    // because it is guaranteed to not match.
    else if (att->first == "uuid")
    {
    }

    // The mechanism for filtering on tags is +/-<tag>.
    else if (att->first == "tags")
    {
    }

    // Generic attribute matching.
    else
    {
      f.push_back (att->second);
      debug ("auto filter: " +
             att->first +
             (att->second.mod () != "" ?
               ("." + att->second.mod () + ":") :
               ":") +
             att->second.value ());
    }
  }

  // Include tagAdditions.
  foreach (tag, tagAdditions)
  {
    f.push_back (Att ("tags", "has", *tag));
    debug ("auto filter: +" + *tag);
  }

  // Include tagRemovals.
  foreach (tag, tagRemovals)
  {
    f.push_back (Att ("tags", "hasnt", *tag));
    debug ("auto filter: -" + *tag);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::header (const std::string& input)
{
  headers.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::footnote (const std::string& input)
{
  footnotes.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::debug (const std::string& input)
{
  debugMessages.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::clearMessages ()
{
  headers.clear ();
  footnotes.clear ();
  debugMessages.clear ();
}

////////////////////////////////////////////////////////////////////////////////
