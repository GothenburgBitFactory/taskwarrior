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
#include <Parser.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  try
  {
    Parser parser;
    parser.initialize (argc, argv);
    parser.appendStdin ();
    parser.findOverrides ();

    Alias alias;
    alias.resolve (parser.tree ());

    // Reports.
    parser.entity ("report",     "active");
    parser.entity ("report",     "all");
    parser.entity ("report",     "blocked");
    parser.entity ("report",     "blocking");
    parser.entity ("report",     "burndown.daily");
    parser.entity ("report",     "burndown.monthly");
    parser.entity ("report",     "burndown.weekly");
    parser.entity ("report",     "completed");
    parser.entity ("report",     "ghistory.annual");
    parser.entity ("report",     "ghistory.monthly");
    parser.entity ("report",     "history.annual");
    parser.entity ("report",     "history.monthly");
    parser.entity ("report",     "information");
    parser.entity ("report",     "list");
    parser.entity ("report",     "long");
    parser.entity ("report",     "ls");
    parser.entity ("report",     "minimal");
    parser.entity ("report",     "newest");
    parser.entity ("report",     "next");
    parser.entity ("report",     "oldest");
    parser.entity ("report",     "overdue");
    parser.entity ("report",     "projects");
    parser.entity ("report",     "ready");
    parser.entity ("report",     "recurring");
    parser.entity ("report",     "summary");
    parser.entity ("report",     "tags");
    parser.entity ("report",     "unblocked");
    parser.entity ("report",     "waiting");

    // Read-only commands.
    parser.entity ("readcmd",    "export");
    parser.entity ("readcmd",    "info");
    parser.entity ("readcmd",    "list");
    parser.entity ("readcmd",    "next");
    parser.entity ("readcmd",    "projects");

    // Write commands.
    parser.entity ("writecmd",   "add");
    parser.entity ("writecmd",   "annotate");
    parser.entity ("writecmd",   "delete");
    parser.entity ("writecmd",   "denotate");
    parser.entity ("writecmd",   "done");
    parser.entity ("writecmd",   "modify");
    parser.entity ("writecmd",   "start");
    parser.entity ("writecmd",   "stop");

    // Special commands.
    parser.entity ("specialcmd", "calendar");
    parser.entity ("specialcmd", "edit");
    parser.entity ("writecmd",   "import");

    // Helper commands.
    parser.entity ("helper",     "_aliases");
    parser.entity ("helper",     "_columns");
    parser.entity ("helper",     "_commands");
    parser.entity ("helper",     "_config");
    parser.entity ("helper",     "_get");
    parser.entity ("helper",     "_ids");
    parser.entity ("helper",     "_projects");
    parser.entity ("helper",     "_show");
    parser.entity ("helper",     "_tags");
    parser.entity ("helper",     "_udas");
    parser.entity ("helper",     "_urgency");
    parser.entity ("helper",     "_uuids");
    parser.entity ("helper",     "_version");
    parser.entity ("helper",     "_zshcommands");
    parser.entity ("helper",     "_zshids");
    parser.entity ("helper",     "_zshuuids");

    // Attributes (columns).
    parser.entity ("attribute",  "depends");
    parser.entity ("attribute",  "description");
    parser.entity ("attribute",  "due");
    parser.entity ("attribute",  "end");
    parser.entity ("attribute",  "entry");
    parser.entity ("attribute",  "id");
    parser.entity ("attribute",  "imask");
    parser.entity ("attribute",  "mask");
    parser.entity ("attribute",  "modified");
    parser.entity ("attribute",  "parent");
    parser.entity ("attribute",  "priority");
    parser.entity ("attribute",  "project");
    parser.entity ("attribute",  "recur");
    parser.entity ("attribute",  "scheduled");
    parser.entity ("attribute",  "start");
    parser.entity ("attribute",  "status");
    parser.entity ("attribute",  "tags");
    parser.entity ("attribute",  "until");
    parser.entity ("attribute",  "urgency");
    parser.entity ("attribute",  "uuid");
    parser.entity ("attribute",  "wait");

    // Pseudo-attributes.
    parser.entity ("pseudo",     "limit");

    // UDAs.
    parser.entity ("attribute",  "duration");
    parser.entity ("uda",        "duration");

    // Modifiers.
    parser.entity ("modifier",   "before");
    parser.entity ("modifier",   "under");
    parser.entity ("modifier",   "below");
    parser.entity ("modifier",   "after");
    parser.entity ("modifier",   "over");
    parser.entity ("modifier",   "above");
    parser.entity ("modifier",   "none");
    parser.entity ("modifier",   "any");
    parser.entity ("modifier",   "is");
    parser.entity ("modifier",   "equals");
    parser.entity ("modifier",   "isnt");
    parser.entity ("modifier",   "not");
    parser.entity ("modifier",   "has");
    parser.entity ("modifier",   "contains");
    parser.entity ("modifier",   "hasnt");
    parser.entity ("modifier",   "startswith");
    parser.entity ("modifier",   "left");
    parser.entity ("modifier",   "endswith");
    parser.entity ("modifier",   "right");
    parser.entity ("modifier",   "word");
    parser.entity ("modifier",   "noword");

    // Operators.
    parser.entity ("operator",   "^");
    parser.entity ("operator",   "!");
    parser.entity ("operator",   "_neg_");
    parser.entity ("operator",   "_pos_");
    parser.entity ("operator",   "_hastag_");
    parser.entity ("operator",   "_notag_");
    parser.entity ("operator",   "*");
    parser.entity ("operator",   "/");
    parser.entity ("operator",   "%");
    parser.entity ("operator",   "+");
    parser.entity ("operator",   "-");
    parser.entity ("operator",   "<=");
    parser.entity ("operator",   ">=");
    parser.entity ("operator",   ">");
    parser.entity ("operator",   "<");
    parser.entity ("operator",   "=");
    parser.entity ("operator",   "==");
    parser.entity ("operator",   "!=");
    parser.entity ("operator",   "~");
    parser.entity ("operator",   "!~");
    parser.entity ("operator",   "and");
    parser.entity ("operator",   "or");
    parser.entity ("operator",   "xor");
    parser.entity ("operator",   "(");
    parser.entity ("operator",   ")");

    parser.findBinary ();
    parser.findCommand ();
    parser.findUUIDList ();
    parser.findIdSequence ();
    parser.injectDefaults ();

    Tree* tree = parser.parse ();
    if (tree)
      std::cout << tree->dump ();

    std::cout << "\n"
              << "  \033[1;37;42mFILTER\033[0m "
              << parser.getFilterExpression ()
              << "\n";
  }

  catch (const std::string& error)
  {
    std::cout << "Error: " << error << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
