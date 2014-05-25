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
#include <sstream>
#include <algorithm>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <CmdIDs.h>

extern Context context;

std::string zshColonReplacement = ",";

////////////////////////////////////////////////////////////////////////////////
CmdIDs::CmdIDs ()
{
  _keyword     = "ids";
  _usage       = "task <filter> ids";
  _description = STRING_CMD_IDS_USAGE_RANGE;
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdIDs::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  context.tdb2.commit ();

  // Find number of matching tasks.
  std::vector <int> ids;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    if (task->id)
      ids.push_back (task->id);

  std::sort (ids.begin (), ids.end ());
  output = compressIds (ids) + "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionIds::CmdCompletionIds ()
{
  _keyword     = "_ids";
  _usage       = "task <filter> _ids";
  _description = STRING_CMD_IDS_USAGE_LIST;
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionIds::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  context.tdb2.commit ();

  std::vector <int> ids;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    if (task->getStatus () != Task::deleted &&
        task->getStatus () != Task::completed)
      ids.push_back (task->id);

  std::sort (ids.begin (), ids.end ());
  join (output, "\n", ids);
  output += "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCompletionIds::CmdZshCompletionIds ()
{
  _keyword     = "_zshids";
  _usage       = "task <filter> _zshids";
  _description = STRING_CMD_IDS_USAGE_ZSH;
  _read_only   = true;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCompletionIds::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  context.tdb2.commit ();

  std::stringstream out;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    if (task->getStatus () != Task::deleted &&
        task->getStatus () != Task::completed)
      out << task->id
          << ":"
          << str_replace(task->get ("description"), ":", zshColonReplacement)
          << "\n";

  output = out.str ();

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdUUIDs::CmdUUIDs ()
{
  _keyword     = "uuids";
  _usage       = "task <filter> uuids";
  _description = STRING_CMD_UUIDS_USAGE_RANGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUUIDs::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  context.tdb2.commit ();

  std::vector <std::string> uuids;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    uuids.push_back (task->get ("uuid"));

  std::sort (uuids.begin (), uuids.end ());
  join (output, ",", uuids);
  output += "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionUuids::CmdCompletionUuids ()
{
  _keyword     = "_uuids";
  _usage       = "task <filter> _uuids";
  _description = STRING_CMD_UUIDS_USAGE_LIST;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionUuids::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  context.tdb2.commit ();

  std::vector <std::string> uuids;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    uuids.push_back (task->get ("uuid"));

  std::sort (uuids.begin (), uuids.end ());
  join (output, "\n", uuids);
  output += "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCompletionUuids::CmdZshCompletionUuids ()
{
  _keyword     = "_zshuuids";
  _usage       = "task <filter> _zshuuids";
  _description = STRING_CMD_UUIDS_USAGE_ZSH;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCompletionUuids::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);
  context.tdb2.commit ();

  std::stringstream out;
  std::vector <Task>::iterator task;
  for (task = filtered.begin (); task != filtered.end (); ++task)
    out << task->get ("uuid")
        << ":"
        << str_replace (task->get ("description"), ":", zshColonReplacement)
        << "\n";

  output = out.str ();

  context.headers.clear ();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
