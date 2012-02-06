////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
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

#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <util.h>
#include <main.h>
#include <CmdLog.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdLog::CmdLog ()
{
  _keyword     = "log";
  _usage       = "task          log <mods>";
  _description = STRING_CMD_LOG_USAGE;
  _read_only   = false;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdLog::execute (std::string& output)
{
  int rc = 0;

  // Apply the command line modifications to the new task.
  A3 modifications = context.a3.extract_modifications ();
  Task task;
  modify_task_description_replace (task, modifications);
  task.setStatus (Task::completed);

  // Recurring tasks get a special status.
  if (task.has ("recur"))
    throw std::string (STRING_CMD_LOG_NO_RECUR);

  if (task.has ("wait"))
    throw std::string (STRING_CMD_LOG_NO_WAITING);

  context.tdb2.add (task);
  context.footnote (onProjectChange (task));
  context.tdb2.commit ();

  if (context.verbose ("affected") ||
      context.config.getBoolean ("echo.command")) // Deprecated 2.0
    output = std::string (STRING_CMD_LOG_LOGGED) + "\n";

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
