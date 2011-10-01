////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2011, Paul Beckingham, Federico Hernandez.
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

#define L10N                                           // Localization complete.

#include <sstream>
#include <algorithm>
#include <Context.h>
#include <i18n.h>
#include <text.h>
#include <util.h>
#include <CmdConfig.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdConfig::CmdConfig ()
{
  _keyword     = "config";
  _usage       = "task          config [name [value | '']]";
  _description = STRING_CMD_CONFIG_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdConfig::execute (std::string& output)
{
  int rc = 0;
  std::stringstream out;

  // Get the non-attribute, non-fancy command line arguments.
  std::vector <std::string> words = context.a3.extract_words ();

  // Support:
  //   task config name value    # set name to value
  //   task config name ""       # set name to blank
  //   task config name          # remove name
  if (words.size ())
  {
    std::string name = words[0];
    std::string value = "";

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

      // Read .taskrc (or equivalent)
      std::vector <std::string> contents;
      File::read (context.config._original_file, contents);

      // task config name value
      // task config name ""
      if (words.size () > 1)
      {
        bool found = false;
        std::vector <std::string>::iterator line;
        for (line = contents.begin (); line != contents.end (); ++line)
        {
          // If there is a comment on the line, it must follow the pattern.
          std::string::size_type comment = line->find ("#");
          std::string::size_type pos     = line->find (name + "=");

          if (pos != std::string::npos &&
              (comment == std::string::npos ||
               comment > pos))
          {
            found = true;
            if (confirm (format (STRING_CMD_CONFIG_CONFIRM, name, context.config.get (name), value)))
            {
              if (comment != std::string::npos)
                *line = name + "=" + value + " " + line->substr (comment);
              else
                *line = name + "=" + value;

              change = true;
            }
          }
        }

        // Not found, so append instead.
        if (!found &&
            confirm (format (STRING_CMD_CONFIG_CONFIRM2, name, value)))
        {
          contents.push_back (name + "=" + value);
          change = true;
        }
      }

      // task config name
      else
      {
        bool found = false;
        std::vector <std::string>::iterator line;
        for (line = contents.begin (); line != contents.end (); ++line)
        {
          // If there is a comment on the line, it must follow the pattern.
          std::string::size_type comment = line->find ("#");
          std::string::size_type pos     = line->find (name + "=");

          if (pos != std::string::npos &&
              (comment == std::string::npos ||
               comment > pos))
          {
            found = true;

            // Remove name
            if (confirm (format (STRING_CMD_CONFIG_CONFIRM3, name)))
            {
              *line = "";
              change = true;
            }
          }
        }

        if (!found)
          throw format (STRING_CMD_CONFIG_NO_ENTRY, name);
      }

      // Write .taskrc (or equivalent)
      if (change)
      {
        File::write (context.config._original_file, contents);
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
  _keyword     = "_config";
  _usage       = "task          _config";
  _description = STRING_CMD_HCONFIG_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdCompletionConfig::execute (std::string& output)
{
  std::vector <std::string> configs;
  context.config.all (configs);
  std::sort (configs.begin (), configs.end ());

  std::vector <std::string>::iterator config;
  for (config = configs.begin (); config != configs.end (); ++config)
    output += *config + "\n";

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
