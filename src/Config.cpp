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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include "Config.h"
#include "text.h"
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
// These are default (but overridable) reports.  These entries are necessary
// because these three reports were converted from hard-coded reports to custom
// reports, and therefore need these config file entries.  However, users are
// already used to seeing these five reports, but do not have these entries.
// The choice was a) make users edit their .taskrc files, b) write a .taskrc
// upgrade program to make the change, or c) this.
Config::Config ()
{
  setDefaults ();
}

////////////////////////////////////////////////////////////////////////////////
Config::Config (const std::string& file)
{
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
bool Config::load (const std::string& file, int nest /* = 1 */)
{
  if (nest > 10)
    throw std::string ("Configuration file nested to more than 10 levels deep"
                       " - this has to be a mistake.");

  std::ifstream in;
  in.open (file.c_str (), std::ifstream::in);
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
    {
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
          sequence.push_back (key);
        }
        else
        {
          std::string::size_type include = line.find ("include"); // no i18n.
          if (include != std::string::npos)
          {
            std::string included = expandPath ( trim ( line.substr (include + 7), " \t"));
            if (isAbsolutePath (included))
            {
              if (!access (included.c_str (), F_OK | R_OK))
                this->load (included, nest + 1);
              else
                throw std::string ("Could not read include file '") + included + "'";
            }
            else
              throw std::string ("Can only include files with absolute paths, not '") + included + "'";
          }
          else
            throw std::string ("Malformed entry in ") + file + ": '" + line + "'";
        }
      }
    }

    in.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefaultRC (const std::string& rc, const std::string& data)
{
  // Create a sample .taskrc file.
  std::stringstream contents;
  contents << "# Task program configuration file.\n"
           << "# For more documentation, see http://taskwarrior.org or try 'man task' and 'man taskrc'\n"
           << "\n"
           << "# Files\n"
           << "data.location=" << data << "\n"
           << "locking=on                             # Use file-level locking\n"
           << "\n"
           << "# Terminal\n"
           << "curses=on                              # Use ncurses library to determine terminal width\n"
           << "#defaultwidth=80                       # Without ncurses, assumed width\n"
           << "#editor=vi                             # Preferred text editor\n"
           << "\n"
           << "# Miscellaneous\n"
           << "confirmation=yes                       # Confirmation on delete, big changes\n"
           << "echo.command=yes                       # Details on command just run\n"
           << "next=2                                 # How many tasks per project in next report\n"
           << "bulk=2                                 # > 2 tasks considered 'a lot', for confirmation\n"
           << "nag=You have higher priority tasks.    # Nag message to keep you honest\n"
           << "\n"
           << "# Dates\n"
           << "dateformat=m/d/Y                       # Preferred input and display date format\n"
           << "weekstart=Sunday                       # Sunday or Monday only\n"
           << "displayweeknumber=yes                  # Show week numbers on calendar\n"
           << "due=7                                  # Task is considered due in 7 days\n"
           << "#calendar.details=yes                  # Calendar shows information for tasks w/due dates\n"
           << "#calendar.details.report=list          # Report to use when showing task information in cal\n"
           << "#monthsperline=2                       # Number of calendar months on a line\n"
           << "\n"
           << "# Color controls.\n"
           << "color=on                               # Use color\n"
           << "color.overdue=bold_red                 # Color of overdue tasks\n"
           << "color.due=bold_yellow                  # Color of due tasks\n"
           << "color.pri.H=bold                       # Color of priority:H tasks\n"
           << "#color.pri.M=on_yellow                 # Color of priority:M tasks\n"
           << "#color.pri.L=on_green                  # Color of priority:L tasks\n"
           << "#color.pri.none=white on_blue          # Color of priority:  tasks\n"
           << "color.active=bold_cyan                 # Color of active tasks\n"
           << "color.tagged=yellow                    # Color of tagged tasks\n"
           << "#color.tag.bug=yellow                  # Color of +bug tasks\n"
           << "#color.project.garden=on_green         # Color of project:garden tasks\n"
           << "#color.keyword.car=on_blue             # Color of description.contains:car tasks\n"
           << "#color.recurring=on_red                # Color of recur.any: tasks\n"
           << "#color.header=bold_green               # Color of header messages\n"
           << "#color.footnote=bold_green             # Color of footnote messages\n"
           << "color.calendar.today=cyan              # Color of today in calendar\n"
           << "color.calendar.due=black on yellow     # Color of days with due tasks in calendar\n"
           << "color.calendar.overdue=black on red    # Color of days with overdue tasks in calendar\n"
           << "\n"
           << "#shadow.file=/tmp/shadow.txt           # Location of shadow file\n"
           << "#shadow.command=list                   # Task command for shadow file\n"
           << "#shadow.notify=on                      # Footnote when updated\n"
           << "\n"
           << "#default.project=foo                   # Unless otherwise specified\n"
           << "#default.priority=M                    # Unless otherwise specified\n"
           << "default.command=list                   # Unless otherwise specified\n"
           << "\n"
           << "alias.rm=delete\n"
           << "\n"
           << "# Fields: id,uuid,project,priority,priority_long,entry,entry_time,\n"
           << "#         start,entry_time,due,recur,recurrence_indicator,age,\n"
           << "#         age_compact,active,tags,tag_indicator,description,\n"
           << "#         description_only,end,end_time\n"
           << "# Description:   This report is ...\n"
           << "# Sort:          due+,priority-,project+\n"
           << "# Filter:        pro:x pri:H +bug limit:10\n"
           << "\n"
           << "# task long\n"
           << "report.long.description=Lists all task, all data, matching the specified criteria\n"
           << "report.long.columns=id,project,priority,entry,start,due,recur,age,tags,description\n"
           << "report.long.labels=ID,Project,Pri,Added,Started,Due,Recur,Age,Tags,Description\n"
           << "report.long.sort=due+,priority-,project+\n"
           << "report.long.filter=status:pending\n"
           << "\n"
           << "# task list\n"
           << "report.list.description=Lists all tasks matching the specified criteria\n"
           << "report.list.columns=id,project,priority,due,active,age,description\n"
           << "report.list.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.list.sort=due+,priority-,project+\n"
           << "report.list.filter=status:pending\n"
           << "\n"
           << "# task ls\n"
           << "report.ls.description=Minimal listing of all tasks matching the specified criteria\n"
           << "report.ls.columns=id,project,priority,description\n"
           << "report.ls.labels=ID,Project,Pri,Description\n"
           << "report.ls.sort=priority-,project+\n"
           << "report.ls.filter=status:pending\n"
           << "\n"
           << "# task newest\n"
           << "report.newest.description=Shows the newest tasks\n"
           << "report.newest.columns=id,project,priority,due,active,age,description\n"
           << "report.newest.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.newest.sort=id-\n"
           << "report.newest.filter=status:pending limit:10\n"
           << "\n"
           << "# task oldest\n"
           << "report.oldest.description=Shows the oldest tasks\n"
           << "report.oldest.columns=id,project,priority,due,active,age,description\n"
           << "report.oldest.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.oldest.sort=id+\n"
           << "report.oldest.filter=status:pending limit:10\n"
           << "\n"
           << "# task overdue\n"
           << "report.overdue.description=Lists overdue tasks matching the specified criteria\n"
           << "report.overdue.columns=id,project,priority,due,active,age,description\n"
           << "report.overdue.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.overdue.sort=due+,priority-,project+\n"
           << "report.overdue.filter=status:pending due.before:today\n"
           << "\n"
           << "# task active\n"
           << "report.active.description=Lists active tasks matching the specified criteria\n"
           << "report.active.columns=id,project,priority,due,active,age,description\n"
           << "report.active.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.active.sort=due+,priority-,project+\n"
           << "report.active.filter=status:pending start.any:\n"
           << "\n"
           << "# task completed\n"
           << "report.completed.description=Lists completed tasks matching the specified criteria\n"
           << "report.completed.columns=end,project,priority,age,description\n"
           << "report.completed.labels=Complete,Project,Pri,Age,Description\n"
           << "report.completed.sort=end+,priority-,project+\n"
           << "report.completed.filter=status:completed\n"
           << "\n"
           << "# task recurring\n"
           << "report.recurring.description=Lists recurring tasks matching the specified criteria\n"
           << "report.recurring.columns=id,project,priority,due,recur,active,age,description\n"
           << "report.recurring.labels=ID,Project,Pri,Due,Recur,Active,Age,Description\n"
           << "report.recurring.sort=due+,priority-,project+\n"
           << "report.recurring.filter=status:pending parent.any:\n"
           << "\n"
           << "# task waiting\n"
           << "report.waiting.description=Lists all waiting tasks matching the specified criteria\n"
           << "report.waiting.columns=id,project,priority,wait,age,description\n"
           << "report.waiting.labels=ID,Project,Pri,Wait,Age,Description\n"
           << "report.waiting.sort=wait+,priority-,project+\n"
           << "report.waiting.filter=status:waiting\n"
           << "\n"
           << "# task all\n"
           << "report.all.description=Lists all tasks matching the specified criteria\n"
           << "report.all.columns=id,project,priority,due,active,age,description\n"
           << "report.all.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.all.sort=due+,priority-,project+\n"
           << "\n"
           << "# task next\n"
           << "report.next.description=Lists the most urgent tasks\n"
           << "report.next.columns=id,project,priority,due,active,age,description\n"
           << "report.next.labels=ID,Project,Pri,Due,Active,Age,Description\n"
           << "report.next.sort=due+,priority-,project+\n"
           << "report.next.filter=status:pending\n"
           << "\n";

  spit (rc, contents.str ());
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefaultData (const std::string& data)
{
  if (access (data.c_str (), F_OK))
    mkdir (data.c_str (), S_IRWXU);
}

////////////////////////////////////////////////////////////////////////////////
void Config::setDefaults ()
{
  set ("report.long.description",      "Lists all task, all data, matching the specified criteria");      // TODO i18n
  set ("report.long.columns",          "id,project,priority,entry,start,due,recur,age,tags,description"); // TODO i18n
  set ("report.long.labels",           "ID,Project,Pri,Added,Started,Due,Recur,Age,Tags,Description");    // TODO i18n
  set ("report.long.sort",             "due+,priority-,project+");                                        // TODO i18n
  set ("report.long.filter",           "status:pending");                                                 // TODO i18n

  set ("report.list.description",      "Lists all tasks matching the specified criteria");                // TODO i18n
  set ("report.list.columns",          "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.list.labels",           "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.list.sort",             "due+,priority-,project+");                                        // TODO i18n
  set ("report.list.filter",           "status:pending");                                                 // TODO i18n

  set ("report.ls.description",        "Minimal listing of all tasks matching the specified criteria");   // TODO i18n
  set ("report.ls.columns",            "id,project,priority,description");                                // TODO i18n
  set ("report.ls.labels",             "ID,Project,Pri,Description");                                     // TODO i18n
  set ("report.ls.sort",               "priority-,project+");                                             // TODO i18n
  set ("report.ls.filter",             "status:pending");                                                 // TODO i18n

  set ("report.newest.description",    "Shows the newest tasks");                                         // TODO i18n
  set ("report.newest.columns",        "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.newest.labels",         "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.newest.sort",           "id-");                                                            // TODO i18n
  set ("report.newest.filter",         "status:pending limit:10");                                        // TODO i18n

  set ("report.oldest.description",    "Shows the oldest tasks");                                         // TODO i18n
  set ("report.oldest.columns",        "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.oldest.labels",         "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.oldest.sort",           "id+");                                                            // TODO i18n
  set ("report.oldest.filter",         "status:pending limit:10");                                        // TODO i18n

  set ("report.overdue.description",   "Lists overdue tasks matching the specified criteria");            // TODO i18n
  set ("report.overdue.columns",       "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.overdue.labels",        "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.overdue.sort",          "due+,priority-,project+");                                        // TODO i18n
  set ("report.overdue.filter",        "status:pending due.before:today");                                // TODO i18n

  set ("report.active.description",    "Lists active tasks matching the specified criteria");             // TODO i18n
  set ("report.active.columns",        "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.active.labels",         "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.active.sort",           "due+,priority-,project+");                                        // TODO i18n
  set ("report.active.filter",         "status:pending start.any:");                                      // TODO i18n

  set ("report.completed.description", "Lists completed tasks matching the specified criteria");          // TODO i18n
  set ("report.completed.columns",     "end,project,priority,age,description");                           // TODO i18n
  set ("report.completed.labels",      "Complete,Project,Pri,Age,Description");                           // TODO i18n
  set ("report.completed.sort",        "end+,priority-,project+");                                        // TODO i18n
  set ("report.completed.filter",      "status:completed");                                               // TODO i18n

  set ("report.recurring.description", "Lists recurring tasks matching the specified criteria");          // TODO i18n
  set ("report.recurring.columns",     "id,project,priority,due,recur,active,age,description");           // TODO i18n
  set ("report.recurring.labels",      "ID,Project,Pri,Due,Recur,Active,Age,Description");                // TODO i18n
  set ("report.recurring.sort",        "due+,priority-,project+");                                        // TODO i18n
  set ("report.recurring.filter",      "status:pending parent.any:");                                     // TODO i18n

  set ("report.waiting.description",   "Lists all waiting tasks matching the specified criteria");        // TODO i18n
  set ("report.waiting.columns",       "id,project,priority,wait,age,description");                       // TODO i18n
  set ("report.waiting.labels",        "ID,Project,Pri,Wait,Age,Description");                            // TODO i18n
  set ("report.waiting.sort",          "wait+,priority-,project+");                                       // TODO i18n
  set ("report.waiting.filter",        "status:waiting");                                                 // TODO i18n

  set ("report.all.description",       "Lists all tasks matching the specified criteria");                // TODO i18n
  set ("report.all.columns",           "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.all.labels",            "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.all.sort",              "due+,priority-,project+");                                        // TODO i18n

  set ("report.next.description",      "Lists the most urgent tasks");                                    // TODO i18n
  set ("report.next.columns",          "id,project,priority,due,active,age,description");                 // TODO i18n
  set ("report.next.labels",           "ID,Project,Pri,Due,Active,Age,Description");                      // TODO i18n
  set ("report.next.sort",             "due+,priority-,project+");                                        // TODO i18n
  set ("report.next.filter",           "status:pending");                                                 // TODO i18n

  set ("alias.rm",                     "delete");                                                         // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
void Config::clear ()
{
  std::map <std::string, std::string>::clear ();
  sequence.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
const std::string Config::get (const char* key)
{
  return this->get (std::string (key));
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.  If a default_value
// is present, it will be the returned value in the event of a missing key.
const std::string Config::get (
  const char* key,
  const char* default_value)
{
  return this->get (std::string (key), std::string (default_value));
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
const std::string Config::get (const std::string& key)
{
  return (*this)[key];
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.  If a default_value
// is present, it will be the returned value in the event of a missing key.
const std::string Config::get (
  const std::string& key,
  const std::string& default_value)
{
  if ((*this).find (key) != (*this).end ())
    return (*this)[key];

  return default_value;
}

////////////////////////////////////////////////////////////////////////////////
bool Config::get (const std::string& key, const bool default_value)
{
  if ((*this).find (key) != (*this).end ())
  {
    std::string value = lowerCase ((*this)[key]);

    if (value == "t"      ||  // TODO i18n
        value == "true"   ||  // TODO i18n
        value == "1"      ||  // no i18n
        value == "yes"    ||  // TODO i18n
        value == "on"     ||  // TODO i18n
        value == "enable" ||  // TODO i18n
        value == "enabled")   // TODO i18n
      return true;

    return false;
  }

  return default_value;
}

////////////////////////////////////////////////////////////////////////////////
int Config::get (const std::string& key, const int default_value)
{
  if ((*this).find (key) != (*this).end ())
    return atoi ((*this)[key].c_str ());

  return default_value;
}

////////////////////////////////////////////////////////////////////////////////
double Config::get (const std::string& key, const double default_value)
{
  if ((*this).find (key) != (*this).end ())
    return atof ((*this)[key].c_str ());

  return default_value;
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
void Config::getSequence (std::vector<std::string>& items)
{
  items = sequence;
}

////////////////////////////////////////////////////////////////////////////////
std::string Config::checkForDuplicates ()
{
  std::vector <std::string> duplicates;
  std::map <std::string, int> unique;

  foreach (i, sequence)
  {
    if (unique.find (*i) != unique.end ())
      duplicates.push_back (*i);
    else
      unique[*i] = 0;
  }

  std::stringstream out;
  if (duplicates.size ())
  {
    out << "Found duplicate entries for:" << std::endl;

    foreach (i, duplicates)
      out << "  " << *i << std::endl;

    out << std::endl;
  }

  return out.str ();
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
