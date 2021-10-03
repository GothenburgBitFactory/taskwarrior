////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
#include <CmdHelp.h>
#include <algorithm>
#include <Table.h>
#include <Context.h>
#include <shared.h>
#include <format.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdHelp::CmdHelp ()
{
  _keyword               = "help";
  _usage                 = "task          help ['usage']";
  _description           = "Displays this usage help text";
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::misc;
}

////////////////////////////////////////////////////////////////////////////////
int CmdHelp::execute (std::string& output)
{
  auto words = Context::getContext ().cli2.getWords ();
  if (words.size () == 1 && closeEnough ("usage", words[0]))
    output = '\n'
           + composeUsage ()
           + '\n';
  else
    output = '\n'
           + composeUsage ()
           + '\n'
           + "Documentation for Taskwarrior can be found using 'man task', 'man taskrc', 'man "
             "task-color', 'man task-sync' or at https://taskwarrior.org\n"
             "\n"
             "The general form of commands is:\n"
             "  task [<filter>] <command> [<mods>]\n"
             "\n"
             "The <filter> consists of zero or more restrictions on which tasks to select, "
             "such as:\n"
             "  task                                      <command> <mods>\n"
             "  task 28                                   <command> <mods>\n"
             "  task +weekend                             <command> <mods>\n"
             "  task project:Home due.before:today        <command> <mods>\n"
             "  task ebeeab00-ccf8-464b-8b58-f7f2d606edfb <command> <mods>\n"
             "\n"
             "By default, filter elements are combined with an implicit 'and' operator, but "
             "'or' and 'xor' may also be used, provided parentheses are included:\n"
             "  task '(/[Cc]at|[Dd]og/ or /[0-9]+/)'      <command> <mods>\n"
             "\n"
             "A filter may target specific tasks using ID or UUID numbers.  To specify "
             "multiple tasks use one of these forms:\n"
             "  task 1,2,3                                    delete\n"
             "  task 1-3                                      info\n"
             "  task 1,2-5,19                                 modify pri:H\n"
             "  task 4-7 ebeeab00-ccf8-464b-8b58-f7f2d606edfb info\n"
             "\n"
             "The <mods> consist of zero or more changes to apply to the selected tasks, "
             "such as:\n"
             "  task <filter> <command> project:Home\n"
             "  task <filter> <command> +weekend +garden due:tomorrow\n"
             "  task <filter> <command> Description/annotation text\n"
             "  task <filter> <command> /from/to/     <- replace first match\n"
             "  task <filter> <command> /from/to/g    <- replace all matches\n"
             "\n"
             "Tags are arbitrary words, any quantity:\n"
             "  +tag       The + means add the tag\n"
             "  -tag       The - means remove the tag\n"
             "\n"
             "Built-in attributes are:\n"
             "  description:    Task description text\n"
             "  status:         Status of task - pending, completed, deleted, waiting\n"
             "  project:        Project name\n"
             "  priority:       Priority\n"
             "  due:            Due date\n"
             "  recur:          Recurrence frequency\n"
             "  until:          Expiration date of a task\n"
             "  limit:          Desired number of rows in report, or 'page'\n"
             "  wait:           Date until task becomes pending\n"
             "  entry:          Date task was created\n"
             "  end:            Date task was completed/deleted\n"
             "  start:          Date task was started\n"
             "  scheduled:      Date task is scheduled to start\n"
             "  modified:       Date task was last modified\n"
             "  depends:        Other tasks that this task depends upon\n"
             "\n"
             "Attribute modifiers make filters more precise.  Supported modifiers are:\n"
             "\n"
             "  Modifiers         Example            Equivalent           Meaning\n"
             "  ----------------  -----------------  -------------------  -------------------------\n"
             "                    due:today          due = today          Fuzzy match\n"
             "  not               due.not:today      due != today         Fuzzy non-match\n"
             "  before, below     due.before:today   due < today          Exact date comparison\n"
             "  after, above      due.after:today    due >= tomorrow      Exact date comparison\n"
             "  none              project.none:      project == ''        Empty\n"
             "  any               project.any:       project !== ''       Not empty\n"
             "  is, equals        project.is:x       project == x         Exact match\n"
             "  isnt              project.isnt:x     project !== x        Exact non-match\n"
             "  has, contains     desc.has:Hello     desc ~ Hello         Pattern match\n"
             "  hasnt,            desc.hasnt:Hello   desc !~ Hello        Pattern non-match\n"
             "  startswith, left  desc.left:Hel      desc ~ '^Hel'        Beginning match\n"
             "  endswith, right   desc.right:llo     desc ~ 'llo$'        End match\n"
             "  word              desc.word:Hello    desc ~ '\\bHello\\b'   Boundaried word match\n"
             "  noword            desc.noword:Hello  desc !~ '\\bHello\\b'  Boundaried word non-match\n"
             "\n"
             "Alternately algebraic expressions support:\n"
             "  and  or  xor            Logical operators\n"
             "  <  <=  =  !=  >=  >     Relational operators\n"
             "  (  )                    Precedence\n"
             "\n"
             "  task due.before:eom priority.not:L   list\n"
             "  task '(due < eom and priority != L)'  list\n"
             "\n"
             "The default .taskrc file can be overridden with:\n"
             "  task ... rc:<alternate file> ...\n"
             "  task ... rc:~/.alt_taskrc ...\n"
             "\n"
             "The values in .taskrc (or alternate) can be overridden with:\n"
             "  task ... rc.<name>=<value> ...\n"
             "  task rc.color=off list\n"
             "\n"
             "Any command or attribute name may be abbreviated if still unique:\n"
             "  task list project:Home\n"
             "  task li       pro:Home\n"
             "\n"
             "Some task descriptions need to be escaped because of the shell:\n"
             "  task add \"quoted ' quote\"\n"
             "  task add escaped \\' quote\n"
             "\n"
             "The argument -- tells Taskwarrior to treat all other args as description, even "
             "if they would otherwise be attributes or tags:\n"
             "  task add -- project:Home needs scheduling\n"
             "\n"
             "Many characters have special meaning to the shell, including:\n"
             "  $ ! ' \" ( ) ; \\ ` * ? { } [ ] < > | & % # ~\n"
             "\n";

             /*
               TODO To be included later, before the 'precedence' line.

               "  +  -                    Addition, Subtraktion\n" \
               "  !                       Negation\n" \
               "  ~  !~                   Treffer, kein Treffer\n" \
             */

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
std::string CmdHelp::composeUsage () const
{
  Table view;
  view.width (Context::getContext ().getWidth ());
  view.add ("");
  view.add ("");
  view.add ("");

  // Static first row.
  auto row = view.addRow ();
  view.set (row, 0, "Usage:");
  view.set (row, 1, "task");
  view.set (row, 2, "Runs rc.default.command, if specified.");

  // Obsolete method of getting a list of all commands.
  std::vector <std::string> all;
  for (auto& cmd : Context::getContext ().commands)
    all.push_back (cmd.first);

  // Sort alphabetically by usage.
  std::sort (all.begin (), all.end ());

  // Add the regular commands.
  for (auto& name : all)
  {
    if (name[0] != '_')
    {
      row = view.addRow ();
      view.set (row, 1, Context::getContext ().commands[name]->usage ());
      view.set (row, 2, Context::getContext ().commands[name]->description ());
    }
  }

  // Add the helper commands.
  for (auto& name : all)
  {
    if (name[0] == '_')
    {
      row = view.addRow ();
      view.set (row, 1, Context::getContext ().commands[name]->usage ());
      view.set (row, 2, Context::getContext ().commands[name]->description ());
    }
  }

  // Add the aliases commands.
  row = view.addRow ();
  view.set (row, 1, " ");

  for (auto& alias : Context::getContext ().config)
  {
    if (alias.first.substr (0, 6) == "alias.")
    {
      row = view.addRow ();
      view.set (row, 1, alias.first.substr (6));
      view.set (row, 2, format ("Aliased to '{1}'", alias.second));
    }
  }

  return view.render ();
}

////////////////////////////////////////////////////////////////////////////////
