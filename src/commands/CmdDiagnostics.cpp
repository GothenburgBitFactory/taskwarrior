////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <cmake.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <RX.h>
#include <Context.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif

#ifdef HAVE_LIBGNUTLS
#include <gnutls/gnutls.h>
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
  Color bold;
  if (context.color ())
    bold = Color ("bold");

  std::stringstream out;
  out << "\n"
      << bold.colorize (PACKAGE_STRING)
      << "\n";

  out << "   " << STRING_CMD_DIAG_PLATFORM << ": "
      <<
#if defined (DARWIN)
         "Darwin"
#elif defined (SOLARIS)
         "Solaris"
#elif defined (CYGWIN)
         "Cygwin"
#elif defined (HAIKU)
         "Haiku"
#elif defined (OPENBSD)
         "OpenBSD"
#elif defined (FREEBSD)
         "FreeBSD"
#elif defined (NETBSD)
         "NetBSD"
#elif defined (LINUX)
         "Linux"
#elif defined (KFREEBSD)
         "GNU/kFreeBSD"
#elif defined (GNUHURD)
         "GNU/Hurd"
#else
         STRING_CMD_DIAG_UNKNOWN
#endif
      << "\n\n";

  // Compiler.
  out << bold.colorize (STRING_CMD_DIAG_COMPILER)
      << "\n"
#ifdef __VERSION__
      << "    " << STRING_CMD_DIAG_VERSION << ": "
      << __VERSION__ << "\n"
#endif
      << "       " << STRING_CMD_DIAG_CAPS << ":"
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
      << " +c"      << 8 * sizeof (char)
      << " +i"      << 8 * sizeof (int)
      << " +l"      << 8 * sizeof (long)
      << " +vp"     << 8 * sizeof (void*)
      << " +time_t" << 8 * sizeof (time_t)
      << "\n";

  // Compiler compliance level.
  std::string compliance = "non-compliant";
#ifdef __cplusplus
  int level = __cplusplus;
  if (level == 199711)
    compliance = "C++98/03";
  else if (level == 201103)
    compliance = "C++11";
  else
    compliance = format (level);
#endif
  out << " " << STRING_CMD_DIAG_COMPLIANCE
      << ": "
      << compliance
      << "\n\n";

  out << bold.colorize (STRING_CMD_DIAG_FEATURES)
      << "\n"

  // Build date.
      << "      " << STRING_CMD_DIAG_BUILT << ": " << __DATE__ << " " << __TIME__ << "\n"
#ifdef HAVE_COMMIT
      << "     " << STRING_CMD_DIAG_COMMIT << ": " << COMMIT << "\n"
#endif
      << "      CMake: " << CMAKE_VERSION << "\n"
      << "       " << STRING_CMD_DIAG_CAPS << ":"
#ifdef HAVE_LIBPTHREAD
      << " +pthreads"
#else
      << " -pthreads"
#endif

#ifdef HAVE_LIBGNUTLS
      << " +tls"
#else
      << " -tls"
#endif
      << "\n";

  out << "    libuuid: "
#ifdef HAVE_UUID_UNPARSE_LOWER
      << "libuuid + uuid_unparse_lower"
#else
      << "libuuid, no uuid_unparse_lower"
#endif
      << "\n";

  out << "  libgnutls: "
#ifdef HAVE_LIBGNUTLS
#ifdef GNUTLS_VERSION
      << GNUTLS_VERSION
#elif defined LIBGNUTLS_VERSION
      << LIBGNUTLS_VERSION
#endif
#else
      << "n/a"
#endif
      << "\n";

  out << " Build type: "
#ifdef CMAKE_BUILD_TYPE
      << CMAKE_BUILD_TYPE
#else
      << "-"
#endif
      << "\n\n";

  // Config: .taskrc found, readable, writable
  out << bold.colorize (STRING_CMD_DIAG_CONFIG)
      << "\n"
      << "       File: " << context.config._original_file._data << " "
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
  out << "       Data: " << location._data << " "
      << (location.exists ()
           ? STRING_CMD_DIAG_FOUND
           : STRING_CMD_DIAG_MISSING)
      << ", " << (location.is_directory () ? "dir" : "?")
      << ", mode "
      << std::setbase (8)
      << location.mode ()
      << "\n";

  char* env = getenv ("TASKRC");
  if (env)
    out << "     TASKRC: "
        << env
        << "\n";

  env = getenv ("TASKDATA");
  if (env)
    out << "   TASKDATA: "
        << env
        << "\n";

  out << "    Locking: "
      << (context.config.getBoolean ("locking")
           ? STRING_CMD_DIAG_ENABLED
           : STRING_CMD_DIAG_DISABLED)
      << "\n";

  out << "         GC: "
      << (context.config.getBoolean ("gc")
           ? STRING_CMD_DIAG_ENABLED
           : STRING_CMD_DIAG_DISABLED)
      << "\n";

  // Determine rc.editor/$EDITOR/$VISUAL.
  char* peditor;
  if (context.config.get ("editor") != "")
    out << "  rc.editor: " << context.config.get ("editor") << "\n";
  else if ((peditor = getenv ("VISUAL")) != NULL)
    out << "    $VISUAL: " << peditor << "\n";
  else if ((peditor = getenv ("EDITOR")) != NULL)
    out << "    $EDITOR: " << peditor << "\n";

  out << "     Server: "
      << context.config.get ("taskd.server")
      << "\n";

  if (context.config.get ("taskd.ca") != "")
    out << "         CA: "
        << context.config.get ("taskd.ca")
        << (File (context.config.get ("taskd.ca")).readable ()
             ? " (readable)" : " (not readable)")
        << "\n";

  std::string trust_value = context.config.get ("taskd.trust");
  if (trust_value == "strict" ||
      trust_value == "ignore hostname" ||
      trust_value == "allow all")
    out << "      Trust: " << trust_value << "\n";
  else
    out << "      Trust: Bad value - see 'man taskrc'\n";

  out << "Certificate: "
      << context.config.get ("taskd.certificate")
      << (File (context.config.get ("taskd.certificate")).readable ()
           ? " (readable)" : " (not readable)")
      << "\n";

  out << "        Key: "
      << context.config.get ("taskd.key")
      << (File (context.config.get ("taskd.key")).readable ()
           ? " (readable)" : " (not readable)")
      << "\n";

  out << "    Ciphers: "
      << context.config.get ("taskd.ciphers")
      << "\n";

  // Get credentials, but mask out the key.
  std::string credentials = context.config.get ("taskd.credentials");
  std::string::size_type last_slash = credentials.rfind ('/');
  if (last_slash != std::string::npos)
    credentials = credentials.substr (0, last_slash)
                + "/"
                + std::string (credentials.length () - last_slash - 1, '*');

  out << "      Creds: "
      << credentials
      << "\n\n";

  // Disaply hook status.
  out << bold.colorize (STRING_CMD_DIAG_HOOKS)
      << "\n"
      << "    Scripts: "
      << (context.config.getBoolean ("hooks") ? "Enabled" : "Disabled")
      << "\n";

  std::vector <std::string> hooks = context.hooks.list ();
  if (hooks.size ())
  {
    std::vector <std::string>::iterator h;
    for (h = hooks.begin (); h != hooks.end (); ++h)
    {
      Path p (*h);
      out << "             "
          << *h
          << (p.executable () ? " (executable)" : " (not executable)")
          << (p.is_link () ? " (symlink)" : "")
          << "\n";
    }
  }
  else
    out << "             (none)\n";

  out << "\n";

  // Verify UUIDs are all unique.
  out << bold.colorize (STRING_CMD_DIAG_TESTS)
      << "\n";
  {
    // Determine terminal details.
    const char* term = getenv ("TERM");
    out << "      $TERM: "
        << (term ? term : STRING_CMD_DIAG_NONE)
        << " ("
        << context.getWidth ()
        << "x"
        << context.getHeight ()
        << ")\n";

    // Scan tasks for duplicate UUIDs.
    std::vector <Task> all = context.tdb2.all_tasks ();
    std::map <std::string, int> seen;
    std::vector <std::string> dups;
    std::string uuid;
    std::vector <Task>::iterator i;
    for (i = all.begin (); i != all.end (); ++i)
    {
      uuid = i->get ("uuid");
      if (seen.find (uuid) != seen.end ())
        dups.push_back (uuid);
      else
         seen[uuid] = 0;
    }

    out << "       Dups: "
        << format (STRING_CMD_DIAG_UUID_SCAN, all.size ())
        << "\n";

    if (dups.size ())
    {
      std::vector <std::string>::iterator d;
      for (d = dups.begin (); d != dups.end (); ++d)
        out << "             " << format (STRING_CMD_DIAG_UUID_DUP, *d) << "\n";
    }
    else
    {
      out << "             " << STRING_CMD_DIAG_UUID_NO_DUP
          << "\n";
    }
  }

  out << "\n";
  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
