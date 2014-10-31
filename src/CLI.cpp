////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2014, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <algorithm>
#include <iostream> // TODO Remove.
#include <Context.h>
#include <Nibbler.h>
#include <Lexer.h>
#include <CLI.h>
#include <Color.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

extern Context context;

// Overridden by rc.abbreviation.minimum.
static int minimumMatchLength = 3;

// Alias expansion limit. Any more indicates some kind of error.
static int safetyValveDefault = 10;

////////////////////////////////////////////////////////////////////////////////
A::A ()
: _name ("")
{
}

////////////////////////////////////////////////////////////////////////////////
A::A (const std::string& name, const std::string& raw)
{
  _name = name;
  attribute ("raw", raw);
}

////////////////////////////////////////////////////////////////////////////////
A::A (const std::string& name, const int raw)
{
  _name = name;
  attribute ("raw", raw);
}

////////////////////////////////////////////////////////////////////////////////
A::A (const std::string& name, const double raw)
{
  _name = name;
  attribute ("raw", raw);
}

////////////////////////////////////////////////////////////////////////////////
A::~A ()
{
}

////////////////////////////////////////////////////////////////////////////////
A::A (const A& other)
: _name (other._name)
, _tags (other._tags)
, _attributes (other._attributes)
{
}

////////////////////////////////////////////////////////////////////////////////
A& A::operator= (const A& other)
{
  if (this != &other)
  {
    _name       = other._name;
    _tags       = other._tags;
    _attributes = other._attributes;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void A::clear ()
{
  _name = "";
  _tags.clear ();
  _attributes.clear ();
}

////////////////////////////////////////////////////////////////////////////////
bool A::hasTag (const std::string& tag) const
{
  if (std::find (_tags.begin (), _tags.end (), tag) != _tags.end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void A::tag (const std::string& tag)
{
  if (! hasTag (tag))
    _tags.push_back (tag);
}

////////////////////////////////////////////////////////////////////////////////
void A::unTag (const std::string& tag)
{
  std::vector <std::string>::iterator i;
  for (i = _tags.begin (); i != _tags.end (); ++i)
  {
    if (*i == tag)
    {
      _tags.erase (i);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void A::unTagAll ()
{
  _tags.clear ();
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
void A::attribute (const std::string& name, const std::string& value)
{
  _attributes[name] = value;
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
void A::attribute (const std::string& name, const int value)
{
  _attributes[name] = format (value);
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
void A::attribute (const std::string& name, const double value)
{
  _attributes[name] = format (value, 1, 8);
}

////////////////////////////////////////////////////////////////////////////////
// Accessor for attributes.
const std::string A::attribute (const std::string& name) const
{
  // Prevent autovivification.
  std::map<std::string, std::string>::const_iterator i = _attributes.find (name);
  if (i != _attributes.end ())
    return i->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void A::removeAttribute (const std::string& name)
{
  _attributes.erase (name);
}

////////////////////////////////////////////////////////////////////////////////
const std::string A::dump () const
{
  std::string output = _name;

  // Dump attributes.
  std::string atts;
  std::map <std::string, std::string>::const_iterator a;
  for (a = _attributes.begin (); a != _attributes.end (); ++a)
  {
    if (a != _attributes.begin ())
      atts += " ";

    atts += a->first + "='\033[33m" + a->second + "\033[0m'";
  }

  if (atts.length ())
    output += " " + atts;

  // Dump tags.
  std::string tags;
  std::vector <std::string>::const_iterator tag;
  for (tag = _tags.begin (); tag != _tags.end (); ++tag)
  {
    if (tags.length ())
      tags += ' ';

         if (*tag == "BINARY")       tags += "\033[1;37;44m"           + *tag + "\033[0m";
    else if (*tag == "CMD")          tags += "\033[1;37;46m"           + *tag + "\033[0m";
    else if (*tag == "FILTER")       tags += "\033[1;37;42m"           + *tag + "\033[0m";
    else if (*tag == "MODIFICATION") tags += "\033[1;37;43m"           + *tag + "\033[0m";
    else if (*tag == "RC")           tags += "\033[1;37;41m"           + *tag + "\033[0m";
    else if (*tag == "CONFIG")       tags += "\033[1;37;101m"          + *tag + "\033[0m";
    else if (*tag == "?")            tags += "\033[38;5;255;48;5;232m" + *tag + "\033[0m";
    else                             tags += "\033[32m"                + *tag + "\033[0m";
  }

  if (tags.length ())
    output += ' ' + tags;

  return output;
}

////////////////////////////////////////////////////////////////////////////////
CLI::CLI ()
{
}

////////////////////////////////////////////////////////////////////////////////
CLI::~CLI ()
{
}

////////////////////////////////////////////////////////////////////////////////
void CLI::alias (const std::string& name, const std::string& value)
{
  _aliases.insert (std::pair <std::string, std::string> (name, value));
}

////////////////////////////////////////////////////////////////////////////////
void CLI::entity (const std::string& category, const std::string& name)
{
  if (_entities.find (category) == _entities.end ())
    _entities.insert (std::pair <std::string, std::string> (category, name));
}

////////////////////////////////////////////////////////////////////////////////
// Capture the original, intact command line arguments.
void CLI::initialize (int argc, const char** argv)
{
  // Clean what needs to be cleaned. Everything in this case.
  _original_args.clear ();
  _id_ranges.clear ();
  _uuid_list.clear ();

  _original_args.push_back (argv[0]);

  for (int i = 1; i < argc; ++i)
    addArg (argv[i]);

  analyze ();
}

////////////////////////////////////////////////////////////////////////////////
// Capture a single argument, and recalc everything.
void CLI::add (const std::string& arg)
{
  // Clean the ID/UUID lists, because they will be rebuilt.
  _id_ranges.clear ();
  _uuid_list.clear ();

  addArg (arg);
  analyze ();
}

////////////////////////////////////////////////////////////////////////////////
// Intended to be called after ::initialize() and ::add(), to perform the final
// analysis. Analysis is also performed directly after the above, because there
// is a need to extract overrides early, before entities are proviedd.
void CLI::analyze (bool parse /* = true */)
{
  // Clean what needs to be cleaned. Most in this case.
  _args.clear ();
  _id_ranges.clear ();
  _uuid_list.clear ();

  for (int i = 0; i < _original_args.size (); ++i)
  {
    if (i == 0)
    {
      A a ("arg", _original_args[i]);
      a.tag ("ORIGINAL");
      a.tag ("BINARY");

      std::string basename = "task";
      std::string raw = _original_args[i];
      std::string::size_type slash = raw.rfind ('/');
      if (slash != std::string::npos)
        basename = raw.substr (slash + 1);

      a.attribute ("basename", basename);
      if (basename == "cal" || basename == "calendar")
        a.tag ("CALENDAR");
      else if (basename == "task" || basename == "tw" || basename == "t")
        a.tag ("TW");

      _args.push_back (a);
    }
    else
    {
      A a ("arg", _original_args[i]);
      a.tag ("ORIGINAL");
      _args.push_back (a);
    }
  }

  // Find argument types.
  aliasExpansion ();
  findOverrides ();
  categorize ();

  if (parse)
  {
    // Remove all the syntactic sugar for FILTERs.
    desugarTags ();
    desugarAttributes ();
    desugarAttributeModifiers ();
    desugarPatterns ();
    findIDs ();
    findUUIDs ();
    insertIDExpr ();
    findOperators ();
    findAttributes ();
    insertJunctions ();
    desugarPlainArgs ();

    // Decompose the elements for MODIFICATIONs.
    decomposeModAttributes ();
    decomposeModAttributeModifiers ();
    decomposeModTags ();
    decomposeModSubstitutions ();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Extract all the FILTER-tagged items.
const std::string CLI::getFilter ()
{
  std::string filter = "";
  if (_args.size ())
  {
    std::vector <A>::const_iterator a;
    for (a = _args.begin (); a != _args.end (); ++a)
    {
      if (a->hasTag ("FILTER"))
      {
        if (filter != "")
          filter += ' ';

        std::string term = a->attribute ("name");
        if (term == "")
          term = a->attribute ("raw");

        filter += term;
      }
    }

    // Only apply parentheses for non-trivial filters.
    if (filter != "")
      filter = "( " + filter + " )";
  }

  return filter;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> CLI::getWords ()
{
  // Re-analyze the arguments, but do not de-sugar or decompose any.  Analysis
  // only.
  analyze (false);

  std::vector <std::string> words;
  std::vector <A>::const_iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (! a->hasTag ("BINARY") &&
        ! a->hasTag ("RC")     &&
        ! a->hasTag ("CONFIG") &&
        ! a->hasTag ("CMD")    &&
        ! a->hasTag ("TERMINATOR") &&
        a->hasTag ("ORIGINAL"))
    {
      words.push_back (a->attribute ("raw"));
    }
  }

  return words;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> CLI::getModifications ()
{
  std::vector <std::string> modifications;

  // TODO Processing here.

  return modifications;
}

////////////////////////////////////////////////////////////////////////////////
// Search for 'value' in _entities category, return canonicalized value.
bool CLI::canonicalize (
  std::string& canonicalized,
  const std::string& category,
  const std::string& value) const
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range (category);

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
  {
    // Shortcut: if an exact match is found, success.
    if (value == e->second)
    {
      canonicalized = value;
      return true;
    }

    options.push_back (e->second);
  }

  // Match against the options, throw away results.
  std::vector <std::string> matches;
  if (autoComplete (value, options, matches, minimumMatchLength) == 1)
  {
    canonicalized = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
const std::string CLI::dump (const std::string& title /* = "CLI Parser" */) const
{
  std::stringstream out;

  out << "\033[1m" << title << "\033[0m\n"
      << "  _original_args\n    ";
  Color colorOrigArgs ("gray10 on gray4");
  std::vector <std::string>::const_iterator i;
  for (i = _original_args.begin (); i != _original_args.end (); ++i)
  {
    if (i != _original_args.begin ())
      out << ' ';
    out << colorOrigArgs.colorize (*i);
  }
  out << "\n";

  out << "  _args\n";
  std::vector <A>::const_iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
    out << "    " << a->dump () << "\n";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// Note: This seems silly - it's essentially performing a low-quality parse. But
//       that is really all that is needed - to separate the args that need to
//       be lexed from those that need to be left alone.
//
// Either the arg is appended to _original_args intact, or the lexemes are.
void CLI::addArg (const std::string& arg)
{
  // Do not lex these constructs.
  if (isTerminator     (arg) ||     // --
      isRCOverride     (arg) ||     // rc:<file>
      isConfigOverride (arg) ||     // rc.<attr>:<value>
      isTag            (arg) ||     // [+-]<tag>
      isUUIDList       (arg) ||     // <uuid>,[uuid ...]
      isUUID           (arg) ||     // <uuid>
      isIDSequence     (arg) ||     // <id>[-<id>][,<id>[-<id>] ...]
      isID             (arg) ||     // <id>
      isPattern        (arg) ||     // /<pattern</
      isSubstitution   (arg) ||     // /<from>/<to>/[g]
      isAttribute      (arg) ||     // <name>[.[~]<modﬁfier>]:<value>
      isOperator       (arg))       // <operator>
  {
    _original_args.push_back (arg);
  }

  // Do not lex, unless lexing reveals OPs.
  else
  {
    // Lex each argument.  If there are multiple lexemes, create sub branches,
    // otherwise no change.
    std::string lexeme;
    Lexer::Type type;
    Lexer lex (arg);
    lex.ambiguity (false);

    std::vector <std::pair <std::string, Lexer::Type> > lexemes;
    while (lex.token (lexeme, type))
      lexemes.push_back (std::pair <std::string, Lexer::Type> (lexeme, type));

    // This one looks interesting.
    if (lexemes.size () > 1)
    {
      bool foundOP = false;
      std::vector <std::pair <std::string, Lexer::Type> >::iterator l;
      for (l = lexemes.begin (); l != lexemes.end (); ++l)
        if (l->second == Lexer::typeOperator)
          foundOP = true;

      if (foundOP)
      {
        for (l = lexemes.begin (); l != lexemes.end (); ++l)
          _original_args.push_back (l->first);
      }
      else
        _original_args.push_back (arg);
    }
    else
      _original_args.push_back (arg);
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::aliasExpansion ()
{
  bool action;
  int counter = 0;
  do
  {
    action = false;
    std::vector <A> reconstructed;

    bool terminated = false;
    std::string raw;
    std::vector <A>::iterator i;
    for (i = _args.begin (); i != _args.end (); ++i)
    {
      raw = i->attribute ("raw");

      if (raw == "--")
        terminated = true;

      if (! terminated)
      {
        if (_aliases.find (raw) != _aliases.end ())
        {
          std::vector <std::string> lexed;
          Lexer::token_split (lexed, _aliases[raw]);

          std::vector <std::string>::iterator l;
          for (l = lexed.begin (); l != lexed.end (); ++l)
          {
            A a ("argLex", *l);
            a.tag ("ALIAS");
            a.tag ("LEX");
            reconstructed.push_back (a);
          }

          action = true;
        }
        else
          reconstructed.push_back (*i);
      }
      else
        reconstructed.push_back (*i);
    }

    _args = reconstructed;
  }
  while (action && counter++ < safetyValveDefault);
}

////////////////////////////////////////////////////////////////////////////////
void CLI::findOverrides ()
{
  std::string raw;
  bool terminated = false;
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    raw = a->attribute ("raw");

    if (raw == "--")
      terminated = true;

    if (terminated)
      continue;

    if (isRCOverride (raw))
    {
      a->tag ("RC");
      a->attribute ("file", raw.substr (3));
    }
    else if (isConfigOverride (raw))
    {
      std::string::size_type sep = raw.find ('=', 3);
      if (sep == std::string::npos)
        sep = raw.find (':', 3);
      if (sep != std::string::npos)
      {
        a->tag ("CONFIG");
        a->attribute ("name", raw.substr (3, sep - 3));
        a->attribute ("value", raw.substr (sep + 1));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::categorize ()
{
  bool foundCommand = false;
  bool readOnly = false;
  bool terminated = false;

  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    std::string raw = a->attribute ("raw");

    if (raw == "--")
    {
      a->unTagAll ();
      a->tag ("ORIGINAL");
      a->tag ("TERMINATOR");
      terminated = true;
      continue;
    }

    if (terminated)
    {
      a->unTagAll ();
      a->tag ("ORIGINAL");
      a->tag ("TERMINATED");
      a->tag ("WORD");
    }

    if (raw.find (' ') != std::string::npos)
      a->tag ("QUOTED");

    std::string canonical;
    if (! terminated   &&
        ! foundCommand &&
        canonicalize (canonical, "cmd", raw))
    {
      readOnly = ! exactMatch ("writecmd", canonical);

      a->tag ("CMD");
      a->tag (readOnly ? "READCMD" : "WRITECMD");
      a->attribute ("canonical", canonical);
      foundCommand = true;
    }
    else if (a->hasTag ("TERMINATOR") ||
             a->hasTag ("BINARY")     ||
             a->hasTag ("CONFIG")     ||
             a->hasTag ("RC"))
    {
      // NOP
    }
    else if (foundCommand && ! readOnly)
    {
      a->tag ("MODIFICATION");

      // If the argument contains a space, it was quoted.  Record that.
      if (! noSpaces (raw))
        a->tag ("QUOTED");
    }
    else if (!foundCommand || (foundCommand && readOnly))
    {
      a->tag ("FILTER");

      // If the argument contains a space, it was quoted.  Record that.
      if (! noSpaces (raw))
        a->tag ("QUOTED");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Search for exact 'value' in _entities category.
bool CLI::exactMatch (
  const std::string& category,
  const std::string& value) const
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range (category);

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
  {
    // Shortcut: if an exact match is found, success.
    if (value == e->second)
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// +tag --> tags _hastag_ tag
// -tag --> tags _notag_ tag
void CLI::desugarTags ()
{
  std::vector <A> reconstructed;
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      Nibbler n (a->attribute ("raw"));
      std::string tag;
      std::string sign;

      if (n.getN (1, sign)             &&
          (sign == "+" || sign == "-") &&
          n.getUntilEOS (tag)          &&
          tag.find (' ') == std::string::npos)
      {
        A left ("argTag", "tags");
        left.tag ("ATTRIBUTE");
        left.tag ("FILTER");
        reconstructed.push_back (left);

        A op ("argTag", sign == "+" ? "_hastag_" : "_notag_");
        op.tag ("OP");
        op.tag ("FILTER");
        reconstructed.push_back (op);

        A right ("argTag", tag);
        right.tag ("LITERAL");
        right.tag ("FILTER");
        reconstructed.push_back (right);
      }
      else
        reconstructed.push_back (*a);
    }
    else
      reconstructed.push_back (*a);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
// <name>:['"][<value>]['"] --> name = value
void CLI::desugarAttributes ()
{
  std::vector <A> reconstructed;
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      // Look for a valid attribute name.
      bool found = false;
      Nibbler n (a->attribute ("raw"));
      std::string name;
      if (n.getName (name) &&
          name.length ())
      {
        if (n.skip (':') ||
            n.skip ('='))
        {
          std::string value;
          if (n.getQuoted   ('"', value)  ||
              n.getQuoted   ('\'', value) ||
              n.getUntilEOS (value)       ||
              n.depleted ())
          {
            if (value == "")
              value = "''";

            std::string canonical;
            if (canonicalize (canonical, "uda", name))
            {
              A lhs ("argUDA", name);
              lhs.attribute ("name", canonical);
              lhs.tag ("UDA");
              lhs.tag ("ATTRIBUTE");
              lhs.tag ("FILTER");

              A op ("argUDA", "=");
              op.tag ("OP");
              op.tag ("FILTER");

              A rhs ("argUDA", value);
              rhs.tag ("LITERAL");
              rhs.tag ("FILTER");

              reconstructed.push_back (lhs);
              reconstructed.push_back (op);
              reconstructed.push_back (rhs);
              found = true;
            }

            else if (canonicalize (canonical, "pseudo", name))
            {
              A lhs ("argPseudo", name);
              lhs.attribute ("name", canonical);
              lhs.tag ("PSEUDO");
              reconstructed.push_back (lhs);
              found = true;
            }

            else if (canonicalize (canonical, "attribute", name))
            {
              A lhs ("argAtt", name);
              lhs.attribute ("name", canonical);
              lhs.tag ("ATTRIBUTE");
              lhs.tag ("FILTER");

              std::string operatorLiteral = "=";
              if (canonical == "status")
                operatorLiteral = "==";

              A op ("argAtt", operatorLiteral);
              op.tag ("OP");
              op.tag ("FILTER");

              A rhs ("argAtt", value);
              rhs.tag ("LITERAL");
              rhs.tag ("FILTER");

              reconstructed.push_back (lhs);
              reconstructed.push_back (op);
              reconstructed.push_back (rhs);
              found = true;
            }
          }
        }
      }

      if (!found)
        reconstructed.push_back (*a);
    }
    else
      reconstructed.push_back (*a);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
// <name>.[~]<mod>[:=]['"]<value>['"] --> name <op> value
void CLI::desugarAttributeModifiers ()
{
  std::vector <A> reconstructed;
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      // Look for a valid attribute name.
      bool found = false;
      Nibbler n (a->attribute ("raw"));
      std::string name;
      if (n.getUntil (".", name) &&
          name.length ())
      {
        std::string canonical;
        if (canonicalize (canonical, "attribute", name) ||
            canonicalize (canonical, "uda",       name))
        {
          if (n.skip ('.'))
          {
            std::string sense = "+";
            if (n.skip ('~'))
              sense = "-";

            std::string modifier;
            n.getUntilOneOf (":=", modifier);

            if (n.skip (':') ||
                n.skip ('='))
            {
              std::string value;
              if (n.getQuoted   ('"', value)  ||
                  n.getQuoted   ('\'', value) ||
                  n.getUntilEOS (value)       ||
                  n.depleted ())
              {
                if (value == "")
                  value = "''";

                A lhs ("argAttMod", name);
                lhs.tag ("ATTMOD");
                lhs.tag ("FILTER");

                lhs.attribute ("name", canonical);
                lhs.attribute ("modifier", modifier);
                lhs.attribute ("sense", sense);

                A op ("argAttmod", "");
                op.tag ("FILTER");

                A rhs ("argAttMod", "");
                rhs.tag ("FILTER");

                if (modifier == "before" || modifier == "under" || modifier == "below")
                {
                  op.attribute ("raw", "<");
                  op.tag ("OP");
                  rhs.attribute ("raw", value);
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "after" || modifier == "over" || modifier == "above")
                {
                  op.attribute ("raw", ">");
                  op.tag ("OP");
                  rhs.attribute ("raw", value);
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "none")
                {
                  op.attribute ("raw", "==");
                  op.tag ("OP");
                  rhs.attribute ("raw", "''");
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "any")
                {
                  op.attribute ("raw", "!=");
                  op.tag ("OP");
                  rhs.attribute ("raw", "''");
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "is" || modifier == "equals")
                {
                  op.attribute ("raw", "==");
                  op.tag ("OP");
                  rhs.attribute ("raw", value);
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "isnt" || modifier == "not")
                {
                  op.attribute ("raw", "!=");
                  op.tag ("OP");
                  rhs.attribute ("raw", value);
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "has" || modifier == "contains")
                {
                  op.attribute ("raw", "~");
                  op.tag ("OP");
                  rhs.attribute ("raw", value);
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "hasnt")
                {
                  op.attribute ("raw", "!~");
                  op.tag ("OP");
                  rhs.attribute ("raw", value);
                  rhs.tag ("LITERAL");
                }
                else if (modifier == "startswith" || modifier == "left")
                {
                  op.attribute ("raw", "~");
                  op.tag ("OP");
                  rhs.attribute ("raw", "'^" + value + "'");
                  rhs.tag ("REGEX");
                }
                else if (modifier == "endswith" || modifier == "right")
                {
                  op.attribute ("raw", "~");
                  op.tag ("OP");
                  rhs.attribute ("raw", "'" + value + "$'");
                  rhs.tag ("REGEX");
                }
                else if (modifier == "word")
                {
                  op.attribute ("raw", "~");
                  op.tag ("OP");
#if defined (DARWIN)
                  rhs.attribute ("raw", value);
#elif defined (SOLARIS)
                  rhs.attribute ("raw", "'\\<" + value + "\\>'");
#else
                  rhs.attribute ("raw", "'\\b" + value + "\\b'");
#endif
                  rhs.tag ("REGEX");
                }
                else if (modifier == "noword")
                {
                  op.attribute ("raw", "!~");
                  op.tag ("OP");
#if defined (DARWIN)
                  rhs.attribute ("raw", value);
#elif defined (SOLARIS)
                  rhs.attribute ("raw", "'\\<" + value + "\\>'");
#else
                  rhs.attribute ("raw", "'\\b" + value + "\\b'");
#endif
                  rhs.tag ("REGEX");
                }
                else
                  throw format (STRING_PARSER_UNKNOWN_ATTMOD, modifier);

                reconstructed.push_back (lhs);
                reconstructed.push_back (op);
                reconstructed.push_back (rhs);
                found = true;
              }
            }
          }
        }
      }

      if (!found)
        reconstructed.push_back (*a);
    }
    else
      reconstructed.push_back (*a);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
// /pattern/ --> description ~ 'pattern'
void CLI::desugarPatterns ()
{
  std::vector <A> reconstructed;
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      Nibbler n (a->attribute ("raw"));
      std::string pattern;

      if (n.getQuoted ('/', pattern) &&
          n.depleted () &&
          pattern.length () > 0)
      {
        A lhs ("argPattern", "description");
        lhs.tag ("ATTRIBUTE");
        lhs.tag ("FILTER");
        reconstructed.push_back (lhs);

        A op ("argPattern", "~");
        op.tag ("OP");
        op.tag ("FILTER");
        reconstructed.push_back (op);

        A rhs ("argPattern", "'" + pattern + "'");
        rhs.tag ("LITERAL");
        rhs.tag ("FILTER");
        reconstructed.push_back (rhs);
      }
      else
        reconstructed.push_back (*a);
    }
    else
      reconstructed.push_back (*a);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
// An ID sequence can be:
//
//   a single ID:          1
//   a list of IDs:        1,3,5
//   a list of IDs:        1 3 5
//   a range:              5-10
//   or a combination:     1,3,5-10 12
//
void CLI::findIDs ()
{
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      bool found = false;

      // IDs have a limited character set.
      std::string raw = a->attribute ("raw");
      if (raw.find_first_not_of ("0123456789,-") == std::string::npos)
      {
        // Container for min/max ID ranges.
        std::vector <std::pair <int, int> > ranges;

        // Split the ID list into elements.
        std::vector <std::string> elements;
        split (elements, raw, ',');

        bool is_an_id = true;
        std::vector <std::string>::iterator e;
        for (e = elements.begin (); e != elements.end (); ++e)
        {
          // Split the ID range into min/max.
          std::vector <std::string> terms;
          split (terms, *e, '-');

          if (terms.size () == 1)
          {
            if (! digitsOnly (terms[0]))
            {
              is_an_id = false;
              break;
            }

            Nibbler n (terms[0]);
            int id;
            if (n.getUnsignedInt (id) &&
                n.depleted ())
            {
              ranges.push_back (std::pair <int, int> (id, id));
            }
            else
            {
              is_an_id = false;
              break;
            }
          }
          else if (terms.size () == 2)
          {
            if (! digitsOnly (terms[0]) ||
                ! digitsOnly (terms[1]))
            {
              is_an_id = false;
              break;
            }

            Nibbler n_min (terms[0]);
            Nibbler n_max (terms[1]);
            int id_min;
            int id_max;
            if (n_min.getUnsignedInt (id_min) &&
                n_min.depleted ()             &&
                n_max.getUnsignedInt (id_max) &&
                n_max.depleted ())
            {
              if (id_min > id_max)
                throw std::string (STRING_PARSER_RANGE_INVERTED);

              ranges.push_back (std::pair <int, int> (id_min, id_max));
            }
            else
            {
              is_an_id = false;
              break;
            }
          }
          else
          {
            is_an_id = false;
            break;
          }
        }

        if (is_an_id)
        {
          a->tag ("ID");

          // Save the ranges.
          std::vector <std::pair <int, int> >::iterator r;
          for (r = ranges.begin (); r != ranges.end (); ++r)
            _id_ranges.push_back (*r);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::findUUIDs ()
{
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      // UUIDs have a limited character set.
      std::string raw = a->attribute ("raw");
      if (raw.find_first_not_of ("0123456789abcdefABCDEF-,") == std::string::npos)
      {
        Nibbler n (raw);
        std::vector <std::string> uuidList;
        std::string uuid;
        if (n.getUUID (uuid) ||
            n.getPartialUUID (uuid))
        {
          uuidList.push_back (uuid);

          while (n.skip (','))
          {
            if (! n.getUUID (uuid) &&
                ! n.getPartialUUID (uuid))
              throw std::string (STRING_PARSER_UUID_AFTER_COMMA);

            uuidList.push_back (uuid);
          }

          if (n.depleted ())
          {
            a->tag ("UUID");

            // Save the list.
            std::vector <std::string>::iterator u;
            for (u = uuidList.begin (); u != uuidList.end (); ++u)
              _uuid_list.push_back (*u);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::insertIDExpr ()
{
  // Iterate over all args. The first ID/UUID arg found will be replaced by
  // the combined ID clause. All other ID/UUID args are removed.
  bool foundID = false;
  std::vector <A> reconstructed;
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER") &&
        (a->hasTag ("ID") ||
         a->hasTag ("UUID")))
    {
      if (! foundID)
      {
        foundID = true;

        // Construct a single sequence that represents all _id_ranges and
        // _uuid_list in one clause. This is essentially converting this:
        //
        //   1,2-3 uuid,uuid uuid 4
        //
        // into:
        //
        //   (
        //        ( id == 1 )
        //     or ( ( id >= 2 ) and ( id <= 3 ) )
        //     or ( id == 4 )
        //     or ( uuid = $UUID )
        //     or ( uuid = $UUID )
        //   )

        // Building block operators.
        A openParen  ("argSeq", "(");    openParen.tag  ("FILTER"); openParen.tag  ("OP");
        A closeParen ("argSeq", ")");    closeParen.tag ("FILTER"); closeParen.tag ("OP");
        A opOr       ("argSeq", "or");   opOr.tag       ("FILTER"); opOr.tag       ("OP");
        A opAnd      ("argSeq", "and");  opAnd.tag      ("FILTER"); opAnd.tag      ("OP");
        A opSimilar  ("argSeq", "=");    opSimilar.tag  ("FILTER"); opSimilar.tag  ("OP");
        A opEqual    ("argSeq", "==");   opEqual.tag    ("FILTER"); opEqual.tag    ("OP");
        A opGTE      ("argSeq", ">=");   opGTE.tag      ("FILTER"); opGTE.tag      ("OP");
        A opLTE      ("argSeq", "<=");   opLTE.tag      ("FILTER"); opLTE.tag      ("OP");

        // Building block attributes.
        A argID ("argSeq", "id");
        argID.tag ("FILTER");
        argID.tag ("ATTRIBUTE");

        A argUUID ("argSeq", "uuid");
        argUUID.tag ("FILTER");
        argUUID.tag ("ATTRIBUTE");

        reconstructed.push_back (openParen);

        // Add all ID ranges.
        std::vector <std::pair <int, int> >::iterator r;
        for (r = _id_ranges.begin (); r != _id_ranges.end (); ++r)
        {
          if (r != _id_ranges.begin ())
            reconstructed.push_back (opOr);

          if (r->first == r->second)
          {
            reconstructed.push_back (openParen);
            reconstructed.push_back (argID);
            reconstructed.push_back (opEqual);

            A value ("argSeq", r->first);
            value.tag ("FILTER");
            value.tag ("LITERAL");
            value.tag ("NUMBER");
            reconstructed.push_back (value);

            reconstructed.push_back (closeParen);
          }
          else
          {
            reconstructed.push_back (openParen);
            reconstructed.push_back (argID);
            reconstructed.push_back (opGTE);

            A startValue ("argSeq", r->first);
            startValue.tag ("FILTER");
            startValue.tag ("LITERAL");
            startValue.tag ("NUMBER");
            reconstructed.push_back (startValue);

            reconstructed.push_back (opAnd);
            reconstructed.push_back (argID);
            reconstructed.push_back (opLTE);

            A endValue ("argSeq", r->second);
            endValue.tag ("FILTER");
            endValue.tag ("LITERAL");
            endValue.tag ("NUMBER");
            reconstructed.push_back (endValue);

            reconstructed.push_back (closeParen);
          }
        }

        // Combine the ID and UUID sections wiþh 'or'.
        if (_id_ranges.size () &&
            _uuid_list.size ())
          reconstructed.push_back (opOr);

        // Add all UUID list items.
        std::vector <std::string>::iterator u;
        for (u = _uuid_list.begin (); u != _uuid_list.end (); ++u)
        {
          if (u != _uuid_list.begin ())
            reconstructed.push_back (opOr);

          reconstructed.push_back (openParen);
          reconstructed.push_back (argUUID);
          reconstructed.push_back (opSimilar);

          A value ("argSeq", "'" + *u + "'");
          value.tag ("FILTER");
          value.tag ("LITERAL");
          value.tag ("STRING");
          reconstructed.push_back (value);

          reconstructed.push_back (closeParen);
        }

        reconstructed.push_back (closeParen);
      }

      // No 'else' which cause all other ID/UUID args to be eaten.
    }
    else
      reconstructed.push_back (*a);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
void CLI::desugarPlainArgs ()
{
  std::vector <A> reconstructed;
  std::vector <A>::iterator a;
  std::vector <A>::iterator prev = _args.begin ();
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a != prev                 && // Not the first arg.
        ! prev->hasTag ("OP")     && // An OP before protects the arg.
        a->hasTag ("FILTER")      &&
        ! a->hasTag ("ATTRIBUTE") &&
        ! a->hasTag ("ATTMOD")    &&
        ! a->hasTag ("OP")        &&
        ! a->hasTag ("REGEX")     &&
        ! a->hasTag ("LITERAL"))
    {
      A lhs ("argPattern", "description");
      lhs.tag ("ATTRIBUTE");
      lhs.tag ("FILTER");
      reconstructed.push_back (lhs);

      A op ("argPattern", "~");
      op.tag ("OP");
      op.tag ("FILTER");
      reconstructed.push_back (op);

      A rhs ("argPattern", "'" + a->attribute ("raw") + "'");
      rhs.tag ("LITERAL");
      rhs.tag ("FILTER");
      reconstructed.push_back (rhs);
    }
    else
      reconstructed.push_back (*a);

    prev = a;
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
void CLI::findOperators ()
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range ("operator");

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
    options.push_back (e->second);

  // Walk the arguments and tag as OP.
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
    if (a->hasTag ("FILTER"))
      if (std::find (options.begin (), options.end (), a->attribute ("raw")) != options.end ())
        a->tag ("OP");
}

////////////////////////////////////////////////////////////////////////////////
void CLI::findAttributes ()
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range ("attribute");

  // Extract a list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
    options.push_back (e->second);

  // Walk the arguments and tag as OP.
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
    if (a->hasTag ("FILTER"))
      if (std::find (options.begin (), options.end (), a->attribute ("raw")) != options.end ())
        a->tag ("ATTRIBUTE");
}

////////////////////////////////////////////////////////////////////////////////
// Two consecutive FILTER, non-OP arguments that are not "(" or ")" need an
// "and" operator inserted between them.
//
//   ) <non-op>         -->  ) and <non-op>
//   <non-op> (         -->  <non-op> <and> (
//   ) (                -->  ) and (
//   <non-op> <non-op>  -->  <non-op> and <non-op>
//
void CLI::insertJunctions ()
{
  std::vector <A> reconstructed;
  std::vector <A>::iterator prev = _args.begin ();
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("FILTER"))
    {
      // The prev iterator should be the first FILTER arg.
      if (prev == _args.begin ())
        prev = a;

      // Insert AND between terms.
      else if (a != prev)
      {
        if ((! prev->hasTag ("OP")          && a->attribute ("raw") == "(") ||
            (! prev->hasTag ("OP")          && ! a->hasTag ("OP"))          ||
            (prev->attribute ("raw") == ")" && ! a->hasTag ("OP"))          ||
            (prev->attribute ("raw") == ")" && a->attribute ("raw") == "("))
        {
          A opOr ("argOp", "and");
          opOr.tag ("FILTER");
          opOr.tag ("OP");
          reconstructed.push_back (opOr);
        }
      }

      // Previous FILTER arg.
      prev = a;
    }

    reconstructed.push_back (*a);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
void CLI::decomposeModAttributes ()
{
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("TERMINATOR"))
      break;

    if (a->hasTag ("MODIFICATION"))
    {
      // Look for a valid attribute name.
      Nibbler n (a->attribute ("raw"));
      std::string name;
      if (n.getName (name) &&
          name.length ())
      {
        if (n.skip (':'))
        {
          std::string value;
          if (n.getQuoted   ('"', value)  ||
              n.getQuoted   ('\'', value) ||
              n.getUntilEOS (value)       ||
              n.depleted ())
          {
            if (value == "")
              value = "''";

            std::string canonical;
            if (canonicalize (canonical, "uda", name))
            {
              a->attribute ("name", canonical);
              a->attribute ("value", value);
              a->tag ("UDA");
              a->tag ("MODIFIABLE");
            }

            else if (canonicalize (canonical, "attribute", name))
            {
              a->attribute ("name", canonical);
              a->attribute ("value", value);
              a->tag ("ATTRIBUTE");

              std::map <std::string, Column*>::const_iterator col;
              col = context.columns.find (canonical);
              if (col != context.columns.end () &&
                  col->second->modifiable ())
              {
                a->tag ("MODIFIABLE");
              }
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::decomposeModAttributeModifiers ()
{
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("TERMINATOR"))
      break;

    if (a->hasTag ("MODIFICATION"))
    {
      // Look for a valid attribute name.
      Nibbler n (a->attribute ("raw"));
      std::string name;
      if (n.getUntil (".", name) &&
          name.length ())
      {
        std::string canonical;
        if (canonicalize (canonical, "attribute", name) ||
            canonicalize (canonical, "uda",       name))
        {
          if (n.skip ('.'))
          {
            std::string sense = "+";
            if (n.skip ('~'))
              sense = "-";

            std::string modifier;
            n.getUntilOneOf (":=", modifier);

            if (n.skip (':') ||
                n.skip ('='))
            {
              std::string value;
              if (n.getQuoted   ('"', value)  ||
                  n.getQuoted   ('\'', value) ||
                  n.getUntilEOS (value)       ||
                  n.depleted ())
              {
                if (value == "")
                  value = "''";

                std::string canonical;
                if (canonicalize (canonical, "uda", name))
                {
                  a->attribute ("name", canonical);
                  a->attribute ("modifier", modifier);
                  a->attribute ("sense", sense);
                  a->attribute ("value", value);
                  a->tag ("UDA");
                  a->tag ("MODIFIABLE");
                }

                else if (canonicalize (canonical, "attribute", name))
                {
                  a->attribute ("name", canonical);
                  a->attribute ("modifier", modifier);
                  a->attribute ("sense", sense);
                  a->attribute ("value", value);
                  a->tag ("ATTMOD");

                  std::map <std::string, Column*>::const_iterator col;
                  col = context.columns.find (canonical);
                  if (col != context.columns.end () &&
                      col->second->modifiable ())
                  {
                    a->tag ("MODIFIABLE");
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::decomposeModTags ()
{
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("TERMINATOR"))
      break;

    if (a->hasTag ("MODIFICATION"))
    {
      Nibbler n (a->attribute ("raw"));
      std::string tag;
      std::string sign;

      if (n.getN (1, sign)             &&
          (sign == "+" || sign == "-") &&
          n.getUntilEOS (tag)          &&
          tag.find (' ') == std::string::npos)
      {
        a->attribute ("name", tag);
        a->attribute ("sign", sign);
        a->tag ("TAG");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void CLI::decomposeModSubstitutions ()
{
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    if (a->hasTag ("TERMINATOR"))
      break;

    if (a->hasTag ("MODIFICATION"))
    {
      std::string raw = a->attribute ("raw");
      Nibbler n (raw);
      std::string from;
      std::string to;
      bool global = false;
      if (n.getQuoted ('/', from) &&
          n.backN ()              &&
          n.getQuoted ('/', to))
      {
        if (n.skip ('g'))
          global = true;

        if (n.depleted () &&
            ! Directory (raw).exists ())
        {
          a->tag ("SUBSTITUTION");
          a->attribute ("from", from);
          a->attribute ("to", to);
          a->attribute ("global", global ? 1 : 0);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isTerminator (const std::string& raw) const
{
  return (raw == "--");
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isRCOverride (const std::string& raw) const
{
  if (raw.length () > 3 && raw.substr (0, 3) == "rc:")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isConfigOverride (const std::string& raw) const
{
  if (raw.length () > 3 && raw.substr (0, 3) == "rc.")
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isTag (const std::string& raw) const
{
  if (raw.size () >= 2                 &&
      (raw[0] == '+' || raw[0] == '-') &&
      raw.find (' ') == std::string::npos)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isUUIDList (const std::string& raw) const
{
  // UUIDs have a limited character set.
  if (raw.find_first_not_of ("0123456789abcdefABCDEF-,") == std::string::npos)
  {
    Nibbler n (raw);
    std::string token;
    if (n.getUUID (token) ||
        n.getPartialUUID (token))
    {
      while (n.skip (','))
        if (! n.getUUID (token) &&
            ! n.getPartialUUID (token))
          return false;

      if (n.depleted ())
        return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isUUID (const std::string& raw) const
{
  // UUIDs have a limited character set.
  if (raw.find_first_not_of ("0123456789abcdefABCDEF-") == std::string::npos)
  {
    Nibbler n (raw);
    std::string token;
    if (n.getUUID (token) || n.getPartialUUID (token))
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isIDSequence (const std::string& raw) const
{
  if (raw.find_first_not_of ("0123456789,-") == std::string::npos)
  {
    // Split the ID list into elements.
    std::vector <std::string> elements;
    split (elements, raw, ',');

    std::vector <std::string>::iterator e;
    for (e = elements.begin (); e != elements.end (); ++e)
    {
      // Split the ID range into min/max.
      std::vector <std::string> terms;
      split (terms, *e, '-');

      if (terms.size () == 1 &&
          ! isID (terms[0]))
        return false;

      else if (terms.size () == 2 &&
          (! isID (terms[0]) ||
           ! isID (terms[1])))
        return false;

      else if (terms.size () > 2)
        return false;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isID (const std::string& raw) const
{
  return digitsOnly (raw);
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isPattern (const std::string& raw) const
{
  if (raw.length () > 2 &&
      raw[0] == '/'     &&
      raw[raw.length () - 1] == '/')
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// The non-g case is caught by ::isPattern, but not categorized, so it doesn't
// matter.
bool CLI::isSubstitution (const std::string& raw) const
{
  if (raw.length () > 3             &&      // /x// = length 4
      raw[0] == '/'                 &&
      raw[raw.length () - 2] == '/' &&
      raw[raw.length () - 1] == 'g')
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Covers attribute and attribute modifiers.
// <attr>.[~]<mod>[:=]...
bool CLI::isAttribute (const std::string& raw) const
{
  std::string::size_type colon = raw.find (":");
  std::string::size_type equal = raw.find ("=");

  std::string attr = "";
  if (colon != std::string::npos)
    attr = raw.substr (0, colon);
  else if (equal != std::string::npos)
    attr = raw.substr (0, equal);
  else
    return false;

  std::string::size_type dot = attr.find (".");
  std::string mod = "";
  if (dot != std::string::npos)
  {
    mod = attr.substr (dot + 1);
    attr = attr.substr (0, dot);

    if (mod[0] == '~')
      mod = mod.substr (1);

/*
    TODO Entities are not loaded yet. Hmm.

    if (! canonicalize (mod, "modifier", mod))
      return false;
*/
  }

/*
  TODO Entities are not loaded yet. Hmm.

  if (! canonicalize (attr, "attribute", attr))
    return false;
*/

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CLI::isOperator (const std::string& raw) const
{
  // Find the category.
  std::pair <std::multimap <std::string, std::string>::const_iterator, std::multimap <std::string, std::string>::const_iterator> c;
  c = _entities.equal_range ("operator");

  // Walk the list of entities for category.
  std::vector <std::string> options;
  std::multimap <std::string, std::string>::const_iterator e;
  for (e = c.first; e != c.second; ++e)
    if (raw == e->second)
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
