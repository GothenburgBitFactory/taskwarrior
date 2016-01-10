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
#include <vector>
#include <stdlib.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <Command.h>
#include <main.h>

#include <CmdAdd.h>
#include <CmdAliases.h>
#include <CmdAnnotate.h>
#include <CmdAppend.h>
#include <CmdAttributes.h>
#include <CmdBurndown.h>
#include <CmdCalc.h>
#include <CmdCalendar.h>
#include <CmdColor.h>
#include <CmdColumns.h>
#include <CmdCommands.h>
#include <CmdConfig.h>
#include <CmdContext.h>
#include <CmdCount.h>
#include <CmdCustom.h>
#include <CmdDelete.h>
#include <CmdDenotate.h>
#include <CmdDiagnostics.h>
#include <CmdDone.h>
#include <CmdDuplicate.h>
#include <CmdEdit.h>
#ifdef HAVE_EXECUTE
#include <CmdExec.h>
#endif
#include <CmdExport.h>
#include <CmdGet.h>
#include <CmdHelp.h>
#include <CmdHistory.h>
#include <CmdIDs.h>
#include <CmdImport.h>
#include <CmdInfo.h>
#include <CmdLog.h>
#include <CmdLogo.h>
#include <CmdModify.h>
#include <CmdPrepend.h>
#include <CmdProjects.h>
#include <CmdReports.h>
#include <CmdShow.h>
#include <CmdStart.h>
#include <CmdStats.h>
#include <CmdStop.h>
#include <CmdSummary.h>
#include <CmdSync.h>
#include <CmdTags.h>
#include <CmdTimesheet.h>
#include <CmdUDAs.h>
#include <CmdUndo.h>
#include <CmdUnique.h>
#include <CmdUrgency.h>
#include <CmdVersion.h>

#include <Context.h>
#include <ColProject.h>
#include <ColDue.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
void Command::factory (std::map <std::string, Command*>& all)
{
  Command* c;

  c = new CmdAdd ();                all[c->keyword ()] = c;
  c = new CmdAnnotate ();           all[c->keyword ()] = c;
  c = new CmdAppend ();             all[c->keyword ()] = c;
  c = new CmdBurndownDaily ();      all[c->keyword ()] = c;
  c = new CmdBurndownMonthly ();    all[c->keyword ()] = c;
  c = new CmdBurndownWeekly ();     all[c->keyword ()] = c;
  c = new CmdCalc ();               all[c->keyword ()] = c;
  c = new CmdCalendar ();           all[c->keyword ()] = c;
  c = new CmdColor ();              all[c->keyword ()] = c;
  c = new CmdColumns ();            all[c->keyword ()] = c;
  c = new CmdCommands ();           all[c->keyword ()] = c;
  c = new CmdCompletionAliases ();  all[c->keyword ()] = c;
  c = new CmdCompletionColumns ();  all[c->keyword ()] = c;
  c = new CmdCompletionCommands (); all[c->keyword ()] = c;
  c = new CmdCompletionConfig ();   all[c->keyword ()] = c;
  c = new CmdCompletionContext ();  all[c->keyword ()] = c;
  c = new CmdCompletionIds ();      all[c->keyword ()] = c;
  c = new CmdCompletionUDAs ();     all[c->keyword ()] = c;
  c = new CmdCompletionUuids ();    all[c->keyword ()] = c;
  c = new CmdCompletionProjects (); all[c->keyword ()] = c;
  c = new CmdCompletionTags ();     all[c->keyword ()] = c;
  c = new CmdCompletionVersion ();  all[c->keyword ()] = c;
  c = new CmdConfig ();             all[c->keyword ()] = c;
  c = new CmdContext ();            all[c->keyword ()] = c;
  c = new CmdCount ();              all[c->keyword ()] = c;
  c = new CmdDelete ();             all[c->keyword ()] = c;
  c = new CmdDenotate ();           all[c->keyword ()] = c;
  c = new CmdDiagnostics ();        all[c->keyword ()] = c;
  c = new CmdDone ();               all[c->keyword ()] = c;
  c = new CmdDuplicate ();          all[c->keyword ()] = c;
  c = new CmdEdit ();               all[c->keyword ()] = c;
#ifdef HAVE_EXECUTE
  c = new CmdExec ();               all[c->keyword ()] = c;
#endif
  c = new CmdExport ();             all[c->keyword ()] = c;
  c = new CmdGet ();                all[c->keyword ()] = c;
  c = new CmdGHistoryMonthly ();    all[c->keyword ()] = c;
  c = new CmdGHistoryAnnual ();     all[c->keyword ()] = c;
  c = new CmdHelp ();               all[c->keyword ()] = c;
  c = new CmdHistoryMonthly ();     all[c->keyword ()] = c;
  c = new CmdHistoryAnnual ();      all[c->keyword ()] = c;
  c = new CmdIDs ();                all[c->keyword ()] = c;
  c = new CmdImport ();             all[c->keyword ()] = c;
  c = new CmdInfo ();               all[c->keyword ()] = c;
  c = new CmdLog ();                all[c->keyword ()] = c;
  c = new CmdLogo ();               all[c->keyword ()] = c;
  c = new CmdModify ();             all[c->keyword ()] = c;
  c = new CmdPrepend ();            all[c->keyword ()] = c;
  c = new CmdProjects ();           all[c->keyword ()] = c;
  c = new CmdReports ();            all[c->keyword ()] = c;
  c = new CmdShow ();               all[c->keyword ()] = c;
  c = new CmdShowRaw ();            all[c->keyword ()] = c;
  c = new CmdStart ();              all[c->keyword ()] = c;
  c = new CmdStats ();              all[c->keyword ()] = c;
  c = new CmdStop ();               all[c->keyword ()] = c;
  c = new CmdSummary ();            all[c->keyword ()] = c;
  c = new CmdSync ();               all[c->keyword ()] = c;
  c = new CmdTags ();               all[c->keyword ()] = c;
  c = new CmdTimesheet ();          all[c->keyword ()] = c;
  c = new CmdUDAs ();               all[c->keyword ()] = c;
  c = new CmdUndo ();               all[c->keyword ()] = c;
  c = new CmdUnique ();             all[c->keyword ()] = c;
  c = new CmdUrgency ();            all[c->keyword ()] = c;
  c = new CmdUUIDs ();              all[c->keyword ()] = c;
  c = new CmdVersion ();            all[c->keyword ()] = c;
  c = new CmdZshAttributes ();      all[c->keyword ()] = c;
  c = new CmdZshCommands ();        all[c->keyword ()] = c;
  c = new CmdZshCompletionIds ();   all[c->keyword ()] = c;
  c = new CmdZshCompletionUuids (); all[c->keyword ()] = c;

  // Instantiate a command object for each custom report.
  std::vector <std::string> reports;
  for (auto &i : context.config)
  {
    if (i.first.substr (0, 7) == "report.")
    {
      std::string report = i.first.substr (7);
      auto columns = report.find (".columns");
      if (columns != std::string::npos)
        reports.push_back (report.substr (0, columns));
    }
  }

  for (auto &report : reports)
  {
    // Make sure a custom report does not clash with a built-in command.
    if (all.find (report) != all.end ())
      throw format (STRING_CMD_CONFLICT, report);

    c = new CmdCustom (
              report,
              "task <filter> " + report,
              context.config.get ("report." + report + ".description"));

    all[c->keyword ()] = c;
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::map <Command::Category, std::string> Command::categoryNames =
{
  // These strings are intentionally not l10n'd: they are used as identifiers.
   {Command::Category::unassigned,   "unassigned"} // should never happen
  ,{Command::Category::metadata,     "metadata"}
  ,{Command::Category::report,       "report"}
  ,{Command::Category::operation,    "operation"}
  ,{Command::Category::context,      "context"}
  ,{Command::Category::graphs,       "graphs"   }
  ,{Command::Category::config,       "config"   }
  ,{Command::Category::migration,    "migration"}
  ,{Command::Category::misc,         "misc"     }
  ,{Command::Category::internal,     "internal"}
  ,{Command::Category::UNDOCUMENTED, "undocumented"}
};

////////////////////////////////////////////////////////////////////////////////
Command::Command ()
: _keyword ("")
, _usage ("")
, _description ("")
, _read_only (true)
, _displays_id (true)
, _needs_confirm (false)
, _needs_gc (true)
, _uses_context (false)
, _accepts_filter (false)
, _accepts_modifications (false)
, _accepts_miscellaneous (false)
, _category(Category::unassigned)
, _permission_quit (false)
, _permission_all (false)
, _first_iteration (true)
{
}

////////////////////////////////////////////////////////////////////////////////
Command::~Command ()
{
}

////////////////////////////////////////////////////////////////////////////////
std::string Command::keyword () const
{
  return _keyword;
}

////////////////////////////////////////////////////////////////////////////////
std::string Command::usage () const
{
  return _usage;
}

////////////////////////////////////////////////////////////////////////////////
std::string Command::description () const
{
  return _description;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::read_only () const
{
  return _read_only;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::displays_id () const
{
  return _displays_id;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::needs_gc () const
{
  return _needs_gc;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::uses_context () const
{
  return _uses_context;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::accepts_filter () const
{
  return _accepts_filter;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::accepts_modifications () const
{
  return _accepts_modifications;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::accepts_miscellaneous () const
{
  return _accepts_miscellaneous;
}

////////////////////////////////////////////////////////////////////////////////
Command::Category Command::category () const
{
  return _category;
}

////////////////////////////////////////////////////////////////////////////////
// Returns true or false indicating whether to proceed with a write command, on
// a per-task basis, after (potentially) asking for permission.
//
// Factors:
//   filtered.size ()
//   rc.bulk
//   rc.confirmation
//   this->_read_only
bool Command::permission (
  const std::string& question,
  unsigned int quantity)
{
  // Read-only commands do not need to seek permission.  Write commands are
  // granted permission automatically if the 'all' selection was made in an
  // earlier call.  Or if the 'all' option has already been made.
  if (_read_only ||
      _permission_all)
    return true;

  // If the 'quit' selection has already been made.
  if (_permission_quit)
    return false;

  // What remains are write commands that have not yet selected 'all' or 'quit'.
  // Describe the task.
  bool         confirmation = context.config.getBoolean ("confirmation");
  unsigned int bulk         = context.config.getInteger ("bulk");

  // Quantity 1 modifications have optional confirmation, and only (y/n).
  if (quantity == 1)
  {
    if (!_needs_confirm ||
        !confirmation)
      return true;

    bool answer = confirm (question);
    return answer;
  }

  // 1 < Quantity < bulk modifications have optional confirmation, in the (y/n/a/q)
  // style. Bulk = 0 denotes infinite bulk.
  if ((bulk == 0 || quantity < bulk) && (!_needs_confirm || !confirmation))
    return true;

  if (context.verbose ("blank") && !_first_iteration)
    std::cout << "\n";
  int answer = confirm4 (question);
  _first_iteration = false;
  switch (answer)
  {
  case 1:                           return true;     // yes
  case 2: _permission_all  = true;  return true;     // all
  case 3: _permission_quit = true;  return false;    // quit
  }

  return false;  // This line keeps the compiler happy.
}

////////////////////////////////////////////////////////////////////////////////
