////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2017, Paul Beckingham, Federico Hernandez.
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
#include <Config.h>
#include <Context.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <Datetime.h>
#include <FS.h>
#include <Timer.h>
#include <JSON.h>
#include <Lexer.h>
#include <shared.h>
#include <format.h>
#include <util.h>
#include <i18n.h>

extern Context context;

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
// Read the Configuration file and populate the *this map.  The file format is
// simply lines with name=value pairs.  Whitespace between name, = and value is
// not tolerated, but blank lines and comments starting with # are allowed.
//
// Nested files are now supported, with the following construct:
//   include /absolute/path/to/file
//
void Config::load (const std::string& file, int nest /* = 1 */)
{
  Timer timer;

  if (nest > 10)
    throw std::string (STRING_CONFIG_OVERNEST);

  // Capture the original name.
  _original_file = File (file);

  // Read the file, then parse the contents.
  std::string contents;
  if (File::read (file, contents) && contents.length ())
    parse (contents, nest);

  context.debugTiming (format ("Config::load ({1})", file), timer);
}

////////////////////////////////////////////////////////////////////////////////
void Config::parse (const std::string& input, int nest /* = 1 */)
{
  // Shortcut case for default constructor.
  if (input.length () == 0)
    return;

  // Split the input into lines.
  auto lines = split (input, '\n');

  // Parse each line.
  for (auto& line : lines)
  {
    // Remove comments.
    auto pound = line.find ("#"); // no i18n
    if (pound != std::string::npos)
      line = line.substr (0, pound);

    line = Lexer::trim (line, " \t"); // no i18n

    // Skip empty lines.
    if (line.length () > 0)
    {
      auto equal = line.find ("="); // no i18n
      if (equal != std::string::npos)
      {
        std::string key   = Lexer::trim (line.substr (0, equal), " \t"); // no i18n
        std::string value = Lexer::trim (line.substr (equal+1, line.length () - equal), " \t"); // no i18n

        (*this)[key] = json::decode (value);
      }
      else
      {
        auto include = line.find ("include"); // no i18n.
        if (include != std::string::npos)
        {
          Path included (Lexer::trim (line.substr (include + 7), " \t"));
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
bool Config::has (const std::string& key)
{
  return (*this).find (key) != (*this).end ();
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
std::string Config::get (const std::string& key)
{
  auto found = find (key);
  if (found != end ())
    return found->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int Config::getInteger (const std::string& key)
{
  auto found = find (key);
  if (found != end ())
    return strtoimax (found->second.c_str (), nullptr, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
double Config::getReal (const std::string& key)
{
  //NOTE: Backwards compatible handling of next coefficient.
  //TODO: Remove.
  if (key == "urgency.user.tag.next.coefficient" and has ("urgency.next.coefficient"))
    return getReal ("urgency.next.coefficient");

  auto found = find (key);
  if (found != end ())
    return strtod (found->second.c_str (), nullptr);

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
bool Config::getBoolean (const std::string& key)
{
  auto found = find (key);
  if (found != end ())
  {
    std::string value = Lexer::lowerCase ((*this)[key]);
    if (value == "t"      ||  // TODO Deprecate
        value == "true"   ||  // TODO Deprecate
        value == "1"      ||
        value == "+"      ||  // TODO Deprecate
        value == "y"      ||  // TODO Deprecate
        value == "yes"    ||  // TODO Deprecate
        value == "on"     ||  // TODO Deprecate
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
  for (auto& it : *this)
    items.push_back (it.first);
}

////////////////////////////////////////////////////////////////////////////////
