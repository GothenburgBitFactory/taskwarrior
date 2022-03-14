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
#include <Context.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <FS.h>
#include <Eval.h>
#include <Variant.h>
#include <Datetime.h>
#include <Duration.h>
#include <shared.h>
#include <format.h>
#include <main.h>
#include <regex>

#ifdef HAVE_COMMIT
#include <commit.h>
#endif

#include <stdio.h>
#include <sys/ioctl.h>

#ifdef SOLARIS
#include <sys/termios.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// This string is parsed and used as default values for configuration.
std::string configurationDefaults =
  "# Taskwarrior program configuration file.\n"
  "# For more documentation, see https://taskwarrior.org or try 'man task', 'man task-color',\n"
  "# 'man task-sync' or 'man taskrc'\n"
  "\n"
  "# Here is an example of entries that use the default, override and blank values\n"
  "#   variable=foo   -- By specifying a value, this overrides the default\n"
  "#   variable=      -- By specifying no value, this means no default\n"
  "#   #variable=foo  -- By commenting out the line, or deleting it, this uses the default\n"
  "\n"
  "# You can also refence environment variables:\n"
  "#   variable=$HOME/task\n"
  "#   variable=$VALUE\n"
  "\n"
  "# Use the command 'task show' to see all defaults and overrides\n"
  "\n"
  "# Files\n"
  "data.location=~/.task\n"
  "locking=1                                      # Use file-level locking\n"
  "gc=1                                           # Garbage-collect data files - DO NOT CHANGE unless you are sure\n"
  "exit.on.missing.db=0                           # Whether to exit if ~/.task is not found\n"
  "hooks=1                                        # Master control switch for hooks\n"
  "\n"
  "# Terminal\n"
  "detection=1                                    # Detects terminal width\n"
  "defaultwidth=80                                # Without detection, assumed width\n"
  "defaultheight=24                               # Without detection, assumed height\n"
  "avoidlastcolumn=0                              # Fixes Cygwin width problem\n"
  "hyphenate=1                                    # Hyphenates lines wrapped on non-word-breaks\n"
  "#editor=vi                                     # Preferred text editor\n"
  "reserved.lines=1                               # Assume a 1-line prompt\n"
  "\n"
  "# Miscellaneous\n"
  "# verbose=                                     # Comma-separated list.  May contain any subset of:\n"
  "# affected,blank,context,default,edit,filter,footnote,header,label,new-id,new-uuid,override,project,recur,special,sync\n"
  "verbose=affected,blank,context,edit,header,footnote,label,new-id,project,special,sync,override,recur\n"
  "confirmation=1                                 # Confirmation on delete, big changes\n"
  "recurrence=1                                   # Enable recurrence\n"
  "recurrence.confirmation=prompt                 # Confirmation for propagating changes among recurring tasks (yes/no/prompt)\n"
  "allow.empty.filter=1                           # An empty filter gets a warning and requires confirmation\n"
  "indent.annotation=2                            # Indent spaces for annotations\n"
  "indent.report=0                                # Indent spaces for whole report\n"
  "row.padding=0                                  # Left and right padding for each row of report\n"
  "column.padding=1                               # Spaces between each column in a report\n"
  "bulk=3                                         # 3 or more tasks considered a bulk change and is confirmed\n"
  "nag=You have more urgent tasks.                # Nag message to keep you honest\n"                      // TODO
  "search.case.sensitive=1                        # Setting to no allows case insensitive searches\n"
  "active.indicator=*                             # What to show as an active task indicator\n"
  "tag.indicator=+                                # What to show as a tag indicator\n"
  "dependency.indicator=D                         # What to show as a dependency indicator\n"
  "recurrence.indicator=R                         # What to show as a task recurrence indicator\n"
  "recurrence.limit=1                             # Number of future recurring pending tasks\n"
  "undo.style=side                                # Undo style - can be 'side', or 'diff'\n"
  "regex=1                                        # Assume all search/filter strings are regexes\n"
  "xterm.title=0                                  # Sets xterm title for some commands\n"
  "expressions=infix                              # Prefer infix over postfix expressions\n"
  "json.array=1                                   # Enclose JSON output in [ ]\n"
  "abbreviation.minimum=2                         # Shortest allowed abbreviation\n"
  "news.version=                                  # Latest version highlights read by the user\n"
  "\n"
  "# Dates\n"
  "dateformat=Y-M-D                               # Preferred input and display date format\n"
  "dateformat.holiday=YMD                         # Preferred input date format for holidays\n"
  "dateformat.edit=Y-M-D H:N:S                    # Preferred display date format when editing\n"
  "dateformat.info=Y-M-D H:N:S                    # Preferred display date format for information\n"
  "dateformat.report=                             # Preferred display date format for reports\n"
  "dateformat.annotation=                         # Preferred display date format for annotations\n"
  "date.iso=1                                     # Enable ISO date support\n"
  "weekstart=sunday                               # Sunday or Monday only\n"
  "displayweeknumber=1                            # Show week numbers on calendar\n"
  "due=7                                          # Task is considered due in 7 days\n"
  "\n"
  "# Calendar controls\n"
  "calendar.legend=1                              # Display the legend on calendar\n"
  "calendar.details=sparse                        # Calendar shows information for tasks w/due dates: full, sparse or none\n"
  "calendar.details.report=list                   # Report to use when showing task information in cal\n"
  "calendar.offset=0                              # Apply an offset value to control the first month of the calendar\n"
  "calendar.offset.value=-1                       # The number of months the first month of the calendar is moved\n"
  "calendar.holidays=none                         # Show public holidays on calendar:full, sparse or none\n"
  "#calendar.monthsperline=3                      # Number of calendar months on a line\n"
  "\n"
  "# Journal controls\n"
  "journal.time=0                                 # Record start/stop commands as annotation\n"
  "journal.time.start.annotation=Started task     # Annotation description for the start journal entry\n"
  "journal.time.stop.annotation=Stopped task      # Annotation description for the stop  journal entry\n"
  "journal.info=1                                 # Display task journal with info command\n"
  "\n"
  "# Dependency controls\n"
  "dependency.reminder=1                          # Nags on dependency chain violations\n"
  "dependency.confirmation=1                      # Should dependency chain repair be confirmed?\n"
  "\n"
  "# Urgency Coefficients\n"
  "urgency.user.tag.next.coefficient=15.0         # Urgency coefficient for 'next' special tag\n"
  "urgency.due.coefficient=12.0                   # Urgency coefficient for due dates\n"
  "urgency.blocking.coefficient=8.0               # Urgency coefficient for blocking tasks\n"
  "urgency.active.coefficient=4.0                 # Urgency coefficient for active tasks\n"
  "urgency.scheduled.coefficient=5.0              # Urgency coefficient for scheduled tasks\n"
  "urgency.age.coefficient=2.0                    # Urgency coefficient for age\n"
  "urgency.annotations.coefficient=1.0            # Urgency coefficient for annotations\n"
  "urgency.tags.coefficient=1.0                   # Urgency coefficient for tags\n"
  "urgency.project.coefficient=1.0                # Urgency coefficient for projects\n"
  "urgency.blocked.coefficient=-5.0               # Urgency coefficient for blocked tasks\n"
  "urgency.waiting.coefficient=-3.0               # Urgency coefficient for waiting status\n"
  "urgency.inherit=0                              # Recursively inherit highest urgency value from blocked tasks\n"
  "urgency.age.max=365                            # Maximum age in days\n"
  "\n"
  "#urgency.user.project.foo.coefficient=5.0      # Urgency coefficients for 'foo' project\n"
  "#urgency.user.tag.foo.coefficient=5.0          # Urgency coefficients for 'foo' tag\n"
  "#urgency.uda.foo.coefficient=5.0               # Urgency coefficients for UDA 'foo'\n"
  "\n"
  "# Color controls.\n"
  "color=1                                        # Enable color\n"
  "\n"
  "# Here is the rule precedence order, highest to lowest.\n"
  "# Note that these are just the color rule names, without the leading 'color.'\n"
  "#      and any trailing '.value'.\n"
  "rule.precedence.color=deleted,completed,active,keyword.,tag.,project.,overdue,scheduled,due.today,due,blocked,blocking,recurring,tagged,uda.\n"
  "\n"
  "# General decoration\n"
  "rule.color.merge=1\n"
  "color.label=\n"
  "color.label.sort=\n"
  "color.alternate=on gray2\n"
  "color.header=color3\n"
  "color.footnote=color3\n"
  "color.warning=bold red\n"
  "color.error=white on red\n"
  "color.debug=color4\n"
  "\n"
  "# Task state\n"
  "color.completed=\n"
  "color.deleted=\n"
  "color.active=rgb555 on rgb410\n"
  "color.recurring=rgb013\n"
  "color.scheduled=on rgb001\n"
  "color.until=\n"
  "color.blocked=white on color8\n"
  "color.blocking=black on color15\n"
  "\n"
  "# Project\n"
  "color.project.none=\n"
  "\n"
  "# Priority UDA\n"
  "color.uda.priority.H=color255\n"
  "color.uda.priority.L=color245\n"
  "color.uda.priority.M=color250\n"
  "\n"
  "# Tags\n"
  "color.tag.next=rgb440\n"
  "color.tag.none=\n"
  "color.tagged=rgb031\n"
  "\n"
  "# Due\n"
  "color.due.today=rgb400\n"
  "color.due=color1\n"
  "color.overdue=color9\n"
  "\n"
  "# Report: burndown\n"
  "color.burndown.done=on rgb010\n"
  "color.burndown.pending=on color9\n"
  "color.burndown.started=on color11\n"
  "\n"
  "# Report: history\n"
  "color.history.add=color0 on rgb500\n"
  "color.history.delete=color0 on rgb550\n"
  "color.history.done=color0 on rgb050\n"
  "\n"
  "# Report: summary\n"
  "color.summary.background=white on color0\n"
  "color.summary.bar=black on rgb141\n"
  "\n"
  "# Command: calendar\n"
  "color.calendar.due.today=color15 on color1\n"
  "color.calendar.due=color0 on color1\n"
  "color.calendar.holiday=color0 on color11\n"
  "color.calendar.scheduled=rgb013 on color15\n"
  "color.calendar.overdue=color0 on color9\n"
  "color.calendar.today=color15 on rgb013\n"
  "color.calendar.weekend=on color235\n"
  "color.calendar.weeknumber=rgb013\n"
  "\n"
  "# Command: sync\n"
  "color.sync.added=rgb010\n"
  "color.sync.changed=color11\n"
  "color.sync.rejected=color9\n"
  "\n"
  "# Command: undo\n"
  "color.undo.after=color2\n"
  "color.undo.before=color1\n"
  "\n"
  "# UDA priority\n"
  "uda.priority.type=string                       # UDA priority is a string type\n"
  "uda.priority.label=Priority                    # UDA priority has a display label'\n"
  "uda.priority.values=H,M,L,                     # UDA priority values are 'H', 'M', 'L' or ''\n"
  "                                               # UDA priority sorting is 'H' > 'M' > 'L' > '' (highest to lowest)\n"
  "#uda.priority.default=M                        # UDA priority default value of 'M'\n"
  "urgency.uda.priority.H.coefficient=6.0         # UDA priority coefficient for value 'H'\n"
  "urgency.uda.priority.M.coefficient=3.9         # UDA priority coefficient for value 'M'\n"
  "urgency.uda.priority.L.coefficient=1.8         # UDA priority coefficient for value 'L'\n"
  "\n"
  "#default.project=foo                           # Default project for 'add' command\n"
  "#default.due=eom                               # Default due date for 'add' command\n"
  "#default.scheduled=eom                         # Default scheduled date for 'add' command\n"
  "default.command=next                           # When no arguments are specified\n"
  "default.timesheet.filter=( +PENDING and start.after:now-4wks ) or ( +COMPLETED and end.after:now-4wks )\n"
  "\n"
  "_forcecolor=0                                  # Forces color to be on, even for non TTY output\n"
  "complete.all.tags=0                            # Include old tag names in '_ags' command\n"
  "list.all.projects=0                            # Include old project names in 'projects' command\n"
  "summary.all.projects=0                         # Include old project names in 'summary' command\n"
  "list.all.tags=0                                # Include old tag names in 'tags' command\n"
  "print.empty.columns=0                          # Print columns which have no data for any task\n"
  "debug=0                                        # Display diagnostics\n"
  "debug.tls=0                                    # Sync diagnostics\n"
  "sugar=1                                        # Syntactic sugar\n"
  "obfuscate=0                                    # Obfuscate data for error reporting\n"
  "fontunderline=1                                # Uses underlines rather than -------\n"
  "\n"
  "# WARNING: Please read the documentation (man task-sync) before setting up\n"
  "#          Taskwarrior for Taskserver synchronization.\n"
  "#taskd.ca=<certificate file>\n"
  "#taskd.certificate=<certificate file>\n"
  "#taskd.credentials=<organization>/<name>/<password>\n"
  "#taskd.server=<server>:<port>\n"
  "taskd.trust=strict\n"
  "#taskd.trust=ignore hostname\n"
  "#taskd.trust=allow all\n"
  "taskd.ciphers=NORMAL\n"
  "\n"
  "# Aliases - alternate names for commands\n"
  "alias.rm=delete                                # Alias for the delete command\n"
  "alias.history=history.monthly                  # Prefer monthly over annual history reports\n"
  "alias.ghistory=ghistory.monthly                # Prefer monthly graphical over annual history reports\n"
  "alias.burndown=burndown.weekly                 # Prefer the weekly burndown chart\n"
  "\n"
  "# Reports\n"
  "\n"
  "report.long.description=All details of tasks\n"
  "report.long.labels=ID,A,Created,Mod,Deps,P,Project,Tags,Recur,Wait,Sched,Due,Until,Description\n"
  "report.long.columns=id,start.active,entry,modified.age,depends,priority,project,tags,recur,wait.remaining,scheduled,due,until,description\n"
  "report.long.filter=status:pending -WAITING\n"
  "report.long.sort=modified-\n"
  "report.long.context=1\n"
  "\n"
  "report.list.description=Most details of tasks\n"
  "report.list.labels=ID,Active,Age,D,P,Project,Tags,R,Sch,Due,Until,Description,Urg\n"
  "report.list.columns=id,start.age,entry.age,depends.indicator,priority,project,tags,recur.indicator,scheduled.countdown,due,until.remaining,description.count,urgency\n"
  "report.list.filter=status:pending -WAITING\n"
  "report.list.sort=start-,due+,project+,urgency-\n"
  "report.list.context=1\n"
  "\n"
  "report.ls.description=Few details of tasks\n"
  "report.ls.labels=ID,A,D,Project,Tags,R,Wait,S,Due,Until,Description\n"
  "report.ls.columns=id,start.active,depends.indicator,project,tags,recur.indicator,wait.remaining,scheduled.countdown,due.countdown,until.countdown,description.count\n"
  "report.ls.filter=status:pending -WAITING\n"
  "report.ls.sort=start-,description+\n"
  "report.ls.context=1\n"
  "\n"
  "report.minimal.description=Minimal details of tasks\n"
  "report.minimal.labels=ID,Project,Tags,Description\n"
  "report.minimal.columns=id,project,tags.count,description.count\n"
  "report.minimal.filter=status:pending\n"
  "report.minimal.sort=project+/,description+\n"
  "report.minimal.context=1\n"
  "\n"
  "report.newest.description=Newest tasks\n"
  "report.newest.labels=ID,Active,Created,Age,Mod,D,P,Project,Tags,R,Wait,Sch,Due,Until,Description\n"
  "report.newest.columns=id,start.age,entry,entry.age,modified.age,depends.indicator,priority,project,tags,recur.indicator,wait.remaining,scheduled.countdown,due,until.age,description\n"
  "report.newest.filter=status:pending\n"
  "report.newest.sort=entry-\n"
  "report.newest.context=1\n"
  "\n"
  "report.oldest.description=Oldest tasks\n"
  "report.oldest.labels=ID,Active,Created,Age,Mod,D,P,Project,Tags,R,Wait,Sch,Due,Until,Description\n"
  "report.oldest.columns=id,start.age,entry,entry.age,modified.age,depends.indicator,priority,project,tags,recur.indicator,wait.remaining,scheduled.countdown,due,until.age,description\n"
  "report.oldest.filter=status:pending\n"
  "report.oldest.sort=entry+\n"
  "report.oldest.context=1\n"
  "\n"
  "report.overdue.description=Overdue tasks\n"
  "report.overdue.labels=ID,Active,Age,Deps,P,Project,Tag,R,S,Due,Until,Description,Urg\n"
  "report.overdue.columns=id,start.age,entry.age,depends,priority,project,tags,recur.indicator,scheduled.countdown,due,until,description,urgency\n"
  "report.overdue.filter=status:pending and +OVERDUE\n"
  "report.overdue.sort=urgency-,due+\n"
  "report.overdue.context=1\n"
  "\n"
  "report.active.description=Active tasks\n"
  "report.active.labels=ID,Started,Active,Age,D,P,Project,Tags,Recur,W,Sch,Due,Until,Description\n"
  "report.active.columns=id,start,start.age,entry.age,depends.indicator,priority,project,tags,recur,wait,scheduled.remaining,due,until,description\n"
  "report.active.filter=status:pending and +ACTIVE\n"
  "report.active.sort=project+,start+\n"
  "report.active.context=1\n"
  "\n"
  "report.completed.description=Completed tasks\n"
  "report.completed.labels=ID,UUID,Created,Completed,Age,Deps,P,Project,Tags,R,Due,Description\n"
  "report.completed.columns=id,uuid.short,entry,end,entry.age,depends,priority,project,tags,recur.indicator,due,description\n"
  "report.completed.filter=status:completed\n"
  "report.completed.sort=end+\n"
  "report.completed.context=1\n"
  "\n"
  "report.recurring.description=Recurring Tasks\n"
  "report.recurring.labels=ID,Active,Age,D,P,Parent,Project,Tags,Recur,Sch,Due,Until,Description,Urg\n"
  "report.recurring.columns=id,start.age,entry.age,depends.indicator,priority,parent.short,project,tags,recur,scheduled.countdown,due,until.remaining,description,urgency\n"
  "report.recurring.filter=(status:pending and +CHILD) or (status:recurring and +PARENT)\n"
  "report.recurring.sort=due+,urgency-,entry+\n"
  "report.recurring.context=1\n"
  "\n"
  "report.waiting.description=Waiting (hidden) tasks\n"
  "report.waiting.labels=ID,A,Age,D,P,Project,Tags,R,Wait,Remaining,Sched,Due,Until,Description\n"
  "report.waiting.columns=id,start.active,entry.age,depends.indicator,priority,project,tags,recur.indicator,wait,wait.remaining,scheduled,due,until,description\n"
  "report.waiting.filter=+WAITING\n"
  "report.waiting.sort=due+,wait+,entry+\n"
  "report.waiting.context=1\n"
  "\n"
  "report.all.description=All tasks\n"
  "report.all.labels=ID,St,UUID,A,Age,Done,D,P,Project,Tags,R,Wait,Sch,Due,Until,Description\n"
  "report.all.columns=id,status.short,uuid.short,start.active,entry.age,end.age,depends.indicator,priority,project.parent,tags.count,recur.indicator,wait.remaining,scheduled.remaining,due,until.remaining,description\n"
  "report.all.sort=entry-\n"
  "report.all.context=1\n"
  "\n"
  "report.next.description=Most urgent tasks\n"
  "report.next.labels=ID,Active,Age,Deps,P,Project,Tag,Recur,S,Due,Until,Description,Urg\n"
  "report.next.columns=id,start.age,entry.age,depends,priority,project,tags,recur,scheduled.countdown,due.relative,until.remaining,description,urgency\n"
  "report.next.filter=status:pending -WAITING limit:page\n"
  "report.next.sort=urgency-\n"
  "report.next.context=1\n"
  "\n"
  "report.ready.description=Most urgent actionable tasks\n"
  "report.ready.labels=ID,Active,Age,D,P,Project,Tags,R,S,Due,Until,Description,Urg\n"
  "report.ready.columns=id,start.age,entry.age,depends.indicator,priority,project,tags,recur.indicator,scheduled.countdown,due.countdown,until.remaining,description,urgency\n"
  "report.ready.filter=+READY\n"
  "report.ready.sort=start-,urgency-\n"
  "report.ready.context=1\n"
  "\n"
  "report.blocked.description=Blocked tasks\n"
  "report.blocked.columns=id,depends,project,priority,due,start.active,entry.age,description\n"
  "report.blocked.labels=ID,Deps,Proj,Pri,Due,Active,Age,Description\n"
  "report.blocked.sort=due+,priority-,start-,project+\n"
  "report.blocked.filter=status:pending -WAITING +BLOCKED\n"
  "report.blocked.context=1\n"
  "\n"
  "report.unblocked.description=Unblocked tasks\n"
  "report.unblocked.columns=id,depends,project,priority,due,start.active,entry.age,description\n"
  "report.unblocked.labels=ID,Deps,Proj,Pri,Due,Active,Age,Description\n"
  "report.unblocked.sort=due+,priority-,start-,project+\n"
  "report.unblocked.filter=status:pending -WAITING -BLOCKED\n"
  "report.unblocked.context=1\n"
  "\n"
  "report.blocking.description=Blocking tasks\n"
  "report.blocking.labels=ID,UUID,A,Deps,Project,Tags,R,W,Sch,Due,Until,Description,Urg\n"
  "report.blocking.columns=id,uuid.short,start.active,depends,project,tags,recur,wait,scheduled.remaining,due.relative,until.remaining,description.count,urgency\n"
  "report.blocking.sort=urgency-,due+,entry+\n"
  "report.blocking.filter=status:pending -WAITING +BLOCKING\n"
  "report.blocking.context=1\n"
  "\n"
  "report.timesheet.filter=(+PENDING and start.after:now-4wks) or (+COMPLETED and end.after:now-4wks)\n"
  "report.timesheet.context=0\n"
  "\n";

// Supported modifiers, synonyms on the same line.
static const char* modifierNames[] =
{
  "before",     "under",    "below",
  "after",      "over",     "above",
  "by",
  "none",
  "any",
  "is",         "equals",
  "isnt",       "not",
  "has",        "contains",
  "hasnt",
  "startswith", "left",
  "endswith",   "right",
  "word",
  "noword"
};

Context* Context::context;

////////////////////////////////////////////////////////////////////////////////
Context& Context::getContext ()
{
  return *Context::context;
}

////////////////////////////////////////////////////////////////////////////////
void Context::setContext (Context* context)
{
  Context::context = context;
}

////////////////////////////////////////////////////////////////////////////////
Context::~Context ()
{
  for (auto& com : commands)
    delete com.second;

  for (auto& col : columns)
    delete col.second;
}

////////////////////////////////////////////////////////////////////////////////
int Context::initialize (int argc, const char** argv)
{
  timer_total.start ();
  int rc = 0;
  home_dir = getenv ("HOME");

  std::vector <std::string> searchPaths { TASK_RCDIR };

  try
  {
    ////////////////////////////////////////////////////////////////////////////
    //
    // [1] Load the correct config file.
    //     - Default to ~/.taskrc (ctor).
    //     - If no ~/.taskrc, use $XDG_CONFIG_HOME/task/taskrc if exists, or
    //       ~/.config/task/taskrc if $XDG_CONFIG_HOME is unset
    //     - Allow $TASKRC override.
    //     - Allow command line override rc:<file>
    //     - Load resultant file.
    //     - Apply command line overrides to the config.
    //
    ////////////////////////////////////////////////////////////////////////////

    bool taskrc_overridden = false;

    // XDG_CONFIG_HOME doesn't count as an override (no warning header)
    if (! rc_file.exists ())
    {
      // Use XDG_CONFIG_HOME if defined, otherwise default to ~/.config
      std::string xdg_config_home;
      const char* env_xdg_config_home = getenv ("XDG_CONFIG_HOME");

      if (env_xdg_config_home)
        xdg_config_home = format ("{1}", env_xdg_config_home);
      else
        xdg_config_home = format ("{1}/.config", home_dir);

      // Ensure the path does not end with '/'
      if (xdg_config_home.back () == '/')
        xdg_config_home.pop_back();

      // https://github.com/GothenburgBitFactory/libshared/issues/32
      std::string rcfile_path = format ("{1}/task/taskrc", xdg_config_home);

      File maybe_rc_file = File (rcfile_path);
      if ( maybe_rc_file.exists ())
        rc_file = maybe_rc_file;
    }

    char *override = getenv ("TASKRC");
    if (override)
    {
      rc_file = File (override);
      taskrc_overridden = true;
    }

    taskrc_overridden =
        CLI2::getOverride (argc, argv, rc_file) || taskrc_overridden;

    // Artificial scope for timing purposes.
    {
      Timer timer;
      config.parse (configurationDefaults, 1, searchPaths);
      config.load (rc_file._data, 1, searchPaths);
      debugTiming (format ("Config::load ({1})", rc_file._data), timer);
    }

    CLI2::applyOverrides (argc, argv);

    if (taskrc_overridden && verbose ("override"))
      header (format ("TASKRC override: {1}", rc_file._data));

    ////////////////////////////////////////////////////////////////////////////
    //
    // [2] Locate the data directory.
    //     - Default to ~/.task (ctor).
    //     - Allow $TASKDATA override.
    //     - Allow command line override rc.data.location:<dir>
    //     - Inform TDB2 where to find data.
    //     - Create the rc_file and data_dir, if necessary.
    //
    ////////////////////////////////////////////////////////////////////////////

    bool taskdata_overridden = false;

    override = getenv ("TASKDATA");
    if (override)
    {
      data_dir = Directory (override);
      config.set ("data.location", data_dir._data);
      taskdata_overridden = true;
    }

    taskdata_overridden =
        CLI2::getDataLocation (argc, argv, data_dir) || taskdata_overridden;

    if (taskdata_overridden && verbose ("override"))
      header (format ("TASKDATA override: {1}", data_dir._data));

    tdb2.set_location (data_dir);
    createDefaultConfig ();

    ////////////////////////////////////////////////////////////////////////////
    //
    // [3] Instantiate Command objects and capture command entities.
    //
    ////////////////////////////////////////////////////////////////////////////

    Command::factory (commands);
    for (auto& cmd : commands)
      cli2.entity ("cmd", cmd.first);

    ////////////////////////////////////////////////////////////////////////////
    //
    // [4] Instantiate Column objects and capture column entities.
    //
    ////////////////////////////////////////////////////////////////////////////

    Column::factory (columns);
    for (auto& col : columns)
      cli2.entity ("attribute", col.first);

    cli2.entity ("pseudo", "limit");

    ////////////////////////////////////////////////////////////////////////////
    //
    // [5] Capture modifier and operator entities.
    //
    ////////////////////////////////////////////////////////////////////////////

    for (auto& modifierName : modifierNames)
      cli2.entity ("modifier", modifierName);

    for (auto& op : Eval::getOperators ())
      cli2.entity ("operator", op);

    for (auto& op : Eval::getBinaryOperators ())
      cli2.entity ("binary_operator", op);

    ////////////////////////////////////////////////////////////////////////////
    //
    // [6] Complete the Context initialization.
    //
    ////////////////////////////////////////////////////////////////////////////

    initializeColorRules ();
    staticInitialization ();
    propagateDebug ();
    loadAliases ();

    ////////////////////////////////////////////////////////////////////////////
    //
    // [7] Parse the command line.
    //
    ////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < argc; i++)
      cli2.add (argv[i]);

    cli2.analyze ();

    // Extract a recomposed command line.
    auto foundDefault = false;
    auto foundAssumed = false;
    std::string combined;
    for (auto& a : cli2._args)
    {
      if (combined.length ())
        combined += ' ';

      combined += a.attribute ("raw");

      if (a.hasTag ("DEFAULT"))
        foundDefault = true;

      if (a.hasTag ("ASSUMED"))
        foundAssumed = true;
    }

    if (verbose ("default")) {
      if (foundDefault)
        header ("[" + combined + "]");

      if (foundAssumed)
        header ("No command specified - assuming 'information'.");
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    // [8] Initialize hooks.
    //
    ////////////////////////////////////////////////////////////////////////////

    hooks.initialize ();
  }

  catch (const std::string& message)
  {
    error (message);
    rc = 2;
  }

  catch (int)
  {
    // Hooks can terminate processing by throwing integers.
    rc = 4;
  }

  catch (const std::regex_error& e)
  {
    std::cout << "regex_error caught: " << e.what() << '\n';
  }
  catch (...)
  {
    error ("knknown error. Please report.");
    rc = 3;
  }

  // On initialization failure...
  if (rc)
  {
    // Dump all debug messages, controlled by rc.debug.
    if (config.getBoolean ("debug"))
    {
      for (auto& d : debugMessages)
        if (color ())
          std::cerr << colorizeDebug (d) << '\n';
        else
          std::cerr << d << '\n';
    }

    // Dump all headers, controlled by 'header' verbosity token.
    if (verbose ("header"))
    {
      for (auto& h : headers)
        if (color ())
          std::cerr << colorizeHeader (h) << '\n';
        else
          std::cerr << h << '\n';
    }

    // Dump all footnotes, controlled by 'footnote' verbosity token.
    if (verbose ("footnote"))
    {
      for (auto& f : footnotes)
        if (color ())
          std::cerr << colorizeFootnote (f) << '\n';
        else
          std::cerr << f << '\n';
    }

    // Dump all errors, non-maskable.
    // Colorized as footnotes.
    for (auto& e : errors)
      if (color ())
        std::cerr << colorizeFootnote (e) << '\n';
      else
        std::cerr << e << '\n';
  }

  time_init_us += timer_total.total_us ();
  return rc;
}

////////////////////////////////////////////////////////////////////////////////
int Context::run ()
{
  int rc;
  std::string output;

  try
  {
    hooks.onLaunch ();
    rc = dispatch (output);
    tdb2.commit ();           // Harmless if called when nothing changed.
    hooks.onExit ();          // No chance to update data.

    timer_total.stop ();
    time_total_us += timer_total.total_us ();

    std::stringstream s;
    s << "Perf "
      << PACKAGE_STRING
      << ' '
#ifdef HAVE_COMMIT
      << COMMIT
#else
      << '-'
#endif
      << ' '
      << Datetime ().toISO ()

      << " init:"   << time_init_us
      << " load:"   << time_load_us
      << " gc:"     << (time_gc_us > 0 ? time_gc_us - time_load_us : time_gc_us)
      << " filter:" << time_filter_us
      << " commit:" << time_commit_us
      << " sort:"   << time_sort_us
      << " render:" << time_render_us
      << " hooks:"  << time_hooks_us
      << " other:"  << time_total_us  -
                       time_init_us   -
                       time_gc_us     -
                       time_filter_us -
                       time_commit_us -
                       time_sort_us   -
                       time_render_us -
                       time_hooks_us
      << " total:"  << time_total_us
      << '\n';
    debug (s.str ());
  }

  catch (const std::string& message)
  {
    error (message);
    rc = 2;
  }

  catch (int)
  {
    // Hooks can terminate processing by throwing integers.
    rc = 4;
  }

  catch (...)
  {
    error ("Unknown error. Please report.");
    rc = 3;
  }

  // Dump all debug messages, controlled by rc.debug.
  if (config.getBoolean ("debug"))
  {
    for (auto& d : debugMessages)
      if (color ())
        std::cerr << colorizeDebug (d) << '\n';
      else
        std::cerr << d << '\n';
  }

  // Dump all headers, controlled by 'header' verbosity token.
  if (verbose ("header"))
  {
    for (auto& h : headers)
      if (color ())
        std::cerr << colorizeHeader (h) << '\n';
      else
        std::cerr << h << '\n';
  }

  // Dump the report output.
  std::cout << output;

  // Dump all footnotes, controlled by 'footnote' verbosity token.
  if (verbose ("footnote"))
  {
    for (auto& f : footnotes)
      if (color ())
        std::cerr << colorizeFootnote (f) << '\n';
      else
        std::cerr << f << '\n';
  }

  // Dump all errors, non-maskable.
  // Colorized as footnotes.
  for (auto& e : errors)
    if (color ())
      std::cerr << colorizeError (e) << '\n';
    else
      std::cerr << e << '\n';

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
// Dispatch to the command found by the CLI parser.
int Context::dispatch (std::string &out)
{
  // Autocomplete args against keywords.
  std::string command = cli2.getCommand ();
  if (command != "")
  {
    updateXtermTitle ();
    updateVerbosity ();

    Command* c = commands[command];
    assert (c);

    // The command know whether they need a GC.
    if (c->needs_gc () &&
        ! tdb2.read_only ())
    {
      run_gc = config.getBoolean ("gc");
      tdb2.gc ();
    }
    else
    {
      run_gc = false;
    }

/*
    // Only read-only commands can be run when TDB2 is read-only.
    // TODO Implement TDB2::read_only
    if (tdb2.read_only () && !c->read_only ())
      throw std::string ("");
*/

    // This is something that is only needed for write commands with no other
    // filter processing.
    if (c->accepts_modifications () &&
        ! c->accepts_filter ())
    {
      cli2.prepareFilter ();
    }

    // With rc.debug.parser == 2, there are more tree dumps than you might want,
    // but we need the rc.debug.parser == 1 case covered also, with the final
    // tree.
    if (config.getBoolean ("debug") &&
        config.getInteger ("debug.parser") == 1)
      debug (cli2.dump ("Parse Tree (before command-specifіc processing)"));

    return c->execute (out);
  }

  assert (commands["help"]);
  return commands["help"]->execute (out);
}

////////////////////////////////////////////////////////////////////////////////
int Context::getWidth ()
{
  // Determine window size.
  auto width = config.getInteger ("defaultwidth");

  // A zero width value means 'infinity', which is approximated here by 2^16.
  if (width == 0)
    return 65536;

  if (config.getBoolean ("detection"))
  {
    if (terminal_width == 0 &&
        terminal_height == 0)
    {
      unsigned short buff[4];
      if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];
      }
    }

    width = terminal_width;

    // Ncurses does this, and perhaps we need to as well, to avoid a problem on
    // Cygwin where the display goes right up to the terminal width, and causes
    // an odd color wrapping problem.
    if (config.getBoolean ("avoidlastcolumn"))
      --width;
  }

  return width;
}

////////////////////////////////////////////////////////////////////////////////
int Context::getHeight ()
{
  // Determine window size.
  auto height = config.getInteger ("defaultheight");

  // A zero height value means 'infinity', which is approximated here by 2^16.
  if (height == 0)
    return 65536;

  if (config.getBoolean ("detection"))
  {
    if (terminal_width == 0 &&
        terminal_height == 0)
    {
      unsigned short buff[4];
      if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &buff) != -1)
      {
        terminal_height = buff[0];
        terminal_width = buff[1];
      }
    }

    height = terminal_height;
  }

  return height;
}

////////////////////////////////////////////////////////////////////////////////
std::string Context::getTaskContext (const std::string& kind, std::string name, bool fallback /* = true */)
{
  // Consider currently selected context, if none specified
  if (name.empty ())
    name = config.get ("context");

  // Detect if any context is set, and bail out if not
  if (! name.empty ())
    debug (format ("Applying context '{1}'", name));
  else
  {
    debug ("No context set");
    return "";
  }

  // Figure out the context string for this kind (read/write)
  std::string contextString = "";

  if (! config.has ("context." + name + "." + kind) && kind == "read")
  {
    debug ("Specific " + kind + " context for '" + name + "' not defined. ");
    if (fallback)
    {
      debug ("Trying to interpret old-style context definition as read context.");
      contextString = config.get ("context." + name);
    }
  }
  else
    contextString = config.get ("context." + name + "." + kind);

  debug (format ("Detected context string: {1}", contextString.empty() ? "(empty)" : contextString));
  return contextString;
}

////////////////////////////////////////////////////////////////////////////////
bool Context::color ()
{
  if (determine_color_use)
  {
    // What the config says.
    use_color = config.getBoolean ("color");

    // Only tty's support color.
    if (! isatty (STDOUT_FILENO))
    {
      // No ioctl.
      config.set ("detection", "off");
      config.set ("color",     "off");

      // Files don't get color.
      use_color = false;
    }

    // Override.
    if (config.getBoolean ("_forcecolor"))
    {
      config.set ("color", "on");
      use_color = true;
    }

    // No need to go through this again.
    determine_color_use = false;
  }

  // Cached result.
  return use_color;
}

////////////////////////////////////////////////////////////////////////////////
// Support verbosity levels:
//
//   rc.verbose=1          Show all feedback.
//   rc.verbose=0          Show regular feedback.
//   rc.verbose=nothing    Show the absolute minimum.
//   rc.verbose=one,two    Show verbosity for 'one' and 'two' only.
//
// TODO This mechanism is clunky, and should slowly evolve into something more
//      logical and consistent.  This should probably mean that 'nothing' should
//      take the place of '0'.
bool Context::verbose (const std::string& token)
{
  if (verbosity.empty ())
  {
    verbosity_legacy = config.getBoolean ("verbose");
    for (auto& token : split (config.get ("verbose"), ','))
      verbosity.insert (token);

    // Regular feedback means almost everything.
    // This odd test is to see if a Boolean-false value is a real one, which
    // means it is not 1/true/T/yes/on, but also should not be one of the
    // valid tokens either.
    if (! verbosity_legacy && ! verbosity.empty ())
    {
      std::string v = *(verbosity.begin ());
      if (v != "nothing"  &&
          v != "affected" &&  // This list must be complete.
          v != "blank"    &&  //
          v != "context"  &&  //
          v != "default"  &&  //
          v != "edit"     &&  //
          v != "filter"   &&  //
          v != "footnote" &&  //
          v != "header"   &&  //
          v != "label"    &&  //
          v != "new-id"   &&  //
          v != "new-uuid" &&  //
          v != "override" &&  //
          v != "project"  &&  //
          v != "recur"    &&  //
          v != "special"  &&  //
          v != "sync")
      {
        // This list emulates rc.verbose=off in version 1.9.4.
        verbosity = {"blank", "label", "new-id", "edit"};
      }
    }

    // Some flags imply "footnote" verbosity being active.  Make it so.
    if (! verbosity.count ("footnote"))
    {
      // TODO: Some of these may not use footnotes yet.  They should.
      for (auto flag : {"affected", "new-id", "new-uuid", "project", "override", "recur"})
      {
        if (verbosity.count (flag))
        {
          verbosity.insert ("footnote");
          break;
        }
      }
    }

    // Some flags imply "header" verbosity being active.  Make it so.
    if (! verbosity.count ("header"))
    {
      for (auto flag : {"default"})
      {
        if (verbosity.count (flag))
        {
          verbosity.insert ("header");
          break;
        }
      }
    }
  }

  // rc.verbose=true|y|yes|1|on overrides all.
  if (verbosity_legacy)
    return true;

  // rc.verbose=nothing overrides all.
  if (verbosity.size () == 1 &&
      *(verbosity.begin ()) == "nothing")
    return false;

  // Specific token match.
  if (verbosity.count (token))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> Context::getColumns () const
{
  std::vector <std::string> output;
  for (auto& col : columns)
    output.push_back (col.first);

  return output;
}

////////////////////////////////////////////////////////////////////////////////
// A value of zero mean unlimited.
// A value of 'page' means however many screen lines there are.
// A value of a positive integer is a row/task limit.
void Context::getLimits (int& rows, int& lines)
{
  rows = 0;
  lines = 0;

  // This is an integer specified as a filter (limit:10).
  auto limit = config.get ("limit");
  if (limit != "")
  {
    if (limit == "page")
    {
      rows = 0;
      lines = getHeight ();
    }
    else
    {
      rows = (int) strtol (limit.c_str (), nullptr, 10);
      lines = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// The 'Task' object, among others, is shared between projects.  To make this
// easier, it has been decoupled from Context.
void Context::staticInitialization ()
{
  CLI2::minimumMatchLength           = config.getInteger ("abbreviation.minimum");
  Lexer::minimumMatchLength          = config.getInteger ("abbreviation.minimum");

  Task::defaultProject               = config.get ("default.project");
  Task::defaultDue                   = config.get ("default.due");
  Task::defaultScheduled             = config.get ("default.scheduled");

  Task::searchCaseSensitive          = Variant::searchCaseSensitive = config.getBoolean ("search.case.sensitive");
  Task::regex                        = Variant::searchUsingRegex    = config.getBoolean ("regex");
  Lexer::dateFormat                  = Variant::dateFormat          = config.get ("dateformat");

  Datetime::isoEnabled               = config.getBoolean ("date.iso");
  Datetime::standaloneDateEnabled    = false;
  Datetime::standaloneTimeEnabled    = false;
  Duration::standaloneSecondsEnabled = false;

  TDB2::debug_mode                   = config.getBoolean ("debug");

  for (auto& rc : config)
  {
    if (rc.first.substr (0, 4) == "uda." &&
        rc.first.substr (rc.first.length () - 7, 7) == ".values")
    {
      std::string name = rc.first.substr (4, rc.first.length () - 7 - 4);
      auto values = split (rc.second, ',');

      for (auto r = values.rbegin(); r != values.rend (); ++r)
        Task::customOrder[name].push_back (*r);
    }
  }

  for (auto& col : columns)
  {
    Task::attributes[col.first] = col.second->type ();
    Lexer::attributes[col.first] = col.second->type ();
  }

  Task::urgencyProjectCoefficient     = config.getReal ("urgency.project.coefficient");
  Task::urgencyActiveCoefficient      = config.getReal ("urgency.active.coefficient");
  Task::urgencyScheduledCoefficient   = config.getReal ("urgency.scheduled.coefficient");
  Task::urgencyWaitingCoefficient     = config.getReal ("urgency.waiting.coefficient");
  Task::urgencyBlockedCoefficient     = config.getReal ("urgency.blocked.coefficient");
  Task::urgencyAnnotationsCoefficient = config.getReal ("urgency.annotations.coefficient");
  Task::urgencyTagsCoefficient        = config.getReal ("urgency.tags.coefficient");
  Task::urgencyDueCoefficient         = config.getReal ("urgency.due.coefficient");
  Task::urgencyBlockingCoefficient    = config.getReal ("urgency.blocking.coefficient");
  Task::urgencyAgeCoefficient         = config.getReal ("urgency.age.coefficient");
  Task::urgencyAgeMax                 = config.getReal ("urgency.age.max");

  // Tag- and project-specific coefficients.
  for (auto& var : config.all ())
    if (var.substr (0, 13) == "urgency.user." ||
        var.substr (0, 12) == "urgency.uda.")
      Task::coefficients[var] = config.getReal (var);
}

////////////////////////////////////////////////////////////////////////////////
void Context::createDefaultConfig ()
{
  // Do we need to create a default rc?
  if (rc_file._data != "" && ! rc_file.exists ())
  {
    if (config.getBoolean ("confirmation") &&
        ! confirm ( format ("A configuration file could not be found in {1}\n\nWould you like a sample {2} created, so Taskwarrior can proceed?", home_dir, rc_file._data)))
      throw std::string ("Cannot proceed without rc file.");

    // Override data.location in the defaults.
    auto loc = configurationDefaults.find ("data.location=~/.task");
    //                                 loc+0^          +14^   +21^

    Datetime now;
    std::stringstream contents;
    contents << "# [Created by "
             << PACKAGE_STRING
             << ' '
             << now.toString ("m/d/Y H:N:S")
             << "]\n"
             << configurationDefaults.substr (0, loc + 14)
             << data_dir._original
             << "\n\n# To use the default location of the XDG directories,\n"
             << "# move this configuration file from ~/.taskrc to ~/.config/task/taskrc and uncomment below\n"
             << "\n#data.location=~/.local/share/task\n"
             << "#hooks.location=~/.config/task/hooks\n"
             << "\n# Color theme (uncomment one to use)\n"
             << "#include light-16.theme\n"
             << "#include light-256.theme\n"
             << "#include dark-16.theme\n"
             << "#include dark-256.theme\n"
             << "#include dark-red-256.theme\n"
             << "#include dark-green-256.theme\n"
             << "#include dark-blue-256.theme\n"
             << "#include dark-violets-256.theme\n"
             << "#include dark-yellow-green.theme\n"
             << "#include dark-gray-256.theme\n"
             << "#include dark-gray-blue-256.theme\n"
             << "#include solarized-dark-256.theme\n"
             << "#include solarized-light-256.theme\n"
             << "#include no-color.theme\n"
             << '\n';

    // Write out the new file.
    if (! File::write (rc_file._data, contents.str ()))
      throw format ("Could not write to '{1}'.", rc_file._data);
  }

  // Create data location, if necessary.
  Directory d (data_dir);
  if (! d.exists ())
  {
    if (config.getBoolean ("exit.on.missing.db"))
      throw std::string ("Error: rc.data.location does not exist - exiting according to rc.exit.on.missing.db setting.");

    d.create ();

    if (config.has ("hooks.location"))
      d = Directory (config.get ("hooks.location"));
    else
      d += "hooks";

    d.create ();
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::decomposeSortField (
  const std::string& field,
  std::string& key,
  bool& ascending,
  bool& breakIndicator)
{
  int length = field.length ();

  int decoration = 1;
  breakIndicator = false;
  if (field[length - decoration] == '/')
  {
    breakIndicator = true;
    ++decoration;
  }

  if (field[length - decoration] == '+')
  {
    ascending = true;
    key = field.substr (0, length - decoration);
  }
  else if (field[length - decoration] == '-')
  {
    ascending = false;
    key = field.substr (0, length - decoration);
  }
  else
  {
    ascending = true;
    key = field;
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::debugTiming (const std::string& details, const Timer& timer)
{
  std::stringstream out;
  out << "Timer "
      << details
      << ' '
      << std::setprecision (6)
      << std::fixed
      << timer.total_us () / 1.0e6
      << " sec";
  debug (out.str ());
}

////////////////////////////////////////////////////////////////////////////////
CurrentTask Context::withCurrentTask (const Task *task)
{
  return CurrentTask(*this, task);
}

////////////////////////////////////////////////////////////////////////////////
// This capability is to answer the question of 'what did I just do to generate
// this output?'.
void Context::updateXtermTitle ()
{
  if (config.getBoolean ("xterm.title") && isatty (STDOUT_FILENO))
  {
    auto command = cli2.getCommand ();
    std::string title;

    for (auto a = cli2._args.begin (); a != cli2._args.end (); ++a)
    {
      if (a != cli2._args.begin ())
        title += ' ';

      title += a->attribute ("raw");
    }

    std::cout << "]0;task " << command << ' ' << title << "";
  }
}

////////////////////////////////////////////////////////////////////////////////
// This function allows a clean output if the command is a helper subcommand.
void Context::updateVerbosity ()
{
  auto command = cli2.getCommand ();
  if (command != "" &&
      command[0] == '_')
  {
    verbosity = {"nothing"};
  }
}

////////////////////////////////////////////////////////////////////////////////
void Context::loadAliases ()
{
  for (auto& i : config)
    if (i.first.substr (0, 6) == "alias.")
      cli2.alias (i.first.substr (6), i.second);
}

////////////////////////////////////////////////////////////////////////////////
// Using the general rc.debug setting automaticalls sets debug.tls, debug.hooks
// and debug.parser, unless they already have values, which by default they do
// not.
void Context::propagateDebug ()
{
  if (config.getBoolean ("debug"))
  {
    if (! config.has ("debug.tls"))
      config.set ("debug.tls", 2);

    if (! config.has ("debug.hooks"))
      config.set ("debug.hooks", 1);

    if (! config.has ("debug.parser"))
      config.set ("debug.parser", 1);
  }
  else
  {
    if ((config.has ("debug.hooks")  && config.getInteger ("debug.hooks")) ||
        (config.has ("debug.parser") && config.getInteger ("debug.parser")) )
      config.set ("debug", true);
  }
}

////////////////////////////////////////////////////////////////////////////////
// No duplicates.
void Context::header (const std::string& input)
{
  if (input.length () &&
      std::find (headers.begin (), headers.end (), input) == headers.end ())
    headers.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
// No duplicates.
void Context::footnote (const std::string& input)
{
  if (input.length () &&
      std::find (footnotes.begin (), footnotes.end (), input) == footnotes.end ())
    footnotes.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
// No duplicates.
void Context::error (const std::string& input)
{
  if (input.length () &&
      std::find (errors.begin (), errors.end (), input) == errors.end ())
    errors.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
void Context::debug (const std::string& input)
{
  if (input.length ())
    debugMessages.push_back (input);
}

////////////////////////////////////////////////////////////////////////////////
CurrentTask::CurrentTask (Context &context, const Task *task)
  : context {context}, previous {context.currentTask}
{
  context.currentTask = task;
}

////////////////////////////////////////////////////////////////////////////////
CurrentTask::~CurrentTask ()
{
  context.currentTask = previous;
}

////////////////////////////////////////////////////////////////////////////////

// vim ts=2:sw=2
