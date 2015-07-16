////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <sstream>
#include <Context.h>
#include <JSON.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>
#include <CmdImport.h>
#include <CmdModify.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdImport::CmdImport ()
{
  _keyword     = "import";
  _usage       = "task          import [<file> ...]";
  _description = STRING_CMD_IMPORT_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImport::execute (std::string& output)
{
  int rc = 0;
  int count = 0;

  // Get filenames from command line arguments.
  std::vector <std::string> words = context.cli2.getWords ();
  if (! words.size () || (words.size () == 1 && words[0] == "-"))
  {
    // No files or only "-" specified, import tasks from STDIN.
    std::vector <std::string> lines;
    std::string line;

    std::cout << format (STRING_CMD_IMPORT_FILE, "STDIN") << "\n";

    while (std::getline (std::cin, line))
      lines.push_back (line);

    if (lines.size () > 0)
      count = import (lines);
  }
  else
  {
    // Import tasks from all specified files.
    for (auto& word : words)
    {
      File incoming (word);
      if (! incoming.exists ())
        throw format (STRING_CMD_IMPORT_MISSING, word);

      std::cout << format (STRING_CMD_IMPORT_FILE, word) << "\n";

      // Load the file.
      std::vector <std::string> lines;
      incoming.read (lines);

      count += import (lines);
    }
  }

  context.footnote (format (STRING_CMD_IMPORT_SUMMARY, count));
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImport::import (std::vector <std::string>& lines)
{
  int count = 0;
  for (auto& line : lines)
  {
    std::string object = trimLeft (
                          trimRight (
                            trimRight (
                              trim (line),
                              ","),
                            "]"),
                          "[");

    // Skip blanks.  May be caused by the trim calls above.
    if (! object.length ())
      continue;

    // Parse the whole thing.
    Task task (object);

    // Check whether the imported task is new or a modified existing task.
    Task before;
    if (context.tdb2.get (task.get ("uuid"), before))
    {
      // "modified:" is automatically set to the current time when a task is
      // changed.  If the imported task has a modification timestamp we need
      // to ignore it in taskDiff() in order to check for meaningful
      // differences.  Setting it to the previous value achieves just that.
      task.set ("modified", before.get ("modified"));
      if (taskDiff (before, task))
      {
        CmdModify modHelper;
        modHelper.checkConsistency (before, task);
        count += modHelper.modifyAndUpdate (before, task);
        std::cout << " mod  ";
      }
      else
      {
        std::cout << " skip ";
      }
    }
    else
    {
      context.tdb2.add (task);
      std::cout << " add  ";
      ++count;
    }

    std::cout << task.get ("uuid")
              << " "
              << task.get ("description")
              << "\n";
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////
