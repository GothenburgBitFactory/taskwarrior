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
// cmake.h include header must come first

#include <CmdVersion.h>
#include <Context.h>
#include <Datetime.h>
#include <Table.h>

#include <sstream>
#ifdef HAVE_COMMIT
#include <commit.h>
#endif
#include <format.h>
#include <shared.h>

////////////////////////////////////////////////////////////////////////////////
CmdVersion::CmdVersion() {
  _keyword = "version";
  _usage = "task          version";
  _description = "Shows the Taskwarrior version number";
  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdVersion::execute(std::string& output) {
  std::stringstream out;

  // Create a table for the disclaimer.
  int width = Context::getContext().getWidth();
  Table disclaimer;
  disclaimer.width(width);
  disclaimer.add("");
  disclaimer.set(disclaimer.addRow(), 0,
                 "Taskwarrior may be copied only under the terms of the MIT license, which may be "
                 "found in the Taskwarrior source kit.");

  // Create a table for the URL.
  Table link;
  link.width(width);
  link.add("");
  link.set(link.addRow(), 0,
           "Documentation for Taskwarrior can be found using 'man task', 'man taskrc', 'man "
           "task-color', 'man task-sync' or at https://taskwarrior.org");

  Datetime now;

  Color bold;
  if (Context::getContext().color()) bold = Color("bold");

  out << '\n'
      << format("{1} {2} built for ", bold.colorize(PACKAGE), bold.colorize(VERSION)) << osName()
      << '\n'
      << "Copyright (C) 2006 - " << now.year() << " T. Babej, P. Beckingham, F. Hernandez." << '\n'
      << '\n'
      << disclaimer.render() << '\n'
      << link.render() << '\n';

  output = out.str();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionVersion::CmdCompletionVersion() {
  _keyword = "_version";
  _usage = "task          _version";
  _description = "Shows only the Taskwarrior version number";
  _read_only = true;
  _displays_id = false;
  _needs_gc = false;
  _needs_recur_update = false;
  _uses_context = false;
  _accepts_filter = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionVersion::execute(std::string& output) {
#ifdef HAVE_COMMIT
  output = std::string(VERSION) + std::string(" (") + std::string(COMMIT) + std::string(")");
#else
  output = VERSION;
#endif
  output += '\n';
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
