////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
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
#include <CmdDiagnostics.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <RX.h>
#include <Context.h>
#include <i18n.h>
#include <shared.h>
#include <format.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif

#ifdef HAVE_LIBGNUTLS
#include <gnutls/gnutls.h>
#endif


extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdDiagnostics::CmdDiagnostics ()
{
  _keyword               = "diagnostics";
  _usage                 = "task          diagnostics";
  _description           = STRING_CMD_DIAG_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::misc;
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
  out << '\n'
      << bold.colorize (PACKAGE_STRING)
      << '\n';

  out << "   " << STRING_CMD_DIAG_PLATFORM << ": " << osName ()
      << "\n\n";

  // Compiler.
  out << bold.colorize (STRING_CMD_DIAG_COMPILER)
      << '\n'
#ifdef __VERSION__
      << "    " << STRING_CMD_DIAG_VERSION << ": "
      << __VERSION__ << '\n'
#endif
      << "       " << STRING_CMD_DIAG_CAPS << ':'
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
      << '\n';

  // Compiler compliance level.
  out << ' ' << STRING_CMD_DIAG_COMPLIANCE
      << ": "
      << cppCompliance ()
      << "\n\n";

  out << bold.colorize (STRING_CMD_DIAG_FEATURES)
      << '\n'

  // Build date.
      << "      " << STRING_CMD_DIAG_BUILT << ": " << __DATE__ << ' ' << __TIME__ << '\n'
#ifdef HAVE_COMMIT
      << "     " << STRING_CMD_DIAG_COMMIT << ": " << COMMIT << '\n'
#endif
      << "      CMake: " << CMAKE_VERSION << '\n';

  out << "    libuuid: "
#ifdef HAVE_UUID_UNPARSE_LOWER
      << "libuuid + uuid_unparse_lower"
#else
      << "libuuid, no uuid_unparse_lower"
#endif
      << '\n';

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
      << '\n';

  out << " Build type: "
#ifdef CMAKE_BUILD_TYPE
      << CMAKE_BUILD_TYPE
#else
      << '-'
#endif
      << "\n\n";

  // Config: .taskrc found, readable, writable
  File rcFile (context.config.file ());
  out << bold.colorize (STRING_CMD_DIAG_CONFIG)
      << '\n'
      << "       File: " << rcFile._data << ' '
      << (rcFile.exists ()
           ? STRING_CMD_DIAG_FOUND
           : STRING_CMD_DIAG_MISSING)
      << ", " << rcFile.size () << ' ' << "bytes"
      << ", mode "
      << std::setbase (8)
      << rcFile.mode ()
      << '\n';

  // Config: data.location found, readable, writable
  File location (context.config.get ("data.location"));
  out << "       Data: " << location._data << ' '
      << (location.exists ()
           ? STRING_CMD_DIAG_FOUND
           : STRING_CMD_DIAG_MISSING)
      << ", " << (location.is_directory () ? "dir" : "?")
      << ", mode "
      << std::setbase (8)
      << location.mode ()
      << '\n';

  char* env = getenv ("TASKRC");
  if (env)
    out << "     TASKRC: "
        << env
        << '\n';

  env = getenv ("TASKDATA");
  if (env)
    out << "   TASKDATA: "
        << env
        << '\n';

  out << "    Locking: "
      << (context.config.getBoolean ("locking")
           ? STRING_CMD_DIAG_ENABLED
           : STRING_CMD_DIAG_DISABLED)
      << '\n';

  out << "         GC: "
      << (context.config.getBoolean ("gc")
           ? STRING_CMD_DIAG_ENABLED
           : STRING_CMD_DIAG_DISABLED)
      << '\n';

  // Determine rc.editor/$EDITOR/$VISUAL.
  char* peditor;
  if (context.config.get ("editor") != "")
    out << "  rc.editor: " << context.config.get ("editor") << '\n';
  else if ((peditor = getenv ("VISUAL")) != NULL)
    out << "    $VISUAL: " << peditor << '\n';
  else if ((peditor = getenv ("EDITOR")) != NULL)
    out << "    $EDITOR: " << peditor << '\n';

  out << "     Server: "
      << context.config.get ("taskd.server")
      << '\n';

  auto ca_pem = context.config.get ("taskd.ca");
  out << "         CA: ";
  if (ca_pem != "")
  {
    File file_ca (ca_pem);
    if (file_ca.exists ())
      out << ca_pem
          << (file_ca.readable () ? ", readable, " : ", not readable, ")
          << file_ca.size ()
          << " bytes\n";
    else
      out << "not found\n";
  }
  else
    out << "-\n";

  auto cert_pem = context.config.get ("taskd.certificate");
  out << "Certificate: ";
  if (cert_pem != "")
  {
    File file_cert (cert_pem);
    if (file_cert.exists ())
      out << cert_pem
          << (file_cert.readable () ? ", readable, " : ", not readable, ")
          << file_cert.size ()
          << " bytes\n";
    else
      out << "not found\n";
  }
  else
    out << "-\n";

  auto key_pem = context.config.get ("taskd.key");
  out << "        Key: ";
  if (key_pem != "")
  {
    File file_key (key_pem);
    if (file_key.exists ())
      out << key_pem
          << (file_key.readable () ? ", readable, " : ", not readable, ")
          << file_key.size ()
          << " bytes\n";
    else
      out << "not found\n";
  }
  else
    out << "-\n";

  auto trust_value = context.config.get ("taskd.trust");
  if (trust_value == "strict" ||
      trust_value == "ignore hostname" ||
      trust_value == "allow all")
    out << "      Trust: " << trust_value << '\n';
  else
    out << "      Trust: Bad value - see 'man taskrc'\n";

  out << "    Ciphers: "
      << context.config.get ("taskd.ciphers")
      << '\n';

  // Get credentials, but mask out the key.
  auto credentials = context.config.get ("taskd.credentials");
  auto last_slash = credentials.rfind ('/');
  if (last_slash != std::string::npos)
    credentials = credentials.substr (0, last_slash)
                + '/'
                + std::string (credentials.length () - last_slash - 1, '*');

  out << "      Creds: "
      << credentials
      << "\n\n";

  // Display hook status.
  Path hookLocation;
  if (context.config.has ("hooks.location"))
  {
    hookLocation = Path (context.config.get ("hooks.location"));
  }
  else
  {
    hookLocation = Path (context.config.get ("data.location"));
    hookLocation += "hooks";
  }

  out << bold.colorize (STRING_CMD_DIAG_HOOKS)
      << '\n'
      << "     System: "
      << (context.config.getBoolean ("hooks") ? STRING_CMD_DIAG_HOOK_ENABLE : STRING_CMD_DIAG_HOOK_DISABLE)
      << '\n'
      << "   Location: "
      << static_cast <std::string> (hookLocation)
      << '\n';

  auto hooks = context.hooks.list ();
  if (hooks.size ())
  {
    unsigned int longest = 0;
    for (auto& hook : hooks)
      if (hook.length () > longest)
        longest = hook.length ();
    longest -= hookLocation._data.length () + 1;

    out << "     Active: ";
    int count = 0;
    for (auto& hook : hooks)
    {
      Path p (hook);
      if (! p.is_directory ())
      {
        std::string name = p.name ();

        if (p.executable () &&
            (name.substr (0, 6) == "on-add"    ||
             name.substr (0, 9) == "on-modify" ||
             name.substr (0, 9) == "on-launch" ||
             name.substr (0, 7) == "on-exit"))
        {
          out << (count++ ? "             " : "");

          out.width (longest);
          out << std::left << name
              << format (" ({1})", STRING_CMD_DIAG_HOOK_EXEC)
              << (p.is_link () ? format (" ({1})", STRING_CMD_DIAG_HOOK_SYMLINK) : "")
              << '\n';
        }
      }
    }

    if (! count)
      out << '\n';

    out << "   Inactive: ";
    count = 0;
    for (auto& hook : hooks)
    {
      Path p (hook);
      if (! p.is_directory ())
      {
        std::string name = p.name ();

        if (! p.executable () ||
            (name.substr (0, 6) != "on-add"    &&
             name.substr (0, 9) != "on-modify" &&
             name.substr (0, 9) != "on-launch" &&
             name.substr (0, 7) != "on-exit"))
        {
          out << (count++ ? "             " : "");

          out.width (longest);
          out << std::left << name
              << (p.executable () ? format (" ({1})", STRING_CMD_DIAG_HOOK_EXEC) : format (" ({1})", STRING_CMD_DIAG_HOOK_NO_EXEC))
              << (p.is_link () ? format (" ({1})", STRING_CMD_DIAG_HOOK_SYMLINK) : "")
              << ((name.substr (0, 6) == "on-add" ||
                   name.substr (0, 9) == "on-modify" ||
                   name.substr (0, 9) == "on-launch" ||
                   name.substr (0, 7) == "on-exit") ? "" : format (" ({1})", STRING_CMD_DIAG_HOOK_NAME))
              << '\n';
        }
      }
    }

    if (! count)
      out << '\n';
  }
  else
    out << format ("             ({1})\n", STRING_CMD_DIAG_NONE);

  out << '\n';

  // Verify UUIDs are all unique.
  out << bold.colorize (STRING_CMD_DIAG_TESTS)
      << '\n';

  // Report terminal dimensions.
  out << "   Terminal: "
      << context.getWidth ()
      << 'x'
      << context.getHeight ()
      << '\n';

  // Scan tasks for duplicate UUIDs.
  auto all = context.tdb2.all_tasks ();
  std::map <std::string, int> seen;
  std::vector <std::string> dups;
  std::string uuid;
  for (auto& i : all)
  {
    uuid = i.get ("uuid");
    if (seen.find (uuid) != seen.end ())
      dups.push_back (uuid);
    else
       seen[uuid] = 0;
  }

  out << "       Dups: "
      << format (STRING_CMD_DIAG_UUID_SCAN, all.size ())
      << '\n';

  if (dups.size ())
  {
    for (auto& d : dups)
      out << "             " << format (STRING_CMD_DIAG_UUID_DUP, d) << '\n';
  }
  else
  {
    out << "             " << STRING_CMD_DIAG_UUID_NO_DUP
        << '\n';
  }

  // Check all the UUID references

  bool noBrokenRefs = true;
  out << " Broken ref: "
      << format (STRING_CMD_DIAG_REF_SCAN, all.size ())
      << '\n';

  for (auto& task : all)
  {
    // Check dependencies
    for (auto& uuid : task.getDependencyUUIDs ())
    {
      if (! context.tdb2.has (uuid))
      {
        out << "             "
            << format (STRING_CMD_DIAG_MISS_DEP, task.get ("uuid"), uuid)
            << '\n';
        noBrokenRefs = false;
      }
    }

    // Check recurrence parent
    auto parentUUID = task.get ("parent");

    if (parentUUID != "" && ! context.tdb2.has (parentUUID))
    {
      out << "             "
          << format (STRING_CMD_DIAG_MISS_PAR, task.get ("uuid"), parentUUID)
          << '\n';
      noBrokenRefs = false;
    }
  }

  if (noBrokenRefs)
    out << "             " << STRING_CMD_DIAG_REF_OK
        << '\n';

  out << '\n';
  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
