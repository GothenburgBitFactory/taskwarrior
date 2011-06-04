////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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
#include <algorithm>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Context.h>
#include <Directory.h>
#include <File.h>
#include <Timer.h>
#include <text.h>
#include <util.h>
#include <main.h>
#include <i18n.h>
#include <../cmake.h>

////////////////////////////////////////////////////////////////////////////////
Context::Context ()
: rc_file ()
, data_dir ()
, extension_dir ()
, config ()
, tdb ()
, tdb2 ()
, dom ()
, determine_color_use (true)
, use_color (true)
, verbosity_legacy (false)
, terminal_width (0)
, terminal_height (0)
{
}

////////////////////////////////////////////////////////////////////////////////
Context::~Context ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Context::initialize (int argc, const char** argv)
{
  Timer t ("Context::initialize");

  // char** argv --> std::vector <std::string> Context::args.
  // TODO Handle "cal" case here.
  args.capture (argc, argv);

  // echo one two -- three | task zero --> task zero one two
  // 'three' is left in the input buffer.
  args.append_stdin ();

  // Assume default .taskrc and .task locations.
  assumeLocations ();

  // Process 'rc:<file>' command line override, and remove the argument from the
  // Context::args.
  args.rc_override (home_dir, rc_file);

  // Dump any existing values and load rc file.
  config.clear ();
  config.load  (rc_file);

  // The data location, Context::data_dir, is determined from the assumed
  // location (~/.task), or set by data.location in the config file, or
  // overridden by rc.data.location on the command line.
  std::string location;
  args.get_data_location (location);
  data_dir = Directory (location);
  extension_dir = data_dir.data + "/extensions";

  // Create missing config file and data directory, if necessary.
  createDefaultConfig ();

  // Handle Aliases.
  loadAliases ();
  args.resolve_aliases ();

  // Apply rc overrides to Context::config, capturing raw args for later use.
  args.apply_overrides ();

  // Initialize the color rules, if necessary.
  if (color ())
    initializeColorRules ();

  // Instantiate built-in command objects.
  Command::factory (commands);

  // Finally categorize all arguments.
  args.categorize ();

  // TODO Instantiate extension command objects.
  // TODO Instantiate default command object.

  // TODO Instantiate extension UDA objects.
  // TODO Instantiate extension format objects.

  // If there is a locale variant (en-US.<variant>), then strip it.
  std::string locale = config.get ("locale");
  std::string::size_type period = locale.find ('.');
  if (period != std::string::npos)
    locale = locale.substr (0, period);

  // Initialize the database.
  tdb.clear ();
  tdb.location (data_dir);

  // Hook system init, plus post-start event occurring at the first possible
  // moment after hook initialization.
  hooks.initialize ();
  hooks.trigger ("on-launch");
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  int rc;
  std::string output;

  try
  {
    rc = dispatch (output);
  }

  catch (const std::string& error)
  {
    footnote (error);
    rc = 2;
  }

  catch (...)
  {
    footnote (STRING_UNKNOWN_ERROR);
    rc = 3;
  }

  // Dump all debug messages, controlled by rc.debug.
  if (config.getBoolean ("debug"))
  {
    foreach (d, debugMessages)
      if (color ())
        std::cout << colorizeDebug (*d) << "\n";
      else
        std::cout << *d << "\n";

    args.dump ("Argument categorization");
  }

  // Dump all headers, controlled by 'header' verbosity token.
  if (verbose ("header"))
    foreach (h, headers)
      if (color ())
        std::cout << colorizeHeader (*h) << "\n";
      else
        std::cout << *h << "\n";

  // Dump the report output.
  std::cout << output;

  // Dump all footnotes, controlled by 'footnote' verbosity token.
  if (verbose ("footnote"))
    foreach (f, footnotes)
      if (color ())
        std::cout << colorizeFootnote (*f) << "\n";
      else
        std::cout << *f << "\n";

  hooks.trigger ("on-exit");
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// Locate and dispatch to the command whose keyword matches via autoComplete
// with the earliest argument.
int Context::dispatch (std::string &out)
{
  Timer t ("Context::dispatch");

  // Autocomplete args against keywords.
  std::string command;
  if (args.find_command (command))
  {
    updateXtermTitle ();

    Command* c = commands[command];

    // GC is invoked prior to running any command that displays task IDs.
    if (c->displays_id ())
      tdb.gc ();

    return c->execute (out);
  }

  // TODO Need to invoke 'information' when a sequence/filter is present, but
  //      no command is specified.

  return commands["help"]->execute (out);
}

////////////////////////////////////////////////////////////////////////////////
bool Context::color ()
{
  if (determine_color_use)
  {
    // What the config says.
    use_color = config.getBoolean ("color");

    // Only tty's support color.
    if (! isatty (fileno (stdout)))
    {
      // No ioctl.
      config.set ("detection", "off");
      config.set ("color",     "off");

      // Files don't get color.
      use_color = false;
    }

    // Override.
    if (config.getBoolean ("_forcecolor"))
    {
      config.set ("color", "on");
      use_color = true;
    }

    // No need to go through this again.
    determine_color_use = false;

    debug ("Context::color --> " + std::string (use_color ? "on" : "off"));
  }

  // Cached result.
  return use_color;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Support verbosity levels.
bool Context::verbose (const std::string& token)
{
  if (! verbosity.size ())
  {
    verbosity_legacy = config.getBoolean ("verbose");
    split (verbosity, config.get ("verbose"), ',');
  }

  if (verbosity_legacy)
    return true;

  if (std::find (verbosity.begin (), verbosity.end (), token) != verbosity.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// TODO OBSOLETE
void Context::shadow ()
{
/*
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

    if (shadowFile.data == dataLocation + "/undo.data")
      throw std::string ("Configuration variable 'shadow.file' is set to "
                         "overwrite your undo log.  Please change it.");

    std::string oldDetection = config.get ("detection");
    std::string oldColor  = config.get ("color");

    clear ();

    // Run report.  Use shadow.command, using default.command as a fallback
    // with "list" as a default.
    std::string command = config.get ("shadow.command");
    if (command == "")
      command = config.get ("default.command");

    split (args, command, ' ');

    //initialize ();
    config.set ("detection", "off");
    config.set ("color",     "off");

//    parse ();
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

    config.set ("detection", oldDetection);
    config.set ("color",     oldColor);

    // Optionally display a notification that the shadow file was updated.
    if (config.getBoolean ("shadow.notify"))
      footnote (std::string ("[Shadow file '") + shadowFile.data + "' updated.]");

    inShadow = false;
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
void Context::disallowModification () const
{
/*
  if (task.size ()         ||
      subst.mFrom != ""    ||
      tagAdditions.size () ||
      tagRemovals.size ())
    throw std::string ("The '")
//        + cmd.command
        + "' command does not allow further modification of a task.";
*/
}

////////////////////////////////////////////////////////////////////////////////
void Context::assumeLocations ()
{
  // Set up default locations.
  struct passwd* pw = getpwuid (getuid ());
  if (!pw)
    throw std::string (STRING_NO_HOME);

  home_dir = pw->pw_dir;
  rc_file  = File      (home_dir + "/.taskrc");
  data_dir = Directory (home_dir + "./task");
}

////////////////////////////////////////////////////////////////////////////////
void Context::createDefaultConfig ()
{
  // Do we need to create a default rc?
  if (! rc_file.exists ())
  {
    if (!confirm (format (STRING_CONTEXT_CREATE_RC, home_dir, rc_file.data)))
      throw std::string (STRING_CONTEXT_NEED_RC);

    config.createDefaultRC (rc_file, data_dir);
  }

  // Create data location, if necessary.
  config.createDefaultData (data_dir);

  // Create extension directory, if necessary.
  if (! extension_dir.exists ())
    extension_dir.create ();
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadAliases ()
{
  aliases.clear ();

  std::vector <std::string> vars;
  config.all (vars);

  std::vector <std::string>::iterator var;
  for (var = vars.begin (); var != vars.end (); ++var)
    if (var->substr (0, 6) == "alias.")
      aliases[var->substr (6)] = config.get (*var);
}

////////////////////////////////////////////////////////////////////////////////
void Context::decomposeSortField (
  const std::string& field,
  std::string& key,
  bool& ascending)
{
  int length = field.length ();

  if (field[length - 1] == '+')
  {
    ascending = true;
    key = field.substr (0, length - 1);
  }
  else if (field[length - 1] == '-')
  {
    ascending = false;
    key = field.substr (0, length - 1);
  }
  else
  {
    ascending = true;
    key = field;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Note: The reason some of these are commented out is because the ::clear
// method is not really "clear" but "clear_some".  Some members do not need to
// be initialized.  That makes this method something of a misnomer.  So be it.
void Context::clear ()
{
  tdb.clear ();            // TODO Obsolete
//  tdb2.clear ();
  args.clear ();

  clearMessages ();
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
  else if (a.name () == "project" && (a.mod () == "" || a.mod () == "not"))
  {
    if (a.value () != "")
    {
      if (a.mod () == "not")
      {
        f.push_back (Att ("project", "startswith", a.value (), "negative"));
        debug ("auto filter: " + a.name () + ".~startswith:" + a.value ());
      }
      else
      {
        f.push_back (Att ("project", "startswith", a.value ()));
        debug ("auto filter: " + a.name () + ".startswith:" + a.value ());
      }
    }
    else
    {
      f.push_back (Att ("project", "is", a.value ()));
      debug ("auto filter: " + a.name () + ".is:" + a.value ());
    }
  }

  // Recurrence periods are matched left-most.
  else if (a.name () == "recur" && a.mod () == "")
  {
    if (a.value () != "")
    {
      f.push_back (Att ("recur", "startswith", a.value ()));
      debug ("auto filter: " + a.name () + ".startswith:" + a.value ());
    }
    else
    {
      f.push_back (Att ("recur", "is", a.value ()));
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

  // Note: Tags are handled via the +/-<tag> syntax, but also via attribute
  // modifiers.

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
/*
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
*/
}

////////////////////////////////////////////////////////////////////////////////
// This capability is to answer the question of 'what did I just do to generate
// this output?'.
void Context::updateXtermTitle ()
{
  if (config.getBoolean ("xterm.title"))
  {
    std::string command;
    args.find_command (command);

    std::string title;
    join (title, " ", args.list ());
    std::cout << "]0;task " << command << " " << title << "" << std::endl;
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
