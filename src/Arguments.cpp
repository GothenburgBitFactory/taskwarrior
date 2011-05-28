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

#include <iostream>
#include <sys/select.h>
#include <Context.h>
#include <Nibbler.h>
#include <text.h>
#include <Arguments.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Arguments::Arguments ()
{
}

////////////////////////////////////////////////////////////////////////////////
Arguments::~Arguments ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::capture (int argc, char** argv)
{
  for (int i = 0; i < argc; ++i)
  {
/*
    if (i == 0)
    {
      std::string::size_type cal = context.program.find ("/cal");
      if (context.program == "cal" ||
          (cal != std::string::npos && context.program.length () == cal + 4))
        this->push_back ("calendar");
    }
    else
*/
      this->push_back (argv[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::append_stdin ()
{
  // Capture any stdin args.
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO (&fds);
  FD_SET (STDIN_FILENO, &fds);
  select (STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
  if (FD_ISSET (0, &fds))
  {
    std::string arg;
    while (std::cin >> arg)
    {
      if (arg == "--")
        break;

      this->push_back (arg);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::rc_override (
  std::string& home,
  File& rc,
  std::string& override)
{
  // Is there an override for rc:<file>?
  std::vector <std::string>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    // Nothing after -- is to be interpreted in any way.
    if (*arg == "--")
      break;

    else if (arg->substr (0, 3) == "rc:")
    {
      override = *arg;
      rc = File (arg->substr (3));
      home = rc;

      std::string::size_type last_slash = rc.data.rfind ("/");
      if (last_slash != std::string::npos)
        home = rc.data.substr (0, last_slash);
      else
        home = ".";

      this->erase (arg);
      context.header ("Using alternate .taskrc file " + rc.data); // TODO i18n
      break; // Must break - iterator is dead.
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::get_data_location (std::string& data)
{
  std::string location = context.config.get ("data.location");
  if (location != "")
    data = location;

  // Are there any overrides for data.location?
  std::vector <std::string>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (*arg == "--")
      break;
    else if (arg->substr (0, 16) == "rc.data.location" &&
             ((*arg)[16] == ':' || (*arg)[16] == '='))
    {
      data = arg->substr (17);
      context.header ("Using alternate data.location " + data); // TODO i18n
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Extracts any rc.name:value args and sets the name/value in context.config,
// leaving only the plain args.
void Arguments::apply_overrides (std::string& var_overrides)
{
  std::vector <std::string> filtered;
  bool foundTerminator = false;

  std::vector <std::string>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (*arg == "--")
    {
      foundTerminator = true;
      filtered.push_back (*arg);
    }
    else if (!foundTerminator && arg->substr (0, 3) == "rc.")
    {
      std::string name;
      std::string value;
      Nibbler n (*arg);
      if (n.getLiteral ("rc.")         &&  // rc.
          n.getUntilOneOf (":=", name) &&  //    xxx
          n.skipN (1))                     //       :
      {
        n.getUntilEOS (value);  // Don't care if it's blank.

        context.config.set (name, value);
        context.footnote ("Configuration override " + arg->substr (3));

        // Overrides are retained for potential use by the default command.
        var_overrides += " " + *arg;
      }
      else
        context.footnote ("Problem with override: " + *arg);
    }
    else
      filtered.push_back (*arg);
  }

  // Overwrite args with the filtered subset.
  this->clear ();
  for (arg = filtered.begin (); arg != filtered.end (); ++arg)
    this->push_back (*arg);
}

////////////////////////////////////////////////////////////////////////////////
// An alias must be a distinct word on the command line.
// Aliases may not recurse.
void Arguments::resolve_aliases ()
{
  std::vector <std::string> expanded;
  bool something = false;

  std::vector <std::string>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    std::map <std::string, std::string>::iterator match =
      context.aliases.find (*arg);

    if (match != context.aliases.end ())
    {
      context.debug (std::string ("Arguments::resolve_aliases '")
                     + *arg
                     + "' --> '"
                     + context.aliases[*arg]
                     + "'");

      std::vector <std::string> words;
      splitq (words, context.aliases[*arg], ' ');

      std::vector <std::string>::iterator word;
      for (word = words.begin (); word != words.end (); ++word)
        expanded.push_back (*word);

      something = true;
    }
    else
      expanded.push_back (*arg);
  }

  // Only overwrite if something happened.
  if (something)
  {
    this->clear ();
    for (arg = expanded.begin (); arg != expanded.end (); ++arg)
      this->push_back (*arg);
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string Arguments::combine ()
{
  std::string combined;
  join (combined, " ", *(std::vector <std::string>*)this);
  return combined;
}

////////////////////////////////////////////////////////////////////////////////
