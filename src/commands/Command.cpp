////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#define L10N                                           // Localization complete.

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <Expression.h>
#include <Att.h>
#include <Timer.h>
#include <text.h>
#include <i18n.h>
#include <Command.h>

#include <CmdAdd.h>
#include <CmdAnnotate.h>
#include <CmdAppend.h>
#include <CmdBurndown.h>
#include <CmdCalendar.h>
#include <CmdColor.h>
#include <CmdCommands.h>
#include <CmdConfig.h>
#include <CmdCount.h>
#include <CmdCustom.h>
#include <CmdDelete.h>
#include <CmdDenotate.h>
#include <CmdDiagnostics.h>
#include <CmdDone.h>
#include <CmdDuplicate.h>
#include <CmdEdit.h>
#include <CmdExec.h>
#include <CmdHelp.h>
#include <CmdHistory.h>
#include <CmdIDs.h>
#include <CmdImport.h>
#include <CmdInfo.h>
#include <CmdInstall.h>
#include <CmdLog.h>
#include <CmdLogo.h>
#include <CmdMerge.h>
#include <CmdModify.h>
#include <CmdPrepend.h>
#include <CmdProjects.h>
#include <CmdPull.h>
#include <CmdPush.h>
#include <CmdQuery.h>
#include <CmdReports.h>
#include <CmdShell.h>
#include <CmdShow.h>
#include <CmdStart.h>
#include <CmdStatistics.h>
#include <CmdStop.h>
#include <CmdSummary.h>
#include <CmdTags.h>
#include <CmdTimesheet.h>
#include <CmdUndo.h>
#include <CmdUrgency.h>
#include <CmdVersion.h>
#include <Context.h>

#include <ColProject.h>
#include <ColPriority.h>
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
  c = new CmdCalendar ();           all[c->keyword ()] = c;
  c = new CmdColor ();              all[c->keyword ()] = c;
  c = new CmdCompletionCommands (); all[c->keyword ()] = c;
  c = new CmdCompletionConfig ();   all[c->keyword ()] = c;
  c = new CmdCompletionIds ();      all[c->keyword ()] = c;
  c = new CmdCompletionProjects (); all[c->keyword ()] = c;
  c = new CmdCompletionTags ();     all[c->keyword ()] = c;
  c = new CmdCompletionVersion ();  all[c->keyword ()] = c;
  c = new CmdConfig ();             all[c->keyword ()] = c;
  c = new CmdCount ();              all[c->keyword ()] = c;
  c = new CmdDelete ();             all[c->keyword ()] = c;
  c = new CmdDenotate ();           all[c->keyword ()] = c;
  c = new CmdDiagnostics ();        all[c->keyword ()] = c;
  c = new CmdDone ();               all[c->keyword ()] = c;
  c = new CmdDuplicate ();          all[c->keyword ()] = c;
  c = new CmdEdit ();               all[c->keyword ()] = c;
  c = new CmdExec ();               all[c->keyword ()] = c;
  c = new CmdGHistoryMonthly ();    all[c->keyword ()] = c;
  c = new CmdGHistoryAnnual ();     all[c->keyword ()] = c;
  c = new CmdHelp ();               all[c->keyword ()] = c;
  c = new CmdHistoryMonthly ();     all[c->keyword ()] = c;
  c = new CmdHistoryAnnual ();      all[c->keyword ()] = c;
  c = new CmdIDs ();                all[c->keyword ()] = c;
  c = new CmdImport ();             all[c->keyword ()] = c;
  c = new CmdInfo ();               all[c->keyword ()] = c;
  c = new CmdInstall ();            all[c->keyword ()] = c;
  c = new CmdLog ();                all[c->keyword ()] = c;
  c = new CmdLogo ();               all[c->keyword ()] = c;
  c = new CmdMerge ();              all[c->keyword ()] = c;
  c = new CmdModify ();             all[c->keyword ()] = c;
  c = new CmdPrepend ();            all[c->keyword ()] = c;
  c = new CmdProjects ();           all[c->keyword ()] = c;
  c = new CmdPull ();               all[c->keyword ()] = c;
  c = new CmdPush ();               all[c->keyword ()] = c;
  c = new CmdQuery ();              all[c->keyword ()] = c;
  c = new CmdReports ();            all[c->keyword ()] = c;
  c = new CmdShell ();              all[c->keyword ()] = c;
  c = new CmdShow ();               all[c->keyword ()] = c;
  c = new CmdStart ();              all[c->keyword ()] = c;
  c = new CmdStatistics ();         all[c->keyword ()] = c;
  c = new CmdStop ();               all[c->keyword ()] = c;
  c = new CmdSummary ();            all[c->keyword ()] = c;
  c = new CmdTags ();               all[c->keyword ()] = c;
  c = new CmdTimesheet ();          all[c->keyword ()] = c;
  c = new CmdUndo ();               all[c->keyword ()] = c;
  c = new CmdUrgency ();            all[c->keyword ()] = c;
  c = new CmdVersion ();            all[c->keyword ()] = c;
  c = new CmdZshCommands ();        all[c->keyword ()] = c;
  c = new CmdZshCompletionIds ();   all[c->keyword ()] = c;

  // Instantiate a command object for each custom report.
  std::vector <std::string> variables;
  context.config.all (variables);

  std::vector <std::string> reports;
  std::vector <std::string>::iterator i;
  for (i = variables.begin (); i != variables.end (); ++i)
  {
    if (i->substr (0, 7) == "report.")
    {
      std::string report = i->substr (7);
      std::string::size_type columns = report.find (".columns");
      if (columns != std::string::npos)
        reports.push_back (report.substr (0, columns));
    }
  }

  std::vector <std::string>::iterator report;
  for (report = reports.begin (); report != reports.end (); ++report)
  {
    // Make sure a custom report does not clash with a built-in command.
    if (all.find (*report) != all.end ())
      throw format (STRING_CMD_CONFLICT, *report);

    c = new CmdCustom (
              *report,
              "task " + *report + " [<filter>]",
              context.config.get ("report." + *report + ".description"));

    all[c->keyword ()] = c;
  }
}

////////////////////////////////////////////////////////////////////////////////
Command::Command ()
: _usage ("")
, _description ("")
, _read_only (true)
, _displays_id (true)
{
}

////////////////////////////////////////////////////////////////////////////////
Command::Command (const Command& other)
{
  _usage       = other._usage;
  _description = other._description;
  _read_only   = other._read_only;
  _displays_id = other._displays_id;
}

////////////////////////////////////////////////////////////////////////////////
Command& Command::operator= (const Command& other)
{
  if (this != &other)
  {
    _usage       = other._usage;
    _description = other._description;
    _read_only   = other._read_only;
    _displays_id = other._displays_id;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::operator== (const Command& other) const
{
  return _usage       == other._usage       &&
         _description == other._description &&
         _read_only   == other._read_only   &&
         _displays_id == other._displays_id;
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
void Command::filter (std::vector <Task>& input, std::vector <Task>& output)
{
  Timer timer ("Command::filter");

  Arguments f;
  if (read_only ())
    f = context.args.extract_read_only_filter ();
  else
    f = context.args.extract_write_filter ();

  if (f.size ())
  {
    Expression e (f);

    std::vector <Task>::iterator task;
    for (task = input.begin (); task != input.end (); ++task)
      if (e.eval (*task))
        output.push_back (*task);
  }
  else
    output = input;
}

////////////////////////////////////////////////////////////////////////////////
// Apply the modifications in arguments to the task.
void Command::modify_task (Task& task, Arguments& arguments)
{
  std::string description;

  std::vector <Triple>::iterator arg;
  for (arg = arguments.begin (); arg != arguments.end (); ++arg)
  {
    // Attributes are essentially name:value pairs, and correspond directly
    // to stored attributes.
    if (arg->_third == "attr")
    {
      std::string name;
      std::string value;
      Arguments::extract_attr (arg->_first, name, value);

      // TODO All 'value's must be eval'd first.

      // Dependencies must be resolved to UUIDs.
      if (name == "depends")
      {
        // Convert ID to UUID.
        std::vector <std::string> deps;
        split (deps, value, ',');

        // Apply or remove dendencies in turn.
        std::vector <std::string>::iterator i;
        for (i = deps.begin (); i != deps.end (); i++)
        {
          int id = strtol (i->c_str (), NULL, 10);
          if (id < 0)
            task.removeDependency (-id);
          else
            task.addDependency (id);
        }
      }

      // By default, just add it.
      else
        task.set (name, value);
    }

    // Tags need special handling because they are essentially a vector stored
    // in a single string, therefore Task::{add,remove}Tag must be called as
    // appropriate.
    else if (arg->_third == "tag")
    {
      char type;
      std::string value;
      Arguments::extract_tag (arg->_first, type, value);

      if (type == '+')
        task.addTag (value);
      else
        task.removeTag (value);
    }

    // Words and operators are aggregated into a description.
    else if (arg->_third == "word" ||
             arg->_third == "op")
    {
      if (description.length ())
        description += " ";

      description += arg->_first;
    }

    // Substitutions.
    else if (arg->_third == "subst")
    {
      std::string from;
      std::string to;
      bool global;
      Arguments::extract_subst (arg->_first, from, to, global);
      task.substitute (from, to, global);
    }

    // Any additional argument types are indicative of a failure in
    // Arguments::extract_modifications.
    else
      throw format (STRING_CMD_MOD_UNEXPECTED, arg->_first);
  }

  // Only update description if one was specified.
  if (description.length ())
    task.set ("description", description);
}

////////////////////////////////////////////////////////////////////////////////
void Command::apply_defaults (Task& task)
{
  // Provide an entry date unless user already specified one.
  if (task.get ("entry") == "")
    task.setEntry ();

  // Recurring tasks get a special status.
  if (task.has ("due") &&
      task.has ("recur"))
  {
    task.setStatus (Task::recurring);
    task.set ("mask", "");
  }

  // Tasks with a wait: date get a special status.
  else if (task.has ("wait"))
    task.setStatus (Task::waiting);

  // By default, tasks are pending.
  else
    task.setStatus (Task::pending);

  // Override with default.project, if not specified.
  if (task.get ("project") == "")
  {
    std::string defaultProject = context.config.get ("default.project");
    if (defaultProject != "" &&
        context.columns["project"]->validate (defaultProject))
      task.set ("project", defaultProject);
  }

  // Override with default.priority, if not specified.
  if (task.get ("priority") == "")
  {
    std::string defaultPriority = context.config.get ("default.priority");
    if (defaultPriority != "" &&
        context.columns["priority"]->validate (defaultPriority))
      task.set ("priority", defaultPriority);
  }

  // Override with default.due, if not specified.
  if (task.get ("due") == "")
  {
    std::string defaultDue = context.config.get ("default.due");
    if (defaultDue != "" &&
        context.columns["due"]->validate (defaultDue))
      // TODO Determine whether this could/should be eval'd first.
      task.set ("due", Date (defaultDue).toEpoch ());
  }
}

////////////////////////////////////////////////////////////////////////////////
