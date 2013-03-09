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

#include <Readline.h>

////////////////////////////////////////////////////////////////////////////////
std::string Readline::gets (const std::string& prompt)
{
  // Get a line from the user.
  char *line_read = rl::readline (prompt.c_str ());

  // If the line has any text in it, save it on the history.
  if (line_read && *line_read)
    rl::add_history (line_read);

  std::string ret (line_read);
  free (line_read);

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
Wordexp::Wordexp (const std::string &str)
{
  wordexp (str.c_str (), &_p, 0);
}

////////////////////////////////////////////////////////////////////////////////
Wordexp::~Wordexp ()
{
  wordfree(&_p);
}

////////////////////////////////////////////////////////////////////////////////
int Wordexp::argc ()
{
  return _p.we_wordc;
}

////////////////////////////////////////////////////////////////////////////////
char** Wordexp::argv ()
{
  return _p.we_wordv;
}

////////////////////////////////////////////////////////////////////////////////
