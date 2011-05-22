////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the
//
//     Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor,
//     Boston, MA
//     02110-1301
//     USA
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <File.h>
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
