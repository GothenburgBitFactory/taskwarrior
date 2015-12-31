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
#include <CmdImport.h>
#include <CmdModify.h>
#include <iostream>
#include <sstream>
#include <Context.h>
#include <Filter.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdImport::CmdImport ()
{
  _keyword               = "import";
  _usage                 = "task          import [<file> ...]";
  _description           = STRING_CMD_IMPORT_USAGE;
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::migration;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImport::execute (std::string&)
{
  int rc = 0;
  int count = 0;

  // Get filenames from command line arguments.
  std::vector <std::string> words = context.cli2.getWords ();
  if (! words.size () || (words.size () == 1 && words[0] == "-"))
  {
    std::cout << format (STRING_CMD_IMPORT_FILE, "STDIN") << "\n";

    std::string json;
    std::string line;
    while (std::getline (std::cin, line))
      json += line + "\n";

    if (nontrivial (json))
      count = import (json);
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
      std::string json;
      incoming.read (json);
      if (nontrivial (json))
        count += import (json);
    }
  }

  context.footnote (format (STRING_CMD_IMPORT_SUMMARY, count));
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdImport::import (const std::string& input)
{
  int count = 0;
  try
  {
    json::value* root = json::parse (input);
    if (root)
    {
      // Single object parse. Input looks like:
      //   { ... }
      if (root->type () == json::j_object)
      {
        // For each object element...
        json::object* root_obj = (json::object*)root;
        importSingleTask (root_obj);
        ++count;
      }

      // Multiple object array. Input looks like:
      //   [ { ... } , { ... } ]
      else if (root->type () == json::j_array)
      {
        json::array* root_arr = (json::array*)root;

        // For each object element...
        for (auto& element : root_arr->_data)
        {
          // For each object element...
          json::object* root_obj = (json::object*)element;
          importSingleTask (root_obj);
          ++count;
        }
      }

      delete root;
    }
  }

  // If an exception is caught, then it is because the free-form JSON
  // objects/array above failed to parse. This is an indication that the input
  // is an old-style line-by-line set of JSON objects, because both an array of
  // objects, and a single object have failed to parse..
  //
  // Input looks like:
  //   { ... }
  //   { ... }
  catch (std::string& e)
  {
    std::vector <std::string> lines;
    split (lines, input, '\n');

    for (auto& line : lines)
    {
      if (line.length ())
      {
        json::value* root = json::parse (line);
        if (root)
        {
          importSingleTask ((json::object*) root);
          ++count;
          delete root;
        }
      }
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////
void CmdImport::importSingleTask (json::object* obj)
{
  // Parse the whole thing, validate the data.
  Task task (obj);

  bool hasGeneratedEntry = not task.has ("entry");
  bool hasExplicitEnd = task.has ("end");

  task.validate ();

  bool hasGeneratedEnd = not hasExplicitEnd and task.has ("end");

  // Check whether the imported task is new or a modified existing task.
  Task before;
  if (context.tdb2.get (task.get ("uuid"), before))
  {
    // We need to neglect updates from attributes with dynamic defaults
    // unless they have been explicitly specified on import.
    //
    // There are three attributes with dynamic defaults, besides uuid:
    //   - modified: Ignored in any case.
    //   - entry: Ignored if generated.
    //   - end: Ignored if generated.

    // The 'modified' attribute is ignored in any case, since if it
    // were the only difference between the tasks, it would have been
    // neglected anyway, since it is bumped on each modification.
    task.set ("modified", before.get ("modified"));

    // Other generated values are replaced by values from existing task,
    // so that they are ignored on comparison.
    if (hasGeneratedEntry)
      task.set ("entry", before.get ("entry"));

    if (hasGeneratedEnd)
      task.set ("end", before.get ("end"));

    if (before.data != task.data)
    {
      CmdModify modHelper;
      modHelper.checkConsistency (before, task);
      modHelper.modifyAndUpdate (before, task);
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
  }

  std::cout << task.get ("uuid")
            << " "
            << task.get ("description")
            << "\n";
}

////////////////////////////////////////////////////////////////////////////////
