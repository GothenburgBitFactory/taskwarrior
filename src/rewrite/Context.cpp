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

#include <pwd.h>
#include "Context.h"
#include "text.h"
#include "util.h"
#include "task.h"
#include "../auto.h"

////////////////////////////////////////////////////////////////////////////////
Context::Context ()
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
  config   = other.config;
  filter   = other.filter;
  keymap   = other.keymap;
  sequence = other.sequence;
  task     = other.task;
  tdb      = other.tdb;
}

////////////////////////////////////////////////////////////////////////////////
Context& Context::operator= (const Context& other)
{
  throw std::string ("unimplemented Context::operator=");
  if (this != &other)
  {
    config   = other.config;
    filter   = other.filter;
    keymap   = other.keymap;
    sequence = other.sequence;
    task     = other.task;
    tdb      = other.tdb;
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
  // Load the config file from the home directory.  If the file cannot be
  // found, offer to create a sample one.
  loadCorrectConfigFile (argc, argv);

  // When redirecting output to a file, do not use color, curses.
  if (!isatty (fileno (stdout)))
  {
    config.set ("curses", "off");

    if (! config.get (std::string ("_forcecolor"), false))
      config.set ("color",  "off");
  }

  // TODO Handle "--version, -v" right here.

  // init TDB.
  std::string location = config.get ("data.location");
  std::vector <std::string> all;
  split (all, location, ',');
  foreach (path, all)
    tdb.location (expandPath (*path));

  // Allow user override of file locking.  Solaris/NFS machines may want this.
  if (! config.get ("locking", true))
    tdb.noLock ();

  // TODO Load pending.data.
  // TODO Load completed.data.
  // TODO Load deleted.data.
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  throw std::string ("unimplemented Context::run");
  // TODO Dispatch to command handlers.
  // TODO Auto shadow update.
  // TODO Auto gc.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadCorrectConfigFile (int argc, char** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    if (! strncmp (argv[i], "rc:", 3))
    {
      if (! access (&(argv[i][3]), F_OK))
      {
        std::string file = &(argv[i][3]);
        config.load (file);
        return;
      }
      else
        throw std::string ("Could not read configuration file '") + &(argv[i][3]) + "'";
    }
  }

  struct passwd* pw = getpwuid (getuid ());
  if (!pw)
    throw std::string ("Could not read home directory from passwd file.");

  std::string file = pw->pw_dir;
  config.createDefault (file);
}

////////////////////////////////////////////////////////////////////////////////
