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
    // Prepare the Context object.


    A3t a3t (argc, argv);

    // Reports.
    a3t.entity ("report",     "list");
    a3t.entity ("report",     "next");

    // Read-only commands.
    a3t.entity ("writecmd",   "export");
    a3t.entity ("readcmd",    "info");
    a3t.entity ("readcmd",    "list");
    a3t.entity ("readcmd",    "next");
    a3t.entity ("readcmd",    "projects");

    // Write commands.
    a3t.entity ("writecmd",   "add");
    a3t.entity ("writecmd",   "annotate");
    a3t.entity ("writecmd",   "delete");
    a3t.entity ("writecmd",   "denotate");
    a3t.entity ("writecmd",   "done");
    a3t.entity ("writecmd",   "modify");
    a3t.entity ("writecmd",   "start");
    a3t.entity ("writecmd",   "stop");

    // Special commands.
    a3t.entity ("specialcmd", "calendar");
    a3t.entity ("specialcmd", "edit");
    a3t.entity ("writecmd",   "import");

    // Helper commands.
    a3t.entity ("helper",     "_get");
    a3t.entity ("helper",     "_query");

    // Attributes (columns).
    a3t.entity ("attribute",  "description");
    a3t.entity ("attribute",  "due");
    a3t.entity ("attribute",  "priority");
    a3t.entity ("attribute",  "project");
    a3t.entity ("attribute",  "uuid");
    a3t.entity ("attribute",  "duration"); // UDAs are included.

    // Pseudo-attributes.
    a3t.entity ("pseudo",     "limit");

    // UDAs.
    a3t.entity ("uda",        "duration");

    // Modifiers.
    a3t.entity ("modifier",   "before");
    a3t.entity ("modifier",   "under");
    a3t.entity ("modifier",   "below");
    a3t.entity ("modifier",   "after");
    a3t.entity ("modifier",   "over");
    a3t.entity ("modifier",   "above");
    a3t.entity ("modifier",   "none");
    a3t.entity ("modifier",   "any");
    a3t.entity ("modifier",   "is");
    a3t.entity ("modifier",   "equals");
    a3t.entity ("modifier",   "isnt");
    a3t.entity ("modifier",   "not");
    a3t.entity ("modifier",   "has");
    a3t.entity ("modifier",   "contains");
    a3t.entity ("modifier",   "hasnt");
    a3t.entity ("modifier",   "startswith");
    a3t.entity ("modifier",   "left");
    a3t.entity ("modifier",   "endswith");
    a3t.entity ("modifier",   "right");
    a3t.entity ("modifier",   "word");
    a3t.entity ("modifier",   "noword");

    // Operators.
    a3t.entity ("operator",   "and");
    a3t.entity ("operator",   "or");
    a3t.entity ("operator",   "xor");
    a3t.entity ("operator",   "<=");
    a3t.entity ("operator",   ">=");
    a3t.entity ("operator",   "!~");
    a3t.entity ("operator",   "!=");
    a3t.entity ("operator",   "=");
    a3t.entity ("operator",   ">");
    a3t.entity ("operator",   "~");
    a3t.entity ("operator",   "!");
    a3t.entity ("operator",   "_hastag_");
    a3t.entity ("operator",   "_notag_");
    a3t.entity ("operator",   "-");
    a3t.entity ("operator",   "*");
    a3t.entity ("operator",   "/");
    a3t.entity ("operator",   "+");
    a3t.entity ("operator",   "-");
    a3t.entity ("operator",   "<");
    a3t.entity ("operator",   "(");
    a3t.entity ("operator",   ")");

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
