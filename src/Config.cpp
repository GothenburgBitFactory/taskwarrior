///////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham.
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
#include "File.h"
#include "Config.h"
#include "text.h"
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
// This string is used in two ways:
// 1) It is used to create a new .taskrc file, by copying it directly to disk.
// 2) It is parsed and used as default values for all Config.get calls.
std::string Config::defaults =
  "# Task program configuration file.\n"
  "# For more documentation, see http://taskwarrior.org or try 'man task' and 'man taskrc'\n"
  "\n"
  "# Files\n"
  "data.location=~/.task\n"
  "locking=on                             # Use file-level locking\n"
  "\n"
  "# Terminal\n"
  "curses=on                              # Use ncurses library to determine terminal width\n"
  "defaultwidth=80                        # Without ncurses, assumed width\n"
  "#editor=vi                             # Preferred text editor\n"
  "\n"
  "# Miscellaneous\n"
  "confirmation=yes                       # Confirmation on delete, big changes\n"
  "echo.command=yes                       # Details on command just run\n"
  "annotation.details=2                   # Level of verbosity for annotations in reports\n"
  "next=2                                 # How many tasks per project in next report\n"
  "bulk=2                                 # > 2 tasks considered 'a lot', for confirmation\n"
  "nag=You have higher priority tasks.    # Nag message to keep you honest\n"                      // TODO
  "\n"
  "# Dates\n"
  "dateformat=m/d/Y                       # Preferred input and display date format\n"
  "#reportdateformat=m/d/Y                # Preferred display date format for repors\n"
  "weekstart=Sunday                       # Sunday or Monday only\n"                               // TODO
  "displayweeknumber=yes                  # Show week numbers on calendar\n"                       // TODO
  "due=7                                  # Task is considered due in 7 days\n"
  "#calendar.details=yes                  # Calendar shows information for tasks w/due dates\n"
  "#calendar.details.report=list          # Report to use when showing task information in cal\n"  // TODO
  "#monthsperline=3                       # Number of calendar months on a line\n"                 // TODO
  "\n"
  "# Color controls.\n"
  "color=on                               # Enable color\n"
  "color.overdue=bold red                 # Color of overdue tasks\n"
  "color.due=bold yellow                  # Color of due tasks\n"
  "color.pri.H=bold                       # Color of priority:H tasks\n"
  "#color.pri.M=on yellow                 # Color of priority:M tasks\n"
  "#color.pri.L=on green                  # Color of priority:L tasks\n"
  "#color.pri.none=white on blue          # Color of priority:  tasks\n"
  "color.active=bold cyan                 # Color of active tasks\n"
  "color.tagged=yellow                    # Color of tagged tasks\n"
  "#color.tag.bug=yellow                  # Color of +bug tasks\n"
  "#color.project.garden=on green         # Color of project:garden tasks\n"
  "#color.keyword.car=on blue             # Color of description.contains:car tasks\n"
  "#color.recurring=on red                # Color of recur.any: tasks\n"
  "#color.header=bold green               # Color of header messages\n"
  "#color.footnote=bold green             # Color of footnote messages\n"
  "#color.alternate=on rgb253             # Alternate color for line coloring\n"
  "color.calendar.today=black on cyan     # Color of today in calendar\n"
  "color.calendar.due=black on green      # Color of days with due tasks in calendar\n"
  "color.calendar.overdue=black on red    # Color of days with overdue tasks in calendar\n"
  "color.calendar.weekend=black on white  # Color of weekend days in calendar\n"
  "#color.debug=magenta                   # Color of diagnostic output\n"
  "color.pri.H=bold                       # Color of priority:H tasks\n"
  "color.history.add=on red               # Color of added tasks in the history reports\n"
  "color.history.delete=on yellow         # Color of deleted tasks in the history reports\n"
  "color.history.done=on green            # Color of completed tasks in the history reports\n"
  "\n"
  "# Shadow file support\n"
  "#shadow.file=/tmp/shadow.txt           # Location of shadow file\n"
  "#shadow.command=list                   # Task command for shadow file\n"
  "#shadow.notify=on                      # Footnote when updated\n"
  "\n"
  "#default.project=foo                   # Default project for 'add' command\n"
  "#default.priority=M                    # Default priority for 'add' command\n"
  "default.command=list                   # When no arguments are specified\n"                     // TODO
  "\n"
  "_forcecolor=no                         # Forces color to be on, even for non TTY output\n"
  "blanklines=true                        # Use more whitespace in output\n"
  "complete.all.projects=no               # Include old project names in 'projects' command\n"     // TODO
  "complete.all.tags=no                   # Include old tag names in 'tags' command\n"             // TODO
  "debug=no                               # Display diagnostics\n"
  "fontunderline=yes                      # Uses underlines rather than -------\n"
  "shell.prompt=task>                     # Prompt used by the shell command\n"                    // TODO
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
  "# Aliases - alternate names for commands\n"
  "alias.rm=delete                        # Alias for the delete command\n"
  "\n"
  "# Fields: id,uuid,project,priority,priority_long,entry,entry_time,\n"                           // TODO
  "#         start,entry_time,due,recur,recurrence_indicator,age,\n"                               // TODO
  "#         age_compact,active,tags,tag_indicator,description,\n"                                 // TODO
  "#         description_only,end,end_time\n"                                                      // TODO
  "# Description:   This report is ...\n"
  "# Sort:          due+,priority-,project+\n"
  "# Filter:        pro:x pri:H +bug limit:10\n"
  "\n"
  "# task long\n"
  "report.long.description=Lists all task, all data, matching the specified criteria\n"
  "report.long.columns=id,project,priority,entry,start,due,recur,age,tags,description\n"
  "report.long.labels=ID,Project,Pri,Added,Started,Due,Recur,Age,Tags,Description\n"
  "report.long.sort=due+,priority-,project+\n"
  "report.long.filter=status:pending\n"
  "\n"
  "# task list\n"
  "report.list.description=Lists all tasks matching the specified criteria\n"
  "report.list.columns=id,project,priority,due,active,age,description\n"
  "report.list.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.list.sort=due+,priority-,project+\n"
  "report.list.filter=status:pending\n"
  "\n"
  "# task ls\n"
  "report.ls.description=Minimal listing of all tasks matching the specified criteria\n"
  "report.ls.columns=id,project,priority,description\n"
  "report.ls.labels=ID,Project,Pri,Description\n"
  "report.ls.sort=priority-,project+\n"
  "report.ls.filter=status:pending\n"
  "\n"
  "# task minimal\n"
  "report.minimal.description=A really minimal listing\n"
  "report.minimal.columns=id,project,description\n"
  "report.minimal.labels=ID,Project,Description\n"
  "report.minimal.sort=project+,description+\n"
  "report.minimal.filter=status:pending\n"
  "\n"
  "# task newest\n"
  "report.newest.description=Shows the newest tasks\n"
  "report.newest.columns=id,project,priority,due,active,age,description\n"
  "report.newest.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.newest.sort=id-\n"
  "report.newest.filter=status:pending limit:10\n"
  "\n"
  "# task oldest\n"
  "report.oldest.description=Shows the oldest tasks\n"
  "report.oldest.columns=id,project,priority,due,active,age,description\n"
  "report.oldest.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.oldest.sort=id+\n"
  "report.oldest.filter=status:pending limit:10\n"
  "\n"
  "# task overdue\n"
  "report.overdue.description=Lists overdue tasks matching the specified criteria\n"
  "report.overdue.columns=id,project,priority,due,active,age,description\n"
  "report.overdue.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.overdue.sort=due+,priority-,project+\n"
  "report.overdue.filter=status:pending due.before:today\n"
  "\n"
  "# task active\n"
  "report.active.description=Lists active tasks matching the specified criteria\n"
  "report.active.columns=id,project,priority,due,active,age,description\n"
  "report.active.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.active.sort=due+,priority-,project+\n"
  "report.active.filter=status:pending start.any:\n"
  "\n"
  "# task completed\n"
  "report.completed.description=Lists completed tasks matching the specified criteria\n"
  "report.completed.columns=end,project,priority,age,description\n"
  "report.completed.labels=Complete,Project,Pri,Age,Description\n"
  "report.completed.sort=end+,priority-,project+\n"
  "report.completed.filter=status:completed\n"
  "\n"
  "# task recurring\n"
  "report.recurring.description=Lists recurring tasks matching the specified criteria\n"
  "report.recurring.columns=id,project,priority,due,recur,active,age,description\n"
  "report.recurring.labels=ID,Project,Pri,Due,Recur,Active,Age,Description\n"
  "report.recurring.sort=due+,priority-,project+\n"
  "report.recurring.filter=status:pending parent.any:\n"
  "\n"
  "# task waiting\n"
  "report.waiting.description=Lists all waiting tasks matching the specified criteria\n"
  "report.waiting.columns=id,project,priority,wait,age,description\n"
  "report.waiting.labels=ID,Project,Pri,Wait,Age,Description\n"
  "report.waiting.sort=wait+,priority-,project+\n"
  "report.waiting.filter=status:waiting\n"
  "\n"
  "# task all\n"
  "report.all.description=Lists all tasks matching the specified criteria\n"
  "report.all.columns=id,project,priority,due,active,age,description\n"
  "report.all.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.all.sort=due+,priority-,project+\n"
  "\n"
  "# task next\n"
  "report.next.description=Lists the most urgent tasks\n"
  "report.next.columns=id,project,priority,due,active,age,description\n"
  "report.next.labels=ID,Project,Pri,Due,Active,Age,Description\n"
  "report.next.sort=due+,priority-,project+\n"
  "report.next.filter=status:pending\n"
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
              throw std::string ("Could not read include file '") + included.data + "'";
          }
          else
            throw std::string ("Can only include files with absolute paths, not '") + included.data + "'";
        }
        else
          throw std::string ("Malformed entry '") + line + "'";
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

  std::string contents = defaults.substr (0, loc + 14) +
                         data +
                         defaults.substr (loc + 21, std::string::npos);

  // Write out the new file.
  if (! File::write (rc, contents))
    throw std::string ("Could not write to '") + rc + "'";
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
void Config::all (std::vector<std::string>& items)
{
  foreach (i, *this)
    items.push_back (i->first);
}

////////////////////////////////////////////////////////////////////////////////
std::string Config::checkForDeprecatedColor ()
{
  int count = 0;
  std::vector <std::string> deprecated;
  foreach (i, *this)
  {
    if (i->first.find ("color.") != std::string::npos)
    {
      std::string value = get (i->first);
      if (value.find ("_") != std::string::npos)
      {
        ++count;
        deprecated.push_back (i->first);
      }
    }
  }

  std::stringstream out;
  if (count)
  {
    out << "Your .taskrc file contains color settings that use deprecated "
        << "underscores.  Please check:"
        << std::endl;

    foreach (i, deprecated)
      out << "  " << *i << "=" << get (*i) << std::endl;

    out << std::endl;
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
