///////////////////////////////////////////////////////////////////////////////
// Copyright 2005 - 2008, Paul Beckingham.  All rights reserved.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <sstream>
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
      unsigned int pound = line.find ("#");
      if (pound != std::string::npos)
        line = line.substr (0, pound);

      line = trim (line, " \t");

      // Skip empty lines.
      if (line.length () > 0)
      {
        unsigned int equal = line.find ("=");
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
