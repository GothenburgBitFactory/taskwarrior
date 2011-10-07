////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2011, Paul Beckingham, Federico Hernandez.
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


#define L10N                                           // Localization complete.

#include <sstream>
#include <stdlib.h>
#include <Context.h>
#include <ViewText.h>
#include <cmake.h>
#include <commit.h>
#include <CmdVersion.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdVersion::CmdVersion ()
{
  _keyword     = "version";
  _usage       = "task          version";
  _description = STRING_CMD_VERSION_USAGE;
  _read_only   = true;
  _displays_id = false;
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
  disclaimer.set (disclaimer.addRow (), 0, STRING_CMD_VERSION_GPL);

  // Create a table for the URL.
  ViewText link;
  link.width (width);
  link.add (Column::factory ("string", ""));
  link.set (link.addRow (), 0, STRING_CMD_VERSION_DOCS);

  Color bold ("bold");

  out << "\n"
      << format (STRING_CMD_VERSION_BUILT,
                 (context.color () ? bold.colorize (PACKAGE) : PACKAGE),
                 (context.color () ? bold.colorize (VERSION) : VERSION))

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
      << STRING_CMD_VERSION_UNKNOWN
#endif

#ifdef HAVE_LIBLUA
      << "-lua"
#endif

      << "\n"
      << STRING_CMD_VERSION_COPY
      << "\n"
#ifdef HAVE_LIBLUA
      << STRING_CMD_VERSION_COPY2
      << "\n"
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
  _usage       = "task          _version";
  _description = STRING_CMD_VERSION_USAGE2;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionVersion::execute (std::string& output)
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
