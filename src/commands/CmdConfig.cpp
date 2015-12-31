////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <CmdConfig.h>
#include <sstream>
#include <algorithm>
#include <Context.h>
#include <JSON.h>
#include <i18n.h>
#include <text.h>
#include <util.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdConfig::CmdConfig ()
{
  _keyword               = "config";
  _usage                 = "task          config [name [value | '']]";
  _description           = STRING_CMD_CONFIG_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::config;
}

////////////////////////////////////////////////////////////////////////////////
bool CmdConfig::setConfigVariable (std::string name, std::string value, bool confirmation /* = false */)
{
  // Read .taskrc (or equivalent)
  std::vector <std::string> contents;
  File::read (context.config._original_file, contents);

  bool found = false;
  bool change = false;

  for (auto& line : contents)
  {
    // If there is a comment on the line, it must follow the pattern.
    auto comment = line.find ("#");
    auto pos     = line.find (name + "=");

    if (pos != std::string::npos &&
        (comment == std::string::npos ||
         comment > pos))
    {
      found = true;
      if (!confirmation ||
          confirm (format (STRING_CMD_CONFIG_CONFIRM, name, context.config.get (name), value)))
      {
        if (comment != std::string::npos)
          line = name + "=" + json::encode (value) + " " + line.substr (comment);
        else
          line = name + "=" + json::encode (value);

        change = true;
      }
    }
  }

  // Not found, so append instead.
  if (!found &&
      (!confirmation ||
       confirm (format (STRING_CMD_CONFIG_CONFIRM2, name, value))))
  {
    contents.push_back (name + "=" + json::encode (value));
    change = true;
  }

  if (change)
    File::write (context.config._original_file, contents);

  return change;
}

////////////////////////////////////////////////////////////////////////////////
int CmdConfig::unsetConfigVariable (std::string name, bool confirmation /* = false */)
{
  // Read .taskrc (or equivalent)
  std::vector <std::string> contents;
  File::read (context.config._original_file, contents);

  bool found = false;
  bool change = false;

  for (auto line = contents.begin (); line != contents.end (); )
  {
    bool lineDeleted = false;

    // If there is a comment on the line, it must follow the pattern.
    auto comment = line->find ("#");
    auto pos     = line->find (name + "=");

    if (pos != std::string::npos &&
        (comment == std::string::npos ||
         comment > pos))
    {
      found = true;

      // Remove name
      if (!confirmation ||
          confirm (format (STRING_CMD_CONFIG_CONFIRM3, name)))
      {
        // vector::erase method returns a valid iterator to the next object
        line = contents.erase (line);
        lineDeleted = true;
        change = true;
      }
    }

    if (! lineDeleted)
      line++;
  }

  if (change)
    File::write (context.config._original_file, contents);

  if ( change && found )
    return 0;
  else if ( found )
    return 1;
  else
    return 2;
}

////////////////////////////////////////////////////////////////////////////////
int CmdConfig::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Get the non-attribute, non-fancy command line arguments.
  std::vector <std::string> words = context.cli2.getWords ();

  // Support:
  //   task config name value    # set name to value
  //   task config name ""       # set name to blank
  //   task config name          # remove name
  if (words.size ())
  {
    bool confirmation = context.config.getBoolean ("confirmation");
    bool found = false;

    std::string name = words[0];
    std::string value = "";

    // Join the remaining words into config variable's value
    if (words.size () > 1)
    {
      for (unsigned int i = 1; i < words.size (); ++i)
      {
        if (i > 1)
          value += " ";

        value += words[i];
      }
    }

    if (name != "")
    {
      bool change = false;

      // task config name value
      // task config name ""
      if (words.size () > 1)
        change = setConfigVariable(name, value, confirmation);

      // task config name
      else
      {
        rc = unsetConfigVariable(name, confirmation);
        if (rc == 0)
        {
          change = true;
          found = true;
        }
        else if (rc == 1)
          found = true;

        if (!found)
          throw format (STRING_CMD_CONFIG_NO_ENTRY, name);
      }

      // Show feedback depending on whether .taskrc has been rewritten
      if (change)
      {
        out << format (STRING_CMD_CONFIG_FILE_MOD,
                       context.config._original_file._data)
            << "\n";
      }
      else
        out << STRING_CMD_CONFIG_NO_CHANGE << "\n";
    }
    else
      throw std::string (STRING_CMD_CONFIG_NO_NAME);

    output = out.str ();
  }
  else
    throw std::string (STRING_CMD_CONFIG_NO_NAME);

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionConfig::CmdCompletionConfig ()
{
  _keyword               = "_config";
  _usage                 = "task          _config";
  _description           = STRING_CMD_HCONFIG_USAGE;
  _read_only             = true;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = false;
  _category              = Command::Category::internal;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionConfig::execute (std::string& output)
{
  std::vector <std::string> configs;
  context.config.all (configs);
  std::sort (configs.begin (), configs.end ());

  for (auto& config : configs)
    output += config + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
