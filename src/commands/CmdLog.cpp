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
#include <CmdLog.h>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <util.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdLog::CmdLog ()
{
  _keyword               = "log";
  _usage                 = "task          log <mods>";
  _description           = STRING_CMD_LOG_USAGE;
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = true;
  _accepts_miscellaneous = false;
  _category              = Command::Category::operation;
}

////////////////////////////////////////////////////////////////////////////////
int CmdLog::execute (std::string& output)
{
  // Apply the command line modifications to the new task.
  Task task;
  task.modify (Task::modReplace, true);
  task.setStatus (Task::completed);

  // Cannot log recurring tasks.
  if (task.has ("recur"))
    throw std::string (STRING_CMD_LOG_NO_RECUR);

  // Cannot log waiting tasks.
  if (task.has ("wait"))
    throw std::string (STRING_CMD_LOG_NO_WAITING);

  context.tdb2.add (task);

  if (context.verbose ("project"))
    context.footnote (onProjectChange (task));

  if (context.verbose ("new-uuid"))
    output = format (STRING_CMD_LOG_LOGGED, task.get ("uuid")) + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
