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
#include <CmdIDs.h>
#include <sstream>
#include <algorithm>
#include <Context.h>
#include <Filter.h>
#include <main.h>
#include <text.h>
#include <i18n.h>

extern Context context;

std::string zshColonReplacement = ",";

////////////////////////////////////////////////////////////////////////////////
CmdIDs::CmdIDs ()
{
  _keyword               = "ids";
  _usage                 = "task <filter> ids";
  _description           = STRING_CMD_IDS_USAGE_RANGE;
  _read_only             = true;
  _displays_id           = true;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdIDs::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Find number of matching tasks.
  std::vector <int> ids;
  for (auto& task : filtered)
    if (task.id)
      ids.push_back (task.id);

  std::sort (ids.begin (), ids.end ());
  output = compressIds (ids) + "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// The vector must be sorted first.  This is a modified version of the run-
// length encoding algorithm.
//
// This function converts the vector:
//
//   [1, 3, 4, 6, 7, 8, 9, 11]
//
// to ths string:
//
//   1,3-4,6-9,11
//
std::string CmdIDs::compressIds (const std::vector <int>& ids)
{
  std::stringstream result;

  int range_start = 0;
  int range_end = 0;

  for (unsigned int i = 0; i < ids.size (); ++i)
  {
    if (i + 1 == ids.size ())
    {
      if (result.str ().length ())
        result << " ";

      if (range_start < range_end)
        result << ids[range_start] << "-" << ids[range_end];
      else
        result << ids[range_start];
    }
    else
    {
      if (ids[range_end] + 1 == ids[i + 1])
      {
        ++range_end;
      }
      else
      {
        if (result.str ().length ())
          result << " ";

        if (range_start < range_end)
          result << ids[range_start] << "-" << ids[range_end];
        else
          result << ids[range_start];

        range_start = range_end = i + 1;
      }
    }
  }

  return result.str ();
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionIds::CmdCompletionIds ()
{
  _keyword               = "_ids";
  _usage                 = "task <filter> _ids";
  _description           = STRING_CMD_IDS_USAGE_LIST;
  _read_only             = true;
  _displays_id           = true;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionIds::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  std::vector <int> ids;
  for (auto& task : filtered)
    if (task.getStatus () != Task::deleted &&
        task.getStatus () != Task::completed)
      ids.push_back (task.id);

  std::sort (ids.begin (), ids.end ());
  join (output, "\n", ids);
  output += "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCompletionIds::CmdZshCompletionIds ()
{
  _keyword               = "_zshids";
  _usage                 = "task <filter> _zshids";
  _description           = STRING_CMD_IDS_USAGE_ZSH;
  _read_only             = true;
  _displays_id           = true;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCompletionIds::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  std::stringstream out;
  for (auto& task : filtered)
    if (task.getStatus () != Task::deleted &&
        task.getStatus () != Task::completed)
      out << task.id
          << ":"
          << str_replace(task.get ("description"), ":", zshColonReplacement)
          << "\n";

  output = out.str ();

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdUUIDs::CmdUUIDs ()
{
  _keyword               = "uuids";
  _usage                 = "task <filter> uuids";
  _description           = STRING_CMD_UUIDS_USAGE_RANGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::metadata;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUUIDs::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  std::vector <std::string> uuids;
  for (auto& task : filtered)
    uuids.push_back (task.get ("uuid"));

  std::sort (uuids.begin (), uuids.end ());
  join (output, " ", uuids);
  output += "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionUuids::CmdCompletionUuids ()
{
  _keyword               = "_uuids";
  _usage                 = "task <filter> _uuids";
  _description           = STRING_CMD_UUIDS_USAGE_LIST;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionUuids::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  std::vector <std::string> uuids;
  for (auto& task : filtered)
    uuids.push_back (task.get ("uuid"));

  std::sort (uuids.begin (), uuids.end ());
  join (output, "\n", uuids);
  output += "\n";

  context.headers.clear ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
CmdZshCompletionUuids::CmdZshCompletionUuids ()
{
  _keyword               = "_zshuuids";
  _usage                 = "task <filter> _zshuuids";
  _description           = STRING_CMD_UUIDS_USAGE_ZSH;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshCompletionUuids::execute (std::string& output)
{
  // Apply filter.
  handleRecurrence ();
  Filter filter;
  std::vector <Task> filtered;
  filter.subset (filtered);

  std::stringstream out;
  for (auto& task : filtered)
    out << task.get ("uuid")
        << ":"
        << str_replace (task.get ("description"), ":", zshColonReplacement)
        << "\n";

  output = out.str ();

  context.headers.clear ();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
