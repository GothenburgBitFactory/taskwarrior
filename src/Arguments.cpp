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
#include <stdlib.h>
#include <sys/select.h>
#include <Context.h>
#include <Nibbler.h>
#include <text.h>
#include <util.h>
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
void Arguments::capture (int argc, const char** argv)
{
  for (int i = 0; i < argc; ++i)
    if (i > 0)
      this->push_back (argv[i]);
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
// Given a vector of command keywords, scan all arguments and locate the first
// argument that matches a keyword.
bool Arguments::extract_command (
  const std::vector <std::string>& keywords,
  std::string& command)
{
  std::vector <std::string>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    std::vector <std::string> matches;
    if (autoComplete (*arg, keywords, matches) == 1)
    {
      if (*arg != matches[0])
        context.debug ("Arguments::extract_command keyword '" + *arg + "' --> '" + matches[0] + "'");
      else
        context.debug ("Arguments::extract_command keyword '" + *arg + "'");

      command = matches[0];
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::remove_command (const std::string& command)
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_filter ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_modifications ()
{
}

////////////////////////////////////////////////////////////////////////////////
// A sequence can be:
//
//   a single ID:          1
//   a list of IDs:        1,3,5
//   a list of IDs:        1 3 5
//   a range:              5-10
//   or a combination:     1,3,5-10 12
//
// If a sequence is followed by a non-number, then subsequent numbers are not
// interpreted as IDs.  For example:
//
//    1 2 three 4
//
// The sequence is "1 2".
//
// The first number found in the command line is assumed to be a sequence.  If
// there are two sequences, only the first is recognized, for example:
//
//    1,2 three 4,5
//
// The sequence is "1,2".
//
void Arguments::extract_sequence (std::vector <int>& sequence)
{
  sequence.clear ();
  std::vector <int> kill;

  bool terminated = false;
  for (unsigned int i = 0; i < this->size (); ++i)
  {
    if (!terminated)
    {
      bool something = false;

      // The '--' argument shuts off all parsing - everything is an argument.
      if ((*this)[i] == "--")
      {
        terminated = true;
      }
      else
      {
        if (isdigit ((*this)[i][0]))
        {
          std::vector <std::string> ranges;
          split (ranges, (*this)[i], ',');

          std::vector <std::string>::iterator it;
          for (it = ranges.begin (); it != ranges.end (); ++it)
          {
            std::vector <std::string> range;
            split (range, *it, '-');

            if (range.size () == 1)
            {
              if (! digitsOnly (range[0]))
                throw std::string ("Invalid ID in sequence.");

              int id = (int)strtol (range[0].c_str (), NULL, 10);
              sequence.push_back (id);
              something = true;
            }
            else if (range.size () == 2)
            {
              if (! digitsOnly (range[0]) ||
                  ! digitsOnly (range[1]))
                throw std::string ("Invalid ID in range.");

              int low  = (int)strtol (range[0].c_str (), NULL, 10);
              int high = (int)strtol (range[1].c_str (), NULL, 10);
              if (low > high)
                throw std::string ("Inverted sequence range high-low.");

              // TODO Is this meaningful?
              if (high - low >= ARGUMENTS_SEQUENCE_MAX_RANGE)
                throw std::string ("ID Range too large.");

              for (int r = low; r <= high; ++r)
                sequence.push_back (r);

              something = true;
            }

            // Not a properly formed sequence, therefore probably text.
            else
              break;
          }
        }

        // Once a sequence has been found, any non-numeric arguments effectively
        // terminate sequence processing.
        else if (sequence.size ())
          terminated = true;
      }

      if (something)
        kill.push_back (i);
    }
  }

  // Now remove args in the kill list.
  for (unsigned int k = 0; k < kill.size (); ++k)
    this->erase (this->begin () + kill[k]);
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_uuids (std::vector <std::string>& uuids)
{
  uuids.clear ();

}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_attrs ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_words ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_tags ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_pattern ()
{
}

////////////////////////////////////////////////////////////////////////////////
// TODO
void Arguments::extract_subst ()
{
}

////////////////////////////////////////////////////////////////////////////////
