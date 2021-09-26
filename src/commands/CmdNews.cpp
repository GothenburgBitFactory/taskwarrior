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
#include <CmdNews.h>
#include <iostream>
#include <csignal>
#include <Table.h>
#include <Context.h>
#include <shared.h>
#include <format.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdNews::CmdNews ()
{
  _keyword               = "news";
  _usage                 = "task          news";
  _description           = "Displays news about the recent releases";
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
static void signal_handler (int s)
{
  if (s == SIGINT)
  {
    std::cout << "\n\nCome back and read about new features later!\n";
    exit (1);
  }
}

void wait_for_keypress ()
{
  signal (SIGINT, signal_handler);

  std::string dummy;
  std::getline (std::cin, dummy);

  signal (SIGINT, SIG_DFL);
}

////////////////////////////////////////////////////////////////////////////////
int CmdNews::execute (std::string& output)
{
  auto words = Context::getContext ().cli2.getWords ();

  std::cout << "Taskwarrior 2.6.0 Release Notes" << std::endl;

  wait_for_keypress ();

  output = "Thank you for catching up on new features!\n";

  return 0;
}
