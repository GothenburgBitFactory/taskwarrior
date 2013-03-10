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

#ifndef INCLUDED_READLINE
#define INCLUDED_READLINE
#define L10N                                           // Localization complete.

#include <string>
#include <stdio.h>
#include <wordexp.h>

namespace rl
{
  // Wrapping readline.h in a namespace to
  // avoid cluttering the global namespace.
  extern "C"
  {
    #include <readline/readline.h>
    #include <readline/history.h>
  }
}

// Static class that offers a C++ API to readline C functions.
class Readline
{
public:
  static std::string gets (const std::string& prompt);
  static bool interactive_mode (const std::istream& in);

private:
  // No construction or destruction.
  Readline (); // Don't implement.
  ~Readline (); // Don't implement.
  Readline (const Readline&); // Don't implement.
  Readline& operator= (const Readline&); // Don't implement.
};

// RAII for wordexp_t
class Wordexp
{
public:
  Wordexp (const std::string &str);
  ~Wordexp ();

  int argc ();
  char** argv ();

private:
  wordexp_t _p;
};

#endif
////////////////////////////////////////////////////////////////////////////////
