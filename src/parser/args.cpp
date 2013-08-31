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
#include <Context.h>
#include <A3t.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  try
  {
    A3t a3t (argc, argv);

    // Reports.
    a3t.identity ("report",     "list");
    a3t.identity ("report",     "next");

    // Read-only commands.
    a3t.identity ("readcmd",    "info");
    a3t.identity ("readcmd",    "projects");

    // Write commands.
    a3t.identity ("writecmd",   "add");
    a3t.identity ("writecmd",   "modify");

    // Special commands.
    a3t.identity ("specialcmd", "calendar");
    a3t.identity ("specialcmd", "edit");

    // Attributes (columns).
    a3t.identity ("attribute",  "description");
    a3t.identity ("attribute",  "due");
    a3t.identity ("attribute",  "priority");
    a3t.identity ("attribute",  "project");
    a3t.identity ("attribute",  "uuid");
    a3t.identity ("attribute",  "duration"); // UDAs are included.

    // Pseudo-attributes.
    a3t.identity ("pseudo",     "limit");

    // UDAs.
    a3t.identity ("uda",        "duration");

    Tree* tree = a3t.parse ();
    if (tree)
      tree->dump ();
  }

  catch (const std::string& error)
  {
    std::cout << "Error: " << error << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
