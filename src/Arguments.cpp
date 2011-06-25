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
  {  "%",      13,         'b',  1,      'l' },    // Modulus
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
        this->push_back (std::make_pair (*part, ""));
    }
    else
      this->push_back (std::make_pair (argv[i], ""));
  }

  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Add a pair with a category of "".
void Arguments::capture (const std::string& arg)
{
  std::vector <std::string> parts;
  if (is_multipart (arg, parts))
  {
    std::vector <std::string>::iterator part;
    for (part = parts.begin (); part != parts.end (); ++part)
      this->push_back (std::make_pair (*part, ""));
  }
  else
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
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (!terminated)
    {
      // Nothing after -- is to be interpreted in any way.
      if (arg->first == "--")
      {
        terminated = true;
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "terminator";
      }

      // program
      else if (arg == this->begin ())
      {
        arg->second = "program";
      }

      // command
      else if (!found_command &&
               is_command (keywords, arg->first))
      {
        found_command = true;
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "command";
      }

      // rc:<file>
      // Note: This doesn't break a sequence chain.
      else if (arg->first.substr (0, 3) == "rc:")
      {
        arg->second = "rc";
      }

      // rc.<name>:<value>
      // Note: This doesn't break a sequence chain.
      else if (arg->first.substr (0, 3) == "rc.")
      {
        arg->second = "override";
      }

      // <id>[-<id>][,...]
      else if (is_id (arg->first))
      {
        if (!found_something_after_sequence)
        {
          found_sequence = true;
          arg->second = "id";
        }
        else
        {
          arg->second = "word";
        }
      }

      // <uuid>[,...]
      else if (is_uuid (arg->first))
      {
        if (!found_something_after_sequence)
        {
          found_sequence = true;
          arg->second = "uuid";
        }
        else
        {
          arg->second = "word";
        }
      }

      // [+-]tag
      else if (is_tag (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "tag";
      }

      // <name>.<modifier>:<value>
      else if (is_attmod (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "attmod";
      }

      // <name>:<value>
      else if (is_attr (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "attr";
      }

      // /<from>/<to>/[g]
      else if (is_subst (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "subst";
      }

      // /pattern/
      else if (enable_patterns && is_pattern (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "pattern";
      }

      // <operator>
      else if (is_operator (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "op";
      }

      // <expression>
      else if (enable_expressions && is_expression (arg->first))
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "exp";
      }

      // If the type is not known, it is treated as a generic word.
      else
      {
        found_non_sequence = true;
        if (found_sequence)
          found_something_after_sequence = true;

        arg->second = "word";
      }
    }

    // All post-termination arguments are simply words.
    else
    {
      found_non_sequence = true;
      if (found_sequence)
        found_something_after_sequence = true;

      arg->second = "word";
    }
  }

  // If no command was specified, and there were no command line arguments
  // then invoke the default command.
  if (!found_command)
  {
    if (found_non_sequence)
    {
      // TODO Invoke the default command.
/*
      // Apply overrides, if any.
      std::string defaultCommand = config.get ("default.command");
      if (defaultCommand != "")
      {
        // Add on the overrides.
        defaultCommand += " " + file_override + " " + var_overrides;

        // Stuff the command line.
        args.clear ();
        split (args, defaultCommand, ' ');
        header ("[task " + trim (defaultCommand) + "]");

        // Reinitialize the context and recurse.
        file_override = "";
        var_overrides = "";
        footnotes.clear ();
        //initialize ();
        parse (args, cmd, task, sequence, subst, filter);
      }
      else
        throw std::string (STRING_TRIVIAL_INPUT);
*/
    }

    // If the command "task 123" is entered, but with no modifier arguments,
    // then the actual command is assumed to be "info".
    else if (!found_non_sequence &&
             found_sequence)
    {
      // TODO Invoke the info command.
      std::cout << STRING_ASSUME_INFO << "\n";
//      parseCmd.command = "info";
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Arguments::rc_override (
  std::string& home,
  File& rc)
{
  // Is there an override for rc:<file>?
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "rc")
    {
      rc = File (arg->first.substr (3));
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
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "override")
    {
      if (arg->first.substr (0, 16) == "rc.data.location" &&
          arg->first[16] == ':')
      {
        data = arg->first.substr (17);
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
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
  {
    if (arg->second == "override")
    {
      std::string name;
      std::string value;
      Nibbler n (arg->first);
      if (n.getLiteral ("rc.")   &&  // rc.
          n.getUntil (':', name) &&  //    xxx
          n.skip (':'))              //       :
       {
        n.getUntilEOS (value);  // May be blank.

        context.config.set (name, value);
        context.footnote ("Configuration override rc." + name + ":" + value);
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
std::string Arguments::find_limit ()
{
  std::vector <std::pair <std::string, std::string> >::iterator arg;
  for (arg = this->begin (); arg != this->end (); ++arg)
    if (arg->first.find ("limit:") != std::string::npos)
      return arg->first.substr (6);

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
// The first number found in the command line is assumed to be a sequence.  If
// there are two sequences, only the first is recognized, for example:
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

  std::vector <std::pair <std::string, std::string> >::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    // Excluded.
    if (i->second == "program"  ||
        i->second == "command"  ||
        i->second == "rc"       ||
        i->second == "override")
    {
      ;
    }

    // Included.
    else if (i->second == "tag"       ||
             i->second == "pattern"   ||
             i->second == "attr"      ||
             i->second == "attmod"    ||
             i->second == "id"        ||
             i->second == "uuid"      ||
             i->second == "op"        ||
             i->second == "exp"       ||
             i->second == "word")
    {
      // "limit" is special - it is recognized but not included in filters.
      if (i->first.find ("limit:") == std::string::npos)
        filter.push_back (*i);
    }

    // Error.
    else
    {
      // substitution
      throw std::string ("A substitution '") + i->first + "' is not allowed "
            "in a read-only command filter.";
    }
  }

  return filter;
}

////////////////////////////////////////////////////////////////////////////////
Arguments Arguments::extract_write_filter ()
{
  Arguments filter;

  std::vector <std::pair <std::string, std::string> >::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    // Only use args prior to command.
    if (i->second == "command")
      break;

    // Excluded.
    else if (i->second == "program"  ||
             i->second == "rc"       ||
             i->second == "override")
    {
      ;
    }

    // Included.
    else if (i->second == "tag"       ||
             i->second == "pattern"   ||
             i->second == "attr"      ||
             i->second == "attmod"    ||
             i->second == "id"        ||
             i->second == "uuid"      ||
             i->second == "op"        ||
             i->second == "exp"       ||
             i->second == "word")
    {
      // "limit" is special - it is recognized but not included in filters.
      if (i->first.find ("limit:") == std::string::npos)
        filter.push_back (*i);
    }

    // Error.
    else
    {
      // substitution
      throw std::string ("A substitutions '")
            + i->first
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
  std::vector <std::pair <std::string, std::string> >::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
  {
    // Only use args after command.
    if (i->second == "command")
    {
      seen_command = true;
    }

    else if (seen_command)
    {
      // Excluded.
      if (i->second == "program"  ||
          i->second == "rc"       ||
          i->second == "override")
      {
      }

      // Included.
      else if (i->second == "tag"   ||
               i->second == "attr"  ||
               i->second == "subst" ||
               i->second == "op"    ||
               i->second == "word")
      {
        // "limit" is special - it is recognized but not included in filters.
        if (i->first.find ("limit:") == std::string::npos)
          modifications.push_back (*i);
      }

      // Error.
      else
      {
        if (i->second == "pattern")
          throw std::string ("A pattern '")
                + i->first
                + "' is not allowed when modifiying a task.";

        else if (i->second == "attmod")
          throw std::string ("An attribute modifier '")
                + i->first
                + "' is not allowed when modifiying a task.";

        else if (i->second == "exp")
          throw std::string ("An expression '")
                + i->first
                + "' is not allowed when modifiying a task.";

        else if (i->second == "id")
          throw std::string ("A task id cannot be modified.");

        else if (i->second == "uuid")
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
}

////////////////////////////////////////////////////////////////////////////////
