////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include <unistd.h>
#include "Context.h"
#include "Directory.h"
#include "File.h"
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
, file_override ("")
, var_overrides ("")
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
  {
    if (i == 0)
    {
      program = argv[i];
      std::string::size_type cal = program.find ("/cal");
      if (program == "cal" ||
          (cal != std::string::npos && program.length () == cal + 4))
        args.push_back ("calendar");
    }
    else
      args.push_back (argv[i]);
  }

  initialize ();

  // Hook system init, plus post-start event occurring at the first possible
  // moment after hook initialization.
  hooks.initialize ();
  hooks.trigger ("post-start");
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

    if (! config.getBoolean ("_forcecolor"))
      config.set ("color",  "off");
  }

  if (config.getBoolean ("color"))
    initializeColorRules ();

  // Load appropriate stringtable as soon after the config file as possible, to
  // allow all subsequent messages to be localizable.
  Directory location (config.get ("data.location"));
  std::string locale = config.get ("locale");

  // If there is a locale variant (en-US.<variant>), then strip it.
  std::string::size_type period = locale.find ('.');
  if (period != std::string::npos)
    locale = locale.substr (0, period);

  if (locale != "")
    stringtable.load (location.data + "/strings." + locale);

  // TODO Handle "--version, -v" right here?

  // init TDB.
  tdb.clear ();
  std::vector <std::string> all;
  split (all, location, ',');
  foreach (path, all)
    tdb.location (*path);
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  int rc;
  std::string output;
  try
  {
    parse ();               // Parse command line.
    rc = dispatch (output); // Dispatch to command handlers.
  }

  catch (const std::string& error)
  {
    footnote (error);
    rc = 2;
  }

  catch (...)
  {
    footnote (stringtable.get (100, "Unknown error."));
    rc = 3;
  }

  // Dump all debug messages.
  hooks.trigger ("pre-debug");
  if (config.getBoolean ("debug"))
    foreach (d, debugMessages)
      if (config.getBoolean ("color") || config.getBoolean ("_forcecolor"))
        std::cout << colorizeDebug (*d) << std::endl;
      else
        std::cout << *d << std::endl;
  hooks.trigger ("post-debug");

  // Dump all headers.
  hooks.trigger ("pre-header");
  foreach (h, headers)
    if (config.getBoolean ("color") || config.getBoolean ("_forcecolor"))
      std::cout << colorizeHeader (*h) << std::endl;
    else
      std::cout << *h << std::endl;
  hooks.trigger ("post-header");

  // Dump the report output.
  hooks.trigger ("pre-output");
  std::cout << output;
  hooks.trigger ("post-output");

  // Dump all footnotes.
  hooks.trigger ("pre-footnote");
  foreach (f, footnotes)
    if (config.getBoolean ("color") || config.getBoolean ("_forcecolor"))
      std::cout << colorizeFootnote (*f) << std::endl;
    else
      std::cout << *f << std::endl;
  hooks.trigger ("post-footnote");

  hooks.trigger ("pre-exit");
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int Context::dispatch (std::string &out)
{
  int rc = 0;

  Timer t ("Context::dispatch");

  hooks.trigger ("pre-dispatch");

  // TODO Just look at this thing.  It cries out for a dispatch table.
       if (cmd.command == "projects")         { rc = handleProjects              (out); }
  else if (cmd.command == "tags")             { rc = handleTags                  (out); }
  else if (cmd.command == "colors")           { rc = handleColor                 (out); }
  else if (cmd.command == "version")          { rc = handleVersion               (out); }
  else if (cmd.command == "config")           { rc = handleConfig                (out); }
  else if (cmd.command == "show")             { rc = handleShow                  (out); }
  else if (cmd.command == "help")             { rc = longUsage                   (out); }
  else if (cmd.command == "stats")            { rc = handleReportStats           (out); }
  else if (cmd.command == "info")             { rc = handleInfo                  (out); }
  else if (cmd.command == "history.monthly")  { rc = handleReportHistoryMonthly  (out); }
  else if (cmd.command == "history.annual")   { rc = handleReportHistoryAnnual   (out); }
  else if (cmd.command == "ghistory.monthly") { rc = handleReportGHistoryMonthly (out); }
  else if (cmd.command == "ghistory.annual")  { rc = handleReportGHistoryAnnual  (out); }
  else if (cmd.command == "summary")          { rc = handleReportSummary         (out); }
  else if (cmd.command == "calendar")         { rc = handleReportCalendar        (out); }
  else if (cmd.command == "timesheet")        { rc = handleReportTimesheet       (out); }
  else if (cmd.command == "add")              { rc = handleAdd                   (out); }
  else if (cmd.command == "log")              { rc = handleLog                   (out); }
  else if (cmd.command == "append")           { rc = handleAppend                (out); }
  else if (cmd.command == "prepend")          { rc = handlePrepend               (out); }
  else if (cmd.command == "annotate")         { rc = handleAnnotate              (out); }
  else if (cmd.command == "denotate")         { rc = handleDenotate              (out); }
  else if (cmd.command == "done")             { rc = handleDone                  (out); }
  else if (cmd.command == "delete")           { rc = handleDelete                (out); }
  else if (cmd.command == "start")            { rc = handleStart                 (out); }
  else if (cmd.command == "stop")             { rc = handleStop                  (out); }
  else if (cmd.command == "export.csv")       { rc = handleExportCSV             (out); }
  else if (cmd.command == "export.ical")      { rc = handleExportiCal            (out); }
  else if (cmd.command == "export.yaml")      { rc = handleExportYAML            (out); }
  else if (cmd.command == "import")           { rc = handleImport                (out); }
  else if (cmd.command == "duplicate")        { rc = handleDuplicate             (out); }
  else if (cmd.command == "edit")             { rc = handleEdit                  (out); }
#ifdef FEATURE_SHELL
  else if (cmd.command == "shell")            {      handleShell                 (   ); }
#endif
  else if (cmd.command == "undo")             {      handleUndo                  (   ); }
  else if (cmd.command == "_merge")           { tdb.gc ();
	                                                  handleMerge                 (out); }
  else if (cmd.command == "_projects")        { rc = handleCompletionProjects    (out); }
  else if (cmd.command == "_tags")            { rc = handleCompletionTags        (out); }
  else if (cmd.command == "_commands")        { rc = handleCompletionCommands    (out); }
  else if (cmd.command == "_ids")             { rc = handleCompletionIDs         (out); }
  else if (cmd.command == "_config")          { rc = handleCompletionConfig      (out); }
  else if (cmd.command == "_version")         { rc = handleCompletionVersion     (out); }
  else if (cmd.command == "_urgency")         { rc = handleUrgency               (out); }
  else if (cmd.command == "" &&
           sequence.size ())                  { rc = handleModify                (out); }

  // Commands that display IDs and therefore need TDB::gc first.
  else if (cmd.validCustom (cmd.command))     { if (!inShadow) tdb.gc ();
                                                rc = handleCustomReport (cmd.command, out); }

  // If the command is not recognized, display usage.
  else                                        { hooks.trigger ("pre-usage-command");
                                                rc = shortUsage (out);
                                                hooks.trigger ("post-usage-command"); }

  // Only update the shadow file if such an update was not suppressed (shadow),
  if (cmd.isWriteCommand () && !inShadow)
    shadow ();

  hooks.trigger ("post-dispatch");
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
void Context::shadow ()
{
  // Determine if shadow file is enabled.
  File shadowFile (config.get ("shadow.file"));
  if (shadowFile.data != "")
  {
    inShadow = true;  // Prevents recursion in case shadow command writes.

    // Check for silly shadow file settings.
    std::string dataLocation = config.get ("data.location");
    if (shadowFile.data == dataLocation + "/pending.data")
      throw std::string ("Configuration variable 'shadow.file' is set to "
                         "overwrite your pending tasks.  Please change it.");

    if (shadowFile.data == dataLocation + "/completed.data")
      throw std::string ("Configuration variable 'shadow.file' is set to "
                         "overwrite your completed tasks.  Please change it.");

    std::string oldCurses = config.get ("curses");
    std::string oldColor  = config.get ("color");

    clear ();

    // Run report.  Use shadow.command, using default.command as a fallback
    // with "list" as a default.
    std::string command = config.get ("shadow.command");
    if (command == "")
      command = config.get ("default.command");

    split (args, command, ' ');

    initialize ();
    config.set ("curses", "off");
    config.set ("color",  "off");

    parse ();
    std::string result;
    (void)dispatch (result);
    std::ofstream out (shadowFile.data.c_str ());
    if (out.good ())
    {
      out << result;
      out.close ();
    }
    else
      throw std::string ("Could not write file '") + shadowFile.data + "'";

    config.set ("curses", oldCurses);
    config.set ("color",  oldColor);

    // Optionally display a notification that the shadow file was updated.
    if (config.getBoolean ("shadow.notify"))
      footnote (std::string ("[Shadow file '") + shadowFile.data + "' updated.]");

    inShadow = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Only allows aliases 10 deep.
std::string Context::canonicalize (const std::string& input) const
{
  std::string canonical = input;

  // First try to autocomplete the alias.
  std::vector <std::string> options;
  std::vector <std::string> matches;
  foreach (name, aliases)
    options.push_back (name->first);

  autoComplete (input, options, matches);
  if (matches.size () == 1)
  {
    canonical = matches[0];

    // Follow the chain.
    int i = 10;  // Safety valve.
    std::map <std::string, std::string>::const_iterator found;
    while ((found = aliases.find (canonical)) != aliases.end () && i-- > 0)
      canonical = found->second;

    if (i < 1)
      return input;
  }

  return canonical;
}

////////////////////////////////////////////////////////////////////////////////
void Context::disallowModification () const
{
  if (task.size ()         ||
      subst.mFrom != ""    ||
      tagAdditions.size () ||
      tagRemovals.size ())
    throw std::string ("The '")
        + cmd.command
        + "' command does not allow further modification of a task.";
}

////////////////////////////////////////////////////////////////////////////////
// Takes a vector of args (foo, rc.name:value, bar), extracts any rc.name:value
// args and sets the name/value in context.config, returning only the plain args
// (foo, bar) as output.
void Context::applyOverrides (
  const std::vector <std::string>& input,
  std::vector <std::string>& output)
{
  bool foundTerminator = false;
  foreach (in, input)
  {
    if (*in == "--")
    {
      foundTerminator = true;
      output.push_back (*in);
    }
    else if (!foundTerminator && in->substr (0, 3) == "rc.")
    {
      std::string name;
      std::string value;
      Nibbler n (*in);
      if (n.getLiteral ("rc.")         &&  // rc.
          n.getUntilOneOf (":=", name) &&  //    xxx
          n.skipN (1))                     //       :
      {
        n.getUntilEOS (value);  // Don't care if it's blank.

        config.set (name, value);
        var_overrides += " " + *in;
        footnote ("Configuration override " + in->substr (3));
      }
      else
        footnote ("Problem with override: " + *in);
    }
    else
      output.push_back (*in);
  }
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
  File      rc   (home + "/.taskrc");
  Directory data (home + "./task");

  // Is there an file_override for rc:?
  foreach (arg, args)
  {
    if (*arg == "--")
      break;
    else if (arg->substr (0, 3) == "rc:")
    {
      file_override = *arg;
      rc = File (arg->substr (3));

      home = rc;
      std::string::size_type last_slash = rc.data.rfind ("/");
      if (last_slash != std::string::npos)
        home = rc.data.substr (0, last_slash);
      else
        home = ".";

      args.erase (arg);
      header ("Using alternate .taskrc file " + rc.data); // TODO i18n
      break;
    }
  }

  // Load rc file.
  config.clear ();       // Dump current values.
  config.load  (rc);     // Load new file.

  if (config.get ("data.location") != "")
    data = Directory (config.get ("data.location"));

  // Are there any var_overrides for data.location?
  foreach (arg, args)
  {
    if (*arg == "--")
      break;
    else if (arg->substr (0, 16) == "rc.data.location" &&
             ((*arg)[16] == ':' || (*arg)[16] == '='))
    {
      data = Directory (arg->substr (17));
      header ("Using alternate data.location " + data.data); // TODO i18n
      break;
    }
  }

  // Do we need to create a default rc?
  if (! rc.exists ())
  {
    if (!confirm ("A configuration file could not be found in " // TODO i18n
                + home
                + "\n\n"
                + "Would you like a sample "
                + rc.data
                + " created, so task can proceed?"))
      throw std::string ("Cannot proceed without rc file.");

    config.createDefaultRC (rc, data);
  }

  // Create data location, if necessary.
  config.createDefaultData (data);

  // Apply rc overrides.
  std::vector <std::string> filtered;
  applyOverrides (args, filtered);
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
      std::string alias = var->substr (6);
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
  bool foundNonSequence            = false;

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
      else if (parseCmd.command != "add"     &&
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

        foundNonSequence = true;

        if (arg->find (',') != std::string::npos)
          throw stringtable.get (TAGS_NO_COMMA,
                                 "Tags are not permitted to contain commas.");

        tagAdditions.push_back (arg->substr (1));
        parseTask.addTag       (arg->substr (1));
      }

      // Tags to remove begin with '-'.
      else if (arg->length () > 1 &&
               (*arg)[0] == '-'   &&
               noSpaces (*arg))
      {
        debug ("parse tag removal '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        foundNonSequence = true;

        if (arg->find (',') != std::string::npos)
          throw stringtable.get (TAGS_NO_COMMA,
                                 "Tags are not permitted to contain commas.");

        tagRemovals.push_back (arg->substr (1));
      }

      // Substitution of description and/or annotation text.
      else if (parseSubst.valid (*arg))
      {
        if (foundSequence)
          foundSomethingAfterSequence = true;

        foundNonSequence = true;

        debug ("parse subst '" + *arg + "'");
        parseSubst.parse (*arg);
      }

      // Atributes - name[.mod]:[value]
      else if (attribute.valid (*arg))
      {
        debug ("parse attribute '" + *arg + "'");
        if (foundSequence)
          foundSomethingAfterSequence = true;

        foundNonSequence = true;

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

          // Preserve modifier in the key, to allow multiple modifiers on the
          // same attribute.  Bug #252.
          if (name != "" && mod != "")
            parseTask[name + "." + mod] = attribute;
          else
            parseTask[name] = attribute;

          autoFilter (attribute, parseFilter);
        }

        // *arg has the appearance of an attribute (foo:bar), but isn't
        // recognized, so downgrade it to part of the description.
        else
        {
          if (foundSequence)
            foundSomethingAfterSequence = true;

          foundNonSequence = true;

          if (descCandidate.length ())
            descCandidate += " ";
          descCandidate += *arg;
        }
      }

      // It might be a command if one has not already been found.
      else if (parseCmd.command == "" &&
               parseCmd.valid (*arg))
      {
        debug ("parse cmd '" + *arg + "'");
        parseCmd.parse (*arg);

        if (foundSequence)
          foundSomethingAfterSequence = true;

        foundNonSequence = true;
      }

      // Anything else is just considered description.
      else
      {
        if (foundSequence)
          foundSomethingAfterSequence = true;

        foundNonSequence = true;

        if (descCandidate.length ())
          descCandidate += " ";
        descCandidate += *arg;
      }
    }

    // Command is terminated, therefore everything subsequently is a description.
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

    foundNonSequence = true;

    // Now convert the description to a filter on each word, if necessary.
    if (parseCmd.isReadOnlyCommand ())
    {
      std::vector <std::string> words;
      split (words, descCandidate, ' ');
      std::vector <std::string>::iterator it;
      for (it = words.begin (); it != words.end (); ++it)
      {
        Att a ("description", "contains", *it);
        autoFilter (a, parseFilter);
      }
    }
  }

  // At this point, either a sequence or a command should have been found.
  if (parseSequence.size () == 0 && parseCmd.command == "")
    parseCmd.parse (descCandidate);

  // Read-only command (reports, status, info ...) use filters.  Write commands
  // (add, done ...) do not.  The filter was constructed iteratively above, but
  // tags were omitted, so they are added now.
  if (parseCmd.isReadOnlyCommand ())
    autoFilter (parseFilter);

  // If no command was specified, and there were no command line arguments
  // then invoke the default command.
  if (parseCmd.command == "")
  {
    if (parseArgs.size () == 0)
    {
      // Apply overrides, if any.
      std::string defaultCommand = config.get ("default.command");
      if (defaultCommand != "")
      {
        // Add on the overrides.
        defaultCommand += " " + file_override + " " + var_overrides;

        // Stuff the command line.
        args.clear ();
        split (args, defaultCommand, ' ');
        header ("[task " + trim (defaultCommand) + "]");

        // Reinitialize the context and recurse.
        file_override = "";
        var_overrides = "";
        footnotes.clear ();
        initialize ();
        parse (args, cmd, task, sequence, subst, filter);
      }
      else
        throw stringtable.get (
          CMD_MISSING,
          "You must specify a command, or a task ID to modify.");
    }

    // If the command "task 123" is entered, but with no modifier arguments,
    // then the actual command is assumed to be "info".
    else if (!foundNonSequence &&
             (parseTask.id != 0 || parseSequence.size () != 0))
    {
      std::cout << "No command - assuming 'info'." << std::endl;
      parseCmd.command = "info";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Note: The reason some of these are commented out is because the ::clear
// method is not really "clear" but "clear_some".  Some members do not need to
// be initialized.  That makes this method something of a misnomer.  So be it.
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
  file_override = "";
  var_overrides = "";
  cmd.command = "";
  tagAdditions.clear ();
  tagRemovals.clear ();

  clearMessages ();
  inShadow = false;
}

////////////////////////////////////////////////////////////////////////////////
// Add all the attributes in the task to the filter.  All except uuid.
void Context::autoFilter (Att& a, Filter& f)
{
  // Words are found in the description using the .has modifier.
  if (a.name () == "description" && a.mod () == "")
  {
    std::vector <std::string> words;
    split (words, a.value (), ' ');
    foreach (word, words)
    {
      f.push_back (Att ("description", "has", *word));
      debug ("auto filter: " + a.name () + ".has:" + *word);
    }
  }

  // Projects are matched left-most.
  else if (a.name () == "project" && a.mod () == "")
  {
    if (a.value () != "")
    {
      f.push_back (Att ("project", "startswith", a.value ()));
      debug ("auto filter: " + a.name () + ".startswith:" + a.value ());
    }
    else
    {
      f.push_back (Att ("project", "is", a.value ()));
      debug ("auto filter: " + a.name () + ".is:" + a.value ());
    }
  }

  // The limit attribute does not participate in filtering, and needs to be
  // specifically handled in handleCustomReport.
  else if (a.name () == "limit")
  {
  }

  // Every task has a unique uuid by default, and it shouldn't be included,
  // because it is guaranteed to not match.
  else if (a.name () == "uuid")
  {
  }

  // The mechanism for filtering on tags is +/-<tag>.
  // Do not handle here - see below.
  else if (a.name () == "tags")
  {
  }

  // Generic attribute matching.
  else
  {
    f.push_back (a);
    debug ("auto filter: " +
           a.name () +
           (a.mod () != "" ?
             ("." + a.mod () + ":") :
             ":") +
           a.value ());
  }
}

////////////////////////////////////////////////////////////////////////////////
// Add all the tags in the task to the filter.
void Context::autoFilter (Filter& f)
{
  // This is now a correct implementation of a filter on the presence or absence
  // of a tag.  The prior code provided the illusion of leftmost partial tag
  // matches, but was really using the 'contains' and 'nocontains' attribute
  // modifiers.  See bug #293.

  // Include tagAdditions.
  foreach (tag, tagAdditions)
  {
    f.push_back (Att ("tags", "word", *tag));
    debug ("auto filter: +" + *tag);
  }

  // Include tagRemovals.
  foreach (tag, tagRemovals)
  {
    f.push_back (Att ("tags", "noword", *tag));
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
