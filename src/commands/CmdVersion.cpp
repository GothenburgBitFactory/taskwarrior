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

#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <cmake.h>
#include <commit.h>
#include <CmdVersion.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdVersion::CmdVersion ()
{
  _keyword     = "version";
  _usage       = "task version";
  _description = "Shows the taskwarrior version number.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdVersion::execute (const std::string& command_line, std::string& output)
{
  std::stringstream out;

  // Create a table for the disclaimer.
  int width = context.getWidth ();
  ViewText disclaimer;
  disclaimer.width (width);
  disclaimer.add (Column::factory ("string", ""));
  disclaimer.set (disclaimer.addRow (), 0,
      "Taskwarrior may be copied only under the terms of the GNU General Public "
      "License, which may be found in the taskwarrior source kit.");

  // Create a table for the URL.
  ViewText link;
  link.width (width);
  link.add (Column::factory ("string", ""));
  link.set (link.addRow (), 0,
    "Documentation for taskwarrior can be found using 'man task', 'man taskrc', "
    "'man task-tutorial', 'man task-color', 'man task-sync', 'man task-faq' or at "
    "http://taskwarrior.org");

  Color bold ("bold");

  out << "\n"
      << (context.color () ? bold.colorize (PACKAGE) : PACKAGE)
      << " "
      << (context.color () ? bold.colorize (VERSION) : VERSION)
      << " built for "

#if defined (DARWIN)
      << "darwin"
#elif defined (SOLARIS)
      << "solaris"
#elif defined (CYGWIN)
      << "cygwin"
#elif defined (OPENBSD)
      << "openbsd"
#elif defined (HAIKU)
      << "haiku"
#elif defined (FREEBSD)
      << "freebsd"
#elif defined (LINUX)
      << "linux"
#else
      << "unknown"
#endif

#ifdef HAVE_LIBREADLINE
      << "-readline"
#endif

#ifdef HAVE_LIBLUA
      << "-lua"
#endif

      << "\n"
      << "Copyright (C) 2006 - 2011 P. Beckingham, F. Hernandez.\n"
#ifdef HAVE_LIBLUA
      << "Portions of this software Copyright (C) 1994 – 2008 Lua.org, PUC-Rio.\n"
#endif
      << "\n"
      << disclaimer.render ()
      << "\n"
      << link.render ()
      << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionVersion::CmdCompletionVersion ()
{
  _keyword     = "_version";
  _usage       = "task _version";
  _description = "Shows only the taskwarrior version number.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionVersion::execute (
  const std::string& command_line,
  std::string& output)
{
#ifdef HAVE_COMMIT
  output = COMMIT;
#else
  output = VERSION;
#endif
  output += "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
