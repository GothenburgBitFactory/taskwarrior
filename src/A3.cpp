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
, _limit ("")
{
}

////////////////////////////////////////////////////////////////////////////////
A3::A3 (const A3& other)
{
  std::vector <Arg>::operator= (other);
  _read_only_command = other._read_only_command;
  _limit             = other._limit;
}

////////////////////////////////////////////////////////////////////////////////
A3& A3::operator= (const A3& other)
{
  std::vector <Arg>::operator= (other);
  _read_only_command = other._read_only_command;
  _limit             = other._limit;

  return *this;
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
        context.debug ("No command or sequence found - assuming default.command.");
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
        context.debug ("Sequence and filter, but no command found - assuming 'modify' command.");
        capture_first ("modify");
      }

      // Information command.
      else
      {
        context.debug ("Sequence but no command found - assuming 'information' command.");
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
  return _limit;
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

  filter = postfix (infix (sequence (expand (tokenize (filter)))));
  context.a3._limit = filter._limit;
  return filter;
}

////////////////////////////////////////////////////////////////////////////////
const A3 A3::extract_modifications () const
{
  A3 mods;
  mods._limit = _limit;

  bool before_command = true;
  std::vector <Arg>::const_iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_category == "command")
      before_command = false;

    else if (! before_command)
    {
      if (arg->_category == "rc" ||
          arg->_category == "override")
        ;
      else
        mods.push_back (*arg);
    }
  }

  mods = tokenize (mods);
  context.a3._limit = mods._limit;
  return mods;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> A3::extract_words () const
{
  std::vector <std::string> words;
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
      words.push_back (arg->_raw);
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

  // For identifying sequence versus non-sequence.
  bool terminated                     = false;
  bool found_sequence                 = false;
  bool found_something_after_sequence = false;

  std::string s;
  int i;
  double d;
  time_t t;
  while (! n.depleted ())
  {
    if (!terminated)
    {
      if (n.getLiteral ("--"))
        terminated = true;

      else if (n.getQuoted ('"', s, true) ||
          n.getQuoted ('\'', s, true))
      {
        output.push_back (Arg (s, "string"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_subst (n, s))
      {
        output.push_back (Arg (s, "subst"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_pattern (n, s))
      {
        output.push_back (Arg (s, "pattern"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_tag (n, s))
      {
        output.push_back (Arg (s, "tag"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (n.getOneOf (operators, s))
      {
        output.push_back (Arg (s, "op"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_attr (n, s))
      {
        // The "limit:xxx" attribute is not stored, but the value is retained.
        if (s.length () > 6 &&
            s.substr (0, 6) == "limit:")
        {
          output._limit = s.substr (6);
        }
        else
        {
          output.push_back (Arg (s, "attr"));
          if (found_sequence)
            found_something_after_sequence = true;
        }
      }

      else if (is_attmod (n, s))
      {
        output.push_back (Arg (s, "attmod"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_dom (n, s))
      {
        output.push_back (Arg (s, "dom"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (n.getDateISO (t))
      {
        output.push_back (Arg (Date (t).toISO (), "date"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (n.getDate (date_format, t))
      {
        output.push_back (Arg (Date (t).toString (date_format), "date"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_duration (n, s))
      {
        output.push_back (Arg (s, "duration"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (is_id (n, s))
      {
        if (found_something_after_sequence)
        {
          output.push_back (Arg (s, "num"));
        }
        else
        {
          output.push_back (Arg (s, "id"));
          found_sequence = true;
        }
      }

      else if (is_uuid (n, s))
      {
        if (found_something_after_sequence)
        {
          output.push_back (Arg (s, "num"));
        }
        else
        {
          output.push_back (Arg (s, "uuid"));
          found_sequence = true;
        }
      }

      else if (n.getNumber (d))
      {
        output.push_back (Arg (format (d), "num"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (n.getInt (i))
      {
        output.push_back (Arg (format (i), "int"));
        if (found_sequence)
          found_something_after_sequence = true;
      }

      else if (n.getName (s) ||
               n.getWord (s))
      {
        if (Date::valid (s))
          output.push_back (Arg (s, "date"));
        else
          output.push_back (Arg (s, "word"));

        if (found_sequence)
          found_something_after_sequence = true;
      }

      else
      {
        if (! n.getUntilWS (s))
          n.getUntilEOS (s);

        output.push_back (Arg (s, "word"));
        if (found_sequence)
          found_something_after_sequence = true;
      }
    }
    else
    {
      if (n.getUntilEOS (s))
      {
        output.push_back (Arg (s, "word"));
        if (found_sequence)
          found_something_after_sequence = true;
      }
    }

    n.skipWS ();
  }

  output.dump ("A3::tokenize");
  return output;
}

////////////////////////////////////////////////////////////////////////////////
// Insert 'and' operators between adjacent non-operators.
//
//   ) <non-op>         -->  ) and <non-op>
//   <non-op> (         -->  <non-op> <and> (
//   ) (                -->  ) and (
//   <non-op> <non-op>  -->  <non-op> and <non-op>
//
const A3 A3::infix (const A3& input) const
{
  Arg previous ("?", "op");

  A3 modified;
  modified._limit = input._limit;

  std::vector <Arg>::const_iterator arg;
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    // Old-style filters need 'and' conjunctions.
    if ((previous._category != "op" || previous._raw == ")") &&
        (arg->_category     != "op" || arg->_raw     == "("))
    {
      modified.push_back (Arg ("and", "op"));
    }

    // Now insert the adjacent non-operator.
    modified.push_back (*arg);
    previous = *arg;
  }

  modified.dump ("A3::infix");
  return modified;
}

////////////////////////////////////////////////////////////////////////////////
const A3 A3::expand (const A3& input) const
{
  A3 expanded;
  expanded._limit = input._limit;

  std::vector <Arg>::const_iterator arg;
  std::vector <Arg>::const_iterator previous = input.begin ();
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    // name:value  -->  name = value
    if (arg->_category == "attr")
    {
      std::string name;
      std::string value;
      A3::extract_attr (arg->_raw, name, value);

      expanded.push_back (Arg (name,  "dom"));
      expanded.push_back (Arg ("=",   "op"));
      expanded.push_back (Arg (value, "word"));
    }

    // name.mod:value  -->  name <op sub mod> value
    else if (arg->_category == "attmod")
    {
      std::string name;
      std::string mod;
      std::string value;
      std::string sense;
      extract_attmod (arg->_raw, name, mod, value, sense);

      // name.before:value  -->  name < value
      if (mod == "before" || mod == "under" || mod == "below")
      {
        expanded.push_back (Arg (name,  "dom"));
        expanded.push_back (Arg ("<",   "op"));
        expanded.push_back (Arg (value, "word"));
      }

      // name.after:value  -->  name > value
      else if (mod == "after" || mod == "over" || mod == "above")
      {
        expanded.push_back (Arg (name,  "dom"));
        expanded.push_back (Arg (">",   "op"));
        expanded.push_back (Arg (value, "word"));
      }

      // name.none:  -->  name == ""
      else if (mod == "none")
      {
        expanded.push_back (Arg (name, "dom"));
        expanded.push_back (Arg ("==", "op"));
        expanded.push_back (Arg ("",   "string"));
      }

      // name.any:  -->  name != ""
      else if (mod == "any")
      {
        expanded.push_back (Arg (name, "dom"));
        expanded.push_back (Arg ("!=", "op"));
        expanded.push_back (Arg ("",   "string"));
      }

      // name.is:value  -->  name = value
      else if (mod == "is" || mod == "equals")
      {
        expanded.push_back (Arg (name,  "dom"));
        expanded.push_back (Arg ("=",   "op"));
        expanded.push_back (Arg (value, "word"));
      }

      // name.isnt:value  -->  name != value
      else if (mod == "isnt" || mod == "not")
      {
        expanded.push_back (Arg (name,  "dom"));
        expanded.push_back (Arg ("!=",  "op"));
        expanded.push_back (Arg (value, "word"));
      }

      // name.has:value  -->  name ~ value
      else if (mod == "has" || mod == "contains")
      {
        expanded.push_back (Arg (name,  "dom"));
        expanded.push_back (Arg ("~",   "op"));
        expanded.push_back (Arg (value, "rx"));
      }

      // name.hasnt:value  -->  name !~ value
      else if (mod == "hasnt")
      {
        expanded.push_back (Arg (name,  "dom"));
        expanded.push_back (Arg ("!~",  "op"));
        expanded.push_back (Arg (value, "rx"));
      }

      // name.startswith:value  -->  name ~ ^value
      else if (mod == "startswith" || mod == "left")
      {
        expanded.push_back (Arg (name,        "dom"));
        expanded.push_back (Arg ("~",         "op"));
        expanded.push_back (Arg ("^" + value, "rx"));
      }

      // name.endswith:value  -->  name ~ value$
      else if (mod == "endswith" || mod == "right")
      {
        expanded.push_back (Arg (name,        "dom"));
        expanded.push_back (Arg ("~",         "op"));
        expanded.push_back (Arg (value + "$", "rx"));
      }

      // name.word:value  -->  name ~ \bvalue\b
      else if (mod == "word")
      {
        expanded.push_back (Arg (name,                  "dom"));
        expanded.push_back (Arg ("~",                   "op"));
        expanded.push_back (Arg ("\\b" + value + "\\b", "rx"));
      }

      // name.noword:value  -->  name !~ \bvalue\n
      else if (mod == "noword")
      {
        expanded.push_back (Arg (name,                  "dom"));
        expanded.push_back (Arg ("!~",                  "op"));
        expanded.push_back (Arg ("\\b" + value + "\\b", "rx"));
      }
      else
        throw std::string ("Error: unrecognized attribute modifier '") + mod + "'.";
    }

    // [+-]value  -->  tags ~/!~ value
    else if (arg->_category == "tag")
    {
      char type;
      std::string value;
      extract_tag (arg->_raw, type, value);

      expanded.push_back (Arg ("tags",                   "dom"));
      expanded.push_back (Arg (type == '+' ? "~" : "!~", "op"));
      expanded.push_back (Arg (value,                    "string"));
    }

    // word  -->  description ~ word
    // Note: use of previous prevents desc~foo --> desc~desc~foo
    else if (arg->_category == "word" &&
             previous->_category != "op")
    {
      expanded.push_back (Arg ("description", "dom"));
      expanded.push_back (Arg ("~",           "op"));
      expanded.push_back (Arg (arg->_raw,     "string"));
    }

    // /pattern/  -->  description ~ pattern
    else if (arg->_category == "pattern")
    {
      std::string value;
      extract_pattern (arg->_raw, value);

      expanded.push_back (Arg ("description", "dom"));
      expanded.push_back (Arg ("~",           "op"));
      expanded.push_back (Arg (value,         "rx"));
    }

    // Default  -->  preserve
    else
      expanded.push_back (*arg);

    previous = arg;
  }

  expanded.dump ("A3::expand");
  return expanded;
}

////////////////////////////////////////////////////////////////////////////////
// Convert:     1-3,5 7
// To:          (id=1 or id=2 or id=3 or id=5 or id=7)
const A3 A3::sequence (const A3& input) const
{
  A3 sequenced;
  sequenced._limit = input._limit;

  // Extract all the components of a sequence.
  std::vector <int> ids;
  std::vector <std::string> uuids;
  std::vector <Arg>::const_iterator arg;
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    if (arg->_category == "id")
      extract_id (arg->_raw, ids);

    else if (arg->_category == "uuid")
      extract_uuid (arg->_raw, uuids);
  }

  // If there is no sequence, we're done.
  if (ids.size () == 0 && uuids.size () == 0)
    return input;

  // Copy everything up to the first id/uuid.
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    if (arg->_category == "id" || arg->_category == "uuid")
      break;

    sequenced.push_back (*arg);
  }

  // Insert the algebraic form.
  sequenced.push_back (Arg ("(", "op"));

  for (unsigned int i = 0; i < ids.size (); ++i)
  {
    if (i)
      sequenced.push_back (Arg ("or", "op"));

    sequenced.push_back (Arg ("id",           "dom"));
    sequenced.push_back (Arg ("=",            "op"));
    sequenced.push_back (Arg (format(ids[i]), "num"));
  }

  for (unsigned int i = 0; i < uuids.size (); ++i)
  {
    if (ids.size ())
      sequenced.push_back (Arg ("or", "op"));

    sequenced.push_back (Arg ("uuid",   "dom"));
    sequenced.push_back (Arg ("=",      "op"));
    sequenced.push_back (Arg (uuids[i], "num"));
  }

  sequenced.push_back (Arg (")", "op"));

  // Now copy everything after the last id/uuid.
  bool found_id = false;
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    if (arg->_category == "id" || arg->_category == "uuid")
      found_id = true;

    else if (found_id)
      sequenced.push_back (*arg);
  }

  sequenced.dump ("A3::sequence");
  return sequenced;
}

////////////////////////////////////////////////////////////////////////////////
// Dijkstra Shunting Algorithm.
// http://en.wikipedia.org/wiki/Shunting-yard_algorithm
//
//   While there are tokens to be read:
//     Read a token.
//     If the token is an operator, o1, then:
//       while there is an operator token, o2, at the top of the stack, and
//             either o1 is left-associative and its precedence is less than or
//             equal to that of o2,
//             or o1 is right-associative and its precedence is less than that
//             of o2,
//         pop o2 off the stack, onto the output queue;
//       push o1 onto the stack.
//     If the token is a left parenthesis, then push it onto the stack.
//     If the token is a right parenthesis:
//       Until the token at the top of the stack is a left parenthesis, pop
//       operators off the stack onto the output queue.
//       Pop the left parenthesis from the stack, but not onto the output queue.
//       If the token at the top of the stack is a function token, pop it onto
//       the output queue.
//       If the stack runs out without finding a left parenthesis, then there
//       are mismatched parentheses.
//     If the token is a number, then add it to the output queue.
//
//   When there are no more tokens to read:
//     While there are still operator tokens in the stack:
//       If the operator token on the top of the stack is a parenthesis, then
//       there are mismatched parentheses.
//       Pop the operator onto the output queue.
//   Exit.
//
const A3 A3::postfix (const A3& input) const
{
  A3 converted;
  converted._limit = input._limit;

  A3 op_stack;
  char type;
  int precedence;
  char associativity;

  std::vector <Arg>::const_iterator arg;
  for (arg = input.begin (); arg != input.end (); ++arg)
  {
    if (arg->_raw == "(")
    {
      op_stack.push_back (*arg);
    }
    else if (arg->_raw == ")")
    {
      while (op_stack.size () > 0 &&
             op_stack.back ()._raw != "(")
      {
        converted.push_back (op_stack.back ());
        op_stack.pop_back ();
      }

      if (op_stack.size ())
        op_stack.pop_back ();
      else
        throw std::string ("Mismatched parentheses in expression");
    }
    else if (is_operator (arg->_raw, type, precedence, associativity))
    {
      char type2;
      int precedence2;
      char associativity2;
      while (op_stack.size () > 0 &&
             is_operator (op_stack.back ()._raw, type2, precedence2, associativity2) &&
             ((associativity == 'l' && precedence <= precedence2) ||
              (associativity == 'r' && precedence <  precedence2)))
      {
        converted.push_back (op_stack.back ());
        op_stack.pop_back ();
      }

      op_stack.push_back (*arg);
    }
    else
    {
      converted.push_back (*arg);
    }
  }

  while (op_stack.size () != 0)
  {
    if (op_stack.back ()._raw == "(" ||
        op_stack.back ()._raw == ")")
      throw std::string ("Mismatched parentheses in expression");

    converted.push_back (op_stack.back ());
    op_stack.pop_back ();
  }

  converted.dump ("A3::postfix");
  return converted;
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
          n.getUntilWS  (value)       ||
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
  columns.push_back ("limit"); // Special case.

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
//       This needs to be changed to a better solution.  When I think of one.
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

////////////////////////////////////////////////////////////////////////////////
bool A3::extract_pattern (const std::string& input, std::string& pattern)
{
  Nibbler n (input);
  if (n.skip     ('/')          &&
      n.getUntil ('/', pattern) &&
      n.skip     ('/'))
  {
    if (!n.depleted ())
      throw std::string ("Unrecognized character(s) at end of pattern.");

    return true;
  }
  else
    throw std::string ("Malformed pattern.");

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
  }

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
    if (n.skip ('.'))
    {
      if (n.skip ('~'))
        sense = "negative";

      n.getUntil (':', modifier);
    }

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
  }

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
    return true;
  }

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
bool A3::is_operator (
  const std::string& input,
  char& type,
  int& precedence,
  char& associativity)
{
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
  {
    if (operators[i].op == input)
    {
      type          = operators[i].type;
      precedence    = operators[i].precedence;
      associativity = operators[i].associativity;
      return true;
    }
  }

  return false;
}

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
  color_map["subst"]    = Color ("bold cyan on gray4");
  color_map["op"]       = Color ("green on gray4");
  color_map["string"]   = Color ("bold yellow on gray4");
  color_map["rx"]       = Color ("bold yellow on gray4");
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
