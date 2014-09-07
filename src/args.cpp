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
    parser.findOverrides ();

    // Read-only commands.
    parser.entity ("cmd", "active");           parser.entity ("readcmd", "active");
    parser.entity ("cmd", "all");              parser.entity ("readcmd", "all");
    parser.entity ("cmd", "blocked");          parser.entity ("readcmd", "blocked");
    parser.entity ("cmd", "blocking");         parser.entity ("readcmd", "blocking");
    parser.entity ("cmd", "burndown.daily");   parser.entity ("readcmd", "burndown.daily");
    parser.entity ("cmd", "burndown.monthly"); parser.entity ("readcmd", "burndown.monthly");
    parser.entity ("cmd", "burndown.weekly");  parser.entity ("readcmd", "burndown.weekly");
    parser.entity ("cmd", "calc");             parser.entity ("readcmd", "calc");
    parser.entity ("cmd", "calendar");         parser.entity ("readcmd", "calendar");
    parser.entity ("cmd", "colors");           parser.entity ("readcmd", "colors");
    parser.entity ("cmd", "columns");          parser.entity ("readcmd", "columns");
    parser.entity ("cmd", "completed");        parser.entity ("readcmd", "completed");
    parser.entity ("cmd", "config");           parser.entity ("readcmd", "config");
    parser.entity ("cmd", "count");            parser.entity ("readcmd", "count");
    parser.entity ("cmd", "diagnostics");      parser.entity ("readcmd", "diagnostics");
    parser.entity ("cmd", "execute");          parser.entity ("readcmd", "execute");
    parser.entity ("cmd", "export");           parser.entity ("readcmd", "export");
    parser.entity ("cmd", "ghistory.annual");  parser.entity ("readcmd", "ghistory.annual");
    parser.entity ("cmd", "ghistory.monthly"); parser.entity ("readcmd", "ghistory.monthly");
    parser.entity ("cmd", "help");             parser.entity ("readcmd", "help");
    parser.entity ("cmd", "history.annual");   parser.entity ("readcmd", "history.annual");
    parser.entity ("cmd", "history.monthly");  parser.entity ("readcmd", "history.monthly");
    parser.entity ("cmd", "ids");              parser.entity ("readcmd", "ids");
    parser.entity ("cmd", "information");      parser.entity ("readcmd", "information");
    parser.entity ("cmd", "list");             parser.entity ("readcmd", "list");
    parser.entity ("cmd", "logo");             parser.entity ("readcmd", "logo");
    parser.entity ("cmd", "long");             parser.entity ("readcmd", "long");
    parser.entity ("cmd", "ls");               parser.entity ("readcmd", "ls");
    parser.entity ("cmd", "minimal");          parser.entity ("readcmd", "minimal");
    parser.entity ("cmd", "newest");           parser.entity ("readcmd", "newest");
    parser.entity ("cmd", "next");             parser.entity ("readcmd", "next");
    parser.entity ("cmd", "oldest");           parser.entity ("readcmd", "oldest");
    parser.entity ("cmd", "overdue");          parser.entity ("readcmd", "overdue");
    parser.entity ("cmd", "projects");         parser.entity ("readcmd", "projects");
    parser.entity ("cmd", "ready");            parser.entity ("readcmd", "ready");
    parser.entity ("cmd", "recurring");        parser.entity ("readcmd", "recurring");
    parser.entity ("cmd", "reports");          parser.entity ("readcmd", "reports");
    parser.entity ("cmd", "show");             parser.entity ("readcmd", "show");
    parser.entity ("cmd", "stats");            parser.entity ("readcmd", "stats");
    parser.entity ("cmd", "summary");          parser.entity ("readcmd", "summary");
    parser.entity ("cmd", "tags");             parser.entity ("readcmd", "tags");
    parser.entity ("cmd", "timesheet");        parser.entity ("readcmd", "timesheet");
    parser.entity ("cmd", "udas");             parser.entity ("readcmd", "udas");
    parser.entity ("cmd", "unblocked");        parser.entity ("readcmd", "unblocked");
    parser.entity ("cmd", "uuids");            parser.entity ("readcmd", "uuids");
    parser.entity ("cmd", "version");          parser.entity ("readcmd", "version");
    parser.entity ("cmd", "waiting");          parser.entity ("readcmd", "waiting");

    // Write commands.
    parser.entity ("cmd", "add");              parser.entity ("writecmd", "add");
    parser.entity ("cmd", "annotate");         parser.entity ("writecmd", "annotate");
    parser.entity ("cmd", "append");           parser.entity ("writecmd", "append");
    parser.entity ("cmd", "delete");           parser.entity ("writecmd", "delete");
    parser.entity ("cmd", "denotate");         parser.entity ("writecmd", "denotate");
    parser.entity ("cmd", "done");             parser.entity ("writecmd", "done");
    parser.entity ("cmd", "duplicate");        parser.entity ("writecmd", "duplicate");
    parser.entity ("cmd", "edit");             parser.entity ("writecmd", "edit");
    parser.entity ("cmd", "import");           parser.entity ("writecmd", "import");
    parser.entity ("cmd", "log");              parser.entity ("writecmd", "log");
    parser.entity ("cmd", "modify");           parser.entity ("writecmd", "modify");
    parser.entity ("cmd", "prepend");          parser.entity ("writecmd", "prepend");
    parser.entity ("cmd", "start");            parser.entity ("writecmd", "start");
    parser.entity ("cmd", "stop");             parser.entity ("writecmd", "stop");
    parser.entity ("cmd", "synchronize");      parser.entity ("writecmd", "synchronize");
    parser.entity ("cmd", "undo");             parser.entity ("writecmd", "undo");

    // Helper commands.
    parser.entity ("cmd", "_aliases");         parser.entity ("readcmd", "_aliases");     parser.entity ("helper", "_aliases");
    parser.entity ("cmd", "_columns");         parser.entity ("readcmd", "_columns");     parser.entity ("helper", "_columns");
    parser.entity ("cmd", "_commands");        parser.entity ("readcmd", "_commands");    parser.entity ("helper", "_commands");
    parser.entity ("cmd", "_config");          parser.entity ("readcmd", "_config");      parser.entity ("helper", "_config");
    parser.entity ("cmd", "_get");             parser.entity ("readcmd", "_get");         parser.entity ("helper", "_get");
    parser.entity ("cmd", "_ids");             parser.entity ("readcmd", "_ids");         parser.entity ("helper", "_ids");
    parser.entity ("cmd", "_projects");        parser.entity ("readcmd", "_projects");    parser.entity ("helper", "_projects");
    parser.entity ("cmd", "_show");            parser.entity ("readcmd", "_show");        parser.entity ("helper", "_show");
    parser.entity ("cmd", "_tags");            parser.entity ("readcmd", "_tags");        parser.entity ("helper", "_tags");
    parser.entity ("cmd", "_udas");            parser.entity ("readcmd", "_udas");        parser.entity ("helper", "_udas");
    parser.entity ("cmd", "_urgency");         parser.entity ("readcmd", "_urgency");     parser.entity ("helper", "_urgency");
    parser.entity ("cmd", "_uuids");           parser.entity ("readcmd", "_uuids");       parser.entity ("helper", "_uuids");
    parser.entity ("cmd", "_version");         parser.entity ("readcmd", "_version");     parser.entity ("helper", "_version");
    parser.entity ("cmd", "_zshcommands");     parser.entity ("readcmd", "_zshcommands"); parser.entity ("helper", "_zshcommands");
    parser.entity ("cmd", "_zshids");          parser.entity ("readcmd", "_zshids");      parser.entity ("helper", "_zshids");
    parser.entity ("cmd", "_zshuuids");        parser.entity ("readcmd", "_zshuuids");    parser.entity ("helper", "_zshuuids");

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
    parser.entity ("modifier",   "above");
    parser.entity ("modifier",   "after");
    parser.entity ("modifier",   "any");
    parser.entity ("modifier",   "before");
    parser.entity ("modifier",   "below");
    parser.entity ("modifier",   "contains");
    parser.entity ("modifier",   "endswith");
    parser.entity ("modifier",   "equals");
    parser.entity ("modifier",   "has");
    parser.entity ("modifier",   "hasnt");
    parser.entity ("modifier",   "is");
    parser.entity ("modifier",   "isnt");
    parser.entity ("modifier",   "left");
    parser.entity ("modifier",   "none");
    parser.entity ("modifier",   "not");
    parser.entity ("modifier",   "noword");
    parser.entity ("modifier",   "over");
    parser.entity ("modifier",   "right");
    parser.entity ("modifier",   "startswith");
    parser.entity ("modifier",   "under");
    parser.entity ("modifier",   "word");

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
