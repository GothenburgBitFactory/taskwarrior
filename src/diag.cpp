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
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <rx.h>
#include <File.h>
#include <main.h>
#include <util.h>
#include "../cmake.h"
#ifdef HAVE_COMMIT
#include "../commit.h"
#endif

#ifdef HAVE_LIBLUA
extern "C"
{
  #include <lua.h>
}
#endif

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// This command will generate output that is intended to help diagnose problems.
//
// Although this will change over time, initially this command will answer the
// kind of questions we always have to ask whenever something is wrong.
void handleDiagnostics (std::string& outs)
{
  std::cout << "\n[1m" << PACKAGE_STRING << "[0m\n";

  std::cout << "  Platform: "
            <<
#if defined (DARWIN)
               "Darwin"
#elif defined (SOLARIS)
               "Solaris"
#elif defined (CYGWIN)
               "Cygwin"
#elif defined (OPENBSD)
               "OpenBSD"
#elif defined (HAIKU)
               "Haiku"
#elif defined (FREEBSD)
               "FreeBSD"
#elif defined (LINUX)
               "Linux"
#else
               "<unknown>"
#endif
            << "\n\n";

  // Compiler.
  std::cout << "[1mCompiler[0m\n"
#ifdef __VERSION__
            << "   Version: " << __VERSION__ << "\n"
#endif
            << "      Caps:"
#ifdef __STDC__
            << " +stdc"
#endif
#ifdef __STDC_HOSTED__
            << " +stdc_hosted"
#endif
#ifdef __STDC_VERSION__
            << " +" << __STDC_VERSION__
#endif
#ifdef _POSIX_VERSION
            << " +" << _POSIX_VERSION
#endif
#ifdef _POSIX2_C_VERSION
            << " +" << _POSIX2_C_VERSION
#endif
#ifdef _ILP32
            << " +ILP32"
#endif
#ifdef _LP64
            << " +LP64"
#endif
            << " +c" << sizeof (char)
            << " +i" << sizeof (int)
            << " +l" << sizeof (long)
            << " +vp" << sizeof (void*)
            << "\n\n";

  std::cout << "[1mLibraries[0m\n";

  std::cout << "  Readline: "
#ifdef HAVE_LIBREADLINE
            << std::setbase (16) << RL_READLINE_VERSION
#else
            << "n/a"
#endif
            << "\n";

  std::cout << "       Lua: "
#ifdef HAVE_LIBLUA
            << LUA_RELEASE
#else
            << "n/a"
#endif
            << "\n\n";

  std::cout << "[1mBuild Features[0m\n"
  // Build date.
            << "     Built: " << __DATE__ << " " << __TIME__ << "\n"
#ifdef HAVE_COMMIT
            << "    Commit: " << COMMIT << "\n"
#endif
#ifdef HAVE_CMAKE
            << "     CMake: " << CMAKE_VERSION << "\n"
#endif
            << "      Caps:"
#ifdef HAVE_LIBPTHREAD
            << " +pthreads"
#else
            << " -pthreads"
#endif

#ifdef HAVE_SRANDOM
            << " +srandom"
#else
            << " -srandom"
#endif

#ifdef HAVE_RANDOM
            << " +random"
#else
            << " -random"
#endif

#ifdef HAVE_UUID
            << " +uuid"
#else
            << " -uuid"
#endif
            << "\n\n";

  // Config: .taskrc found, readable, writable
  std::cout << "[1mConfiguration[0m\n"
            << "      File: " << context.config.original_file.data
            << (context.config.original_file.exists () ? " (found)" : " (missing)")
            << ", " << context.config.original_file.size () << " bytes"
            << ", mode "
            << std::setbase (8)
            << context.config.original_file.mode ()
            << "\n";

  // Config: data.location found, readable, writable
  File location (context.config.get ("data.location"));
  std::cout << "      Data: " << location.data
            << (location.exists () ? " (found)" : " (missing)")
            << ", " << (location.is_directory () ? "dir" : "?")
            << ", mode "
            << std::setbase (8)
            << location.mode ()
            << "\n";

  std::cout << "   Locking: "
            << (context.config.getBoolean ("locking") ? "Enabled" : "Disabled")
            << "\n";

  std::cout << "     Regex: "
            << (context.config.getBoolean ("regex") ? "Enabled" : "Disabled")
            << "\n";

  // Determine rc.editor/$EDITOR/$VISUAL.
  char* peditor;
  if (context.config.get ("editor") != "")
    std::cout << " rc.editor: " << context.config.get ("editor") << "\n";
  else if ((peditor = getenv ("VISUAL")) != NULL)
    std::cout << "   $VISUAL: " << peditor << "\n";
  else if ((peditor = getenv ("EDITOR")) != NULL)
    std::cout << "   $EDITOR: " << peditor << "\n";

  std::cout << "\n";

  // External commands.
  std::cout << "[1mExternal Utilities[0m\n";
  {
    std::vector <std::string> matches;
    char buffer [1024] = {0};
    FILE* fp;
    if ((fp = popen ("scp 2>&1", "r")))
    {
      char* p = fgets (buffer, 1023, fp);
      pclose (fp);

      if (p)
        std::cout << "       scp: "
                  << (regexMatch (buffer, "usage") ? "found" : "n/a")
                  << "\n";
    }

    if ((fp = popen ("rsync --version 2>&1", "r")))
    {
      char* p = fgets (buffer, 1023, fp);
      pclose (fp);

      // rsync  version 2.6.9  protocol version 29
      if (p)
      {
        matches.clear ();
        regexMatch (matches, buffer, "version ([0-9]+\\.[0-9]+\\.[0-9]+)");
        std::cout << "     rsync: "
                  << (matches.size () ? matches[0] : "n/a")
                  << "\n";
      }
    }

    if ((fp = popen ("curl --version 2>&1", "r")))
    {
      char* p = fgets (buffer, 1023, fp);
      pclose (fp);

      // curl 7.19.7 (universal-apple-darwin10.0) libcurl/7.19.7 OpenSSL/0.9.8l zlib/1.2.3
      if (p)
      {
        matches.clear ();
        regexMatch (matches, buffer, "curl ([0-9]+\\.[0-9]+\\.[0-9]+)");
        std::cout << "      curl: "
                  << (matches.size () ? matches[0] : "n/a")
                  << "\n";
      }
    }

    std::cout << "\n";
  }

  // Generate 1000 UUIDs and verify they are all unique.
  std::cout << "[1mTests[0m\n";
  {
    std::cout << "  UUID gen: ";
    std::vector <std::string> uuids;
    std::string id;
    for (int i = 0; i < 1000; i++)
    {
      id = uuid ();
      if (std::find (uuids.begin (), uuids.end (), id) != uuids.end ())
      {
        std::cout << "Failed - duplicate UUID at iteration " << i << "\n";
        break;
      }
      else
        uuids.push_back (id);
    }

    if (uuids.size () >= 1000)
      std::cout << "1000 unique UUIDs generated.\n";

    // Determine terminal details.
    const char* term = getenv ("TERM");
    std::cout << "     $TERM: "
              << (term ? term : "-none=")
              << " ("
              << context.getWidth ()
              << "x"
              << context.getHeight ()
              << ")\n";
  }

  std::cout << "\n";
}

////////////////////////////////////////////////////////////////////////////////

