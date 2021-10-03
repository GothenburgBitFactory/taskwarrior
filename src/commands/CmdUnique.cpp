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
#include <CmdUnique.h>
#include <string>
#include <set>
#include <Context.h>
#include <Filter.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
CmdUnique::CmdUnique ()
{
  _keyword               = "_unique";
  _usage                 = "task <filter> _unique <attribute>";
  _description           = "Generates lists of unique attribute values";
  _read_only             = true;
  _displays_id           = true;
  _needs_gc              = true;
  _uses_context          = false;
  _accepts_filter        = true;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdUnique::execute (std::string& output)
{
  // Apply filter.
  Filter filter;
  filter.disableSafety ();
  std::vector <Task> filtered;
  filter.subset (filtered);

  // Find <attribute>.
  std::string attribute {};

  // Just the first arg.
  auto words = Context::getContext ().cli2.getWords ();
  if (words.size () == 0)
    throw std::string ("An attribute must be specified.  See 'task _columns'.");
  attribute = words[0];

  std::string canonical;
  if (! Context::getContext ().cli2.canonicalize (canonical, "attribute", attribute))
    throw std::string ("You must specify an attribute or UDA.");

  // Find the unique set of matching tasks.
  std::set <std::string> values;
  for (auto& task : filtered)
  {
    if (task.has (canonical))
    {
      values.insert (task.get (canonical));
    }
    else if (canonical == "id"                  &&
             task.getStatus () != Task::deleted &&
             task.getStatus () != Task::completed)
    {
      values.insert (format (task.id));
    }
  }

  // Generate list of values.
  for (auto& value : values)
    output += value + '\n';

  Context::getContext ().headers.clear ();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
