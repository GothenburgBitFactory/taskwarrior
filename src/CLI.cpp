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
#include <iostream>
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
void CLI::entity (const std::string& name, const std::string& value)
{
  _entities.insert (std::pair <std::string, std::string> (name, value));
}

////////////////////////////////////////////////////////////////////////////////
// Capture the original, intact command line arguments.
void CLI::initialize (int argc, const char** argv)
{
  // Clean what needs to be cleaned. Everything in this case.
  _original_args.clear ();
  _args.clear ();

  for (int i = 0; i < argc; ++i)
  {
    _original_args.push_back (argv[i]);

    if (i == 0)
    {
      A a ("arg", argv[i]);
      a.tag ("ORIGINAL");
      a.tag ("BINARY");
      _args.push_back (a);
    }
    else
    {
      A a ("arg", argv[i]);
      a.tag ("ORIGINAL");
      _args.push_back (a);
    }
  }

  aliasExpansion ();
  findOverrides ();
  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
// Capture a single argument, and recalc everything.
void CLI::add (const std::string& arg)
{
  // Clean what needs to be cleaned. Most in this case.
  _args.clear ();
  _original_args.push_back (arg);

  for (int i = 0; i < _original_args.size (); ++i)
  {
    if (i == 0)
    {
      A a ("arg", _original_args[i]);
      a.tag ("ORIGINAL");
      a.tag ("BINARY");
      _args.push_back (a);
    }
    else
    {
      A a ("arg", _original_args[i]);
      a.tag ("ORIGINAL");
      _args.push_back (a);
    }
  }

  aliasExpansion ();
  findOverrides ();
  categorize ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string CLI::getFilter ()
{
// TODO Convert to _args.
/*
  // Remove all the syntactic sugar.
  unsweetenTags ();
  // TODO all the other types: att, attmod, pattern, id, uuid ...
*/

  std::string filter = "";

/*
  if (_filter.size ())
  {
    filter = "(";

    std::vector <A>::const_iterator i;
    for (i = _filter.begin (); i != _filter.end (); ++i)
    {
      if (i != _filter.begin ())
        filter += ' ';

      filter += i->attribute ("raw");
    }

    filter += ')';
  }
*/

  dump ("CLI::getFilter");
  return filter;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> CLI::getWords ()
{
  std::vector <std::string> words;

  // TODO Processing here.

  dump ("CLI::getWords");
  return words;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> CLI::getModifications ()
{
  std::vector <std::string> modifications;

  // TODO Processing here.

  dump ("CLI::getModifications");
  return modifications;
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

    std::vector <A>::iterator i;
    for (i = _args.begin (); i != _args.end (); ++i)
    {
      if (_aliases.find (i->_name) != _aliases.end ())
      {
        std::vector <std::string> lexed;
        Lexer::token_split (lexed, _aliases[i->_name]);

        std::vector <std::string>::iterator l;
        for (l = lexed.begin (); l != lexed.end (); ++l)
        {
          A a ("argLex", *l);
          a.tag ("LEX");
          reconstructed.push_back (a);
        }

        action = true;
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
  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    std::string raw = a->attribute ("raw");

    if (raw.find ("rc:") == 0)
    {
      a->tag ("RC");
      a->attribute ("file", raw.substr (3));
    }
    else if (raw.find ("rc.") == 0)
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

  std::vector <A>::iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
  {
    std::string raw = a->attribute ("raw");
    std::string canonical;
    if (canonicalize (canonical, "cmd", raw))
    {
      readOnly = ! exactMatch ("writecmd", canonical);

      a->tag ("CMD");
      a->tag (readOnly ? "READCMD" : "WRITECMD");
      a->attribute ("canonical", canonical);
      foundCommand = true;
    }
    else if (a->hasTag ("BINARY") ||
             a->hasTag ("CONFIG") ||
             a->hasTag ("RC"))
    {
      // NOP
    }
    else if (foundCommand && ! readOnly)
    {
      a->tag ("MODIFICATION");
    }
    else if (!foundCommand || (foundCommand && readOnly))
    {
      a->tag ("FILTER");
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
// +tag --> tags _hastag_ tag
// -tag --> tags _notag_ tag
void CLI::unsweetenTags ()
{
// TODO Convert from _filter to _args.
/*
  std::vector <A> reconstructed;
  std::vector <A>::iterator i;
  for (i = _filter.begin (); i != _filter.end (); ++i)
  {
    Nibbler n (i->attribute ("raw"));
    std::string tag;
    std::string sign;

    if (n.getN (1, sign)             &&
        (sign == "+" || sign == "-") &&
        n.getUntilEOS (tag)          &&
        tag.find (' ') == std::string::npos)
    {
      A left ("argTag", "tags");
      left.tag ("ATT");
      reconstructed.push_back (left);

      A op ("argTag", sign == "+" ? "_hastag_" : "_notag_");
      op.tag ("OP");
      reconstructed.push_back (op);

      A right ("argTag", tag);
      right.tag ("LITERAL");
      reconstructed.push_back (right);
    }
    else
      reconstructed.push_back (*i);
  }

  _filter = reconstructed;
*/
}

////////////////////////////////////////////////////////////////////////////////
void CLI::dump (const std::string& label) const
{
  std::cout << label << "\n"
            << "  _original_args ";
  Color colorOrigArgs ("gray10 on gray4");
  std::vector <std::string>::const_iterator i;
  for (i = _original_args.begin (); i != _original_args.end (); ++i)
  {
    if (i != _original_args.begin ())
      std::cout << ' ';
    std::cout << colorOrigArgs.colorize (*i);
  }
  std::cout << "\n";

  std::cout << "  _args\n";
  std::vector <A>::const_iterator a;
  for (a = _args.begin (); a != _args.end (); ++a)
    std::cout << "    " << a->dump () << "\n";
}

////////////////////////////////////////////////////////////////////////////////
