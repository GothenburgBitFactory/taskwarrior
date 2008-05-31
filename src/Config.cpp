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
#include <sys/stat.h>
#include <stdlib.h>
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
void Config::createDefault (const std::string& file)
{
  if (confirm (
        "A configuration file could not be found in "
      + file
      + "\n\n"
      + "Would you like a sample .taskrc created, so task can proceed?"))
  {
    // Determine a path to the task directory.
    std::string taskDir = "";
    for (int i = file.length () - 1; i >= 0; --i)
    {
      if (file[i] == '/')
      {
        taskDir = file.substr (0, i) + "/.task";
        if (-1 == access (taskDir.c_str (), F_OK))
          mkdir (taskDir.c_str (), S_IRWXU);
        break;
      }
    }

    if (taskDir != "")
    {
      FILE* out;
      if ((out = fopen (file.c_str (), "w")))
      {
        fprintf (out, "data.location=%s\n", taskDir.c_str ());
        fprintf (out, "command.logging=off\n");
        fprintf (out, "confirmation=yes\n");
        fprintf (out, "#nag=Note: try to stick to high priority tasks.  See \"task next\".\n");
        fprintf (out, "next=2\n");
        fprintf (out, "curses=on\n");
        fprintf (out, "color=on\n");

        fprintf (out, "color.overdue=bold_red\n");
        fprintf (out, "#color.due=on_bright_yellow\n");
        fprintf (out, "#color.pri.H=on_red\n");
        fprintf (out, "#color.pri.M=on_yellow\n");
        fprintf (out, "#color.pri.L=on_green\n");
        fprintf (out, "color.active=bold_cyan\n");
        fprintf (out, "color.tagged=yellow\n");

        fclose (out);

        set ("data.location", taskDir);
        set ("command.logging", "off");
        set ("confirmation", "yes");
        set ("next", 2);
        set ("curses", "on");
        set ("color", "on");
        set ("color.overdue", "red");
        set ("color.active", "cyan");
        set ("color.tagged", "yellow");

        std::cout << "Done." << std::endl;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
const std::string& Config::get (const char* key)
{
  return this->get (std::string (key));
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.  If a default_value
// is present, it will be the returned value in the event of a missing key.
const std::string& Config::get (
  const char* key,
  const char* default_value)
{
  return this->get (std::string (key), std::string (default_value));
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
const std::string& Config::get (const std::string& key)
{
  return (*this)[key];
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.  If a default_value
// is present, it will be the returned value in the event of a missing key.
const std::string& Config::get (
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
// The vector form of Config::get assumes the single value is comma-separated,
// and splits accordingly.
void Config::get (
  const std::string& key,
  std::vector <std::string>& values)
{
  values.clear ();
  split (values, (*this)[key], ',');
}

////////////////////////////////////////////////////////////////////////////////
// The vector form of Config::set joins the values together with commas, and
// stores the single value.
void Config::set (
  const std::string& key,
  const std::vector <std::string>& values)
{
  std::string conjoined;
  join (conjoined, ",", values);
  (*this)[key] = conjoined;
}

////////////////////////////////////////////////////////////////////////////////
// Provide a vector of all configuration keys.
void Config::all (std::vector<std::string>& items)
{
  foreach (i, *this)
    items.push_back (i->first);
}

////////////////////////////////////////////////////////////////////////////////
