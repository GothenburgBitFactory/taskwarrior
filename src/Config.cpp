////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <Directory.h>
#include <Date.h>
#include <File.h>
#include <Timer.h>
#include <JSON.h>
#include <Config.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
// This string is used in two ways:
// 1) It is used to create a new .taskrc file, by copying it directly to disk.
// 2) It is parsed and used as default values for all Config.get calls.
std::string Config::_defaults =
  "# Taskwarrior program configuration file.\n"
  "# For more documentation, see http://taskwarrior.org or try 'man task', 'man task-color',\n"
  "# 'man task-sync' or 'man taskrc'\n"
  "\n"
  "# Here is an example of entries that use the default, override and blank values\n"
  "#   variable=foo   -- By specifying a value, this overrides the default\n"
  "#   variable=      -- By specifying no value, this means no default\n"
  "#   #variable=foo  -- By commenting out the line, or deleting it, this uses the default\n"
  "\n"
  "# Use the command 'task show' to see all defaults and overrides\n"
  "\n"
  "# Files\n"
  "data.location=~/.task\n"
  "locking=on                                     # Use file-level locking\n"
  "gc=on                                          # Garbage-collect data files - DO NOT CHANGE unless you are sure\n"
  "exit.on.missing.db=no                          # Whether to exit if ~/.task is not found\n"
  "hooks=on                                       # Master control switch for hooks\n"
  "\n"
  "# Terminal\n"
  "detection=on                                   # Detects terminal width\n"
  "defaultwidth=80                                # Without detection, assumed width\n"
  "defaultheight=24                               # Without detection, assumed height\n"
  "avoidlastcolumn=no                             # Fixes Cygwin width problem\n"
  "hyphenate=on                                   # Hyphenates lines wrapped on non-word-breaks\n"
  "#editor=vi                                     # Preferred text editor\n"
  "reserved.lines=1                               # Assume a 1-line prompt\n"
  "\n"
  "# Miscellaneous\n"
  "verbose=yes                                    # Provide maximal feedback\n"
  "#verbose=no                                    # Provide regular feedback\n"
  "#verbose=nothing                               # Provide no feedback\n"
  "#                                              # Comma-separated list.  May contain any subset of:\n"
  "#verbose=blank,header,footnote,label,new-id,new-uuid,affected,edit,special,project,sync,filter\n"
  "confirmation=yes                               # Confirmation on delete, big changes\n"
  "recurrence.confirmation=prompt                 # Confirmation for propagating changes among recurring tasks (yes/no/prompt)\n"
  "allow.empty.filter=yes                         # An empty filter gets a warning and requires confirmation\n"
  "indent.annotation=2                            # Indent spaces for annotations\n"
  "indent.report=0                                # Indent spaces for whole report\n"
  "row.padding=0                                  # Left and right padding for each row of report\n"
  "column.padding=1                               # Spaces between each column in a report\n"
  "bulk=3                                         # 3 or more tasks considered a bulk change and is confirmed\n"
  "nag=You have more urgent tasks.                # Nag message to keep you honest\n"                      // TODO
#ifdef CYGWIN
  "search.case.sensitive=no                       # Case-insensitive regex searches are broken under Cygwin\n"
#else
  "search.case.sensitive=yes                      # Setting to no allows case insensitive searches\n"
#endif
  "active.indicator=*                             # What to show as an active task indicator\n"
  "tag.indicator=+                                # What to show as a tag indicator\n"
  "dependency.indicator=D                         # What to show as a dependency indicator\n"
  "recurrence.indicator=R                         # What to show as a task recurrence indicator\n"
  "recurrence.limit=1                             # Number of future recurring pending tasks\n"
  "undo.style=side                                # Undo style - can be 'side', or 'diff'\n"
  "burndown.bias=0.666                            # Weighted mean bias toward recent data\n"
  "regex=yes                                      # Assume all search/filter strings are regexes\n"
  "xterm.title=no                                 # Sets xterm title for some commands\n"
  "expressions=infix                              # Prefer infix over postfix expressions\n"
  "dom=on                                         # Support DOM access\n"
  "json.array=off                                 # Enclose JSON output in [ ]\n"
  "abbreviation.minimum=2                         # Shortest allowed abbreviation\n"
  "\n"
  "# Dates\n"
  "dateformat=Y-M-D                               # Preferred input and display date format\n"
  "dateformat.holiday=YMD                         # Preferred input date format for holidays\n"
  "dateformat.edit=Y-M-D H:N:S                    # Preferred display date format when editing\n"
  "dateformat.info=Y-M-D H:N:S                    # Preferred display date format for information\n"
  "dateformat.report=                             # Preferred display date format for reports\n"
  "dateformat.annotation=                         # Preferred display date format for annotations\n"
  "weekstart="
             STRING_DATE_SUNDAY_LONG
                  "                               # Sunday or Monday only\n"
  "displayweeknumber=yes                          # Show week numbers on calendar\n"
  "due=7                                          # Task is considered due in 7 days\n"
  "\n"
  "# Calendar controls\n"
  "calendar.legend=yes                            # Display the legend on calendar\n"
  "calendar.details=sparse                        # Calendar shows information for tasks w/due dates: full, sparse or none\n"
  "calendar.details.report=list                   # Report to use when showing task information in cal\n"
  "calendar.offset=no                             # Apply an offset value to control the first month of the calendar\n"
  "calendar.offset.value=-1                       # The number of months the first month of the calendar is moved\n"
  "calendar.holidays=none                         # Show public holidays on calendar:full, sparse or none\n"
  "#monthsperline=3                               # Number of calendar months on a line\n"
  "\n"
  "# Journal controls\n"
  "journal.time=no                                # Record start/stop commands as annotation\n"
  "journal.time.start.annotation=Started task     # Annotation description for the start journal entry\n"
  "journal.time.stop.annotation=Stopped task      # Annotation description for the stop  journal entry\n"
  "journal.info=on                                # Display task journal with info command\n"
  "\n"
  "# Dependency controls\n"
  "dependency.reminder=on                         # Nags on dependency chain violations\n"
  "dependency.confirmation=on                     # Should dependency chain repair be confirmed?\n"
  "\n"
  "# Urgency Coefficients\n"
  "urgency.next.coefficient=15.0                  # Urgency coefficient for 'next' special tag\n"
  "urgency.due.coefficient=12.0                   # Urgency coefficient for due dates\n"
  "urgency.blocking.coefficient=8.0               # Urgency coefficient for blocking tasks\n"
  "urgency.priority.coefficient=6.0               # Urgency coefficient for priorities\n"
  "urgency.active.coefficient=4.0                 # Urgency coefficient for active tasks\n"
  "urgency.scheduled.coefficient=5.0              # Urgency coefficient for scheduled tasks\n"
  "urgency.age.coefficient=2.0                    # Urgency coefficient for age\n"
  "urgency.annotations.coefficient=1.0            # Urgency coefficient for annotations\n"
  "urgency.tags.coefficient=1.0                   # Urgency coefficient for tags\n"
  "urgency.project.coefficient=1.0                # Urgency coefficient for projects\n"
  "urgency.blocked.coefficient=-5.0               # Urgency coefficient for blocked tasks\n"
  "urgency.inherit.coefficient=0.0                # Urgency coefficient for blocked tasks inheriting from blocking tasks\n"
  "urgency.waiting.coefficient=-3.0               # Urgency coefficient for waiting status\n"
  "urgency.age.max=365                            # Maximum age in days\n"
  "\n"
  "#urgency.user.project.foo.coefficient=5.0      # Urgency coefficients for 'foo' project\n"
  "#urgency.user.tag.foo.coefficient=5.0          # Urgency coefficients for 'foo' tag\n"
  "#urgency.uda.foo.coefficient=5.0               # Urgency coefficients for UDA 'foo'\n"
  "\n"
  "# Color controls.\n"
  "color=on                                       # Enable color\n"
#ifdef LINUX
  "\n"
  "# basics\n"
  "color.active=bold yellow\n"
  "color.completed=rgb010 on white\n"
  "color.deleted=rgb100 on white\n"
  "color.due.today=on rgb020\n"
  "color.due=on rgb010\n"
  "color.overdue=on rgb100\n"
  "color.blocking=rgb522\n"
  "color.blocked=rgb235\n"
  "color.recurring=\n"
  "color.scheduled=on rgb001\n"
  "color.tagged=\n"
  "color.pri.H=bold white\n"
  "color.pri.M=gray23\n"
  "color.pri.L=gray12\n"
  "color.pri.none=gray23\n"
  "\n"
  "# missing attributes\n"
  "color.project.none=\n"
  "color.tag.none=\n"
  "color.pri.none=\n"
  "\n"
  "# personal color rules (replace 'X' with your value)\n"
  "#color.keyword.X=\n"
  "#color.project.X=\n"
  "color.tag.next=rgb440\n"
  "#color.uda.X=\n"
  "\n"
  "# interface\n"
  "color.alternate=on gray1\n"
  "color.header=color0 on gray11\n"
  "color.label=inverse\n"
  "color.footnote=on gray5\n"
  "\n"
  "# burndown report\n"
  "color.burndown.pending=rgb000 on rgb010\n"
  "color.burndown.started=rgb000 on bright yellow\n"
  "color.burndown.done=rgb000 on rgb020\n"
  "\n"
  "# calendar\n"
  "color.calendar.holiday=bold yellow\n"
  "color.calendar.today=bold white\n"
  "color.calendar.due.today=on rgb020\n"
  "color.calendar.due=on rgb010\n"
  "color.calendar.overdue=on rgb100\n"
  "color.calendar.weekend=on gray5\n"
  "color.calendar.weeknumber=gray8\n"
  "\n"
  "# history report\n"
  "color.history.add=gray14 on rgb010\n"
  "color.history.done=black on rgb020\n"
  "color.history.delete=gray14 on rgb100\n"
  "\n"
  "# summary report (background color only)\n"
  "color.summary.bar=on rgb010\n"
  "color.summary.background=on black\n"
  "\n"
  "# status\n"
  "color.error=red on white\n"
  "color.debug=black on white\n"
  "color.undo.before=yellow\n"
  "color.undo.after=green\n"
  "color.sync.added=green\n"
  "color.sync.changed=yellow\n"
  "color.sync.rejected=red\n"
  "color.warning=bold red\n"
  "\n"
#else
  "\n"
  "# basics\n"
  "color.active=bold yellow\n"
  "color.completed=green on white\n"
  "color.deleted=red on white\n"
  "color.due.today=on yellow\n"
  "color.due=on blue\n"
  "color.overdue=on red\n"
  "color.blocking=bold green\n"
  "color.blocked=bold cyan\n"
  "color.recurring=\n"
  "color.scheduled=on green\n"
  "color.tagged=\n"
  "color.pri.H=bold white\n"
  "color.pri.M=\n"
  "color.pri.L=\n"
  "\n"
  "# missing attributes\n"
  "color.project.none=\n"
  "color.tag.none=\n"
  "color.pri.none=\n"
  "\n"
  "# personal color rules (remove '#' and replace 'X' with your value)\n"
  "#color.keyword.X=\n"
  "#color.project.X=\n"
  "color.tag.next=bold yellow\n"
  "#color.uda.X=\n"
  "\n"
  "# interface\n"
  "color.alternate=\n"
  "color.header=\n"
  "color.label=inverse\n"
  "color.footnote=\n"
  "\n"
  "# burndown report\n"
  "color.burndown.pending=on green\n"
  "color.burndown.started=on bright yellow\n"
  "color.burndown.done=on bright green\n"
  "\n"
  "# calendar\n"
  "color.calendar.holiday=bold yellow\n"
  "color.calendar.today=bold white\n"
  "color.calendar.due.today=on yellow\n"
  "color.calendar.due=on blue\n"
  "color.calendar.overdue=on red\n"
  "color.calendar.weekend=on bright black\n"
  "color.calendar.weeknumber=bold black\n"
  "\n"
  "# history report\n"
  "color.history.add=red on green\n"
  "color.history.done=black on bright green\n"
  "color.history.delete=white on red\n"
  "\n"
  "# summary report (background color only)\n"
  "color.summary.bar=on green\n"
  "color.summary.background=on black\n"
  "\n"
  "# status\n"
  "color.error=red on white\n"
  "color.debug=black on white\n"
  "color.undo.before=yellow\n"
  "color.undo.after=green\n"
  "color.sync.added=green\n"
  "color.sync.changed=yellow\n"
  "color.sync.rejected=red\n"
  "color.warning=bold red\n"
  "\n"
#endif
  "# Here is the rule precedence order, highest to lowest.\n"
  "# Note that these are just the color rule names, without the leading 'color.'\n"
  "#      and any trailing '.value'.\n"
  "rule.precedence.color=deleted,completed,active,keyword.,tag.,uda.,project.,overdue,scheduled,due.today,due,blocked,blocking,recurring,tagged,pri.\n"
  "\n"
  "#default.project=foo                           # Default project for 'add' command\n"
  "#default.priority=M                            # Default priority for 'add' command\n"
  "#default.due=eom                               # Default due date for 'add' command\n"
  "default.command=next                           # When no arguments are specified\n"
  "\n"
  "_forcecolor=no                                 # Forces color to be on, even for non TTY output\n"
  "complete.all.tags=no                           # Include old tag names in '_ags' command\n"
  "list.all.projects=no                           # Include old project names in 'projects' command\n"
  "list.all.tags=no                               # Include old tag names in 'tags' command\n"
  "print.empty.columns=no                         # Print columns which have no data for any task\n"
  "debug=no                                       # Display diagnostics\n"
  "fontunderline=yes                              # Uses underlines rather than -------\n"
  "shell.prompt=task>                             # Prompt used by the shell command\n"
  "\n"
  "# WARNING: Please read the documentation (man task-sync) before setting up\n"
  "#          Taskwarrior for Taskserver synchronization.\n"
  "#taskd.ca <certificate file>\n"
  "#taskd.certificate <certificate file>\n"
  "#taskd.credentials <organization>/<name>/<password>\n"
  "#taskd.server      <server>:<port>\n"
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
  "alias.shell=exec tasksh                        # Alias old shell command to new shell\n"
  "\n"
  "# Reports\n"
  "\n"
  "report.long.description=All details of tasks\n"
  "report.long.labels=ID,A,Created,Mod,Deps,P,Project,Tags,Recur,Wait,Sched,Due,Until,Description\n"
  "report.long.columns=id,start.active,entry,modified.age,depends,priority,project,tags,recur,wait.age,scheduled,due,until,description\n"
  "report.long.filter=status:pending\n"
  "report.long.sort=modified-\n"
  "\n"
  "report.list.description=Most details of tasks\n"
  "report.list.labels=ID,Active,Age,D,P,Project,Tags,R,Sch,Due,Until,Description,Urg\n"
  "report.list.columns=id,start.age,entry.age,depends.indicator,priority,project,tags,recur.indicator,scheduled.countdown,due,until.age,description.count,urgency\n"
  "report.list.filter=status:pending\n"
  "report.list.sort=start-,due+,project+/,urgency-\n"
  "\n"
  "report.ls.description=Few details of tasks\n"
  "report.ls.labels=ID,A,D,Project,Tags,R,Wait,S,Due,Until,Description\n"
  "report.ls.columns=id,start.active,depends.indicator,project,tags,recur.indicator,wait.age,scheduled.countdown,due.countdown,until.countdown,description.count\n"
  "report.ls.filter=status:pending\n"
  "report.ls.sort=start-,description+\n"
  "\n"
  "report.minimal.description=Minimal details of tasks\n"
  "report.minimal.labels=ID,Project,Tags,Description\n"
  "report.minimal.columns=id,project,tags.count,description.count\n"
  "report.minimal.filter=(status:pending or status:waiting)\n"
  "report.minimal.sort=project+/,description+\n"
  "\n"
  "report.newest.description=Newest tasks\n"
  "report.newest.labels=ID,Active,Created,Age,Mod,D,P,Project,Tags,R,Wait,Sch,Due,Until,Description\n"
  "report.newest.columns=id,start.age,entry,entry.age,modified.age,depends.indicator,priority,project,tags,recur.indicator,wait.age,scheduled.countdown,due,until.age,description\n"
  "report.newest.filter=(status:pending or status:waiting) and recur.none:\n"
  "report.newest.sort=entry-\n"
  "\n"
  "report.oldest.description=Oldest tasks\n"
  "report.oldest.labels=ID,Active,Created,Age,Mod,D,P,Project,Tags,R,Wait,Sch,Due,Until,Description\n"
  "report.oldest.columns=id,start.age,entry,entry.age,modified.age,depends.indicator,priority,project,tags,recur.indicator,wait.age,scheduled.countdown,due,until.age,description\n"
  "report.oldest.filter=(status:pending or status:waiting)\n"
  "report.oldest.sort=entry+\n"
  "\n"
  "report.overdue.description=Overdue tasks\n"
  "report.overdue.labels=ID,Active,Age,Deps,P,Project,Tag,R,S,Due,Until,Description,Urg\n"
  "report.overdue.columns=id,start.age,entry.age,depends,priority,project,tags,recur.indicator,scheduled.countdown,due,until,description,urgency\n"
  "report.overdue.filter=(status:pending or status:waiting) and +OVERDUE\n"
  "report.overdue.sort=urgency-,due+\n"
  "\n"
  "report.active.description=Active tasks\n"
  "report.active.labels=ID,Started,Active,Age,D,P,Project,Tags,Recur,W,Sch,Due,Until,Description\n"
  "report.active.columns=id,start,start.age,entry.age,depends.indicator,priority,project,tags,recur,wait.indicator,scheduled.age,due,until,description\n"
  "report.active.filter=status:pending and +ACTIVE\n"
  "report.active.sort=project+,start+\n"
  "\n"
  "report.completed.description=Completed tasks\n"
  "report.completed.labels=ID,UUID,Created,Completed,took,Deps,P,Project,Tags,R,Due,Description\n"
  "report.completed.columns=id,uuid.short,entry,end,entry.age,depends,priority,project,tags,recur.indicator,due,description\n"
  "report.completed.filter=status:completed\n"
  "report.completed.sort=end+\n"
  "\n"
  "report.recurring.description=Recurring Tasks\n"
  "report.recurring.labels=ID,Active,Age,D,P,Project,Tags,Recur,Sch,Due,Until,Description,Urg\n"
  "report.recurring.columns=id,start.age,entry.age,depends.indicator,priority,project,tags,recur,scheduled.countdown,due,until.age,description,urgency\n"
  "report.recurring.filter=(status:pending or status:waiting) and (+PARENT or +CHILD)\n"
  "report.recurring.sort=due+,urgency-,entry+\n"
  "\n"
  "report.waiting.description=Waiting (hidden) tasks\n"
  "report.waiting.labels=ID,A,Age,D,P,Project,Tags,R,Wait,for,Sched,Due,Until,Description\n"
  "report.waiting.columns=id,start.active,entry.age,depends.indicator,priority,project,tags,recur.indicator,wait,wait.age,scheduled,due,until,description\n"
  "report.waiting.filter=+WAITING\n"
  "report.waiting.sort=due+,wait+,entry+\n"
  "\n"
  "report.all.description=All tasks\n"
  "report.all.labels=ID,St,UUID,A,Age,Done,D,P,Project,Tags,R,Wait,Sch,Due,Until,Description\n"
  "report.all.columns=id,status.short,uuid.short,start.active,entry.age,end.age,depends.indicator,priority,project.parent,tags.count,recur.indicator,wait.age,scheduled.age,due,until.age,description\n"
  "report.all.sort=entry-\n"
  "\n"
  "report.next.description=Most urgent tasks\n"
  "report.next.labels=ID,Active,Age,Deps,P,Project,Tag,Recur,S,Due,Until,Description,Urg\n"
  "report.next.columns=id,start.age,entry.age,depends,priority,project,tags,recur,scheduled.countdown,due.age,until.age,description,urgency\n"
  "report.next.filter=status:pending limit:page\n"
  "report.next.sort=start-,urgency-\n"
  "\n"
  "report.ready.description=Most urgent actionable tasks\n"
  "report.ready.labels=ID,Active,Age,D,P,Project,Tags,R,S,Due,Until,Description,Urg\n"
  "report.ready.columns=id,start.age,entry.age,depends.indicator,priority,project,tags,recur.indicator,scheduled.countdown,due.countdown,until.age,description,urgency\n"
  "report.ready.filter=+READY\n"
  "report.ready.sort=start-,urgency-\n"
  "\n"
  "report.blocked.description=Blocked tasks\n"
  "report.blocked.columns=id,depends,project,priority,due,start.active,entry.age,description\n"
  "report.blocked.labels=ID,Deps,Proj,Pri,Due,Active,Age,Description\n"
  "report.blocked.sort=due+,priority-,start-,project+\n"
  "report.blocked.filter=status:pending +BLOCKED\n"
  "\n"
  "report.unblocked.description=Unblocked tasks\n"
  "report.unblocked.columns=id,depends,project,priority,due,start.active,entry.age,description\n"
  "report.unblocked.labels=ID,Deps,Proj,Pri,Due,Active,Age,Description\n"
  "report.unblocked.sort=due+,priority-,start-,project+\n"
  "report.unblocked.filter=status:pending -BLOCKED\n"
  "\n"
  "report.blocking.description=Blocking tasks\n"
  "report.blocking.labels=ID,UUID,A,Deps,Project,Tags,R,W,Sch,Due,Until,Description,Urg\n"
  "report.blocking.columns=id,uuid.short,start.active,depends,project,tags,recur,wait.indicator,scheduled.age,due.age,until.age,description.count,urgency\n"
  "report.blocking.sort=urgency-,due+,entry+\n"
  "report.blocking.filter= status:pending +BLOCKING\n"
  "\n";

////////////////////////////////////////////////////////////////////////////////
// DO NOT CALL Config::setDefaults.
//
// This is a default constructor, and as such is only used to:
//   a) initialize a default Context constructor
//   b) run unit tests
//
// In all real use cases, Config::load is called.
Config::Config ()
: _original_file ()
{
}

////////////////////////////////////////////////////////////////////////////////
Config::Config (const std::string& file)
{
  setDefaults ();
  load (file);
}

////////////////////////////////////////////////////////////////////////////////
// Read the Configuration file and populate the *this map.  The file format is
// simply lines with name=value pairs.  Whitespace between name, = and value is
// not tolerated, but blank lines and comments starting with # are allowed.
//
// Nested files are now supported, with the following construct:
//   include /absolute/path/to/file
//
void Config::load (const std::string& file, int nest /* = 1 */)
{
  Timer timer ("Config::load (" + file + ")");

  if (nest > 10)
    throw std::string (STRING_CONFIG_OVERNEST);

  // First time in, load the default values.
  if (nest == 1)
  {
    setDefaults ();
    _original_file = File (file);
  }

  // Read the file, then parse the contents.
  std::string contents;
  if (File::read (file, contents) && contents.length ())
    parse (contents, nest);
}

////////////////////////////////////////////////////////////////////////////////
void Config::parse (const std::string& input, int nest /* = 1 */)
{
  // Shortcut case for default constructor.
  if (input.length () == 0)
    return;

  // Split the input into lines.
  std::vector <std::string> lines;
  split (lines, input, "\n");

  // Parse each line.
  std::vector <std::string>::iterator it;
  for (it = lines.begin (); it != lines.end (); ++it)
  {
    std::string line = *it;

    // Remove comments.
    std::string::size_type pound = line.find ("#"); // no i18n
    if (pound != std::string::npos)
      line = line.substr (0, pound);

    line = trim (line, " \t"); // no i18n

    // Skip empty lines.
    if (line.length () > 0)
    {
      std::string::size_type equal = line.find ("="); // no i18n
      if (equal != std::string::npos)
      {
        std::string key   = trim (line.substr (0, equal), " \t"); // no i18n
        std::string value = trim (line.substr (equal+1, line.length () - equal), " \t"); // no i18n

        (*this)[key] = json::decode (value);
      }
      else
      {
        std::string::size_type include = line.find ("include"); // no i18n.
        if (include != std::string::npos)
        {
          Path included (trim (line.substr (include + 7), " \t"));
          if (included.is_absolute ())
          {
            if (included.readable ())
              this->load (included, nest + 1);
            else
              throw format (STRING_CONFIG_READ_INCLUDE, included._data);
          }
          else
            throw format (STRING_CONFIG_INCLUDE_PATH, included._data);
        }
        else
          throw format (STRING_CONFIG_BAD_ENTRY, line);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefaultRC (const std::string& rc, const std::string& data)
{
  // Override data.location in the defaults.
  std::string::size_type loc = _defaults.find ("data.location=~/.task");
  //                                      loc+0^          +14^   +21^

  Date now;
  std::stringstream contents;
  contents << "# [Created by "
           << PACKAGE_STRING
           << " "
           << now.toString ("m/d/Y H:N:S")
           << "]\n"
           << _defaults.substr (0, loc + 14)
           << data
           << "\n\n# Color theme (uncomment one to use)\n"
           << "#include " << TASK_RCDIR << "/light-16.theme\n"
           << "#include " << TASK_RCDIR << "/light-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-16.theme\n"
           << "#include " << TASK_RCDIR << "/dark-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-red-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-green-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-blue-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-violets-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-yellow-green.theme\n"
           << "#include " << TASK_RCDIR << "/dark-gray-256.theme\n"
           << "#include " << TASK_RCDIR << "/dark-gray-blue-256.theme\n"
           << "#include " << TASK_RCDIR << "/solarized-dark-256.theme\n"
           << "#include " << TASK_RCDIR << "/solarized-light-256.theme\n"
           << "#include " << TASK_RCDIR << "/no-color.theme\n"
           << "\n";

  // Write out the new file.
  if (! File::write (rc, contents.str ()))
    throw format (STRING_CONFIG_BAD_WRITE, rc);
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefaultData (const std::string& data)
{
  Directory d (data);
  if (! d.exists ())
  {
    if (getBoolean ("exit.on.missing.db"))
      throw std::string ("Error: rc.data.location does not exist - exiting according to rc.exit.on.missing.db setting.");

    d.create ();

    d += "hooks";
    d.create ();
  }
}

////////////////////////////////////////////////////////////////////////////////
void Config::setDefaults ()
{
  parse (_defaults);
}

////////////////////////////////////////////////////////////////////////////////
void Config::clear ()
{
  std::map <std::string, std::string>::clear ();
}

////////////////////////////////////////////////////////////////////////////////
const bool Config::has (const std::string& key)
{
  return (*this).find (key) != (*this).end ();
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
const std::string Config::get (const std::string& key)
{
  return (*this)[key];
}

////////////////////////////////////////////////////////////////////////////////
const int Config::getInteger (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
    return strtoimax ((*this)[key].c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
const double Config::getReal (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
    return strtod ((*this)[key].c_str (), NULL);

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
const bool Config::getBoolean (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
  {
    std::string value = lowerCase ((*this)[key]);
    if (value == "t"      ||  // TODO Deprecate
        value == "true"   ||
        value == "1"      ||
        value == "+"      ||  // TODO Deprecate
        value == "y"      ||
        value == "yes"    ||
        value == "on"     ||
        value == "enable" ||  // TODO Deprecate
        value == "enabled")   // TODO Deprecate
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const int value)
{
  (*this)[key] = format (value);
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const double value)
{
  (*this)[key] = format (value, 1, 8);
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const std::string& value)
{
  (*this)[key] = value;
}

////////////////////////////////////////////////////////////////////////////////
// Provide a vector of all configuration keys.
void Config::all (std::vector<std::string>& items) const
{
  std::map <std::string, std::string>::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
    items.push_back (it->first);
}

////////////////////////////////////////////////////////////////////////////////
