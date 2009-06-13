///////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
bool Config::load (const std::string& file)
{
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
        }
      }
    }

    in.close ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Config::createDefault (const std::string& home)
{
  // Strip trailing slash off home directory, if necessary.
  std::string terminatedHome = home;
  if (home[home.length () - 1] == '/')
    terminatedHome = home.substr (0, home.length () - 1);

  // Determine default names of init file and task directory.
  std::string rcFile  = terminatedHome + "/.taskrc"; // TODO i18n
  std::string dataDir = terminatedHome + "/.task";   // TODO i18n

  // If rcFile is not found, offer to create one.
  if (-1 == access (rcFile.c_str (), F_OK))
  {
    if (confirm (
          "A configuration file could not be found in " // TODO i18n
        + rcFile
        + "\n\n" // no i18n
        + "Would you like a sample .taskrc created, so task can proceed?")) // TODO i18n
    {
      // Create a sample .taskrc file.
      FILE* out;
      if ((out = fopen (rcFile.c_str (), "w")))
      {
        fprintf (out, "data.location=%s\n", dataDir.c_str ());           // TODO i18n
        fprintf (out, "confirmation=yes\n");                             // TODO i18n
        fprintf (out, "echo.command=yes\n");                             // TODO i18n
        fprintf (out, "next=2\n");                                       // TODO i18n
        fprintf (out, "dateformat=m/d/Y\n");                             // TODO i18n
        fprintf (out, "#monthsperline=2\n");                             // TODO i18n
        fprintf (out, "#defaultwidth=80\n");                             // TODO i18n
        fprintf (out, "curses=on\n");                                    // TODO i18n
        fprintf (out, "color=on\n");                                     // TODO i18n
        fprintf (out, "due=7\n");                                        // TODO i18n
        fprintf (out, "nag=You have higher priority tasks.\n");          // TODO i18n
        fprintf (out, "locking=on\n");                                   // TODO i18n
        fprintf (out, "#editor=vi\n");                                   // TODO i18n
        fprintf (out, "weekstart=Sunday\n");                             // TODO i18n
        fprintf (out, "displayweeknumber=yes\n");                        // TODO i18n

        fprintf (out, "color.overdue=bold_red\n");                       // TODO i18n
        fprintf (out, "color.due=bold_yellow\n");                        // TODO i18n
        fprintf (out, "color.pri.H=bold\n");                             // TODO i18n
        fprintf (out, "#color.pri.M=on_yellow\n");                       // TODO i18n
        fprintf (out, "#color.pri.L=on_green\n");                        // TODO i18n
        fprintf (out, "#color.pri.none=white on_blue\n");                // TODO i18n
        fprintf (out, "color.active=bold_cyan\n");                       // TODO i18n
        fprintf (out, "color.tagged=yellow\n");                          // TODO i18n
        fprintf (out, "#color.tag.bug=yellow\n");                        // TODO i18n
        fprintf (out, "#color.project.garden=on_green\n");               // TODO i18n
        fprintf (out, "#color.keyword.car=on_blue\n");                   // TODO i18n
        fprintf (out, "#color.recurring=on_red\n");                      // TODO i18n
        fprintf (out, "#shadow.file=%s/shadow.txt\n", dataDir.c_str ()); // TODO i18n
        fprintf (out, "#shadow.command=list\n");                         // TODO i18n
        fprintf (out, "#shadow.notify=on\n");                            // TODO i18n
        fprintf (out, "#default.project=foo\n");                         // TODO i18n
        fprintf (out, "#default.priority=M\n");                          // TODO i18n
        fprintf (out, "default.command=list\n");                         // TODO i18n

        // Custom reports.
        fprintf (out, "# Fields: id,uuid,project,priority,entry,start,due,recur,age,active,tags,description\n"); // TODO i18n
        fprintf (out, "#         description_only\n");                                                           // TODO i18n
        fprintf (out, "# Description:   This report is ...\n");                                                  // TODO i18n
        fprintf (out, "# Sort:          due+,priority-,project+\n");                                             // TODO i18n
        fprintf (out, "# Filter:        pro:x pri:H +bug\n");                                                    // TODO i18n
        fprintf (out, "# Limit:         10\n");                                                                  // TODO i18n

        fprintf (out, "report.long.description=Lists all task, all data, matching the specified criteria\n");    // TODO i18n
        fprintf (out, "report.long.labels=ID,Project,Pri,Added,Started,Due,Recur,Age,Tags,Description\n");       // TODO i18n
        fprintf (out, "report.long.columns=id,project,priority,entry,start,due,recur,age,tags,description\n");   // TODO i18n
        fprintf (out, "report.long.sort=due+,priority-,project+\n");                                             // TODO i18n

        fprintf (out, "report.list.description=Lists all tasks matching the specified criteria\n");              // TODO i18n
        fprintf (out, "report.list.labels=ID,Project,Pri,Due,Active,Age,Description\n");                         // TODO i18n
        fprintf (out, "report.list.columns=id,project,priority,due,active,age,description\n");                   // TODO i18n
        fprintf (out, "report.list.sort=due+,priority-,project+\n");                                             // TODO i18n
                                      // TODO i18n
        fprintf (out, "report.ls.description=Minimal listing of all tasks matching the specified criteria\n");   // TODO i18n
        fprintf (out, "report.ls.labels=ID,Project,Pri,Description\n");                                          // TODO i18n
        fprintf (out, "report.ls.columns=id,project,priority,description\n");                                    // TODO i18n
        fprintf (out, "report.ls.sort=priority-,project+\n");                                                    // TODO i18n

        fprintf (out, "report.newest.description=Shows the newest tasks\n");                                     // TODO i18n
        fprintf (out, "report.newest.labels=ID,Project,Pri,Due,Active,Age,Description\n");                       // TODO i18n
        fprintf (out, "report.newest.columns=id,project,priority,due,active,age,description\n");                 // TODO i18n
        fprintf (out, "report.newest.sort=id-\n");                                                               // TODO i18n
        fprintf (out, "report.newest.limit=10\n");                                                               // TODO i18n

        fprintf (out, "report.oldest.description=Shows the oldest tasks\n");                                     // TODO i18n
        fprintf (out, "report.oldest.labels=ID,Project,Pri,Due,Active,Age,Description\n");                       // TODO i18n
        fprintf (out, "report.oldest.columns=id,project,priority,due,active,age,description\n");                 // TODO i18n
        fprintf (out, "report.oldest.sort=id+\n");                                                               // TODO i18n
        fprintf (out, "report.oldest.limit=10\n");                                                               // TODO i18n

        fclose (out);

        std::cout << "Done." << std::endl;                                                                       // TODO i18n
      }
    }
  }

  this->load (rcFile);

  // Get the data.location value from the (potentially newly created) .taskrc
  // file.
  dataDir = this->get ("data.location", dataDir); // no i18n
  if (-1 == access (dataDir.c_str (), F_OK))
    mkdir (dataDir.c_str (), S_IRWXU);
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
    return ::atoi ((*this)[key].c_str ());

  return default_value;
}

////////////////////////////////////////////////////////////////////////////////
double Config::get (const std::string& key, const double default_value)
{
  if ((*this).find (key) != (*this).end ())
    return ::atof ((*this)[key].c_str ());

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
