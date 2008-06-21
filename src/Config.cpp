///////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2008, Paul Beckingham.
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
#include "task.h"
#include "Config.h"

////////////////////////////////////////////////////////////////////////////////
Config::Config ()
{
}

////////////////////////////////////////////////////////////////////////////////
Config::Config (const std::string& file)
{
  load (file);
}

////////////////////////////////////////////////////////////////////////////////
// Read the Configuration filee and populate the *this map.  The file format
// is simply lines with name=value pairs.  Whitespace between name, = and value
// is not tolerated, but blank lines and comments starting with # are allowed.
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
      size_type pound = line.find ("#");
      if (pound != std::string::npos)
        line = line.substr (0, pound);

      line = trim (line, " \t");

      // Skip empty lines.
      if (line.length () > 0)
      {
        size_type equal = line.find ("=");
        if (equal != std::string::npos)
        {
          std::string key   = trim (line.substr (0, equal), " \t");
          std::string value = trim (line.substr (equal+1, line.length () - equal), " \t");
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
  std::string rcFile  = terminatedHome + "/.taskrc";
  std::string dataDir = terminatedHome + "/.task";;

  // If rcFile is not found, offer to create one.
  if (-1 == access (rcFile.c_str (), F_OK))
  {
    if (confirm (
          "A configuration file could not be found in "
        + rcFile
        + "\n\n"
        + "Would you like a sample .taskrc created, so task can proceed?"))
    {
      // Create a sample .taskrc file.
      FILE* out;
      if ((out = fopen (rcFile.c_str (), "w")))
      {
        fprintf (out, "data.location=%s\n", dataDir.c_str ());
        fprintf (out, "command.logging=off\n");
        fprintf (out, "confirmation=yes\n");
        fprintf (out, "next=2\n");
        fprintf (out, "dateformat=m/d/Y\n");
        fprintf (out, "showage=yes\n");
        fprintf (out, "monthsperline=1\n");
        fprintf (out, "curses=on\n");
        fprintf (out, "color=on\n");

        fprintf (out, "color.overdue=bold_red\n");
        fprintf (out, "#color.due=on_bright_yellow\n");
        fprintf (out, "#color.pri.H=on_red\n");
        fprintf (out, "#color.pri.M=on_yellow\n");
        fprintf (out, "#color.pri.L=on_green\n");
        fprintf (out, "color.active=bold_cyan\n");
        fprintf (out, "color.tagged=yellow\n");
        fprintf (out, "#color.tag.bug=yellow\n");
        fprintf (out, "#color.project.home=on_green\n");
        fprintf (out, "#color.keyword.car=on_blue\n");

        fclose (out);

        std::cout << "Done." << std::endl;
      }
    }
  }

  this->load (rcFile);

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
bool Config::get (const std::string& key, bool default_value)
{
  if ((*this).find (key) != (*this).end ())
  {
    std::string value = lowerCase ((*this)[key]);

    if (value == "t"    ||
        value == "true" ||
        value == "1"    ||
        value == "yes"  ||
        value == "on")
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
