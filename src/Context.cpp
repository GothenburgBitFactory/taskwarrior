////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Context.h>
#include <Directory.h>
#include <File.h>
#include <text.h>
#include <util.h>
#include <main.h>
#include <i18n.h>
#include <cmake.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif

////////////////////////////////////////////////////////////////////////////////
Context::Context ()
: rc_file ()
, data_dir ()
, extension_dir ()
, config ()
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
  std::map<std::string, Command*>::iterator com;
  for (com = commands.begin (); com != commands.end (); ++com)
    delete com->second;

  std::map<std::string, Column*>::iterator col;
  for (col = columns.begin (); col != columns.end (); ++col)
    delete col->second;
}

////////////////////////////////////////////////////////////////////////////////
int Context::initialize (int argc, const char** argv)
{
  timer_init.start ();
  int rc = 0;

  try
  {
    // char** argv --> std::vector <std::string> Context::a3.
    a3.capture (argc, argv);

    // echo one two -- three | task zero --> task zero one two
    // 'three' is left in the input buffer.
    a3.append_stdin ();

    // Assume default .taskrc and .task locations.
    assumeLocations ();

    // Process 'rc:<file>' command line override, and remove the argument from the
    // Context::a3.
    a3.categorize ();
    a3.rc_override (home_dir, rc_file);

    // TASKRC environment variable overrides the command line.
    char* override = getenv ("TASKRC");
    if (override)
    {
      rc_file = File (override);
      header (format (STRING_CONTEXT_RC_OVERRIDE, rc_file._data));
    }

    // Dump any existing values and load rc file.
    config.clear ();
    config.load  (rc_file);

    // The data location, Context::data_dir, is determined from the assumed
    // location (~/.task), or set by data.location in the config file, or
    // overridden by rc.data.location on the command line.
    std::string location;
    a3.get_data_location (location);
    data_dir = Directory (location);

    override = getenv ("TASKDATA");
    if (override)
    {
      data_dir = Directory (override);
      config.set ("data.location", data_dir._data);
      header (format (STRING_CONTEXT_DATA_OVERRIDE, data_dir._data));
    }

/* TODO Enable this when the time is right, say for 2.1
    extension_dir = data_dir._data + "/extensions";
*/

    // Create missing config file and data directory, if necessary.
    a3.apply_overrides ();
    createDefaultConfig ();

    // Handle Aliases.
    loadAliases ();
    a3.resolve_aliases ();

    // Apply rc overrides to Context::config, capturing raw args for later use.
    a3.apply_overrides ();

    // Initialize the color rules, if necessary.
    if (color ())
      initializeColorRules ();

    // Instantiate built-in command objects.
    Command::factory (commands);

    // Instantiate built-in column objects.
    Column::factory (columns);

    // Static initialization to decouple code.
    staticInitialization ();

    // Categorize all arguments one more time.  THIS IS NECESSARY - it helps the
    // following inject_defaults method determine whether there needs to be a
    // default command assumed.
    a3.categorize ();

    // Handle default command and assumed 'info' command.
    a3.inject_defaults ();

    // The re-categorization allows all injected arguments to be properly given
    // a category.
    a3.categorize ();
    a3.dump ("Context::initialize");

    // TODO Instantiate extension command objects.
    // TODO Instantiate default command object.

    // TODO Instantiate extension column objects.

    // TODO Instantiate extension UDA objects.
    // TODO Instantiate extension format objects.

    // If there is a locale variant (en-US.<variant>), then strip it.
    std::string locale = config.get ("locale");
    std::string::size_type period = locale.find ('.');
    if (period != std::string::npos)
      locale = locale.substr (0, period);

    // Initialize the database.
    tdb2.set_location (data_dir);

    // Hook system init, plus post-start event occurring at the first possible
    // moment after hook initialization.
    hooks.initialize ();
    hooks.trigger ("on-launch");
  }

  catch (const std::string& message)
  {
    error (message);
    rc = 2;
  }

  catch (...)
  {
    error (STRING_UNKNOWN_ERROR);
    rc = 3;
  }

  // Dump all debug messages, controlled by rc.debug.
  if (rc && config.getBoolean ("debug"))
  {
    std::vector <std::string>::iterator d;
    for (d = debugMessages.begin (); d != debugMessages.end (); ++d)
      if (color ())
        std::cerr << colorizeDebug (*d) << "\n";
      else
        std::cerr << *d << "\n";
  }

  // Dump all headers, controlled by 'header' verbosity token.
  if (rc && verbose ("header"))
  {
    std::vector <std::string>::iterator h;
    for (h = headers.begin (); h != headers.end (); ++h)
      if (color ())
        std::cerr << colorizeHeader (*h) << "\n";
      else
        std::cerr << *h << "\n";
  }

  // Dump all footnotes, controlled by 'footnote' verbosity token.
  if (rc && verbose ("footnote"))
  {
    std::vector <std::string>::iterator f;
    for (f = footnotes.begin (); f != footnotes.end (); ++f)
      if (color ())
        std::cerr << colorizeFootnote (*f) << "\n";
      else
        std::cerr << *f << "\n";
  }

  // Dump all errors, non-maskable.
  // Colorized as footnotes.
  if (rc)
  {
    std::vector <std::string>::iterator e;
    for (e = errors.begin (); e != errors.end (); ++e)
      if (color ())
        std::cerr << colorizeFootnote (*e) << "\n";
      else
        std::cerr << *e << "\n";
  }

  timer_init.stop ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  int rc;
  std::string output;

  try
  {
    rc = dispatch (output);

    std::stringstream s;
    s << "Perf "
      << PACKAGE_STRING
      << " "
#ifdef HAVE_COMMIT
      << COMMIT
#else
      << "-"
#endif
      << " "
      << Date ().toISO ()

      << " init:"   << timer_init.total ()
      << " load:"   << timer_load.total ()
      << " gc:"     << timer_gc.total ()
      << " filter:" << timer_filter.total ()
      << " commit:" << timer_commit.total ()
      << " sort:"   << timer_sort.total ()
      << " render:" << timer_render.total ()
      << " total:"  << (timer_init.total ()   +
                        timer_load.total ()   +
                        timer_gc.total ()     +
                        timer_filter.total () +
                        timer_commit.total () +
                        timer_sort.total ()   +
                        timer_render.total ())
      << "\n";
    debug (s.str ());
  }

  catch (const std::string& message)
  {
    error (message);
    rc = 2;
  }

  catch (...)
  {
    error (STRING_UNKNOWN_ERROR);
    rc = 3;
  }

  // Dump all debug messages, controlled by rc.debug.
  if (config.getBoolean ("debug"))
  {
    std::vector <std::string>::iterator d;
    for (d = debugMessages.begin (); d != debugMessages.end (); ++d)
      if (color ())
        std::cerr << colorizeDebug (*d) << "\n";
      else
        std::cerr << *d << "\n";
  }

  // Dump all headers, controlled by 'header' verbosity token.
  if (verbose ("header"))
  {
    std::vector <std::string>::iterator h;
    for (h = headers.begin (); h != headers.end (); ++h)
      if (color ())
        std::cerr << colorizeHeader (*h) << "\n";
      else
        std::cerr << *h << "\n";
  }

  // Dump the report output.
  std::cout << output;

  // Dump all footnotes, controlled by 'footnote' verbosity token.
  if (verbose ("footnote"))
  {
    std::vector <std::string>::iterator f;
    for (f = footnotes.begin (); f != footnotes.end (); ++f)
      if (color ())
        std::cerr << colorizeFootnote (*f) << "\n";
      else
        std::cerr << *f << "\n";
  }

  // Dump all errors, non-maskable.
  // Colorized as footnotes.
  std::vector <std::string>::iterator e;
  for (e = errors.begin (); e != errors.end (); ++e)
    if (color ())
      std::cerr << colorizeError (*e) << "\n";
    else
      std::cerr << *e << "\n";

  hooks.trigger ("on-exit");
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// Locate and dispatch to the command whose keyword matches via autoComplete
// with the earliest argument.
int Context::dispatch (std::string &out)
{
  // Autocomplete args against keywords.
  std::string command;
  if (a3.find_command (command))
  {
    updateXtermTitle ();

    updateVerbosity ();

    Command* c = commands[command];

    // GC is invoked prior to running any command that displays task IDs, if
    // possible.
    bool auto_commit = false;
    if (c->displays_id () && !tdb2.read_only ())
    {
      tdb2.gc ();
      auto_commit = true;
    }

    // Only read-only commands can be run when TDB2 is read-only.
    // TODO Implement TDB2::read_only
/*
    if (tdb2.read_only () && !c->read_only ())
      throw std::string ("");
*/

    int rc = c->execute (out);
    if (auto_commit)
      tdb2.commit ();

    // Write commands cause an update of the shadow file, if configured.
    if (! c->read_only ())
      shadow ();

    return rc;
  }

  assert (commands["help"]);
  return commands["help"]->execute (out);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
bool Context::color ()
{
#ifdef FEATURE_COLOR
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
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Support verbosity levels:
//
//   rc.verbose=1          Show all feedback.
//   rc.verbose=0          Show regular feedback.
//   rc.verbose=nothing    Show the absolute minimum.
//   rc.verbose=one,two    Show verbosity for 'one' and 'two' only.
//
// TODO This mechanism is clunky, and should slowly evolve into something more
//      logical and consistent.  This should probably mean that 'nothing' should
//      take the place of '0'.
bool Context::verbose (const std::string& token)
{
  if (! verbosity.size ())
  {
    verbosity_legacy = config.getBoolean ("verbose");
    split (verbosity, config.get ("verbose"), ',');

    // Regular feedback means almost everything.
    if (!verbosity_legacy               &&
        verbosity.size () == 1          &&
        verbosity[0]      != "nothing"  &&
        verbosity[0]      != "blank"    &&  // This list must be complete.
        verbosity[0]      != "header"   &&  //
        verbosity[0]      != "footnote" &&  //
        verbosity[0]      != "label"    &&  //
        verbosity[0]      != "new-id"   &&  //
        verbosity[0]      != "affected" &&  //
        verbosity[0]      != "edit"     &&  //
        verbosity[0]      != "special"  &&  //
        verbosity[0]      != "project"  &&  //
        verbosity[0]      != "sync")        //
    {
      verbosity.clear ();

      // This list emulates rc.verbose=off in version 1.9.4.
      verbosity.push_back ("blank");
      verbosity.push_back ("label");
      verbosity.push_back ("new-id");
      verbosity.push_back ("edit");
    }
  }

  // rc.verbose=true|y|yes|1|on overrides all.
  if (verbosity_legacy)
    return true;

  // rc.verbose=nothing overrides all.
  if (verbosity[0] == "nothing")
    return false;

  // Specific token match.
  if (std::find (verbosity.begin (), verbosity.end (), token) != verbosity.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// This needs to be taken out and shot, as soon as hooks will allow.
void Context::shadow ()
{
  std::string file_name = config.get ("shadow.file");
  std::string command   = config.get ("shadow.command");
  std::string rcfile    = rc_file;

  // A missing shadow file command uses the default command instead.
  if (command == "")
    command = config.get ("default.command");

  if (file_name != "" &&
      command != "")
  {
    File shadow_file (file_name);

    // Check for dangerous shadow file settings.
    std::string location = config.get ("data.location");
    if (shadow_file._data == location + "/pending.data")
      throw std::string (STRING_CONTEXT_SHADOW_P);

    if (shadow_file._data == location + "/completed.data")
      throw std::string (STRING_CONTEXT_SHADOW_C);

    if (shadow_file._data == location + "/undo.data")
      throw std::string (STRING_CONTEXT_SHADOW_U);

    if (shadow_file._data == location + "/backlog.data")
      throw std::string (STRING_CONTEXT_SHADOW_B);

    // Compose the command.  Put the rc overrides up front, so that they may
    // be overridden by rc.shadow.command.
    command = program               +
              " rc.detection:off"   +  // No need to determine terminal size
              " rc.color:off"       +  // Color off by default
              " rc.gc:off "         +  // GC off, to reduce headaches
              " rc.locking:off"     +  // No file locking
              " rc:" + rcfile + " " +  // Use specified rc file
              command               +  // User specified command
              " >"                  +  // Capture
              shadow_file._data;       // User specified file

    debug ("Running shadow command: " + command);
    system (command.c_str ());

    // Optionally display a notification that the shadow file was updated.
    // TODO Convert to a verbosity token.
    if (config.getBoolean ("shadow.notify"))
      footnote (format (STRING_CONTEXT_SHADOW_UPDATE, shadow_file._data));
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> Context::getColumns () const
{
  std::vector <std::string> output;
  std::map <std::string, Column*>::const_iterator i;
  for (i = columns.begin (); i != columns.end (); ++i)
    output.push_back (i->first);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> Context::getCommands () const
{
  std::vector <std::string> output;
  std::map <std::string, Command*>::const_iterator i;
  for (i = commands.begin (); i != commands.end (); ++i)
    output.push_back (i->first);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
// The 'Task' object, among others, is shared between projects.  To make this
// easier, it has been decoupled from Context.
void Context::staticInitialization ()
{
  Task::defaultProject      = config.get ("default.project");
  Task::defaultPriority     = config.get ("default.priority");
  Task::defaultDue          = config.get ("default.due");
  Task::searchCaseSensitive = config.getBoolean ("search.case.sensitive");
  Task::regex               = config.getBoolean ("regex");

  std::map <std::string, Column*>::iterator i;
  for (i = columns.begin (); i != columns.end (); ++i)
    Task::attributes[i->first] = i->second->type ();

  Task::urgencyPriorityCoefficient    = config.getReal ("urgency.priority.coefficient");
  Task::urgencyProjectCoefficient     = config.getReal ("urgency.project.coefficient");
  Task::urgencyActiveCoefficient      = config.getReal ("urgency.active.coefficient");
  Task::urgencyScheduledCoefficient   = config.getReal ("urgency.scheduled.coefficient");
  Task::urgencyWaitingCoefficient     = config.getReal ("urgency.waiting.coefficient");
  Task::urgencyBlockedCoefficient     = config.getReal ("urgency.blocked.coefficient");
  Task::urgencyAnnotationsCoefficient = config.getReal ("urgency.annotations.coefficient");
  Task::urgencyTagsCoefficient        = config.getReal ("urgency.tags.coefficient");
  Task::urgencyNextCoefficient        = config.getReal ("urgency.next.coefficient");
  Task::urgencyDueCoefficient         = config.getReal ("urgency.due.coefficient");
  Task::urgencyBlockingCoefficient    = config.getReal ("urgency.blocking.coefficient");
  Task::urgencyAgeCoefficient         = config.getReal ("urgency.age.coefficient");

  // Tag- and project-specific coefficients.
  std::vector <std::string> all;
  config.all (all);

  std::vector <std::string>::iterator var;
  for (var = all.begin (); var != all.end (); ++var)
  {
    if (var->substr (0, 13) == "urgency.user." ||
        var->substr (0, 12) == "urgency.uda.")
      Task::coefficients[*var] = config.getReal (*var);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::assumeLocations ()
{
  rc_file  = File      ("~/.taskrc");
  data_dir = Directory ("~/.task");
}

////////////////////////////////////////////////////////////////////////////////
void Context::createDefaultConfig ()
{
  // Do we need to create a default rc?
  if (! rc_file.exists ())
  {
    if (config.getBoolean ("confirmation") &&
        !confirm (format (STRING_CONTEXT_CREATE_RC, home_dir, rc_file._data)))
      throw std::string (STRING_CONTEXT_NEED_RC);

    config.createDefaultRC (rc_file, data_dir._original);
  }

  // Create data location, if necessary.
  config.createDefaultData (data_dir);

  // Create extension directory, if necessary.
/* TODO Enable this when the time is right, say for 2.4
  if (! extension_dir.exists ())
    extension_dir.create ();
*/
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
  tdb2.clear ();
  a3.clear ();

  // Eliminate the command objects.
  std::map <std::string, Command*>::iterator com;
  for (com = commands.begin (); com != commands.end (); ++com)
    delete com->second;

  commands.clear ();

  // Eliminate the column objects.
  std::map <std::string, Column*>::iterator col;
  for (col = columns.begin (); col != columns.end (); ++col)
    delete col->second;

  columns.clear ();

  clearMessages ();
}

////////////////////////////////////////////////////////////////////////////////
// This capability is to answer the question of 'what did I just do to generate
// this output?'.
void Context::updateXtermTitle ()
{
  if (config.getBoolean ("xterm.title") && isatty (fileno (stdout)))
  {
    std::string command;
    a3.find_command (command);

    std::string title;
    join (title, " ", a3.list ());
    std::cout << "]0;task " << command << " " << title << "";
  }
}

////////////////////////////////////////////////////////////////////////////////
// This function allows a clean output if the command is a helper subcommand.
void Context::updateVerbosity ()
{
  std::string command;
  a3.find_command (command);

  if (command[0] == '_')
  {
    verbosity.clear ();
    verbosity.push_back ("nothing");
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::header (const std::string& input)
{
  if (input.length () &&
      std::find (headers.begin (), headers.end (), input) == headers.end ())
    headers.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::footnote (const std::string& input)
{
  if (input.length () &&
      std::find (footnotes.begin (), footnotes.end (), input) == footnotes.end ())
    footnotes.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::error (const std::string& input)
{
  if (input.length () &&
      std::find (errors.begin (), errors.end (), input) == errors.end ())
    errors.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::debug (const std::string& input)
{
  if (input.length () &&
      std::find (debugMessages.begin (), debugMessages.end (), input) == debugMessages.end ())
    debugMessages.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::clearMessages ()
{
  headers.clear ();
  footnotes.clear ();
  errors.clear ();
  debugMessages.clear ();
}

////////////////////////////////////////////////////////////////////////////////
