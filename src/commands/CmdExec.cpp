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
#include <CmdExec.h>
#include <stdlib.h>
#include <Context.h>
#include <shared.h>

////////////////////////////////////////////////////////////////////////////////
CmdExec::CmdExec ()
{
  _keyword               = "execute";
  _usage                 = "task          execute <external command>";
  _description           = "Executes external commands and scripts";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdExec::execute (std::string&)
{
  std::string command = join (" ", Context::getContext ().cli2.getWords ());

  if (command.empty())
  {
    Context::getContext ().error ("Cannot execute an empty command.");
    return 1;
  }
  else
    return system (command.c_str ());
}

////////////////////////////////////////////////////////////////////////////////
