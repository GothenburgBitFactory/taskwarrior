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

#include <iostream>
#include <cstring>
#include <stdlib.h>

#ifdef CYGWIN
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <text.h>
#include <i18n.h>
#include <Color.h>
#include <Context.h>
#include <cmake.h>
#include <Readline.h>

Context context;

#ifdef HAVE_SRANDOM
#define srand(x) srandom(x)
#endif

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  // Set up randomness.
#ifdef CYGWIN
  srand (time (NULL));
#else
  struct timeval tv;
  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);
#endif

  int status = 0;

  if (argc == 2 && !strcmp (argv[1], "--version"))
  {
    std::cout << VERSION << "\n";
  }
  else
  {
    if (argc > 1)
    {
      std::cerr << STRING_SHELL_UNKOWN_OPTION << argv[1] << "\n";
    }

    // Begining initilaization
    status = context.initialize (0, NULL);

    // Display some kind of welcome message.
    Color bold (Color::nocolor, Color::nocolor, false, true, false);
    std::cout << (context.color () ? bold.colorize (PACKAGE_STRING) : PACKAGE_STRING)
              << " shell\n\n"
              << STRING_CMD_SHELL_HELP1 << "\n"
              << STRING_CMD_SHELL_HELP2 << "\n"
              << STRING_CMD_SHELL_HELP3 << "\n\n";

    // Make a copy because context.clear will delete them.
    std::string permanent_overrides;
    std::vector <Arg>::iterator i;
    for (i = context.a3.begin (); i != context.a3.end (); ++i)
    {
      if (i->_category == Arg::cat_rc ||
          i->_category == Arg::cat_override)
      {
        if (i != context.a3.begin ())
          permanent_overrides += " ";

        permanent_overrides += i->_raw;
      }
    }

    std::string input, prompt(context.config.get ("shell.prompt") + " ");

    std::vector <std::string> quit_commands;
    quit_commands.push_back ("quit");
    quit_commands.push_back ("exit");
    quit_commands.push_back ("bye");

    // The event loop.
    while (1)
    {
      context.clear ();

      input = Readline::gets (prompt);
      if (std::find (quit_commands.begin (), quit_commands.end (),
                     lowerCase (input)) != quit_commands.end ())
        break;

      try
      {
        Wordexp w ("task " + trim (input + permanent_overrides));
        status = context.initialize (w.argc (), (const char**)w.argv ());
        if (status == 0)
          status = context.run ();
      }

      catch (const std::string& error)
      {
        std::cerr << error << "\n";
        status = -1;
        break;
      }

      catch (...)
      {
        std::cerr << STRING_UNKNOWN_ERROR << "\n";
        status = -2;
        break;
      }
    }
  }

  // No need to repeat any overrides after the shell quits.
  context.clearMessages ();
  return status;
}

////////////////////////////////////////////////////////////////////////////////
