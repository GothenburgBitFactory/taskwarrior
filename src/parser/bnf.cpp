////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <Context.h>
#include <File.h>
#include <Tree.h>
#include <LRParser.h>
#include <text.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
void usage ()
{
  std::cout << std::endl
            << "Usage: parser [options] <grammar file> <args>"
            << std::endl
            << std::endl
            << "Options are:"
            << std::endl
            << "  -v/--verbose       Increased verbosity"
            << std::endl
            << std::endl;

  exit (-1);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  // Process command line arguments
  std::string grammarFile = "";
  std::string commandLine = "";
  std::vector <std::string> args;
  bool verbose = false;

  for (int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
           if (!strcmp (argv[i], "-h"))         usage ();
      else if (!strcmp (argv[i], "--help"))     usage ();
      else if (!strcmp (argv[i], "-v"))         verbose = true;
      else if (!strcmp (argv[i], "--verbose"))  verbose = true;
      else
      {
        std::cout << "Unrecognized option '" << argv[i] << "'" << std::endl;
        usage ();
      }
    }
    else if (grammarFile == "")
    {
      grammarFile = argv[i];
    }
    else
    {
      if (commandLine != "")
        commandLine += " ";

      commandLine += "'" + std::string (argv[i]) + "'";
    }
  }

  // Display usage for incorrect command line.
  if (grammarFile == "" || commandLine == "")
    usage ();

  try
  {
    std::string grammar;
    if (File::read (grammarFile, grammar))
    {
      // Parse the tokens.
      LRParser p;
      if (verbose) p.verbose ();

      p.grammar (grammar);

      if (verbose) p.dump ();
      Tree* t = p.parse (commandLine);
      if (t)
      {
        t->dump ();
        delete t;
      }
    }
  }

  catch (const std::string& error)
  {
    std::cout << "Error: " << error << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
