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
#include <CmdInstall.h>
#include <Context.h>
#include <i18n.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdInstall::CmdInstall ()
{
  _keyword     = "install";
  _usage       = "task          install <extension> [<extension> ...]";
  _description = STRING_CMD_INSTALL_USAGE;
  _read_only   = true;
  _displays_id = false;
  _category    = Command::Category::UNDOCUMENTED;
}

////////////////////////////////////////////////////////////////////////////////
// Algorithm:
//   Copy file rc.data.location/extensions
//   Generate UUID
//   Call the "install" function once, store results in rc:
//     extension.<uuid>=<JSON>
int CmdInstall::execute (std::string&)
{
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
