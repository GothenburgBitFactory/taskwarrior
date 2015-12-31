////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <CmdVersion.h>
#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdVersion::CmdVersion ()
{
  _keyword               = "version";
  _usage                 = "task          version";
  _description           = STRING_CMD_VERSION_USAGE;
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
int CmdVersion::execute (std::string& output)
{
  std::stringstream out;

  // Create a table for the disclaimer.
  int width = context.getWidth ();
  ViewText disclaimer;
  disclaimer.width (width);
  disclaimer.add (Column::factory ("string", ""));
  disclaimer.set (disclaimer.addRow (), 0, STRING_CMD_VERSION_MIT);

  // Create a table for the URL.
  ViewText link;
  link.width (width);
  link.add (Column::factory ("string", ""));
  link.set (link.addRow (), 0, STRING_CMD_VERSION_DOCS);

  Color bold;
  if (context.color ())
    bold = Color ("bold");

  out << "\n"
      << format (STRING_CMD_VERSION_BUILT, bold.colorize (PACKAGE), bold.colorize (VERSION))

#if defined (DARWIN)
      << "darwin"
#elif defined (SOLARIS)
      << "solaris"
#elif defined (CYGWIN)
      << "cygwin"
#elif defined (HAIKU)
      << "haiku"
#elif defined (OPENBSD)
      << "openbsd"
#elif defined (FREEBSD)
      << "freebsd"
#elif defined (NETBSD)
      << "netbsd"
#elif defined (LINUX)
      << "linux"
#elif defined (KFREEBSD)
      << "gnu-kfreebsd"
#elif defined (GNUHURD)
      << "gnu-hurd"
#else
      << STRING_CMD_VERSION_UNKNOWN
#endif

#if PACKAGE_LANGUAGE != LANGUAGE_ENG_USA
      << " "
      << STRING_LOCALIZATION_DESC
#endif

      << "\n"
      << STRING_CMD_VERSION_COPY
      << "\n"
      << "\n"
      << disclaimer.render ()
      << "\n"
      << link.render ()
      << "\n";

#if PACKAGE_LANGUAGE != LANGUAGE_ENG_USA
  out << STRING_LOCALIZATION_AUTHOR
      << "\n"
      << "\n";
#endif

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionVersion::CmdCompletionVersion ()
{
  _keyword               = "_version";
  _usage                 = "task          _version";
  _description           = STRING_CMD_VERSION_USAGE2;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionVersion::execute (std::string& output)
{
#ifdef HAVE_COMMIT
  output = std::string (VERSION)
         + std::string (" (")
         + std::string (COMMIT)
         + std::string (")");
#else
  output = VERSION;
#endif
  output += "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
