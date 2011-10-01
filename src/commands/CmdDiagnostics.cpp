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

#define L10N                                           // Localization complete.

#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <RX.h>
#include <Context.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <cmake.h>
#include <commit.h>

#ifdef HAVE_LIBLUA
extern "C"
{
  #include <lua.h>
}
#endif

#include <CmdDiagnostics.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDiagnostics::CmdDiagnostics ()
{
  _keyword     = "diagnostics";
  _usage       = "task          diagnostics";
  _description = STRING_CMD_DIAG_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
// This command will generate output that is intended to help diagnose problems.
//
// Although this will change over time, initially this command will answer the
// kind of questions we always have to ask whenever something is wrong.
int CmdDiagnostics::execute (std::string& output)
{
  Color bold ("bold");

  std::stringstream out;
  out << "\n"
      << bold.colorize (PACKAGE_STRING)
      << "\n";

  out << "  " << STRING_CMD_DIAG_PLATFORM << ": "
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
         STRING_CMD_DIAG_UNKNOWN
#endif
      << "\n\n";

  // Compiler.
  out << bold.colorize (STRING_CMD_DIAG_COMPILER)
      << "\n"
#ifdef __VERSION__
      << "   " << STRING_CMD_DIAG_VERSION << ": "
      << __VERSION__ << "\n"
#endif
      << "      " << STRING_CMD_DIAG_CAPS << ":"
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

  out << bold.colorize (STRING_CMD_DIAG_LIBRARIES)
      << "\n";

  out << "       Lua: "
#ifdef HAVE_LIBLUA
      << LUA_RELEASE
#else
      << "n/a"
#endif
      << "\n\n";

  out << bold.colorize (STRING_CMD_DIAG_FEATURES)
      << "\n"

  // Build date.
      << "     " << STRING_CMD_DIAG_BUILT << ": " << __DATE__ << " " << __TIME__ << "\n"
      << "    " << STRING_CMD_DIAG_COMMIT << ": " << COMMIT << "\n"
      << "     CMake: " << CMAKE_VERSION << "\n"
      << "      " << STRING_CMD_DIAG_CAPS << ":"
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
  out << bold.colorize (STRING_CMD_DIAG_CONFIG)
      << "\n"
      << "      File: " << context.config._original_file._data << " "
      << (context.config._original_file.exists ()
           ? STRING_CMD_DIAG_FOUND
           : STRING_CMD_DIAG_MISSING)
      << ", " << context.config._original_file.size () << " " << "bytes"
      << ", mode "
      << std::setbase (8)
      << context.config._original_file.mode ()
      << "\n";

  // Config: data.location found, readable, writable
  File location (context.config.get ("data.location"));
  out << "      Data: " << location._data << " "
      << (location.exists ()
           ? STRING_CMD_DIAG_FOUND
           : STRING_CMD_DIAG_MISSING)
      << ", " << (location.is_directory () ? "dir" : "?")
      << ", mode "
      << std::setbase (8)
      << location.mode ()
      << "\n";

  out << "   Locking: "
      << (context.config.getBoolean ("locking")
           ? STRING_CMD_DIAG_ENABLED
           : STRING_CMD_DIAG_DISABLED)
      << "\n";

  // Determine rc.editor/$EDITOR/$VISUAL.
  char* peditor;
  if (context.config.get ("editor") != "")
    out << " rc.editor: " << context.config.get ("editor") << "\n";
  else if ((peditor = getenv ("VISUAL")) != NULL)
    out << "   $VISUAL: " << peditor << "\n";
  else if ((peditor = getenv ("EDITOR")) != NULL)
    out << "   $EDITOR: " << peditor << "\n";

  out << "\n";

  // External commands.
  out << bold.colorize (STRING_CMD_DIAG_EXTERNAL)
      << "\n";
  {
    std::vector <std::string> matches;
    char buffer [1024] = {0};
    FILE* fp;
    if ((fp = popen ("scp 2>&1", "r")))
    {
      char* p = fgets (buffer, 1023, fp);
      pclose (fp);

      RX r ("usage", false);
      if (p)
        out << "       scp: "
            << (r.match (buffer)
                 ? STRING_CMD_DIAG_FOUND
                 : STRING_CMD_DIAG_MISSING)
            << "\n";
    }

    if ((fp = popen ("rsync --version 2>&1", "r")))
    {
      char* p = fgets (buffer, 1023, fp);
      pclose (fp);

      // rsync  version 2.6.9  protocol version 29
      if (p)
      {
        RX r ("version ([0-9]+\\.[0-9]+\\.[0-9]+)", false);
        matches.clear ();
        r.match (matches, buffer);
        out << "     rsync: "
            << (matches.size () ? matches[0] : STRING_CMD_DIAG_MISSING)
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
        RX r ("curl ([0-9]+\\.[0-9]+\\.[0-9]+)", false);
        matches.clear ();
        r.match (matches, buffer);
        out << "      curl: "
            << (matches.size () ? matches[0] :  STRING_CMD_DIAG_MISSING)
            << "\n";
      }
    }

    out << "\n";
  }

  // Generate 1000 UUIDs and verify they are all unique.
  out << bold.colorize (STRING_CMD_DIAG_TESTS)
      << "\n";
  {
    out << "  UUID gen: ";
    std::vector <std::string> uuids;
    std::string id;
    for (int i = 0; i < 1000; i++)
    {
      id = uuid ();
      if (std::find (uuids.begin (), uuids.end (), id) != uuids.end ())
      {
        out << format (STRING_CMD_DIAG_UUID_BAD, i) << "\n";
        break;
      }
      else
        uuids.push_back (id);
    }

    if (uuids.size () >= 1000)
      out << STRING_CMD_DIAG_UUID_GOOD << "\n";

    // Determine terminal details.
    const char* term = getenv ("TERM");
    out << "     $TERM: "
        << (term ? term : STRING_CMD_DIAG_NONE)
        << " ("
        << context.getWidth ()
        << "x"
        << context.getHeight ()
        << ")\n";
  }

  out << "\n";
  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
