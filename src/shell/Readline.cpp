////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2014, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <Readline.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

////////////////////////////////////////////////////////////////////////////////
std::string Readline::gets (const std::string& prompt)
{
#ifdef HAVE_READLINE
  // Get a line from the user.
  char *line_read = readline (prompt.c_str ());
  if (!line_read) // Exit when CTRL-D is pressed
  {
    std::cout << "exit\n";
    return "exit";
  }
#else
  std::string line_read;
  std::cout << prompt;
  std::getline (std::cin, line_read);
#endif

#ifdef HAVE_READLINE
  // If the line has any text in it, save it on the history.
  if (*line_read)
    add_history (line_read);
#endif

  std::string ret(line_read);

#ifdef HAVE_READLINE
  free (line_read);
#endif

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
bool Readline::interactiveMode (const std::istream& in)
{
  return (&in == &std::cin && isatty (0) == 1);
}

////////////////////////////////////////////////////////////////////////////////
Wordexp::Wordexp (const std::string &str)
{
  _input = str;
  escapeSpecialChars(_input);
#ifdef HAVE_WORDEXP_H
  wordexp (_input.c_str (), &_p, 0);
#endif
}

////////////////////////////////////////////////////////////////////////////////
Wordexp::~Wordexp ()
{
#ifdef HAVE_WORDEXP_H
  wordfree (&_p);
#endif
}

////////////////////////////////////////////////////////////////////////////////
int Wordexp::argc ()
{
#ifdef HAVE_WORDEXP_H
  return _p.we_wordc;
#else
  return 1;
#endif
}

////////////////////////////////////////////////////////////////////////////////
char** Wordexp::argv ()
{
#ifdef HAVE_WORDEXP_H
  return _p.we_wordv;
#else
  return (char**)_input.c_str ();
#endif
}

////////////////////////////////////////////////////////////////////////////////
char* Wordexp::argv (int i)
{
#ifdef HAVE_WORDEXP_H
  return _p.we_wordv[i];
#else
  return (char*)_input.c_str ();
#endif
}

////////////////////////////////////////////////////////////////////////////////
void Wordexp::escapeSpecialChars(std::string& str)
{
  size_t i = 0;
  while ((i = str.find_first_of ("$*?!|&;<>(){}~#@", i)) != std::string::npos)
  {
    str.insert(i, 1, '\\');
    i += 2;
  }
}

////////////////////////////////////////////////////////////////////////////////
