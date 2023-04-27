////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
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
#include <shared.h>
#include <format.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif

////////////////////////////////////////////////////////////////////////////////
CmdDiagnostics::CmdDiagnostics ()
{
  _keyword               = "diagnostics";
  _usage                 = "task          diagnostics";
  _description           = "Platform, build and environment details";
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
  if (Context::getContext ().color ())
    bold = Color ("bold");

  std::stringstream out;
  out << '\n'
      << bold.colorize (PACKAGE_STRING)
      << '\n';

  out << "   Platform: " << osName ()
      << "\n\n";

  // Compiler.
  out << bold.colorize ("Compiler")
      << '\n'
#ifdef __VERSION__
      << "    Version: "
      << __VERSION__ << '\n'
#endif
      << "       Caps:"
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
  out << " Compliance: "
      << cppCompliance ()
      << "\n\n";

  out << bold.colorize ("Build Features")
      << '\n'

#ifdef HAVE_COMMIT
      << "     Commit: " << COMMIT << '\n'
#endif
      << "      CMake: " << CMAKE_VERSION << '\n';

  out << "    libuuid: "
#ifdef HAVE_UUID_UNPARSE_LOWER
      << "libuuid + uuid_unparse_lower"
#else
      << "libuuid, no uuid_unparse_lower"
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
  File rcFile (Context::getContext ().config.file ());
  out << bold.colorize ("Configuration")
      << '\n'
      << "       File: " << rcFile._data << ' '
      << (rcFile.exists ()
           ? "(found)"
           : "(missing)")
      << ", " << rcFile.size () << ' ' << "bytes"
      << ", mode "
      << std::setbase (8)
      << rcFile.mode ()
      << '\n';

  // Config: data.location found, readable, writable
  File location (Context::getContext ().config.get ("data.location"));
  out << "       Data: " << location._data << ' '
      << (location.exists ()
           ? "(found)"
           : "(missing)")
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
      << (Context::getContext ().config.getBoolean ("locking")
           ? "Enabled"
           : "Disabled")
      << '\n';

  out << "         GC: "
      << (Context::getContext ().config.getBoolean ("gc")
           ? "Enabled"
           : "Disabled")
      << '\n';

  // Determine rc.editor/$EDITOR/$VISUAL.
  char* peditor;
  if (Context::getContext ().config.get ("editor") != "")
    out << "  rc.editor: " << Context::getContext ().config.get ("editor") << '\n';
  else if ((peditor = getenv ("VISUAL")) != nullptr)
    out << "    $VISUAL: " << peditor << '\n';
  else if ((peditor = getenv ("EDITOR")) != nullptr)
    out << "    $EDITOR: " << peditor << '\n';

  // Display hook status.
  Path hookLocation;
  if (Context::getContext ().config.has ("hooks.location"))
  {
    hookLocation = Path (Context::getContext ().config.get ("hooks.location"));
  }
  else
  {
    hookLocation = Path (Context::getContext ().config.get ("data.location"));
    hookLocation += "hooks";
  }

  out << bold.colorize ("Hooks")
      << '\n'
      << "     System: "
      << (Context::getContext ().config.getBoolean ("hooks") ? "Enabled" : "Disabled")
      << '\n'
      << "   Location: "
      << static_cast <std::string> (hookLocation)
      << '\n';

  auto hooks = Context::getContext ().hooks.list ();
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
        auto name = p.name ();

        if (p.executable () &&
            (name.substr (0, 6) == "on-add"    ||
             name.substr (0, 9) == "on-modify" ||
             name.substr (0, 9) == "on-launch" ||
             name.substr (0, 7) == "on-exit"))
        {
          out << (count++ ? "             " : "");

          out.width (longest);
          out << std::left << name
              << " (executable)"
              << (p.is_link () ? " (symlink)" : "")
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
        auto name = p.name ();

        if (! p.executable () ||
            (name.substr (0, 6) != "on-add"    &&
             name.substr (0, 9) != "on-modify" &&
             name.substr (0, 9) != "on-launch" &&
             name.substr (0, 7) != "on-exit"))
        {
          out << (count++ ? "             " : "");

          out.width (longest);
          out << std::left << name
              << (p.executable () ? " (executable)" : " (not executable)")
              << (p.is_link () ? " (symlink)" : "")
              << ((name.substr (0, 6) == "on-add" ||
                   name.substr (0, 9) == "on-modify" ||
                   name.substr (0, 9) == "on-launch" ||
                   name.substr (0, 7) == "on-exit") ? "" : "unrecognized hook name")
              << '\n';
        }
      }
    }

    if (! count)
      out << '\n';
  }
  else
    out << "             (-none-)\n";

  out << '\n';

  // Verify UUIDs are all unique.
  out << bold.colorize ("Tests")
      << '\n';

  // Report terminal dimensions.
  out << "   Terminal: "
      << Context::getContext ().getWidth ()
      << 'x'
      << Context::getContext ().getHeight ()
      << '\n';

  // Scan tasks for duplicate UUIDs.
  auto all = Context::getContext ().tdb2.all_tasks ();
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
      << format ("Scanned {1} tasks for duplicate UUIDs:", all.size ())
      << '\n';

  if (dups.size ())
  {
    for (auto& d : dups)
      out << "             " << format ("Found duplicate {1}", d) << '\n';
  }
  else
  {
    out << "             No duplicates found\n";
  }

  // Check all the UUID references

  bool noBrokenRefs = true;
  out << " Broken ref: "
      << format ("Scanned {1} tasks for broken references:", all.size ())
      << '\n';

  for (auto& task : all)
  {
    // Check dependencies
    for (auto& uuid : task.getDependencyUUIDs ())
    {
      if (! Context::getContext ().tdb2.has (uuid))
      {
        out << "             "
            << format ("Task {1} depends on nonexistent task: {2}", task.get ("uuid"), uuid)
            << '\n';
        noBrokenRefs = false;
      }
    }

    // Check recurrence parent
    auto parentUUID = task.get ("parent");

    if (parentUUID != "" && ! Context::getContext ().tdb2.has (parentUUID))
    {
      out << "             "
          << format ("Task {1} has nonexistent recurrence template {2}", task.get ("uuid"), parentUUID)
          << '\n';
      noBrokenRefs = false;
    }
  }

  if (noBrokenRefs)
    out << "             No broken references found\n";

  out << '\n';
  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
