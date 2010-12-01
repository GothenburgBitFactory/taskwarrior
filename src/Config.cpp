///////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include "Directory.h"
#include "Date.h"
#include "File.h"
#include "Config.h"
#include "text.h"
#include "util.h"
#include "../auto.h"

////////////////////////////////////////////////////////////////////////////////
// This string is used in two ways:
// 1) It is used to create a new .taskrc file, by copying it directly to disk.
// 2) It is parsed and used as default values for all Config.get calls.
std::string Config::defaults =
  "# Taskwarrior program configuration file.\n"
  "# For more documentation, see http://taskwarrior.org or try 'man task', 'man task-faq',\n"
  "# 'man task-tutorial', 'man task-color', 'man task-sync' or 'man taskrc'\n"
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
  "\n"
  "# Terminal\n"
  "curses=on                                      # Use ncurses library to determine terminal width\n"
  "defaultwidth=80                                # Without ncurses, assumed width\n"
  "#editor=vi                                     # Preferred text editor\n"
  "\n"
  "# Miscellaneous\n"
  "verbose=yes                                    # Provide extra feedback\n"
  "confirmation=yes                               # Confirmation on delete, big changes\n"
  "echo.command=yes                               # Details on command just run\n"
  "annotations=full                               # Level of verbosity for annotations: full, sparse or none\n"
  "next=2                                         # How many tasks per project in next report\n"
  "bulk=2                                         # > 2 tasks considered 'a lot', for confirmation\n"
  "nag=You have higher priority tasks.            # Nag message to keep you honest\n"                      // TODO
  "search.case.sensitive=yes                      # Setting to no allows case insensitive searches\n"
  "active.indicator=*                             # What to show as an active task indicator\n"
  "tag.indicator=+                                # What to show as a tag indicator\n"
  "recurrence.indicator=R                         # What to show as a task recurrence indicator\n"
  "recurrence.limit=1                             # Number of future recurring pending tasks\n"
  "undo.style=side                                # Undo style - can be 'side', or 'diff'\n"
  "burndown.bias=0.666                            # Weighted mean bias toward recent data\n"
  "regex=no                                       # Assume all search/filter strings are regexes\n"
  "\n"
  "# Dates\n"
  "dateformat=m/d/Y                               # Preferred input and display date format\n"
  "dateformat.holiday=YMD                         # Preferred input date format for holidays\n"
  "dateformat.report=m/d/Y                        # Preferred display date format for reports\n"
  "dateformat.annotation=m/d/Y                    # Preferred display date format for annotations\n"
  "weekstart=Sunday                               # Sunday or Monday only\n"
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
  "urgency.next.coefficient=10.0                  # Urgency coefficients for 'next' special tag\n"
  "urgency.blocking.coefficient=9.0               # Urgency coefficients for blocking tasks\n"
  "urgency.blocked.coefficient=8.0                # Urgency coefficients for blocked tasks\n"
  "urgency.due.coefficient=7.0                    # Urgency coefficients for due dates\n"
  "urgency.priority.coefficient=6.0               # Urgency coefficients for priorities\n"
  "urgency.waiting.coefficient=5.0                # Urgency coefficients for waiting status\n"
  "urgency.active.coefficient=4.0                 # Urgency coefficients for active tasks\n"
  "urgency.project.coefficient=3.0                # Urgency coefficients for projects\n"
  "urgency.tags.coefficient=2.0                   # Urgency coefficients for tags\n"
  "urgency.annotations.coefficient=1.0            # Urgency coefficients for annotations\n"
  "\n"
  "#urgency.user.project.foo.coefficient=5.0      # Urgency coefficients for 'foo' project\n"
  "#urgency.user.tag.foo.coefficient=5.0          # Urgency coefficients for 'foo' tag\n"
  "\n"
  "# Color controls.\n"
  "color=on                                       # Enable color\n"
#ifdef LINUX
  "color.header=color3                            # Color of header messages\n"
  "color.footnote=color3                          # Color of footnote messages\n"
  "color.debug=color3                             # Color of diagnostic output\n"
  "color.alternate=on color233                    # Alternate color for line coloring\n"
  "\n"
  "color.summary.bar=black on rgb141              # Color of summary report progress bar\n"
  "color.summary.background=white on color0       # Color of summary report background\n"
  "\n"
  "color.history.add=color0 on rgb500             # Color of added tasks in ghistory report\n"
  "color.history.done=color0 on rgb050            # Color of completed tasks in ghistory report\n"
  "color.history.delete=color0 on rgb550          # Color of deleted tasks in ghistory report\n"
  "\n"
  "color.burndown.pending=color0 on rgb500        # Color of pending tasks in burndown report\n"
  "color.burndown.done=color0 on rgb050           # Color of completed tasks in burndown report\n"
  "color.burndown.started=color0 on rgb550        # Color of started tasks in burndown report\n"
  "\n"
  "color.sync.added=rgb005                        # Color of added tasks in sync output\n"
  "color.sync.changed=rgb550                      # Color of changed tasks in sync output\n"
  "color.sync.rejected=rgb500                     # Color of rejected tasks in sync output\n"
  "\n"
  "color.undo.before=color1                       # Color of values before a change\n"
  "color.undo.after=color2                        # Color of values after a change\n"
  "\n"
  "color.calendar.today=color15 on rgb013         # Color of today in calendar\n"
  "color.calendar.due=color0 on color1            # Color of days  with due tasks in calendar\n"
  "color.calendar.due.today=color15 on color1     # Color of today with due tasks in calendar\n"
  "color.calendar.overdue=color0 on color9        # Color of days  with overdue tasks in calendar\n"
  "color.calendar.weekend=color235                # Color of weekend days in calendar\n"
  "color.calendar.holiday=color0 on color11       # Color of public holidays in calendar\n"
  "color.calendar.weeknumber=rgb013               # Color of the weeknumbers in calendar\n"
  "\n"
  "# Here are the color rules.\n"
  "color.recurring=rgb013                         # Color of recur.any: tasks\n"
  "color.overdue=color9                           # Color of overdue tasks\n"
  "color.due.today=rgb400                         # Color of tasks due today\n"
  "color.due=color1                               # Color of due tasks\n"
  "#color.keyword.car=on blue                     # Color of description.contains:car tasks\n"
  "#color.project.garden=on green                 # Color of project:garden tasks\n"
  "#color.project.none=                           # Color of tasks with no project\n"
  "#color.tag.bug=yellow                          # Color of +bug tasks\n"
  "#color.tag.none=                               # Color of tag-less tasks\n"
  "color.active=rgb555 on rgb410                  # Color of active tasks\n"
  "color.pri.none=                                # Color of priority:  tasks\n"
  "color.pri.H=rgb255                             # Color of priority:H tasks\n"
  "color.pri.M=rgb250                             # Color of priority:M tasks\n"
  "color.pri.L=rgb245                             # Color of priority:L tasks\n"
  "color.tagged=rgb031                            # Color of tagged tasks\n"
  "color.blocked=white on color8                  # Color of blocked tasks\n"
#else
  "color.header=yellow                            # Color of header messages\n"
  "color.footnote=yellow                          # Color of footnote messages\n"
  "color.debug=yellow                             # Color of diagnostic output\n"
  "color.alternate=                               # Alternate color for line coloring\n"
  "\n"
  "color.summary.bar=black on green               # Color of summary report progress bar\n"
  "color.summary.background=white on black        # Color of summary report background\n"
  "\n"
  "color.history.add=black on red                 # Color of added tasks in ghistory report\n"
  "color.history.done=black on green              # Color of completed tasks in ghistory report\n"
  "color.history.delete=black on yellow           # Color of deleted tasks in ghistory report\n"
  "\n"
  "color.burndown.pending=black on red            # Color of pending tasks in burndown report\n"
  "color.burndown.done=black on green             # Color of completed tasks in burndown report\n"
  "color.burndown.started=black on yellow         # Color of started tasks in burndown report\n"
  "\n"
  "color.sync.added=green                         # Color of added tasks in sync output\n"
  "color.sync.changed=yellow                      # Color of changed tasks in sync output\n"
  "color.sync.rejected=red                        # Color of rejected tasks in sync output\n"
  "\n"
  "color.undo.before=red                          # Color of values before a change\n"
  "color.undo.after=green                         # Color of values after a change\n"
  "\n"
  "color.calendar.today=bold white on bright blue # Color of today in calendar\n"
  "color.calendar.due=white on red                # Color of days  with due tasks in calendar\n"
  "color.calendar.due.today=bold white on red     # Color of today with due tasks in calendar\n"
  "color.calendar.overdue=black on bright red     # Color of days  with overdue tasks in calendar\n"
  "color.calendar.weekend=white on bright black   # Color of weekend days in calendar\n"
  "color.calendar.holiday=black on bright yellow  # Color of public holidays in calendar\n"
  "color.calendar.weeknumber=bold blue            # Color of the weeknumbers in calendar\n"
  "\n"
  "# Here are the color rules.\n"
  "color.recurring=magenta                        # Color of recur.any: tasks\n"
  "color.overdue=bold red                         # Color of overdue tasks\n"
  "color.due.today=red                            # Color of tasks due today\n"
  "color.due=red                                  # Color of due tasks\n"
  "#color.keyword.car=on blue                     # Color of description.contains:car tasks\n"
  "#color.project.garden=on green                 # Color of project:garden tasks\n"
  "#color.project.none=                           # Color of tasks with no project\n"
  "#color.tag.bug=yellow                          # Color of +bug tasks\n"
  "#color.tag.none=                               # Color of tag-less tasks\n"
  "color.active=black on bright green             # Color of active tasks\n"
  "color.pri.none=                                # Color of priority:  tasks\n"
  "color.pri.H=bold white                         # Color of priority:H tasks\n"
  "color.pri.M=white                              # Color of priority:M tasks\n"
  "color.pri.L=                                   # Color of priority:L tasks\n"
  "color.tagged=green                             # Color of tagged tasks\n"
  "color.blocked=black on white                   # Color of blocked tasks\n"
#endif
  "\n"
  "# Here is the rule precedence order, highest to lowest.\n"
  "# Note that these are just the color rule names, without the leading 'color.'\n"
  "#      and any trailing '.value'.\n"
  "rule.precedence.color=due.today,active,blocked,overdue,due,keyword,project,tag,recurring,pri,tagged\n"
  "\n"
  "# Shadow file support\n"
  "#shadow.file=/tmp/shadow.txt                   # Location of shadow file\n"
  "#shadow.command=list                           # Task command for shadow file\n"
  "#shadow.notify=on                              # Footnote when updated\n"
  "\n"
  "#default.project=foo                           # Default project for 'add' command\n"
  "#default.priority=M                            # Default priority for 'add' command\n"
  "default.command=list                           # When no arguments are specified\n"
  "\n"
  "_forcecolor=no                                 # Forces color to be on, even for non TTY output\n"
  "blanklines=true                                # Use more whitespace in output\n"
  "complete.all.projects=no                       # Include old project names in '_projects' command\n"
  "complete.all.tags=no                           # Include old tag names in '_ags' command\n"
  "list.all.projects=no                           # Include old project names in 'projects' command\n"
  "list.all.tags=no                               # Include old tag names in 'tags' command\n"
  "debug=no                                       # Display diagnostics\n"
  "hooks=off                                      # Hook system master switch\n"
  "fontunderline=yes                              # Uses underlines rather than -------\n"
  "shell.prompt=task>                             # Prompt used by the shell command\n"
  "\n"
	"# Merge options\n"
  "#\n"
  "# WARNING: Please read the documentation (man task-sync) before proceeding with these\n"
  "#          synchronization features.  Improperly used, data can be lost!\n"
	"merge.autopush=ask                             # Push database to remote origin after merge: yes, no, ask\n"
	"#merge.default.uri=user@host.xz:.task/         # URI for merge\n"
	"#pull.default.uri=rsync://host.xz/task-backup/ # URI for pull\n"
	"\n"
  "# Import heuristics - alternate names for fields (comma-separated list of names)\n"
  "#import.synonym.bg=?\n"
  "#import.synonym.description=?\n"
  "#import.synonym.due=?\n"
  "#import.synonym.end=?\n"
  "#import.synonym.entry=?\n"
  "#import.synonym.fg=?\n"
  "#import.synonym.id=?\n"
  "#import.synonym.priority=?\n"
  "#import.synonym.project=?\n"
  "#import.synonym.recur=?\n"
  "#import.synonym.start=?\n"
  "#import.synonym.status=?\n"
  "#import.synonym.tags=?\n"
  "#import.synonym.uuid=?\n"
  "\n"
  "# Export Controls\n"
  "export.ical.class=PRIVATE                      # Could be PUBLIC, PRIVATE or CONFIDENTIAL\n"
  "\n"
  "# Aliases - alternate names for commands\n"
  "alias.rm=delete                                # Alias for the delete command\n"
  "alias.history=history.monthly                  # Prefer monthly over annual history reports\n"
  "alias.ghistory=ghistory.monthly                # Prefer monthly graphical over annual history reports\n"
  "alias.export=export.yaml                       # Prefer YAML over CSV or iCal export\n"
  "alias.export.vcalendar=export.ical             # They are the same\n"
  "alias.burndown=burndown.weekly                 # Prefer the weekly burndown chart\n"
  "\n"
  "# Fields: id, uuid, project, priority, priority_long, entry, start, end,\n"
  "#         due, countdown, countdown_compact, age, age_compact, active, tags,\n"
  "#         depends, description_only, description, recur, recurrence_indicator,\n"
  "#         tag_indicator, wait.\n"
  "# Description:   This report is ...\n"
  "# Sort:          due+,priority-,project+\n"
  "# Filter:        pro:x pri:H +bug limit:10\n"
  "# Dateformat:    due date format in reports\n"
  "\n"
  "# task long\n"
  "report.long.description=Lists all task, all data, matching the specified criteria\n"
  "report.long.columns=id,project,priority,entry,start,due,recur,countdown,age,depends,tags,description\n"
  "report.long.labels=ID,Project,Pri,Added,Started,Due,Recur,Countdown,Age,Deps,Tags,Description\n"
  "report.long.sort=due+,priority-,project+,description+\n"
  "report.long.filter=status:pending\n"
  "#report.long.dateformat=m/d/Y\n"
  "#report.long.annotations=full\n"
  "\n"
  "# task list\n"
  "report.list.description=Lists all tasks matching the specified criteria\n"
  "report.list.columns=id,project,priority,due,active,age,description\n"
  "report.list.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.list.sort=due+,priority-,active-,project+,description+\n"
  "report.list.filter=status:pending\n"
  "#report.list.dateformat=m/d/Y\n"
  "#report.list.annotations=full\n"
  "\n"
  "# task ls\n"
  "report.ls.description=Minimal listing of all tasks matching the specified criteria\n"
  "report.ls.columns=id,project,priority,description\n"
  "report.ls.labels=ID,Project,Pri,Description\n"
  "report.ls.sort=priority-,project+,description+\n"
  "report.ls.filter=status:pending\n"
  "#report.ls.dateformat=m/d/Y\n"
  "#report.ls.annotations=full\n"
  "\n"
  "# task minimal\n"
  "report.minimal.description=A really minimal listing\n"
  "report.minimal.columns=id,project,description\n"
  "report.minimal.labels=ID,Project,Description\n"
  "report.minimal.sort=project+,description+\n"
  "report.minimal.filter=status:pending\n"
  "#report.minimal.dateformat=m/d/Y\n"
  "#report.minimal.annotations=full\n"
  "\n"
  "# task newest\n"
  "report.newest.description=Shows the newest tasks\n"
  "report.newest.columns=id,project,priority,due,active,age,description\n"
  "report.newest.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.newest.sort=id-\n"
  "report.newest.filter=status:pending limit:10\n"
  "#report.newest.dateformat=m/d/Y\n"
  "#report.newest.annotations=full\n"
  "\n"
  "# task oldest\n"
  "report.oldest.description=Shows the oldest tasks\n"
  "report.oldest.columns=id,project,priority,due,active,age,description\n"
  "report.oldest.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.oldest.sort=id+\n"
  "report.oldest.filter=status:pending limit:10\n"
  "#report.oldest.dateformat=m/d/Y\n"
  "#report.oldest.annotations=full\n"
  "\n"
  "# task overdue\n"
  "report.overdue.description=Lists overdue tasks matching the specified criteria\n"
  "report.overdue.columns=id,project,priority,due,active,age,description\n"
  "report.overdue.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.overdue.sort=due+,priority-,active-,project+,description+\n"
  "report.overdue.filter=status:pending due.before:now\n"
  "#report.overdue.dateformat=m/d/Y\n"
  "#report.overdue.annotations=full\n"
  "\n"
  "# task active\n"
  "report.active.description=Lists active tasks matching the specified criteria\n"
  "report.active.columns=id,project,priority,due,active,age,description\n"
  "report.active.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.active.sort=due+,priority-,project+,description+\n"
  "report.active.filter=status:pending start.any:\n"
  "#report.active.dateformat=m/d/Y\n"
  "#report.active.annotations=full\n"
  "\n"
  "# task completed\n"
  "report.completed.description=Lists completed tasks matching the specified criteria\n"
  "report.completed.columns=end,project,priority,age,description\n"
  "report.completed.labels=Complete,Project,Pri,Age,Description\n"
  "report.completed.sort=end+,priority-,project+,description+\n"
  "report.completed.filter=status:completed\n"
  "#report.completed.dateformat=m/d/Y\n"
  "#report.completed.annotations=full\n"
  "\n"
  "# task recurring\n"
  "report.recurring.description=Lists recurring tasks matching the specified criteria\n"
  "report.recurring.columns=id,project,priority,due,recur,active,age,description\n"
  "report.recurring.labels=ID,Project,Pri,Due,Recur,Active,Age,Description\n"
  "report.recurring.sort=due+,priority-,active-,project+,description+\n"
  "report.recurring.filter=status:pending parent.any:\n"
  "#report.recurring.dateformat=m/d/Y\n"
  "#report.recurring.annotations=full\n"
  "\n"
  "# task waiting\n"
  "report.waiting.description=Lists all waiting tasks matching the specified criteria\n"
  "report.waiting.columns=id,project,priority,wait,age,description\n"
  "report.waiting.labels=ID,Project,Pri,Wait,Age,Description\n"
  "report.waiting.sort=wait+,priority-,project+,description+\n"
  "report.waiting.filter=status:waiting\n"
  "#report.waiting.dateformat=m/d/Y\n"
  "#report.waiting.annotations=full\n"
  "\n"
  "# task all\n"
  "report.all.description=Lists all tasks matching the specified criteria\n"
  "report.all.columns=id,project,priority,due,end,active,age,description\n"
  "report.all.labels=ID,Project,Pri,Due,Completed,Active,Age,Description\n"
  "report.all.sort=project+,due+,end+,priority-,active-,description+\n"
  "report.all.filter=status.not:deleted\n"
  "#report.all.dateformat=m/d/Y\n"
  "#report.all.annotations=full\n"
  "\n"
  "# task next\n"
  "report.next.description=Lists the most urgent tasks\n"
  "report.next.columns=id,project,priority,due,active,age,description\n"
  "report.next.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.next.sort=due+,priority-,active-,project+,description+\n"
  "report.next.filter=status:pending limit:page depends.none:\n"
  "#report.next.dateformat=m/d/Y\n"
  "#report.next.annotations=full\n"
  "\n"
  "# task blocked\n"
  "report.blocked.description=Lists all blocked tasks matching the specified criteria\n"
  "report.blocked.columns=id,depends,project,priority,due,active,age,description\n"
  "report.blocked.labels=ID,Deps,Project,Pri,Due,Active,Age,Description\n"
  "report.blocked.sort=due+,priority-,active-,project+,description+\n"
  "report.blocked.filter=status:pending depends.any:\n"
  "#report.blocked.dateformat=m/d/Y\n"
  "\n"
  "# task unblocked\n"
  "report.unblocked.description=Lists all unblocked tasks matching the specified criteria\n"
  "report.unblocked.columns=id,depends,project,priority,due,active,age,description\n"
  "report.unblocked.labels=ID,Deps,Project,Pri,Due,Active,Age,Description\n"
  "report.unblocked.sort=due+,priority-,active-,project+,description+\n"
  "report.unblocked.filter=status:pending depends.none:\n"
  "#report.unblocked.dateformat=m/d/Y\n"
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
: original_file ()
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
  if (nest > 10)
    throw std::string ("Configuration file nested to more than 10 levels deep"
                       " - this has to be a mistake.");

  // First time in, load the default values.
  if (nest == 1)
  {
    setDefaults ();
    original_file = File (file);
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

        (*this)[key] = value;
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
              throw std::string ("Could not read include file '") + included.data + "'.";
          }
          else
            throw std::string ("Can only include files with absolute paths, not '") + included.data + "'";
        }
        else
          throw std::string ("Malformed entry '") + line + "'.";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefaultRC (const std::string& rc, const std::string& data)
{
  // Override data.location in the defaults.
  std::string::size_type loc = defaults.find ("data.location=~/.task");
  //                                      loc+0^          +14^   +21^

  Date now;
  std::stringstream contents;
  contents << "# [Created by "
           << PACKAGE_STRING
           << " "
           << now.toString ("m/d/Y H:N:S")
           << "]\n"
           << defaults.substr (0, loc + 14)
           << data
           << "\n\n# Color theme (uncomment one to use)\n"
           << "#include /usr/local/share/doc/task/rc/light-16.theme\n"
           << "#include /usr/local/share/doc/task/rc/light-256.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-16.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-256.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-red-256.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-green-256.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-blue-256.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-violets-256.theme\n"
           << "#include /usr/local/share/doc/task/rc/dark-yellow-green.theme\n"
           << "\n";

  // Write out the new file.
  if (! File::write (rc, contents.str ()))
    throw std::string ("Could not write to '") + rc + "'.";
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefaultData (const std::string& data)
{
  Directory d (data);
  if (! d.exists ())
    d.create ();
}

////////////////////////////////////////////////////////////////////////////////
void Config::setDefaults ()
{
  parse (defaults);
}

////////////////////////////////////////////////////////////////////////////////
void Config::clear ()
{
  std::map <std::string, std::string>::clear ();
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
    return atoi ((*this)[key].c_str ());

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
const double Config::getReal (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
    return atof ((*this)[key].c_str ());

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
const bool Config::getBoolean (const std::string& key)
{
  if ((*this).find (key) != (*this).end ())
  {
    std::string value = lowerCase ((*this)[key]);
    if (value == "t"      ||  // TODO i18n
        value == "true"   ||  // TODO i18n
        value == "1"      ||  // no i18n
        value == "+"      ||  // no i18n
        value == "y"      ||  // TODO i18n
        value == "yes"    ||  // TODO i18n
        value == "on"     ||  // TODO i18n
        value == "enable" ||  // TODO i18n
        value == "enabled")   // TODO i18n
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const int value)
{
  char v[24];
  sprintf (v, "%d", value);
  (*this)[key] = v;
}

////////////////////////////////////////////////////////////////////////////////
void Config::set (const std::string& key, const double value)
{
  char v[32];
  sprintf (v, "%f", value);
  (*this)[key] = v;
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
std::string Config::checkForDeprecatedColor ()
{
  std::vector <std::string> deprecated;
  std::map <std::string, std::string>::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
  {
    if (it->first.find ("color.") != std::string::npos)
    {
      std::string value = get (it->first);
      if (value.find ("_") != std::string::npos)
        deprecated.push_back (it->first);
    }
  }

  std::stringstream out;
  if (deprecated.size ())
  {
    out << "Your .taskrc file contains color settings that use deprecated "
        << "underscores.  Please check:\n";

    std::vector <std::string>::iterator it2;
    for (it2 = deprecated.begin (); it2 != deprecated.end (); ++it2)
      out << "  " << *it2 << "=" << get (*it2) << "\n";

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Config::checkForDeprecatedColumns ()
{
  std::vector <std::string> deprecated;
  std::map <std::string, std::string>::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
  {
    if (it->first.find ("report") == 0)
    {
      std::string value = get (it->first);
      if (value.find ("entry_time") != std::string::npos ||
          value.find ("start_time") != std::string::npos ||
          value.find ("end_time")   != std::string::npos)
        deprecated.push_back (it->first);
    }
  }

  std::stringstream out;
  out << "\n";

  if (deprecated.size ())
  {
    out << "Your .taskrc file contains reports with deprecated columns.  "
        << "Please check for entry_time, start_time or end_time in:\n";

    std::vector <std::string>::iterator it2;
    for (it2 = deprecated.begin (); it2 != deprecated.end (); ++it2)
      out << "  " << *it2 << "=" << get (*it2) << "\n";

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
