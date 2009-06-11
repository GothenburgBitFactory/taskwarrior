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
  std::string out;
  return out;
}

////////////////////////////////////////////////////////////////////////////////
