////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <string.h>
#include <FS.h>
#include <JSON.h>
#include <Context.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  if (argc == 1)
  {
    std::cout << "\nUsage: json_test [-q] <file | JSON> ...\n"
              << "\n"
              << "      -q        quiet, no JSON dump\n"
              << "      <file>    file containing JSON\n"
              << "      <JSON>    JSON string, may need to be quoted\n"
              << "\n";
    return 0;
  }

  bool quiet = false;
  for (int i = 1; i < argc; ++i)
    if (!strcmp (argv[i], "-q"))
      quiet = true;

  for (int i = 1; i < argc; ++i)
  {
    if (strcmp (argv[i], "-q"))
    {
      try
      {
        json::value* root;
        File file (argv[i]);
        if (file.exists ())
        {
          std::string contents;
          file.read (contents);
          root = json::parse (contents);
        }
        else
          root = json::parse (argv[i]);

        if (root && !quiet)
          std::cout << root->dump () << "\n";

        delete root;
      }

      catch (const std::string& e) { std::cout << e << "\n";         }
      catch (...)                  { std::cout << "Unknown error\n"; }
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
