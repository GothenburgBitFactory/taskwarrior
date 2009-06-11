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
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <time.h>

#include "Context.h"
#include "Date.h"
#include "Duration.h"
#include "Table.h"
#include "T.h"
#include "text.h"
#include "util.h"
#include "main.h"

#ifdef HAVE_LIBNCURSES
#include <ncurses.h>
#endif

// Global context for use by all.
Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  // Set up randomness.
#ifdef HAVE_SRANDOM
  srandom (time (NULL));
#else
  srand (time (NULL));
#endif

  int status = 0;

  try
  {
    context.initialize (argc, argv);
    if (context.program.find ("itask") != std::string::npos)
      status = context.interactive ();
    else
      status = context.run ();

// start OBSOLETE
/*
    TDB tdb;
    std::string dataLocation = expandPath (context.config.get ("data.location"));
    tdb.dataDirectory (dataLocation);

    // Allow user override of file locking.  Solaris/NFS machines may want this.
    if (! context.config.get ("locking", true))
      tdb.noLock ();

    // Check for silly shadow file settings.
    std::string shadowFile = expandPath (context.config.get ("shadow.file"));
    if (shadowFile != "")
    {
      if (shadowFile == dataLocation + "/pending.data")
        throw std::string ("Configuration variable 'shadow.file' is set to "
                           "overwrite your pending tasks.  Please change it.");

      if (shadowFile == dataLocation + "/completed.data")
        throw std::string ("Configuration variable 'shadow.file' is set to "
                           "overwrite your completed tasks.  Please change it.");
    }

    std::cout << runTaskCommand (context.args, tdb);
*/
// end OBSOLETE
  }

  catch (std::string& error)
  {
    std::cout << error << std::endl;
    return -1;
  }

  catch (...)
  {
    std::cerr << context.stringtable.get (100, "Unknown error.") << std::endl;
    return -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
// TODO Obsolete
void updateShadowFile ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO Obsolete
std::string runTaskCommand (
  int argc,
  char** argv,
//  TDB& tdb,
  bool gc /* = true */,
  bool shadow /* = true */)
{
/*
  std::vector <std::string> args;
  for (int i = 1; i < argc; ++i)
    if (strncmp (argv[i], "rc:", 3) &&
        strncmp (argv[i], "rc.", 3))
{
std::cout << "arg=" << argv[i] << std::endl;
      args.push_back (argv[i]);
}

  return runTaskCommand (args, tdb, gc, shadow);
*/
  return "";
}

////////////////////////////////////////////////////////////////////////////////
// TODO Obsolete
std::string runTaskCommand (
  std::vector <std::string>& args,
//  TDB& tdb,
  bool gc /* = false */,
  bool shadow /* = false */)
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
*/
  std::string out;
/*
  // Read-only commands with no side effects.
       if (command == "export")             { out = handleExport          (tdb, task); }
  else if (command == "info")               { out = handleInfo            (tdb, task); }
  else if (command == "stats")              { out = handleReportStats     (tdb, task); }
  else if (command == "history")            { out = handleReportHistory   (tdb, task); }
  else if (command == "ghistory")           { out = handleReportGHistory  (tdb, task); }
  else if (command == "calendar")           { out = handleReportCalendar  (tdb, task); }
  else if (command == "summary")            { out = handleReportSummary   (tdb, task); }
  else if (command == "timesheet")          { out = handleReportTimesheet (tdb, task); }

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

  // Only update the shadow file if such an update was not suppressed (shadow),
  // and if an actual change occurred (gcMod || cmdMod).
  if (shadow && (gcMod || cmdMod))
    updateShadowFile (tdb);
*/

  return out;
}

////////////////////////////////////////////////////////////////////////////////
