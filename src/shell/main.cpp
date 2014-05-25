////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstring>
#include <string.h>

#include <text.h>
#include <i18n.h>
#include <Color.h>
#include <Context.h>
#include <Readline.h>

Context context;

#define MAX_ARGUMENTS 256

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  bool read_from_file = false;

  if (argc > 2)
  {
    std::cout << STRING_SHELL_USAGE << "\n";
    return -1;
  }
  else if (argc == 2)
  {
    if (!strcmp (argv[1], "--version"))
    {
      std::cout << VERSION << "\n";
      return 0;
    }
    else if (!strcmp (argv[1], "--help"))
    {
      std::cout << STRING_SHELL_USAGE << "\n";
      return 0;
    }
    else
    {
      // The user has given tasksh a task commands file to execute
      File input_file = File (argv[1]);
      if (!input_file.exists ())
      {
        std::cout << STRING_SHELL_NO_FILE;
        std::cout << STRING_SHELL_USAGE << "\n";
        return -1;
      }

      read_from_file = true;
    }
  }

  // If a file is given, read from it
  std::ifstream fin;
  if (read_from_file)
  {
    fin.open (argv[1]);
  }

  // Commands may be redirected too
  std::istream &in = read_from_file ? fin : std::cin;

  if (Readline::interactiveMode (in))
  {
    // Begining initilaization
    if (context.initialize (0, NULL))
    {
      return -1;
    }

    // Display some kind of welcome message.
    Color bold (Color::nocolor, Color::nocolor, false, true, false);

    std::cout << (context.color () ? bold.colorize (PACKAGE_STRING)
                                   : PACKAGE_STRING)
              << " shell\n\n"
              << STRING_CMD_SHELL_HELP1 << '\n'
              << STRING_CMD_SHELL_HELP2 << '\n'
              << STRING_CMD_SHELL_HELP3 << "\n\n";
  }

  // Make a copy because context.clear will delete them.
  std::string permanent_overrides;
  std::vector <Tree*>::iterator i;
  for (i = context.parser.tree ()->_branches.begin (); i != context.parser.tree ()->_branches.end (); ++i)
  {
    if ((*i)->hasTag ("RC") ||
        (*i)->hasTag ("CONFIG"))
    {
      if (i != context.parser.tree ()->_branches.begin ())
        permanent_overrides += " ";

      permanent_overrides += (*i)->attribute ("raw");
    }
  }

  std::string input;

  std::vector <std::string> quit_commands;
  quit_commands.push_back ("quit");
  quit_commands.push_back ("exit");
  quit_commands.push_back ("bye");

  // The event loop.
  while (in)
  {
    std::string prompt (context.config.get ("shell.prompt") + " ");
    context.clear ();

    if (Readline::interactiveMode (in))
    {
      input = Readline::gets (prompt);

      // if a string has nothing but whitespaces, ignore it
      if (input.find_first_not_of (" \t") == std::string::npos)
        continue;
    }
    else
    {
      std::getline (in, input);

      // if a string has nothing but whitespaces, ignore it
      if (input.find_first_not_of (" \t") == std::string::npos)
        continue;

      std::cout << prompt << input << '\n';
    }

    try
    {
#ifdef HAVE_WORDEXP
      std::string command = "task " + trim (input + permanent_overrides);

      // Escape special chars.
      size_t i = 0;
      while ((i = command.find_first_of ("$*?!|&;<>(){}~#@\\", i)) != std::string::npos)
      {
        command.insert(i, 1, '\\');
        i += 2;
      }

      // Perform expansion.
      wordexp_t p;
      wordexp (command.c_str (), &p, 0);
      char** w = p.we_wordv;

      for (int i = 0; i < p.we_wordc; ++i)
      {
        if (std::find (quit_commands.begin (), quit_commands.end (),
                       lowerCase (w[i])) != quit_commands.end ())
        {
          context.clearMessages ();
          return 0;
        }
      }

      // External calls.
      if (strcmp (w[1], "!") == 0 && p.we_wordc > 2)
      {
        std::string combined = "";
        for (int i = 2; i < p.we_wordc - 1 ; ++i)
        {
          combined += std::string (w[i]) + " ";
        }
        combined += w[p.we_wordc - 1];          // last goes without a blank
        system (combined.c_str ());             // not checked
        continue;
      }

      int status = context.initialize (p.we_wordc, (const char**)p.we_wordv);
      wordfree(&p);
#else
      std::string command = "task " + trim (input + permanent_overrides);
      int arg_count = 0;
      char* arg_vector[MAX_ARGUMENTS];

      char* arg = strtok ((char*)command.c_str (), " ");
      while (arg && arg_count < MAX_ARGUMENTS)
      {
        arg_vector[arg_count++] = arg;
        arg = strtok (0, " ");
      }

      for (int i = 1; i < arg_count; ++i)
      {
        if (std::find (quit_commands.begin (), quit_commands.end (),
                       lowerCase (arg_vector[i])) != quit_commands.end ())
        {
          context.clearMessages ();
          return 0;
        }
      }

      int status = context.initialize (arg_count, (const char**) arg_vector);
#endif

      if (status == 0)
        context.run ();
    }

    catch (const std::string& error)
    {
      std::cerr << error << '\n';
      return -1;
    }

    catch (...)
    {
      std::cerr << STRING_UNKNOWN_ERROR << '\n';
      return -2;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
