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
#include <Lexer.h>
#include <Directory.h>
#include <ViewText.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <Arguments.h>

extern Context context;

static const char* attributeNames[] =
{
  "entry",
  "start",
  "end",
  "parent",
  "uuid",
  "mask",
  "imask",
  "limit",
  "status",
  "description",
  "tags",
  "urgency",
  // Note that annotations are not listed.
};

static const char* modifiableAttributeNames[] =
{
  "project",
  "priority",
  "fg",
  "bg",
  "due",
  "recur",
  "until",
  "wait",
  "depends",
};

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

#define NUM_ATT_NAMES            (sizeof (attributeNames) / sizeof (attributeNames[0]))
#define NUM_MODIFIABLE_ATT_NAMES (sizeof (modifiableAttributeNames) / sizeof (modifiableAttributeNames[0]))
#define NUM_MODIFIER_NAMES       (sizeof (modifierNames) / sizeof (modifierNames[0]))
#define NUM_OPERATORS            (sizeof (operators) / sizeof (operators[0]))

static const char* non_word_chars = " +-*/%()=<>!~";

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
  {
    // The "i != 0" guarantees that argv[0] does not get split, because it may
    // be an absolute path, and Expression::expand_tokens would make a dog's
    // dinner out of it.
    std::vector <std::string> parts;
    if (is_multipart (argv[i], parts) && i != 0)
    {
      std::vector <std::string>::iterator part;
      for (part = parts.begin (); part != parts.end (); ++part)
        this->push_back (Triple (*part, "", ""));
    }
    else
      this->push_back (Triple (argv[i], "", ""));
  }

  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Append a pair with a category of "".
void Arguments::capture (const std::string& arg)
{
  std::vector <std::string> parts;
  if (is_multipart (arg, parts))
  {
    std::vector <std::string>::iterator part;
    for (part = parts.begin (); part != parts.end (); ++part)
      this->push_back (Triple (*part, "", ""));
  }
  else
    this->push_back (Triple (arg, "", ""));

  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Prepend a pair with a category of "".
void Arguments::capture_first (const std::string& arg)
{
  // Break the new argument into parts that comprise a series.
  std::vector <Triple> series;

  std::vector <std::string> parts;
  if (is_multipart (arg, parts))
  {
    std::vector <std::string>::iterator part;
    for (part = parts.begin (); part != parts.end (); ++part)
      series.push_back (Triple (*part, "", ""));
  }
  else
    series.push_back (Triple (arg, "", ""));

  // Locate an appropriate place to insert the series.  This would be
  // immediately after the program and command arguments.
  std::vector <Triple>::iterator position;
  for (position = this->begin (); position != this->end (); ++position)
    if (position->_third != "program" &&
        position->_third != "command")
      break;

  this->insert (position, series.begin (), series.end ());

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

      this->push_back (Triple (arg, "", ""));
      something_happened = true;
    }
  }

  if (something_happened)
    categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Scan all the arguments, and assign a category for each one.
void Arguments::categorize ()
{
  bool terminated                     = false;
  bool found_command                  = false;
  bool found_sequence                 = false;
  bool found_something_after_sequence = false;
  bool found_non_sequence             = false;

  // Configurable support.
  bool enable_expressions = context.config.getBoolean ("expressions");
  bool enable_patterns    = context.config.getBoolean ("patterns");

  // Generate a vector of command keywords against which autoComplete can run.
  std::vector <std::string> keywords;
  std::map <std::string, Command*>::iterator k;
  for (k = context.commands.begin (); k != context.commands.end (); ++k)
    keywords.push_back (k->first);

  // Now categorize every argument.
  std::string ignored;
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (!terminated)
    {
      // Nothing after -- is to be interpreted in any way.
      if (arg->_first == "--")
      {
        terminated = true;
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "terminator";
      }

      // program
      else if (arg == this->begin ())
      {
        arg->_third = "program";  // TODO Is this a problem for expressions that do not contain a program name?
      }

      // command
      else if (!found_command &&
               is_command (keywords, arg->_first))
      {
        found_command = true;
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "command";
      }

      // rc:<file>
      // Note: This doesn't break a sequence chain.
      else if (arg->_first.substr (0, 3) == "rc:")
      {
        arg->_third = "rc";
      }

      // rc.<name>:<value>
      // Note: This doesn't break a sequence chain.
      else if (arg->_first.substr (0, 3) == "rc.")
      {
        arg->_third = "override";
      }

      // <id>[-<id>][,...]
      else if (is_id (arg->_first))
      {
        if (!found_something_after_sequence)
        {
          found_sequence = true;
          arg->_third = "id";
        }
        else
        {
          arg->_third = "word";
        }
      }

      // <uuid>[,...]
      else if (is_uuid (arg->_first))
      {
        if (!found_something_after_sequence)
        {
          found_sequence = true;
          arg->_third = "uuid";
        }
        else
        {
          arg->_third = "word";
        }
      }

      // [+-]tag
      else if (is_tag (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "tag";
      }

      // <name>.<modifier>:<value>
      else if (is_attmod (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "attmod";
      }

      // <name>:<value>
      else if (is_attr (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "attr";
      }

      // /<from>/<to>/[g]
      else if (is_subst (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "subst";
      }

      // /pattern/
      else if (enable_patterns && is_pattern (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "pattern";
      }

      // <operator>
      else if (is_operator (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "op";
      }

      // <expression>
      else if (enable_expressions && is_expression (arg->_first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "exp";
      }

      // If the type is not known, it is treated as a generic word.
      else
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->_third = "word";
      }
    }

    // All post-termination arguments are simply words.
    else
    {
      found_non_sequence = true;
      if (found_sequence)
        found_something_after_sequence = true;

      arg->_third = "word";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::rc_override (
  std::string& home,
  File& rc)
{
  // Is there an override for rc:<file>?
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_third == "rc")
    {
      rc = File (arg->_first.substr (3));
      home = rc;

      std::string::size_type last_slash = rc.data.rfind ("/");
      if (last_slash != std::string::npos)
        home = rc.data.substr (0, last_slash);
      else
        home = ".";

      context.header ("Using alternate .taskrc file " + rc.data);

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
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_third == "override")
    {
      if (arg->_first.substr (0, 16) == "rc.data.location" &&
          arg->_first[16] == ':')
      {
        data = arg->_first.substr (17);
        context.header ("Using alternate data.location " + data);
      }
    }

    // Keep scanning, because if there are multiple rc:file arguments, we
    // want the last one to dominate.
  }
}

////////////////////////////////////////////////////////////////////////////////
// Extracts any rc.name:value args and sets the name/value in context.config,
// leaving only the plain args.
void Arguments::apply_overrides ()
{
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_third == "override")
    {
      std::string name;
      std::string value;
      Nibbler n (arg->_first);
      if (n.getLiteral ("rc.")   &&  // rc.
          n.getUntil (':', name) &&  //    xxx
          n.skip (':'))              //       :
       {
        n.getUntilEOS (value);  // May be blank.

        context.config.set (name, value);
        context.footnote ("Configuration override rc." + name + ":" + value);
      }
      else
        context.footnote ("Problem with override: " + arg->_first);
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

  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    std::map <std::string, std::string>::iterator match =
      context.aliases.find (arg->_first);

    if (match != context.aliases.end ())
    {
      context.debug (std::string ("Arguments::resolve_aliases '")
                     + arg->_first
                     + "' --> '"
                     + context.aliases[arg->_first]
                     + "'");

      std::vector <std::string> words;
      splitq (words, context.aliases[arg->_first], ' ');

      std::vector <std::string>::iterator word;
      for (word = words.begin (); word != words.end (); ++word)
        expanded.push_back (*word);

      something = true;
    }
    else
      expanded.push_back (arg->_first);
  }

  // Only overwrite if something happened.
  if (something)
  {
    this->clear ();
    std::vector <std::string>::iterator e;
    for (e = expanded.begin (); e != expanded.end (); ++e)
      this->push_back (Triple (*e, "", ""));

    categorize ();
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::inject_defaults ()
{
  bool found_command  = false;
  bool found_sequence = false;
  bool found_other    = false;

  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_third == "command")
      found_command = true;

    else if (arg->_third == "id")
      found_sequence = true;

    else if (arg->_third != "program"  &&
             arg->_third != "override" &&
             arg->_third != "rc")
      found_other = true;
  }

  // If no command was specified, and there were no command line arguments
  // then invoke the default command.
  if (!found_command)
  {
    if (found_other || !found_sequence)
    {
      // Apply overrides, if any.
      std::string defaultCommand = context.config.get ("default.command");
      if (defaultCommand != "")
      {
        capture_first (defaultCommand);
        context.header ("[task " + trim (defaultCommand) + "]");
      }
      else
        throw std::string (STRING_TRIVIAL_INPUT);
    }

    // If the command "task 123" is entered, but with no modifier arguments,
    // then the actual command is assumed to be "info".
    else if (found_sequence)
    {
      context.header (STRING_ASSUME_INFO);
      push_back (Triple ("information", "", "command"));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Arguments::list ()
{
  std::vector <std::string> all;
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
    all.push_back (arg->_first);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> Arguments::operator_list ()
{
  std::vector <std::string> all;
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    all.push_back (operators[i].op);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
std::string Arguments::combine ()
{
  std::string combined;

  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg != this->begin ())
      combined += " ";

    combined += arg->_first;
  }

  return combined;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::find_command (std::string& command)
{
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->_third == "command")
    {
      command = arg->_first;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Arguments::find_limit ()
{
  std::vector <Triple>::reverse_iterator arg;
  for (arg = this->rbegin (); arg != this->rend (); ++arg)
    if (arg->_first.find ("limit:") != std::string::npos)
      return arg->_first.substr (6);

  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_multipart (
  const std::string& input,
  std::vector <std::string>& parts)
{
  parts.clear ();
  Nibbler n (input);
  std::string part;
  while (n.getQuoted ('"', part)  ||
         n.getQuoted ('\'', part) ||
         n.getQuoted ('/', part) ||
         n.getUntilWS (part))
  {
    n.skipWS ();
    parts.push_back (part);
  }

  return parts.size () > 1 ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_command (
  const std::vector <std::string>& keywords,
  std::string& command)
{
  std::vector <std::string> matches;
  if (autoComplete (command, keywords, matches) == 1)
  {
    command = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <name>:['"][<value>]['"]
bool Arguments::is_attr (const std::string& input)
{
  Nibbler n (input);
  std::string name;
  std::string value;

  if (n.getUntilOneOf ("=:", name))
  {
    if (name.length () == 0)
      return false;

    if (name.find_first_of (non_word_chars) != std::string::npos)
      return false;

    if (n.skip (':'))
    {
      // Exclude certain URLs, that look like attrs.
      if (input.find ('@') <= n.cursor () ||
          input.find ('/') <= n.cursor ())
        return false;

      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value)  ||
          n.getQuoted   ('\'', value) ||
          n.getUntil    (' ', value)  ||
          n.getUntilEOS (value)       ||
          n.depleted ())
      {
        // Validate and canonicalize attribute name.
        if (is_attribute (name, name))
          return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <name>.<mod>:['"]<value>['"]
bool Arguments::is_attmod (const std::string& input)
{
  Nibbler n (input);
  std::string name;
  std::string modifier;
  std::string value;

  if (n.getUntilOneOf (".", name))
  {
    if (name.length () == 0)
      return false;

    if (name.find_first_of (non_word_chars) != std::string::npos)
      return false;

    if (n.skip ('.'))
    {
      n.skip ('~');
      n.getUntil (':', modifier);

      if (modifier.length () == 0)
        return false;
    }

    if (n.skip (':'))
    {
      // Exclude certain URLs, that look like attrs.
      if (input.find ('@') <= n.cursor () ||
          input.find ('/') <= n.cursor ())
        return false;

      // Both quoted and unquoted Att's are accepted.
      // Consider removing this for a stricter parse.
      if (n.getQuoted   ('"', value)  ||
          n.getQuoted   ('\'', value) ||
          n.getUntil    (' ', value)  ||
          n.getUntilEOS (value)       ||
          n.depleted ())
      {
        // Validate and canonicalize attribute and modifier names.
        if (is_attribute (name, name) &&
            is_modifier (modifier))
          return true;
      }
    }
  }

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
bool Arguments::is_subst (const std::string& input)
{
  std::string from;
  std::string to;
  Nibbler n (input);
  if (n.skip     ('/')       &&
      n.getUntil ('/', from) &&
      n.skip     ('/')       &&
      n.getUntil ('/', to)   &&
      n.skip     ('/'))
  {
    n.skip ('g');
    if (n.depleted ()                 &&
        ! Directory (input).exists () &&   // Ouch - expensive call.
        from.length ())
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// /<pattern>/
bool Arguments::is_pattern (const std::string& input)
{
  Nibbler n (input);
  std::string pattern;
  if (input.length () > 2 &&
      n.getQuoted ('/', pattern, true))
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <id>[-<id>][,<id>[-<id>]]
bool Arguments::is_id (const std::string& input)
{
  Nibbler n (input);
  int id;

  if (n.getUnsignedInt (id))
  {
    if (n.skip ('-'))
    {
      if (!n.getUnsignedInt (id))
        return false;
    }

    while (n.skip (','))
    {
      if (n.getUnsignedInt (id))
      {
        if (n.skip ('-'))
        {
          if (!n.getUnsignedInt (id))
            return false;
        }
      }
      else
        return false;
    }
  }
  else
    return false;

  return n.depleted ();
}

////////////////////////////////////////////////////////////////////////////////
// <uuid>[,...]
bool Arguments::is_uuid (const std::string& input)
{
  Nibbler n (input);
  std::string uuid;

  if (n.getUUID (uuid))
  {
    while (n.skip (','))
    {
      if (!n.getUUID (uuid))
        return false;
    }
  }
  else
    return false;

  return n.depleted ();
}

////////////////////////////////////////////////////////////////////////////////
// [+-]<tag>
bool Arguments::is_tag (const std::string& input)
{
  if (input.length () > 1 &&
      (input[0] == '+' ||
       input[0] == '-') &&
       noSpaces (input) &&
       input.find ('+', 1) == std::string::npos &&
       input.find ('-', 1) == std::string::npos)
  {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_operator (const std::string& input)
{
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    if (operators[i].op == input)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_operator (
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
bool Arguments::is_symbol_operator (const std::string& input)
{
  for (unsigned int i = 0; i < NUM_OPERATORS; ++i)
    if (operators[i].symbol &&
        operators[i].op == input)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_attribute (const std::string& input, std::string& canonical)
{
  // Guess at the full attribute name.
  std::vector <std::string> candidates;
  for (unsigned i = 0; i < NUM_ATT_NAMES; ++i)
  {
    // Short-circuit: exact matches cause immediate return.
    if (attributeNames[i] == input)
    {
      canonical = input;
      return true;
    }

    candidates.push_back (attributeNames[i]);
  }

  for (unsigned i = 0; i < NUM_MODIFIABLE_ATT_NAMES; ++i)
  {
    // Short-circuit: exact matches cause immediate return.
    if (modifiableAttributeNames[i] == input)
    {
      canonical = input;
      return true;
    }

    candidates.push_back (modifiableAttributeNames[i]);
  }

  std::vector <std::string> matches;
  autoComplete (input, candidates, matches);

  if (matches.size () == 1)
  {
    canonical = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_modifier (const std::string& input)
{
  // Guess at the full attribute name.
  std::vector <std::string> candidates;
  for (unsigned i = 0; i < NUM_MODIFIER_NAMES; ++i)
  {
    // Short-circuit: exact matches cause immediate return.
    if (modifierNames[i] == input)
      return true;

    candidates.push_back (modifierNames[i]);
  }

  std::vector <std::string> matches;
  autoComplete (input, candidates, matches);

  if (matches.size () == 1)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Arguments::is_expression (const std::string& input)
{
  std::string unquoted = unquoteText (input);

  // Look for space-separated operators.
  std::vector <std::string> tokens;
  split (tokens, unquoted, ' ');
  std::vector <std::string>::iterator token;
  for (token = tokens.begin (); token != tokens.end (); ++token)
    if (is_operator (*token))
      return true;

  // Look for cuddled operators.
  Lexer lexer (unquoted);
  lexer.skipWhitespace (true);
  lexer.coalesceAlpha (true);
  lexer.coalesceDigits (true);
  lexer.coalesceQuoted (true);

  tokens.clear ();
  lexer.tokenize (tokens);

  for (token = tokens.begin (); token != tokens.end (); ++token)
    if (is_operator (*token))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <name>:['"]<value>['"]
bool Arguments::extract_attr (
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
      throw std::string ("Missing attribute name");

    if (n.skip ('.'))
    {
      if (n.skip ('~'))
        sense = "negative";

      if (n.getUntil (':', modifier))
      {
        if (!Arguments::valid_modifier (modifier))
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
bool Arguments::extract_id (const std::string& input, std::vector <int>& sequence)
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
bool Arguments::extract_uuid (
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
bool Arguments::extract_tag (
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
bool Arguments::extract_operator (
  const std::string& input,
  std::string& op)
{
  op = input;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
Arguments Arguments::extract_read_only_filter ()
{
  Arguments filter;

  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    // Excluded.
    if (arg->_third == "program"  ||
        arg->_third == "command"  ||
        arg->_third == "rc"       ||
        arg->_third == "override")
    {
      ;
    }

    // Included.
    else if (arg->_third == "tag"       ||
             arg->_third == "pattern"   ||
             arg->_third == "attr"      ||
             arg->_third == "attmod"    ||
             arg->_third == "seq"       ||
             arg->_third == "op"        ||
             arg->_third == "exp"       ||
             arg->_third == "word")
    {
      // "limit" is special - it is recognized but not included in filters.
      if (arg->_first.find ("limit:") == std::string::npos)
        filter.push_back (*arg);
    }

    // Error.
    else
    {
      // substitution
      throw std::string ("A substitution '") + arg->_first + "' is not allowed "
            "in a read-only command filter.";
    }
  }

  return filter;
}

////////////////////////////////////////////////////////////////////////////////
Arguments Arguments::extract_write_filter ()
{
  Arguments filter;
  bool before_command = true;

  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    // Only use args prior to command.
    if (arg->_third == "command")
      before_command = false;

    // Excluded.
    else if (arg->_third == "program"  ||
             arg->_third == "rc"       ||
             arg->_third == "override")
    {
      ;
    }

    // Included regardless of position.
    else if (arg->_third == "seq")
    {
      filter.push_back (*arg);
    }

    // Included if prior to command.
    else if (arg->_third == "tag"       ||
             arg->_third == "pattern"   ||
             arg->_third == "attr"      ||
             arg->_third == "attmod"    ||
             arg->_third == "op"        ||
             arg->_third == "exp"       ||
             arg->_third == "word")
    {
      if (before_command)
      {
        // "limit" is special - it is recognized but not included in filters.
        if (arg->_first.find ("limit:") == std::string::npos)
          filter.push_back (*arg);
      }
    }

    // Error.
    else
    {
      // substitution
      throw std::string ("A substitution '")
            + arg->_first
            + "' is not allowed in a read-only command filter.";
    }
  }

  return filter;
}

////////////////////////////////////////////////////////////////////////////////
Arguments Arguments::extract_modifications ()
{
  Arguments modifications;

  bool seen_command = false;
  std::vector <Triple>::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    // Only use args after command.
    if (arg->_third == "command")
    {
      seen_command = true;
    }

    else if (seen_command)
    {
      // Excluded.
      if (arg->_third == "program"  ||
          arg->_third == "rc"       ||
          arg->_third == "override")
      {
      }

      // Included.
      else if (arg->_third == "tag"   ||
               arg->_third == "attr"  ||
               arg->_third == "subst" ||
               arg->_third == "op"    ||
               arg->_third == "word")
      {
        // "limit" is special - it is recognized but not included in filters.
        if (arg->_first.find ("limit:") == std::string::npos)
          modifications.push_back (*arg);
      }

      // Error.
      else
      {
        if (arg->_third == "pattern")
          throw std::string ("A pattern '")
                + arg->_first
                + "' is not allowed when modifiying a task.";

        else if (arg->_third == "attmod")
          throw std::string ("An attribute modifier '")
                + arg->_first
                + "' is not allowed when modifiying a task.";

        else if (arg->_third == "exp")
          throw std::string ("An expression '")
                + arg->_first
                + "' is not allowed when modifiying a task.";

        else if (arg->_third == "id")
          throw std::string ("A task id cannot be modified.");

        else if (arg->_third == "uuid")
          throw std::string ("A task uuid cannot be modified.");
      }
    }
  }

  return modifications;
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
void Arguments::dump (const std::string& label)
{
  // Set up a color mapping.
  std::map <std::string, Color> color_map;
  color_map["program"]  = Color ("white on blue");
  color_map["command"]  = Color ("black on cyan");
  color_map["rc"]       = Color ("bold white on red");
  color_map["override"] = Color ("white on red");
  color_map["tag"]      = Color ("green on gray2");
  color_map["pattern"]  = Color ("cyan on gray2");
  color_map["attr"]     = Color ("bold red on gray2");
  color_map["attmod"]   = Color ("bold red on gray2");
  color_map["id"]       = Color ("yellow on gray2");
  color_map["uuid"]     = Color ("yellow on gray2");
  color_map["subst"]    = Color ("bold cyan on gray2");
  color_map["exp"]      = Color ("bold green on gray2");
  color_map["none"]     = Color ("white on gray2");

  // Fundamentals.
  color_map["lvalue"]   = Color ("bold green on rgb010");
  color_map["op"]       = Color ("white on rgb010");
  color_map["int"]      = Color ("bold yellow on rgb010");
  color_map["number"]   = Color ("bold yellow on rgb010");
  color_map["string"]   = Color ("bold yellow on rgb010");
  color_map["rx"]       = Color ("bold red on rgb010");

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
    std::string arg      = (*this)[i]._first;
    std::string expanded = (*this)[i]._second;
    std::string category = (*this)[i]._third;

    Color c;
    if (color_map[expanded].nontrivial ())
      c = color_map[expanded];
    else if (color_map[category].nontrivial ())
      c = color_map[category];
    else
      c = color_map["none"];

    view.set (0, i, arg,      c);
    view.set (1, i, expanded, c);
    view.set (2, i, category, c);
  }

  out << view.render ();
  context.debug (out.str ());
}

////////////////////////////////////////////////////////////////////////////////
