////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2021, Göteborg Bit Factory.
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
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <Eval.h>
#include <Context.h>
#include <Task.h>
#include <Datetime.h>
#include <Duration.h>
#include <shared.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
// Constants.
bool get (const std::string&, Variant&)
{
/*
  // An example, although a bad one because this is supported by default.
  if (name == "pi") {value = Variant (3.14159165); return true;}
*/

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  int status = 0;

  try
  {
    Context globalContext;
    Context::setContext (&globalContext);

    // Same operating parameters as Context::staticInitialization.
    Datetime::standaloneDateEnabled    = false;
    Datetime::standaloneTimeEnabled    = false;
    Duration::standaloneSecondsEnabled = false;

    bool infix {true};

    // Add a source for constants.
    Eval e;
    e.addSource (get);

    // Combine all the arguments into one expression string.
    std::string expression;
    for (int i = 1; i < argc; i++)
    {
      if (!strcmp (argv[i], "-h") || ! strcmp (argv[i], "--help"))
      {
        std::cout << '\n'
                  << "Usage: " << argv[0] << " [options] '<expression>'\n"
                  << '\n'
                  << "Options:\n"
                  << "  -h|--help         Display this usage\n"
                  << "  -d|--debug        Debug mode\n"
                  << "  -i|--infix        Infix expression (default)\n"
                  << "  -p|--postfix      Postfix expression\n"
                  << '\n';
        exit (1);
      }
      else if (!strcmp (argv[i], "-v") || !strcmp (argv[i], "--version"))
      {
        std::cout << '\n'
                  << format ("calc {1} built for ", VERSION)
                  << osName ()
                  << '\n'
                  << "Copyright (C) 2006 - 2021 T. Babej, P. Beckingham, F. Hernandez."
                  << '\n'
                  << '\n'
                  << "Taskwarrior may be copied only under the terms of the MIT license, which may be found in the Taskwarrior source kit."
                  << '\n'
                  << '\n';

        exit (1);
      }
      else if (!strcmp (argv[i], "-d") || !strcmp (argv[i], "--debug"))
        e.debug (true);
      else if (!strcmp (argv[i], "-i") || !strcmp (argv[i], "--infix"))
        infix = true;
      else if (!strcmp (argv[i], "-p") || !strcmp (argv[i], "--postfix"))
        infix = false;
      else
        expression += std::string (argv[i]) + ' ';
    }

    Variant result;
    if (infix)
      e.evaluateInfixExpression (expression, result);
    else
      e.evaluatePostfixExpression (expression, result);

    // Show any debug output.
    for (const auto& i : Context::getContext ().debugMessages)
      std::cout << i << '\n';

    // Show the result in string form.
    std::cout << (std::string) result
              << '\n';
  }

  catch (const std::string& error)
  {
    std::cerr << error << '\n';
    status = -1;
  }

  catch (...)
  {
    std::cerr << "Unknown error occured.  Oops.\n";
    status = -2;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
