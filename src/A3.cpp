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
#include <Directory.h>
#include <Date.h>
#include <Duration.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <A3.h>

extern Context context;

// Supported modifiers, synonyms on the same line.
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

// Supported operators, borrowed from C++, particularly the precedence.
// Note: table is sorted by length of operator string, so searches match
//       longest first.
static struct
{
  std::string op;
  int         precedence;
  char        type;
  int         symbol;
  char        associativity;
} operators[] =
{
  // Operator  Precedence  Type  Symbol  Associativity
  {  "and",     5,         'b',  0,      'l' },    // Conjunction
  {  "xor",     4,         'b',  0,      'l' },    // Disjunction

  {  "or",      3,         'b',  0,      'l' },    // Disjunction
  {  "<=",     10,         'b',  1,      'l' },    // Less than or equal
  {  ">=",     10,         'b',  1,      'l' },    // Greater than or equal
  {  "!~",      9,         'b',  1,      'l' },    // Regex non-match
  {  "!=",      9,         'b',  1,      'l' },    // Inequal

  {  "=",       9,         'b',  1,      'l' },    // Equal
//  {  "^",      16,         'b',  1,      'r' },    // Exponent
  {  ">",      10,         'b',  1,      'l' },    // Greater than
  {  "~",       9,         'b',  1,      'l' },    // Regex match
  {  "!",      15,         'u',  1,      'r' },    // Not
//  {  "-",      15,         'u',  1,      'r' },    // Unary minus
  {  "*",      13,         'b',  1,      'l' },    // Multiplication
  {  "/",      13,         'b',  1,      'l' },    // Division
//  {  "%",      13,         'b',  1,      'l' },    // Modulus
  {  "+",      12,         'b',  1,      'l' },    // Addition
  {  "-",      12,         'b',  1,      'l' },    // Subtraction
  {  "<",      10,         'b',  1,      'l' },    // Less than
  {  "(",       0,         'b',  1,      'l' },    // Precedence start
  {  ")",       0,         'b',  1,      'l' },    // Precedence end
};

#define NUM_MODIFIER_NAMES       (sizeof (modifierNames) / sizeof (modifierNames[0]))
#define NUM_OPERATORS            (sizeof (operators) / sizeof (operators[0]))

//static const char* non_word_chars = " +-*/%()=<>!~";

////////////////////////////////////////////////////////////////////////////////
A3::A3 ()
: _read_only_command (true)
{
}

////////////////////////////////////////////////////////////////////////////////
A3::~A3 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Add an Arg with a blank category for every argv.
void A3::capture (int argc, const char** argv)
{
  for (int i = 0; i < argc; ++i)
    this->push_back (Arg (argv[i], ""));
}

////////////////////////////////////////////////////////////////////////////////
// Append an Arg with a blank category.
void A3::capture (const std::string& arg)
{
  std::vector <std::string> parts;
    this->push_back (Arg (arg, ""));
}

////////////////////////////////////////////////////////////////////////////////
// Prepend a Arg with a blank category.
void A3::capture_first (const std::string& arg)
{
  // Break the new argument into parts that comprise a series.
  std::vector <Arg> series;
  series.push_back (Arg (arg, ""));

  // Locate an appropriate place to insert the series.  This would be
  // immediately after the program and command arguments.
  std::vector <Arg>::iterator position;
  for (position = this->begin (); position != this->end (); ++position)
    if (position->_category != "program" &&
        position->_category != "command")
      break;

  this->insert (position, series.begin (), series.end ());
}

////////////////////////////////////////////////////////////////////////////////
// Scan all arguments and categorize them as:
//   program
//   rc
//   override
//   command
//   terminator
//   word
// 
void A3::categorize ()
{
  bool terminated    = false;
  bool found_command = false;

  // Generate a vector of command keywords against which autoComplete can run.
  std::vector <std::string> keywords = context.getCommands ();

  // Now categorize every argument.
  std::vector <Arg>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (!terminated)
    {
      // Nothing after -- is to be interpreted in any way.
      if (arg->_raw == "--")
      {
        terminated = true;
        arg->_category = "terminator";
      }

      // program
      else if (arg == this->begin ())
      {
        arg->_category = "program";

        if ((arg->_raw.length () >= 3 &&
             arg->_raw.substr (arg->_raw.length () - 3) == "cal") ||
            (arg->_raw.length () >= 8 &&
             arg->_raw.substr (arg->_raw.length () - 8) == "calendar"))
        {
          arg->_raw = "calendar";
          arg->_category = "command";
          found_command = true;
        }
      }

      // command
      else if (!found_command &&
               is_command (keywords, arg->_raw))
      {
        found_command = true;
        arg->_category = "command";
        _read_only_command = context.commands[arg->_raw]->read_only ();
      }

      // rc:<file>
      // Note: This doesn't break a sequence chain.
      else if (arg->_raw.substr (0, 3) == "rc:")
        arg->_category = "rc";

      // rc.<name>:<value>
      // Note: This doesn't break a sequence chain.
      else if (arg->_raw.substr (0, 3) == "rc.")
        arg->_category = "override";

      // If the type is not known, it is treated as a generic word.
      else
        arg->_category = "word";
    }

    // All post-termination arguments are simply words.
    else
      arg->_category = "word";
  }
}

////////////////////////////////////////////////////////////////////////////////
bool A3::is_command (
  const std::vector <std::string>& keywords,
  std::string& command)
{
  std::vector <std::string> matches;
  if (autoComplete (command,
                    keywords,
                    matches,
                    context.config.getInteger ("abbreviation.minimum")) == 1)
  {
    command = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Add a pair for every word from std::cin, with a category of "".
void A3::append_stdin ()
{
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

      this->push_back (Arg (arg, ""));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3::rc_override (
  std::string& home,
  File& rc)
{
  // Is there an override for rc:<file>?
  std::vector <Arg>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "rc")
    {
      rc = File (arg->_raw.substr (3));
      home = rc;

      std::string::size_type last_slash = rc.data.rfind ("/");
      if (last_slash != std::string::npos)
        home = rc.data.substr (0, last_slash);
      else
        home = ".";

      context.header ("Using alternate .taskrc file " + rc.data);

      // Keep looping, because if there are multiple rc:file arguments, we
      // want the last one to dominate.
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A3::get_data_location (std::string& data)
{
  std::string location = context.config.get ("data.location");
  if (location != "")
    data = location;

  // Are there any overrides for data.location?
  std::vector <Arg>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "override")
    {
      if (arg->_raw.substr (0, 16) == "rc.data.location" &&
          (arg->_raw[16] == ':' || arg->_raw[16] == '='))
      {
        data = arg->_raw.substr (17);
        context.header ("Using alternate data.location " + data);
      }
    }

    // Keep scanning, because if there are multiple overrides, we want the last
    // one to dominate.
  }
}

////////////////////////////////////////////////////////////////////////////////
// An alias must be a distinct word on the command line.
// Aliases may not recurse.
void A3::resolve_aliases ()
{
  std::vector <std::string> expanded;
  bool something;
  int safety_valve = 10;

  do
  {
    something = false;
    std::vector <Arg>::iterator arg;
    for (arg = this->begin (); arg != this->end (); ++arg)
    {
      std::map <std::string, std::string>::iterator match =
        context.aliases.find (arg->_raw);

      if (match != context.aliases.end ())
      {
        context.debug (std::string ("A3::resolve_aliases '")
                       + arg->_raw
                       + "' --> '"
                       + context.aliases[arg->_raw]
                       + "'");

        std::vector <std::string> words;
        splitq (words, context.aliases[arg->_raw], ' ');

        std::vector <std::string>::iterator word;
        for (word = words.begin (); word != words.end (); ++word)
          expanded.push_back (*word);

        something = true;
      }
      else
        expanded.push_back (arg->_raw);
    }

    // Only overwrite if something happened.
    if (something)
    {
      this->clear ();
      std::vector <std::string>::iterator e;
      for (e = expanded.begin (); e != expanded.end (); ++e)
        this->push_back (Arg (*e, ""));

      expanded.clear ();
    }
  }
  while (something && --safety_valve > 0);

  if (safety_valve <= 0)
    context.debug ("Nested alias limit of 10 reached.");
}

////////////////////////////////////////////////////////////////////////////////
// Extracts any rc.name:value args and sets the name/value in context.config,
// leaving only the plain args.
void A3::apply_overrides ()
{
  std::vector <Arg>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "override")
    {
      std::string name;
      std::string value;
      Nibbler n (arg->_raw);
      if (n.getLiteral ("rc.")         &&  // rc.
          n.getUntilOneOf (":=", name) &&  //    xxx
          (n.skip (':') || n.skip ('=')))  //       [:=]
       {
        n.getUntilEOS (value);  // May be blank.

        context.config.set (name, value);
        context.footnote ("Configuration override rc." + name + ":" + value);
      }
      else
        context.footnote ("Problem with override: " + arg->_raw);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// These are some delicate heuristics here.  Tread lightly.
void A3::inject_defaults ()
{
  // Scan the arguments and detect what is present.
  bool found_command  = false;
  bool found_sequence = false;
  bool found_other    = false;

  std::vector <Arg>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "command")
      found_command = true;

/* TODO no "id" or "uuid" categories exist at this time.  Hmm.
    else if (arg->_category == "id" ||
             arg->_category == "uuid")
      found_sequence = true;
*/

    else if (arg->_category != "program"  &&
             arg->_category != "override" &&
             arg->_category != "rc")
      found_other = true;
  }

  // If no command was specified, then a command will be inserted.
  if (!found_command)
  {
    // Default command.
    if (!found_sequence)
    {
      // Apply overrides, if any.
      std::string defaultCommand = context.config.get ("default.command");
      if (defaultCommand != "")
      {
        capture_first (defaultCommand);
        context.header ("[" + combine () + "]");
      }
      else
        throw std::string (STRING_TRIVIAL_INPUT);
    }
    else
    {
      // Modify command.
      if (found_other)
      {
        capture_first ("modify");
      }

      // Information command.
      else
      {
        context.header (STRING_ASSUME_INFO);
        capture_first ("information");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::string A3::combine () const
{
  std::string combined;

  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg != this->begin ())
      combined += " ";

    combined += arg->_raw;
  }

  return combined;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> A3::list () const
{
  std::vector <std::string> all;
  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
    all.push_back (arg->_raw);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
bool A3::find_command (std::string& command) const
{
  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "command")
    {
      command = arg->_raw;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
const std::string A3::find_limit () const
{
  std::vector <Arg>::const_reverse_iterator arg;
  for (arg = this->rbegin (); arg != this->rend (); ++arg)
    if (arg->_raw.find ("limit:") != std::string::npos)
      return arg->_raw.substr (6);

  return "";
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> A3::operator_list ()
{
  std::vector <std::string> all;
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    all.push_back (operators[i].op);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const A3 A3::extract_filter () const
{
  A3 filter;
  bool before_command = true;
  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "command")
      before_command = false;

    if (arg->_category == "program"  ||
        arg->_category == "rc"       ||
        arg->_category == "override" ||
        arg->_category == "command"  ||
        arg->_category == "terminator")
      ;

    else if (before_command || _read_only_command)
      filter.push_back (*arg);
  }

  filter = tokenize (filter);
  return filter;
}

////////////////////////////////////////////////////////////////////////////////
const A3 A3::extract_modifications () const
{
  A3 mods;
  bool before_command = true;
  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "command")
      before_command = false;

    else if (! before_command)
      mods.push_back (*arg);
  }

  return mods;
}

////////////////////////////////////////////////////////////////////////////////
const A3 A3::extract_words () const
{
  A3 words;
  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "program"  ||
        arg->_category == "rc"       ||
        arg->_category == "override" ||
        arg->_category == "command"  ||
        arg->_category == "terminator")
      ;

    else
      words.push_back (*arg);
  }

  return words;
}

////////////////////////////////////////////////////////////////////////////////
const A3 A3::tokenize (const A3& input) const
{
  // Join all the arguments together.
  std::string combined;
  std::vector <Arg>::const_iterator arg;
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    if (arg != input.begin ())
      combined += " ";

   combined += arg->_raw;
  }

  // List of operators for recognition.
  std::vector <std::string> operators = A3::operator_list ();

  // Date format, for both parsing and rendering.
  std::string date_format = context.config.get ("dateformat");

  // Nibble them apart.
  A3 output;
  Nibbler n (combined);
  n.skipWS ();

  std::string s;
  int i;
  double d;
  time_t t;
//  std::cout << "# " << n.dump () << "\n";
  while (! n.depleted ())
  {
    if (n.getQuoted ('"', s, true) ||
        n.getQuoted ('\'', s, true))
    {
//      std::cout << "# string '" << s << "'\n";
      output.push_back (Arg (s, "string"));
    }

    else if (is_subst (n, s))
    {
//      std::cout << "# subst '" << s << "'\n";
      output.push_back (Arg (s, "subst"));
    }

    else if (is_pattern (n, s))
    {
//      std::cout << "# pattern '" << s << "'\n";
      output.push_back (Arg (s, "pattern"));
    }

    else if (is_tag (n, s))
    {
//      std::cout << "# tag '" << s << "'\n";
      output.push_back (Arg (s, "tag"));
    }

    else if (n.getOneOf (operators, s))
    {
//      std::cout << "# operator '" << s << "'\n";
      output.push_back (Arg (s, "op"));
    }

    else if (is_attr (n, s))
    {
//      std::cout << "# attr '" << s << "'\n";
      output.push_back (Arg (s, "attr"));
    }

    else if (is_attmod (n, s))
    {
//      std::cout << "# attmod '" << s << "'\n";
      output.push_back (Arg (s, "attmod"));
    }

    else if (is_dom (n, s))
    {
//      std::cout << "# dom '" << s << "'\n";
      output.push_back (Arg (s, "dom"));
    }

    else if (n.getDateISO (t))
    {
//      std::cout << "# date '" << t << "'\n";
      output.push_back (Arg (Date (t).toISO (), "date"));
    }

    else if (n.getDate (date_format, t))
    {
//      std::cout << "# date '" << t << "'\n";
      output.push_back (Arg (Date (t).toString (date_format), "date"));
    }

    else if (is_duration (n, s))
    {
//      std::cout << "# duration '" << s << "'\n";
      output.push_back (Arg (s, "duration"));
    }

    else if (is_id (n, s))
    {
//      std::cout << "# id '" << s << "'\n";
      output.push_back (Arg (s, "id"));
    }

    else if (is_uuid (n, s))
    {
//      std::cout << "# uuid '" << s << "'\n";
      output.push_back (Arg (s, "uuid"));
    }

    else if (n.getNumber (d))
    {
//      std::cout << "# num '" << d << "'\n";
      output.push_back (Arg (format (d), "num"));
    }

    else if (n.getInt (i))
    {
//      std::cout << "# int '" << i << "'\n";
      output.push_back (Arg (format (i), "int"));
    }

    else if (n.getName (s) ||
             n.getWord (s))       // After DOM
    {
//      std::cout << "# word '" << s << "'\n";
      output.push_back (Arg (s, "word"));
    }

    else
    {
      if (! n.getUntilWS (s))
        n.getUntilEOS (s);

//      std::cout << "# word '" << s << "'\n";
      output.push_back (Arg (s, "word"));
    }

//    std::cout << "# " << n.dump () << "\n";
    n.skipWS ();
//    std::cout << "# " << n.dump () << "\n";
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
// <name>:['"][<value>]['"]
bool A3::is_attr (Nibbler& n, std::string& result)
{
  n.save ();
  std::string name;
  std::string value;

  // If there is a valid attribute name.
  if (n.getName (name) &&
      name.length ()   &&
      is_attribute (name, name))
  {
    if (n.skip (':'))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value)  ||
          n.getQuoted   ('\'', value) ||
          n.getName     (value)       ||
          n.getUntilEOS (value)       ||
          n.depleted ())
      {
/*
        // TODO Reject anything that looks like a URL.
        // Exclude certain URLs, that look like attrs.
        if (value.find ('@') <= n.cursor () ||
            value.find ('/') <= n.cursor ())
          return false;
*/

        result = name + ':' + value;
        return true;
      }
    }
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <name>.<mod>:['"]<value>['"]
bool A3::is_attmod (Nibbler& n, std::string& result)
{
  n.save ();
  std::string name;
  std::string modifier;
  std::string value;

  // If there is a valid attribute name.
  if (n.getName (name) &&
      name.length ()   &&
      is_attribute (name, name))
  {
    if (n.skip ('.'))
    {
      // Skip the negation character.
      n.skip ('~');

      // If there is a valid modifier name.
      if (n.getName (modifier) &&
          modifier.length ()   &&
          is_modifier (modifier, modifier))
      {
        if (n.skip (':'))
        {
          // Both quoted and unquoted Att's are accepted.
          // Consider removing this for a stricter parse.
          if (n.getQuoted   ('"', value)  ||
              n.getQuoted   ('\'', value) ||
              n.getName     (value)       ||
              n.getUntilEOS (value)       ||
              n.depleted ())
          {
/*
     TODO Eliminate anything that looks like a URL.
            // Exclude certain URLs, that look like attrs.
            if (value.find ('@') <= n.cursor () ||
                value.find ('/') <= n.cursor ())
              return false;
*/

            result = name + '.' + modifier + ':' + value;
            return true;
          }
        }
      }
    }
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Canonicalize attribute names.
bool A3::is_attribute (const std::string& input, std::string& canonical)
{
  std::vector <std::string> columns = context.getColumns ();
  std::vector <std::string> matches;
  autoComplete (input,
                columns,
                matches,
                context.config.getInteger ("abbreviation.minimum"));

  if (matches.size () == 1)
  {
    canonical = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Canonicalize modifier names.
bool A3::is_modifier (const std::string& input, std::string& canonical)
{
  std::vector <std::string> candidates;
  for (unsigned int i = 0; i < NUM_MODIFIER_NAMES; ++i)
    candidates.push_back (modifierNames[i]);

  std::vector <std::string> matches;
  autoComplete (input,
                candidates,
                matches,
                context.config.getInteger ("abbreviation.minimum"));

  if (matches.size () == 1)
  {
    canonical = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// DOM references are one of the following:
//
// 1. Fixed string
//    DOM::get_references
// 2. Attribute
//    <attr>
// 3. Task-specific attribute
//    <id>.<attr>
//    <uuid>.<attr>
// 4. Configuration value
//    rc.<name>
//
bool A3::is_dom (Nibbler& n, std::string& result)
{
  n.save ();
  std::string name;
  int id;
  std::string uuid;

  // Fixed string reference.
  std::vector <std::string> refs = context.dom.get_references ();
  if (n.getOneOf (refs, result))
    return true;

  // Configuration.
  if (n.getLiteral ("rc."))
  {
    result = "rc.";
    while (n.getWord (name))
    {
      result += name;

      if (n.skip ('.'))
        result += '.';
    }

    return true;
  }

  n.restore ();

  // <id>.<attr>
  if (n.getInt (id)    &&
      n.skip ('.')     &&
      n.getName (name) &&
      name.length ()   &&
      is_attribute (name, name))
  {
    result = format (id) + '.' + name;
    return true;
  }

  n.restore ();

  // <uuid>.<attr>
  if (n.getUUID (uuid) &&
      n.skip ('.')     &&
      n.getName (name) &&
      name.length ()   &&
      is_attribute (name, name))
  {
    result = uuid + '.' + name;
    return true;
  }

  n.restore ();

  // Attribute.
  if (n.getName (name) &&
      name.length ()   &&
      is_attribute (name, name))
  {
    result = name;
    return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool A3::is_duration (Nibbler& n, std::string& result)
{
  n.save ();

  int quantity;
  std::string unit;

  std::vector <std::string> units = Duration::get_units ();

  if (n.getInt (quantity) &&
      n.getOneOf (units, unit))
  {
    result = format (quantity) + unit;
    return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// /<pattern>/
bool A3::is_pattern (Nibbler& n, std::string& result)
{
  n.save ();
  std::string pattern;
  if (n.getQuoted ('/', pattern) &&
      pattern.length () > 0)
  {
    result = '/' + pattern + '/';
    return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// /<from>/<to>/[g]
//
// Note: one problem with this is that substitutions start with a /, and so any
//       two-directory absolute path, (or three-level, if the third directory is
//       named 'g') can be misinterpreted.  To help (but not solve) this, if a
//       substition exists on the local disk, it is not considered a subst.
//       This needs to be changed to a better solution.
bool A3::is_subst (Nibbler& n, std::string& result)
{
  n.save ();

  std::string from;
  std::string to;
  bool global = false;
  if (n.skip     ('/')       &&
      n.getUntil ('/', from) &&
      from.length ()         &&
      n.skip     ('/')       &&
      n.getUntil ('/', to)   &&
      n.skip     ('/'))
  {
    if (n.skip ('g'))
      global = true;

    result = '/' + from + '/' + to + '/';
    if (global)
      result += 'g';

    if (! Directory (result).exists ())    // Ouch - expensive call.
      return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <id>[-<id>][,<id>[-<id>]]
bool A3::is_id (Nibbler& n, std::string& result)
{
  n.save ();
  std::string::size_type start = n.cursor ();
  int id;

  if (n.getUnsignedInt (id))
  {
    if (n.skip ('-') &&
        !n.getUnsignedInt (id))
    {
      n.restore ();
      return false;
    }

    while (n.skip (','))
    {
      if (n.getUnsignedInt (id))
      {
        if (n.skip ('-'))
        {
          if (!n.getUnsignedInt (id))
          {
            n.restore ();
            return false;
          }
        }
      }
      else
      {
        n.restore ();
        return false;
      }
    }

    std::string::size_type end = n.cursor ();
    n.restore ();
    if (n.getN (end - start, result))
      return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <uuid>[,...]
bool A3::is_uuid (Nibbler& n, std::string& result)
{
  n.save ();
  result = "";
  std::string uuid;
  if (n.getUUID (uuid))
  {
    result += uuid;
    while (n.skip (',') &&
           n.getUUID (uuid))
    {
      result += ',' + uuid;
    }

    return true;
  }

  n.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// [+-]<tag>
bool A3::is_tag (Nibbler& n, std::string& result)
{
  n.save ();

  std::string indicator;
  std::string name;
  if (n.getN (1, indicator)                  &&
      (indicator == "+" || indicator == "-") &&
      n.getName (name)                       &&
      name.length ())
  {
    result = indicator + name;
    return true;
  }

  n.restore ();
  return false;
}














#ifdef NOPE
////////////////////////////////////////////////////////////////////////////////
bool A3::is_operator (
  const std::string& input,
  char& type,
  int& precedence,
  char& associativity)
{
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    if (operators[i].op == input)
    {
      type          = operators[i].type;
      precedence    = operators[i].precedence;
      associativity = operators[i].associativity;
      return true;
    }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool A3::is_symbol_operator (const std::string& input)
{
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    if (operators[i].symbol &&
        operators[i].op == input)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <name>:['"]<value>['"]
bool A3::extract_attr (
  const std::string& input,
  std::string& name,
  std::string& value)
{
  Nibbler n (input);

  // Ensure a clean parse.
  name  = "";
  value = "";

  if (n.getUntil (':', name))
  {
    if (name.length () == 0)
      throw std::string ("Missing attribute name");

    if (n.skip (':'))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value)  ||
          n.getQuoted   ('\'', value) ||
          n.getUntilEOS (value))
      {
        return true;
      }
    }
    else
      throw std::string ("Missing : after attribute name.");
  }
  else
    throw std::string ("Missing : after attribute name.");

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <name>.<mod>:['"]<value>['"]
bool A3::extract_attmod (
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
      throw std::string ("Missing attribute name");

    if (n.skip ('.'))
    {
      if (n.skip ('~'))
        sense = "negative";

      if (n.getUntil (':', modifier))
      {
        if (!A3::valid_modifier (modifier))
          throw std::string ("The name '") + modifier + "' is not a valid modifier.";
      }
      else
        throw std::string ("Missing . or : after modifier.");
    }
    else
      throw std::string ("Missing modifier.");

    if (n.skip (':'))
    {
      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value)  ||
          n.getQuoted   ('\'', value) ||
          n.getUntilEOS (value))
      {
        return true;
      }
    }
    else
      throw std::string ("Missing : after attribute name.");
  }
  else
    throw std::string ("Missing : after attribute name.");

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool A3::extract_subst (
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
bool A3::extract_pattern (const std::string& input, std::string& pattern)
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
// The _first number found in the command line is assumed to be a sequence.  If
// there are two sequences, only the _first is recognized, for example:
//
//    1,2 three 4,5
//
// The sequence is "1,2".
//
bool A3::extract_id (const std::string& input, std::vector <int>& sequence)
{
  Nibbler n (input);

  int id;

  if (n.getUnsignedInt (id))
  {
    sequence.push_back (id);

    if (n.skip ('-'))
    {
      int end;
      if (!n.getUnsignedInt (end))
        throw std::string ("Unrecognized ID after hyphen.");

      if (id > end)
        throw std::string ("Inverted range 'high-low' instead of 'low-high'");

      for (int n = id + 1; n <= end; ++n)
        sequence.push_back (n);
    }

    while (n.skip (','))
    {
      if (n.getUnsignedInt (id))
      {
        sequence.push_back (id);

        if (n.skip ('-'))
        {
          int end;
          if (!n.getUnsignedInt (end))
            throw std::string ("Unrecognized ID after hyphen.");

          if (id > end)
            throw std::string ("Inverted range 'high-low' instead of 'low-high'");

          for (int n = id + 1; n <= end; ++n)
            sequence.push_back (n);
        }
      }
      else
        throw std::string ("Malformed ID");
    }
  }
  else
    throw std::string ("Malformed ID");

  return n.depleted ();
}

////////////////////////////////////////////////////////////////////////////////
bool A3::extract_uuid (
  const std::string& input,
  std::vector <std::string>& sequence)
{
  Nibbler n (input);

  std::string uuid;
  if (n.getUUID (uuid))
  {
    sequence.push_back (uuid);

    while (n.skip (','))
    {
      if (!n.getUUID (uuid))
        throw std::string ("Unrecognized UUID after comma.");

      sequence.push_back (uuid);
    }
  }
  else
    throw std::string ("Malformed UUID");

  if (!n.depleted ())
    throw std::string ("Unrecognized character(s) at end of pattern.");

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool A3::extract_tag (
  const std::string& input,
  char& type,
  std::string& tag)
{
  if (input.length () > 1)
  {
    type = input[0];
    tag  = input.substr (1);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool A3::extract_operator (
  const std::string& input,
  std::string& op)
{
  op = input;
  return true;
}

#endif // NOPE









////////////////////////////////////////////////////////////////////////////////
void A3::dump (const std::string& label)
{
  // Set up a color mapping.
  std::map <std::string, Color> color_map;
  color_map["program"]    = Color ("bold blue on blue");
  color_map["command"]    = Color ("bold cyan on cyan");
  color_map["rc"]         = Color ("bold red on red");
  color_map["override"]   = Color ("bold red on red");
  color_map["terminator"] = Color ("bold yellow on yellow");
  color_map["word"]       = Color ("white on gray4");

  // Filter colors.
  color_map["attr"]     = Color ("bold red on gray4");
  color_map["attmod"]   = Color ("bold red on gray4");
  color_map["pattern"]  = Color ("cyan on gray4");
  color_map["op"]       = Color ("green on gray4");
  color_map["string"]   = Color ("bold yellow on gray4");
  color_map["date"]     = Color ("bold yellow on gray4");
  color_map["dom"]      = Color ("bold white on gray4");
  color_map["duration"] = Color ("magenta on gray4");
  color_map["id"]       = Color ("white on gray4");
  color_map["uuid"]     = Color ("white on gray4");

  // Default.
  color_map["none"]       = Color ("black on white");

  Color color_debug (context.config.get ("color.debug"));
  std::stringstream out;
  out << color_debug.colorize (label)
      << "\n";

  ViewText view;
  view.width (context.getWidth ());
  view.leftMargin (2);
  for (unsigned int i = 0; i < this->size (); ++i)
    view.add (Column::factory ("string", ""));

  view.addRow ();
  view.addRow ();
  view.addRow ();

  for (unsigned int i = 0; i < this->size (); ++i)
  {
    std::string raw      = (*this)[i]._raw;
    std::string category = (*this)[i]._category;

    Color c;
    if (color_map[category].nontrivial ())
      c = color_map[category];
    else
      c = color_map["none"];

    view.set (0, i, raw,      c);
    view.set (2, i, category, c);
  }

  out << view.render ();
  context.debug (out.str ());
}

////////////////////////////////////////////////////////////////////////////////
