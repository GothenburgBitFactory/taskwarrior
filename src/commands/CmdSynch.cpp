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

#include <iostream>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <CmdSynch.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdSynch::CmdSynch ()
{
  _keyword     = "synchronize";
  _usage       = "task          synchronize";
  _description = STRING_CMD_SYNCH_USAGE;
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdSynch::execute (std::string& output)
{
  // TODO Tempporary.
  std::cout << "\n"
            << "Task Server Synchronization is not implemented in 2.0.0beta3.\n"
            << "\n";

  // If no server is set up, quit.
  std::string connection = context.config.get ("taskd.server");
  if (connection == "" ||
      connection.find (':') == std::string::npos)
    throw std::string (STRING_CMD_SYNCH_NO_SERVER);

  // Obtain credentials.
  std::string credentials = context.config.get ("taskd.credentials");

  // TODO Obtain synch key.

  // TODO Compose backlog into ticket.
  // TODO Request synch.

  // TODO Receive synch data.
  // TODO Extract remote mods.
  // TODO Extract new synch key.
  // TODO Apply remote mods.
  // TODO Store new synch key.
  // TODO Truncate backlog.

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
