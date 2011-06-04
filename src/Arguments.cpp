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
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <sys/select.h>
#include <Context.h>
#include <Nibbler.h>
#include <Directory.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <Arguments.h>

extern Context context;

// Synonyms on the same line.
static const char* modifierNames[] =
{
  "before",     "under",    "below",
  "after",      "over",     "above",
  "none",
  "any",
  "is",         "equals",
  "isnt",       "not",
  "has",        "contains",
  "hasnt",
  "startswith", "left",
  "endswith",   "right",
  "word",
  "noword"
};

#define NUM_MODIFIER_NAMES   (sizeof (modifierNames)   / sizeof (modifierNames[0]))

////////////////////////////////////////////////////////////////////////////////
Arguments::Arguments ()
{
}

////////////////////////////////////////////////////////////////////////////////
Arguments::~Arguments ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Add a pair for every argument, with a category of "".
void Arguments::capture (int argc, const char** argv)
{
  for (int i = 0; i < argc; ++i)
    this->push_back (std::make_pair (argv[i], ""));

  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Add a pair with a category of "".
void Arguments::capture (const std::string& arg)
{
  this->push_back (std::make_pair (arg, ""));
  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Add a pair for every word from std::cin, with a category of "".
void Arguments::append_stdin ()
{
  bool something_happened = false;

  // Use 'select' to determine whether there is any std::cin content buffered
  // before trying to read it, to prevent blocking.
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
      // It the terminator token is found, stop reading.
      if (arg == "--")
        break;

      this->push_back (std::make_pair (arg, ""));
      something_happened = true;
    }
  }

  if (something_happened)
    categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Scan all the arguments, and assign a category.
void Arguments::categorize ()
{
  bool terminated = false;

  // Generate a vector of command keywords against which autoComplete can run.
  std::vector <std::string> keywords;
  std::map <std::string, Command*>::iterator k;
  for (k = context.commands.begin (); k != context.commands.end (); ++k)
    keywords.push_back (k->first);

  // First scan for a command.
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->first == "--")
      break;

    std::vector <std::string> matches;
    if (autoComplete (arg->first, keywords, matches) == 1)
    {
      if (arg->first != matches[0])
        context.debug ("Arguments::categorize keyword '" + arg->first + "' --> '" + matches[0] + "'");
      else
        context.debug ("Arguments::categorize keyword '" + arg->first + "'");

      // Not only categorize the command, but overwrite the original command
      // with the full command name.
      arg->first  = matches[0];
      arg->second = "command";

      // Only the first match is a command.
      break;
    }
  }

  // Now categorize every uncategorized argument.
  std::string ignored;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (!terminated)
    {
      // Nothing after -- is to be interpreted in any way.
      if (arg->first == "--")
      {
        terminated = true;
        arg->second = "terminator";
      }

      else if (arg->second != "command")
      {
        // program
        if (arg == this->begin ())
          arg->second = "program";

        // rc:<file>
        else if (arg->first.substr (0, 3) == "rc:")
          arg->second = "rc";

        // rc.<name>[:=]<value>
        else if (arg->first.substr (0, 3) == "rc.")
          arg->second = "override";

        // +tag
        // -tag
        else if (arg->first[0] == '+' ||
                 arg->first[0] == '-')
          arg->second = "tag";

        // /pattern/
        else if (is_pattern (arg->first))
          arg->second = "pattern";

        // 
        // <name>.<modifier>[:=]<value>
        else if (is_attmod (arg->first))
          arg->second = "attmod";

        // <name>[:=]<value>
        else if (is_attr (arg->first))
          arg->second = "attribute";

        // TODO Sequence
        // TODO UUID

        // /<from>/<to>/[g]
        else if (is_subst (arg->first))
          arg->second = "substitution";

        else if (arg->second == "")
          arg->second = "word";
      }
    }

    // All post-termination arguments are simply words.
    else
      arg->second = "word";
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::rc_override (
  std::string& home,
  File& rc,
  std::string& override)
{
  // Is there an override for rc:<file>?
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "rc")
    {
      override = arg->first;
      rc = File (arg->first.substr (3));
      home = rc;

      std::string::size_type last_slash = rc.data.rfind ("/");
      if (last_slash != std::string::npos)
        home = rc.data.substr (0, last_slash);
      else
        home = ".";

      context.header ("Using alternate .taskrc file " + rc.data); // TODO i18n

      // Keep scanning, because if there are multiple rc:file arguments, we
      // want the last one to dominate.
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
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "override")
    {
      if (arg->first.substr (0, 16) == "rc.data.location" &&
               (arg->first[16] == ':' || arg->first[16] == '='))
      {
        data = arg->first.substr (17);
        context.header ("Using alternate data.location " + data); // TODO i18n
      }
    }

    // Keep scanning, because if there are multiple rc:file arguments, we
    // want the last one to dominate.
  }
}

////////////////////////////////////////////////////////////////////////////////
// Extracts any rc.name:value args and sets the name/value in context.config,
// leaving only the plain args.
void Arguments::apply_overrides (std::string& var_overrides)
{
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "override")
    {
      std::string name;
      std::string value;
      Nibbler n (arg->first);
      if (n.getLiteral ("rc.")         &&  // rc.
          n.getUntilOneOf (":=", name) &&  //    xxx
          n.skipN (1))                     //       :
      {
        n.getUntilEOS (value);  // Don't care if it's blank.

        context.config.set (name, value);
        context.footnote ("Configuration override rc." + name + "=" + value);

        // Overrides are retained for potential use by the default command.
        var_overrides += " " + arg->first;
      }
      else
        context.footnote ("Problem with override: " + arg->first);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// An alias must be a distinct word on the command line.
// Aliases may not recurse.
void Arguments::resolve_aliases ()
{
  std::vector <std::string> expanded;
  bool something = false;

  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    std::map <std::string, std::string>::iterator match =
      context.aliases.find (arg->first);

    if (match != context.aliases.end ())
    {
      context.debug (std::string ("Arguments::resolve_aliases '")
                     + arg->first
                     + "' --> '"
                     + context.aliases[arg->first]
                     + "'");

      std::vector <std::string> words;
      splitq (words, context.aliases[arg->first], ' ');

      std::vector <std::string>::iterator word;
      for (word = words.begin (); word != words.end (); ++word)
        expanded.push_back (*word);

      something = true;
    }
    else
      expanded.push_back (arg->first);
  }

  // Only overwrite if something happened.
  if (something)
  {
    this->clear ();
    std::vector <std::string>::iterator e;
    for (e = expanded.begin (); e != expanded.end (); ++e)
      this->push_back (std::make_pair (*e, ""));

    categorize ();
  }
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Arguments::list ()
{
  std::vector <std::string> all;
  std::vector <std::pair <std::string, std::string> >::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    all.push_back (i->first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
std::string Arguments::combine ()
{
  std::string combined;

  std::vector <std::pair <std::string, std::string> >::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    if (i != this->begin ())
      combined += " ";

    combined += i->first;
  }

  return combined;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::find_command (std::string& command)
{
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "command")
    {
      command = arg->first;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//                    ______________
//                    |            |
//                    |            v
// start --> name --> : --> " --> value --> " --> end
//                                   |             ^
//                                   |_____________|
//
bool Arguments::is_attr (const std::string& input)
{
  Nibbler n (input);

  // Ensure a clean parse.
  std::string name;
  std::string value;

  if (n.getUntilOneOf ("=:", name))
  {
    if (n.skip (':') ||
        n.skip ('='))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value) ||
          n.getUntilEOS (value))
      {
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//                                  ______________
//                                  |            |
//                                  |            v
// start --> name --> . --> mod --> : --> " --> value --> " --> end
//            |                     ^              |             ^
//            |_____________________|              |_____________|
//
bool Arguments::is_attmod (const std::string& input)
{
  Nibbler n (input);

  // Ensure a clean parse.
  std::string ignored;

  if (n.getUntil (".", ignored))
  {
    if (n.skip ('.'))
    {
      n.skip ('~');
      n.getUntilOneOf (":=", ignored);
    }

    if (n.skip (':') ||
        n.skip ('='))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', ignored) ||
          n.getUntilEOS (ignored))
      {
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// /<from>/<to>/[g]
bool Arguments::is_subst (const std::string& input)
{
  std::string ignored;
  Nibbler n (input);
  if (n.skip     ('/')            &&
      n.getUntil ('/', ignored)   &&
      n.skip     ('/')            &&
      n.getUntil ('/', ignored)   &&
      n.skip     ('/'))
  {
    n.skip ('g');
    if (n.depleted ())
      return ! Directory (input).exists ();   // Ouch - expensive call.
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// /<pattern>/
bool Arguments::is_pattern (const std::string& input)
{
  if (input[0] == '/'     &&
      input.length () > 2 &&
      input[input.length () - 1] == '/')
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//                    ______________
//                    |            |
//                    |            v
// start --> name --> : --> " --> value --> " --> end
//                                   |             ^
//                                   |_____________|
//
bool Arguments::extract_attr (
  const std::string& input,
  std::string& name,
  std::string& value)
{
  Nibbler n (input);

  // Ensure a clean parse.
  name  = "";
  value = "";

  if (n.getUntilOneOf ("=:", name))
  {
    if (name.length () == 0)
      throw std::string ("Missing attribute name"); // TODO i18n

    if (n.skip (':') ||
        n.skip ('='))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value) ||
          n.getUntilEOS (value))
      {
        return true;
      }
    }
    else
      throw std::string ("Missing : after attribute name."); // TODO i18n
  }
  else
    throw std::string ("Missing : after attribute name."); // TODO i18n

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//                                  ______________
//                                  |            |
//                                  |            v
// start --> name --> . --> mod --> : --> " --> value --> " --> end
//                                                 |             ^
//                                                 |_____________|
//
bool Arguments::extract_attmod (
  const std::string& input,
  std::string& name,
  std::string& modifier,
  std::string& value,
  std::string& sense)
{
  Nibbler n (input);

  // Ensure a clean parse.
  name     = "";
  value    = "";
  modifier = "";
  sense    = "positive";

  if (n.getUntil (".", name))
  {
    if (name.length () == 0)
      throw std::string ("Missing attribute name"); // TODO i18n

    if (n.skip ('.'))
    {
      if (n.skip ('~'))
        sense = "negative";

      if (n.getUntilOneOf (":=", modifier))
      {
        if (!valid_modifier (modifier))
          throw std::string ("The name '") + modifier + "' is not a valid modifier."; // TODO i18n
      }
      else
        throw std::string ("Missing . or : after modifier."); // TODO i18n
    }
    else
      throw std::string ("Missing modifier."); // TODO i18n

    if (n.skip (':') ||
        n.skip ('='))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value) ||
          n.getUntilEOS (value))
      {
        return true;
      }
    }
    else
      throw std::string ("Missing : after attribute name."); // TODO i18n
  }
  else
    throw std::string ("Missing : after attribute name."); // TODO i18n

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::extract_subst (
  const std::string& input,
  std::string& from,
  std::string& to,
  bool& global)
{
  Nibbler n (input);
  if (n.skip     ('/')       &&
      n.getUntil ('/', from) &&
      n.skip     ('/')       &&
      n.getUntil ('/', to)   &&
      n.skip     ('/'))
  {
    global = n.skip ('g');

    if (from == "")
      throw std::string ("Cannot substitute an empty string.");

    if (!n.depleted ())
      throw std::string ("Unrecognized character(s) at end of substitution.");

    return true;
  }
  else
    throw std::string ("Malformed substitution.");

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::extract_pattern (const std::string& input, std::string& pattern)
{
  Nibbler n (input);
  if (n.skip     ('/')          &&
      n.getUntil ('/', pattern) &&
      n.skip     ('/'))
  {
    if (pattern == "")
      throw std::string ("Cannot search for an empty pattern.");

    if (!n.depleted ())
      throw std::string ("Unrecognized character(s) at end of pattern.");

    return true;
  }
  else
    throw std::string ("Malformed pattern.");

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::valid_modifier (const std::string& modifier)
{
  for (unsigned int i = 0; i < NUM_MODIFIER_NAMES; ++i)
    if (modifierNames[i] == modifier)
      return true;

  return false;
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
/*
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
*/

////////////////////////////////////////////////////////////////////////////////
void Arguments::dump (const std::string& label)
{
  // Set up a color mapping.
  std::map <std::string, Color> color_map;
  color_map["program"]      = Color ("white on blue");
  color_map["command"]      = Color ("black on cyan");
  color_map["rc"]           = Color ("bold white on red");
  color_map["override"]     = Color ("white on red");
  color_map["tag"]          = Color ("green on gray3");
  color_map["pattern"]      = Color ("cyan on gray3");
  color_map["attribute"]    = Color ("bold red on gray3");
  color_map["attmod"]       = Color ("bold red on gray3");
  color_map["sequence"]     = Color ("yellow on gray3");
  color_map["uuid"]         = Color ("yellow on gray3");
  color_map["substitution"] = Color ("bold cyan on gray3");
  color_map["none"]         = Color ("white on gray3");

  Color color_debug (context.config.get ("color.debug"));
  std::stringstream out;
  out << color_debug.colorize (label)
      << "\n";

  ViewText view;
  view.width (context.getWidth ());
  view.leftMargin (4);
  for (unsigned int i = 0; i < this->size (); ++i)
    view.add (Column::factory ("string", ""));

  view.addRow ();
  view.addRow ();

  for (unsigned int i = 0; i < this->size (); ++i)
  {
    std::string arg      = (*this)[i].first;
    std::string category = (*this)[i].second;

    Color c;
    if (color_map[category].nontrivial ())
      c = color_map[category];
    else
      c = color_map["none"];

    view.set (0, i, arg,      c);
    view.set (1, i, category, c);
  }

  out << view.render ();
  context.debug (out.str ());
  std::cout << out.str (); // TODO Remove
}

////////////////////////////////////////////////////////////////////////////////
