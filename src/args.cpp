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

#include <iostream>
#include <Context.h>
#include <A3t.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  try
  {
    A3t a3t;
    a3t.initialize (argc, argv);
    a3t.appendStdin ();
    a3t.findOverrides ();

    Alias alias;
    alias.resolve (a3t.tree ());

    // Reports.
    a3t.entity ("report",     "active");
    a3t.entity ("report",     "all");
    a3t.entity ("report",     "blocked");
    a3t.entity ("report",     "blocking");
    a3t.entity ("report",     "burndown.daily");
    a3t.entity ("report",     "burndown.monthly");
    a3t.entity ("report",     "burndown.weekly");
    a3t.entity ("report",     "completed");
    a3t.entity ("report",     "ghistory.annual");
    a3t.entity ("report",     "ghistory.monthly");
    a3t.entity ("report",     "history.annual");
    a3t.entity ("report",     "history.monthly");
    a3t.entity ("report",     "information");
    a3t.entity ("report",     "list");
    a3t.entity ("report",     "long");
    a3t.entity ("report",     "ls");
    a3t.entity ("report",     "minimal");
    a3t.entity ("report",     "newest");
    a3t.entity ("report",     "next");
    a3t.entity ("report",     "oldest");
    a3t.entity ("report",     "overdue");
    a3t.entity ("report",     "projects");
    a3t.entity ("report",     "ready");
    a3t.entity ("report",     "recurring");
    a3t.entity ("report",     "summary");
    a3t.entity ("report",     "tags");
    a3t.entity ("report",     "unblocked");
    a3t.entity ("report",     "waiting");

    // Read-only commands.
    a3t.entity ("readcmd",    "export");
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
    a3t.entity ("helper",     "_aliases");
    a3t.entity ("helper",     "_columns");
    a3t.entity ("helper",     "_commands");
    a3t.entity ("helper",     "_config");
    a3t.entity ("helper",     "_get");
    a3t.entity ("helper",     "_ids");
    a3t.entity ("helper",     "_projects");
    a3t.entity ("helper",     "_show");
    a3t.entity ("helper",     "_tags");
    a3t.entity ("helper",     "_udas");
    a3t.entity ("helper",     "_urgency");
    a3t.entity ("helper",     "_uuids");
    a3t.entity ("helper",     "_version");
    a3t.entity ("helper",     "_zshcommands");
    a3t.entity ("helper",     "_zshids");
    a3t.entity ("helper",     "_zshuuids");

    // Attributes (columns).
    a3t.entity ("attribute",  "depends");
    a3t.entity ("attribute",  "description");
    a3t.entity ("attribute",  "due");
    a3t.entity ("attribute",  "end");
    a3t.entity ("attribute",  "entry");
    a3t.entity ("attribute",  "id");
    a3t.entity ("attribute",  "imask");
    a3t.entity ("attribute",  "mask");
    a3t.entity ("attribute",  "modified");
    a3t.entity ("attribute",  "parent");
    a3t.entity ("attribute",  "priority");
    a3t.entity ("attribute",  "project");
    a3t.entity ("attribute",  "recur");
    a3t.entity ("attribute",  "scheduled");
    a3t.entity ("attribute",  "start");
    a3t.entity ("attribute",  "status");
    a3t.entity ("attribute",  "tags");
    a3t.entity ("attribute",  "until");
    a3t.entity ("attribute",  "urgency");
    a3t.entity ("attribute",  "uuid");
    a3t.entity ("attribute",  "wait");

    // Pseudo-attributes.
    a3t.entity ("pseudo",     "limit");

    // UDAs.
    a3t.entity ("attribute",  "duration");
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
    a3t.entity ("operator",   "^");
    a3t.entity ("operator",   "!");
    a3t.entity ("operator",   "_neg_");
    a3t.entity ("operator",   "_pos_");
    a3t.entity ("operator",   "_hastag_");
    a3t.entity ("operator",   "_notag_");
    a3t.entity ("operator",   "*");
    a3t.entity ("operator",   "/");
    a3t.entity ("operator",   "%");
    a3t.entity ("operator",   "+");
    a3t.entity ("operator",   "-");
    a3t.entity ("operator",   "<=");
    a3t.entity ("operator",   ">=");
    a3t.entity ("operator",   ">");
    a3t.entity ("operator",   "<");
    a3t.entity ("operator",   "=");
    a3t.entity ("operator",   "==");
    a3t.entity ("operator",   "!=");
    a3t.entity ("operator",   "~");
    a3t.entity ("operator",   "!~");
    a3t.entity ("operator",   "and");
    a3t.entity ("operator",   "or");
    a3t.entity ("operator",   "xor");
    a3t.entity ("operator",   "(");
    a3t.entity ("operator",   ")");

    a3t.findBinary ();
    a3t.findCommand ();
    a3t.findUUIDList ();
    a3t.findIdSequence ();
    a3t.injectDefaults ();

    Tree* tree = a3t.parse ();
    if (tree)
      std::cout << tree->dump ();

    std::cout << "\n"
              << "  \033[1;37;42mFILTER\033[0m "
              << a3t.getFilterExpression ()
              << "\n";
  }

  catch (const std::string& error)
  {
    std::cout << "Error: " << error << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
