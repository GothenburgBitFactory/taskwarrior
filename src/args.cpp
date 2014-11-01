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
#include <CLI.h>

Context context;

////////////////////////////////////////////////////////////////////////////////
int main (int argc, const char** argv)
{
  try
  {
    CLI cli;
    cli.initialize (argc, argv);
    cli.findOverrides ();

    // Read-only commands.
    cli.entity ("cmd", "active");           cli.entity ("readcmd", "active");
    cli.entity ("cmd", "all");              cli.entity ("readcmd", "all");
    cli.entity ("cmd", "blocked");          cli.entity ("readcmd", "blocked");
    cli.entity ("cmd", "blocking");         cli.entity ("readcmd", "blocking");
    cli.entity ("cmd", "burndown.daily");   cli.entity ("readcmd", "burndown.daily");
    cli.entity ("cmd", "burndown.monthly"); cli.entity ("readcmd", "burndown.monthly");
    cli.entity ("cmd", "burndown.weekly");  cli.entity ("readcmd", "burndown.weekly");
    cli.entity ("cmd", "calc");             cli.entity ("readcmd", "calc");
    cli.entity ("cmd", "calendar");         cli.entity ("readcmd", "calendar");
    cli.entity ("cmd", "colors");           cli.entity ("readcmd", "colors");
    cli.entity ("cmd", "columns");          cli.entity ("readcmd", "columns");
    cli.entity ("cmd", "completed");        cli.entity ("readcmd", "completed");
    cli.entity ("cmd", "config");           cli.entity ("readcmd", "config");
    cli.entity ("cmd", "count");            cli.entity ("readcmd", "count");
    cli.entity ("cmd", "diagnostics");      cli.entity ("readcmd", "diagnostics");
    cli.entity ("cmd", "execute");          cli.entity ("readcmd", "execute");
    cli.entity ("cmd", "export");           cli.entity ("readcmd", "export");
    cli.entity ("cmd", "ghistory.annual");  cli.entity ("readcmd", "ghistory.annual");
    cli.entity ("cmd", "ghistory.monthly"); cli.entity ("readcmd", "ghistory.monthly");
    cli.entity ("cmd", "help");             cli.entity ("readcmd", "help");
    cli.entity ("cmd", "history.annual");   cli.entity ("readcmd", "history.annual");
    cli.entity ("cmd", "history.monthly");  cli.entity ("readcmd", "history.monthly");
    cli.entity ("cmd", "ids");              cli.entity ("readcmd", "ids");
    cli.entity ("cmd", "information");      cli.entity ("readcmd", "information");
    cli.entity ("cmd", "list");             cli.entity ("readcmd", "list");
    cli.entity ("cmd", "logo");             cli.entity ("readcmd", "logo");
    cli.entity ("cmd", "long");             cli.entity ("readcmd", "long");
    cli.entity ("cmd", "ls");               cli.entity ("readcmd", "ls");
    cli.entity ("cmd", "minimal");          cli.entity ("readcmd", "minimal");
    cli.entity ("cmd", "newest");           cli.entity ("readcmd", "newest");
    cli.entity ("cmd", "next");             cli.entity ("readcmd", "next");
    cli.entity ("cmd", "oldest");           cli.entity ("readcmd", "oldest");
    cli.entity ("cmd", "overdue");          cli.entity ("readcmd", "overdue");
    cli.entity ("cmd", "projects");         cli.entity ("readcmd", "projects");
    cli.entity ("cmd", "ready");            cli.entity ("readcmd", "ready");
    cli.entity ("cmd", "recurring");        cli.entity ("readcmd", "recurring");
    cli.entity ("cmd", "reports");          cli.entity ("readcmd", "reports");
    cli.entity ("cmd", "show");             cli.entity ("readcmd", "show");
    cli.entity ("cmd", "stats");            cli.entity ("readcmd", "stats");
    cli.entity ("cmd", "summary");          cli.entity ("readcmd", "summary");
    cli.entity ("cmd", "tags");             cli.entity ("readcmd", "tags");
    cli.entity ("cmd", "timesheet");        cli.entity ("readcmd", "timesheet");
    cli.entity ("cmd", "udas");             cli.entity ("readcmd", "udas");
    cli.entity ("cmd", "unblocked");        cli.entity ("readcmd", "unblocked");
    cli.entity ("cmd", "uuids");            cli.entity ("readcmd", "uuids");
    cli.entity ("cmd", "version");          cli.entity ("readcmd", "version");
    cli.entity ("cmd", "waiting");          cli.entity ("readcmd", "waiting");

    // Write commands.
    cli.entity ("cmd", "add");              cli.entity ("writecmd", "add");
    cli.entity ("cmd", "annotate");         cli.entity ("writecmd", "annotate");
    cli.entity ("cmd", "append");           cli.entity ("writecmd", "append");
    cli.entity ("cmd", "delete");           cli.entity ("writecmd", "delete");
    cli.entity ("cmd", "denotate");         cli.entity ("writecmd", "denotate");
    cli.entity ("cmd", "done");             cli.entity ("writecmd", "done");
    cli.entity ("cmd", "duplicate");        cli.entity ("writecmd", "duplicate");
    cli.entity ("cmd", "edit");             cli.entity ("writecmd", "edit");
    cli.entity ("cmd", "import");           cli.entity ("writecmd", "import");
    cli.entity ("cmd", "log");              cli.entity ("writecmd", "log");
    cli.entity ("cmd", "modify");           cli.entity ("writecmd", "modify");
    cli.entity ("cmd", "prepend");          cli.entity ("writecmd", "prepend");
    cli.entity ("cmd", "start");            cli.entity ("writecmd", "start");
    cli.entity ("cmd", "stop");             cli.entity ("writecmd", "stop");
    cli.entity ("cmd", "synchronize");      cli.entity ("writecmd", "synchronize");
    cli.entity ("cmd", "undo");             cli.entity ("writecmd", "undo");

    // Helper commands.
    cli.entity ("cmd", "_aliases");         cli.entity ("readcmd", "_aliases");     cli.entity ("helper", "_aliases");
    cli.entity ("cmd", "_columns");         cli.entity ("readcmd", "_columns");     cli.entity ("helper", "_columns");
    cli.entity ("cmd", "_commands");        cli.entity ("readcmd", "_commands");    cli.entity ("helper", "_commands");
    cli.entity ("cmd", "_config");          cli.entity ("readcmd", "_config");      cli.entity ("helper", "_config");
    cli.entity ("cmd", "_get");             cli.entity ("readcmd", "_get");         cli.entity ("helper", "_get");
    cli.entity ("cmd", "_ids");             cli.entity ("readcmd", "_ids");         cli.entity ("helper", "_ids");
    cli.entity ("cmd", "_projects");        cli.entity ("readcmd", "_projects");    cli.entity ("helper", "_projects");
    cli.entity ("cmd", "_show");            cli.entity ("readcmd", "_show");        cli.entity ("helper", "_show");
    cli.entity ("cmd", "_tags");            cli.entity ("readcmd", "_tags");        cli.entity ("helper", "_tags");
    cli.entity ("cmd", "_udas");            cli.entity ("readcmd", "_udas");        cli.entity ("helper", "_udas");
    cli.entity ("cmd", "_urgency");         cli.entity ("readcmd", "_urgency");     cli.entity ("helper", "_urgency");
    cli.entity ("cmd", "_uuids");           cli.entity ("readcmd", "_uuids");       cli.entity ("helper", "_uuids");
    cli.entity ("cmd", "_version");         cli.entity ("readcmd", "_version");     cli.entity ("helper", "_version");
    cli.entity ("cmd", "_zshcommands");     cli.entity ("readcmd", "_zshcommands"); cli.entity ("helper", "_zshcommands");
    cli.entity ("cmd", "_zshids");          cli.entity ("readcmd", "_zshids");      cli.entity ("helper", "_zshids");
    cli.entity ("cmd", "_zshuuids");        cli.entity ("readcmd", "_zshuuids");    cli.entity ("helper", "_zshuuids");

    // Attributes (columns).
    cli.entity ("attribute",  "depends");
    cli.entity ("attribute",  "description");
    cli.entity ("attribute",  "due");
    cli.entity ("attribute",  "end");
    cli.entity ("attribute",  "entry");
    cli.entity ("attribute",  "id");
    cli.entity ("attribute",  "imask");
    cli.entity ("attribute",  "mask");
    cli.entity ("attribute",  "modified");
    cli.entity ("attribute",  "parent");
    cli.entity ("attribute",  "priority");
    cli.entity ("attribute",  "project");
    cli.entity ("attribute",  "recur");
    cli.entity ("attribute",  "scheduled");
    cli.entity ("attribute",  "start");
    cli.entity ("attribute",  "status");
    cli.entity ("attribute",  "tags");
    cli.entity ("attribute",  "until");
    cli.entity ("attribute",  "urgency");
    cli.entity ("attribute",  "uuid");
    cli.entity ("attribute",  "wait");

    // Pseudo-attributes.
    cli.entity ("pseudo",     "limit");

    // UDAs.
    cli.entity ("attribute",  "duration");
    cli.entity ("uda",        "duration");

    // Modifiers.
    cli.entity ("modifier",   "above");
    cli.entity ("modifier",   "after");
    cli.entity ("modifier",   "any");
    cli.entity ("modifier",   "before");
    cli.entity ("modifier",   "below");
    cli.entity ("modifier",   "contains");
    cli.entity ("modifier",   "endswith");
    cli.entity ("modifier",   "equals");
    cli.entity ("modifier",   "has");
    cli.entity ("modifier",   "hasnt");
    cli.entity ("modifier",   "is");
    cli.entity ("modifier",   "isnt");
    cli.entity ("modifier",   "left");
    cli.entity ("modifier",   "none");
    cli.entity ("modifier",   "not");
    cli.entity ("modifier",   "noword");
    cli.entity ("modifier",   "over");
    cli.entity ("modifier",   "right");
    cli.entity ("modifier",   "startswith");
    cli.entity ("modifier",   "under");
    cli.entity ("modifier",   "word");

    // Operators.
    cli.entity ("operator",   "^");
    cli.entity ("operator",   "!");
    cli.entity ("operator",   "_neg_");
    cli.entity ("operator",   "_pos_");
    cli.entity ("operator",   "_hastag_");
    cli.entity ("operator",   "_notag_");
    cli.entity ("operator",   "*");
    cli.entity ("operator",   "/");
    cli.entity ("operator",   "%");
    cli.entity ("operator",   "+");
    cli.entity ("operator",   "-");
    cli.entity ("operator",   "<=");
    cli.entity ("operator",   ">=");
    cli.entity ("operator",   ">");
    cli.entity ("operator",   "<");
    cli.entity ("operator",   "=");
    cli.entity ("operator",   "==");
    cli.entity ("operator",   "!=");
    cli.entity ("operator",   "~");
    cli.entity ("operator",   "!~");
    cli.entity ("operator",   "and");
    cli.entity ("operator",   "or");
    cli.entity ("operator",   "xor");
    cli.entity ("operator",   "(");
    cli.entity ("operator",   ")");

    cli.analyze ();

    std::cout << cli.dump ()
              << "\n"
              << "  \033[1;37;42mFILTER\033[0m "
              << cli.getFilter ()
              << "\n";
  }

  catch (const std::string& error)
  {
    std::cout << "Error: " << error << std::endl;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
