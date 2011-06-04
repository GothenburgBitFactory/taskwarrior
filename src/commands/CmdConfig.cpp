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

#include <sstream>
#include <algorithm>
#include <Context.h>
#include <text.h>
#include <util.h>
#include <CmdConfig.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdConfig::CmdConfig ()
{
  _keyword     = "config";
  _usage       = "task config [name [value | '']]";
  _description = "Add, modify and remove settings in the task configuration.";
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdConfig::execute (std::string& output)
{
  int rc = 0;
/*
  TODO Revise argument handling

  std::stringstream out;

  // Obtain the arguments from the description.  That way, things like '--'
  // have already been handled.
  std::vector <std::string> args;
  split (args, context.task.get ("description"), ' ');

  // Support:
  //   task config name value    # set name to value
  //   task config name ""       # set name to blank
  //   task config name          # remove name
  if (args.size () > 0)
  {
    std::string name = args[0];
    std::string value = "";

    if (args.size () > 1)
    {
      for (unsigned int i = 1; i < args.size (); ++i)
      {
        if (i > 1)
          value += " ";

        value += args[i];
      }
    }

    if (name != "")
    {
      bool change = false;

      // Read .taskrc (or equivalent)
      std::string contents;
      File::read (context.config.original_file, contents);

      // task config name value
      // task config name ""
      if (args.size () > 1 ||
          context.args[context.args.size () - 1] == "")
      {
        // Find existing entry & overwrite
        std::string::size_type pos = contents.find (name + "=");
        if (pos != std::string::npos)
        {
          std::string::size_type eol = contents.find_first_of ("\r\f\n", pos);
          if (eol == std::string::npos)
            throw std::string ("Cannot find EOL after entry '") + name + "'.";

          if (confirm (std::string ("Are you sure you want to change the value of '")
                         + name
                         + "' from '"
                         + context.config.get(name)
                         + "' to '"
                         + value + "'?"))
          {
            contents = contents.substr (0, pos)
                     + name + "=" + value
                     + contents.substr (eol);
            change = true;
          }
        }

        // Not found, so append instead.
        else
        {
          if (confirm (std::string ("Are you sure you want to add '") + name + "' with a value of '" + value + "'?"))
          {
            contents = contents
                     + "\n"
                     + name + "=" + value
                     + "\n";
            change = true;
          }
        }
      }

      // task config name
      else
      {
        // Remove name
        std::string::size_type pos = contents.find (name + "=");
        if (pos == std::string::npos)
          throw std::string ("No entry named '") + name + "' found.";

        std::string::size_type eol = contents.find_first_of ("\r\f\n", pos);
        if (eol == std::string::npos)
          throw std::string ("Cannot find EOL after entry '") + name + "'.";

        if (confirm (std::string ("Are you sure you want to remove '") + name + "'?"))
        {
          contents = contents.substr (0, pos) + contents.substr (eol + 1);
          change = true;
        }
      }

      // Write .taskrc (or equivalent)
      if (change)
      {
        File::write (context.config.original_file, contents);
        out << "Config file "
            << context.config.original_file.data
            << " modified.\n";
      }
      else
        out << "No changes made.\n";
    }
    else
      throw std::string ("Specify the name of a config variable to modify.");
    output = out.str ();
  }
  else
    throw std::string ("Specify the name of a config variable to modify.");
*/

  return rc;
}

////////////////////////////////////////////////////////////////////////////////
CmdCompletionConfig::CmdCompletionConfig ()
{
  _keyword     = "_config";
  _usage       = "task _config";
  _description = "Lists all supported configuration variables, for completion "
                 "purposes.";
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
