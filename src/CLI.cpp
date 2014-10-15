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
A::A (const std::string& name)
: _name (name)
{
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
std::string A::attribute (const std::string& name)
{
  // Prevent autovivification.
  std::map<std::string, std::string>::iterator i = _attributes.find (name);
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
std::string A::dump ()
{
  return "A::dump";
}

////////////////////////////////////////////////////////////////////////////////
CLI::CLI ()
: _program ("")
, _rc ("")
, _command ("")
, _readOnly (false)
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
  _program = "";
  _original_args.clear ();
  _args.clear ();
  _rc = "";
  _overrides.clear ();
  _command = "";
  _readOnly = false;
  _filter.clear ();
  _modifications.clear ();

  _program = argv[0];
  for (int i = 1; i < argc; ++i)
    _original_args.push_back (argv[i]);

  _args = _original_args;

  aliasExpansion ();
  extractOverrides ();
  categorize ();

  dump ("CLI::initialize");
}

////////////////////////////////////////////////////////////////////////////////
// Capture a single argument, and recalc everything.
void CLI::add (const std::string& arg)
{
  // Clean what needs to be cleaned. Most in this case.
  _args.clear ();
  _rc = "";
  _overrides.clear ();
  _command = "";
  _readOnly = false;
  _filter.clear ();
  _modifications.clear ();

  _original_args.push_back (arg);
  _args = _original_args;

  aliasExpansion ();
  extractOverrides ();
  categorize ();

  dump ("CLI::add");
}

////////////////////////////////////////////////////////////////////////////////
const std::string CLI::getFilter ()
{
  // Remove all the syntactic sugar.
  unsweetenTags ();

  std::string filter = "";

  if (_filter.size ())
  {
    filter = "(";

    std::vector <std::string>::const_iterator i;
    for (i = _filter.begin (); i != _filter.end (); ++i)
    {
      if (i != _filter.begin ())
        filter += ' ';

      filter += *i;
    }

    filter += ')';
  }

  return filter;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> CLI::getWords ()
{
  std::vector <std::string> words;
  // TODO Processing here.
  return words;
}

////////////////////////////////////////////////////////////////////////////////
const std::vector <std::string> CLI::getModifications ()
{
  // Remove all the syntactic sugar.

  std::vector <std::string> modifications;
  // TODO Processing here.
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
    std::vector <std::string> reconstructed;

    std::vector <std::string>::iterator i;
    for (i = _args.begin (); i != _args.end (); ++i)
    {
      if (_aliases.find (*i) != _aliases.end ())
      {
        std::vector <std::string> lexed;
        Lexer::token_split (lexed, _aliases[*i]);

        std::vector <std::string>::iterator l;
        for (l = lexed.begin (); l != lexed.end (); ++l)
          reconstructed.push_back (*l);

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
void CLI::extractOverrides ()
{
  std::vector <std::string> reconstructed;

  std::vector <std::string>::iterator i;
  for (i = _args.begin (); i != _args.end (); ++i)
  {
    if (i->find ("rc:") == 0)
    {
      _rc = i->substr (3);
    }
    else if (i->find ("rc.") == 0)
    {
      std::string::size_type sep = i->find ('=', 3);
      if (sep == std::string::npos)
        sep = i->find (':', 3);
      if (sep != std::string::npos)
        _overrides[i->substr (3, sep - 3)] = i->substr (sep + 1);
    }
    else
      reconstructed.push_back (*i);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
void CLI::categorize ()
{
  bool foundCommand = false;

  std::vector <std::string>::iterator i;
  for (i = _args.begin (); i != _args.end (); ++i)
  {
    if (canonicalize (_command, "cmd", *i))
    {
      foundCommand = true;
      _readOnly = ! exactMatch ("writecmd", _command);
    }
    else if (foundCommand && ! _readOnly)
    {
      _modifications.push_back (*i);
    }
    else
    {
      _filter.push_back (*i);
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
  std::vector <std::string> reconstructed;

  std::vector <std::string>::iterator i;
  for (i = _filter.begin (); i != _filter.end (); ++i)
  {
    Nibbler n (*i);
    std::string tag;
    std::string sign;

    if (n.getN (1, sign)             &&
        (sign == "+" || sign == "-") &&
        n.getUntilEOS (tag)          &&
        tag.find (' ') == std::string::npos)
    {
        reconstructed.push_back ("tags");
        reconstructed.push_back (sign == "+" ? "_hastag_" : "_notag_");
        reconstructed.push_back (tag);
    }
    else
      reconstructed.push_back (*i);
  }

  _filter = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
void CLI::dump (const std::string& label) const
{
  std::cout << "# " << label << "\n"
            << "#   _program       " << _program << "\n";

  std::cout << "#   _original_args ";
  Color colorOrigArgs ("gray10 on gray4");
  std::vector <std::string>::const_iterator i;
  for (i = _original_args.begin (); i != _original_args.end (); ++i)
  {
    if (i != _original_args.begin ())
      std::cout << ' ';
    std::cout << colorOrigArgs.colorize (*i);
  }
  std::cout << "\n";

  std::cout << "#   _args          ";
  Color colorArgs ("gray16 on gray4");
  for (i = _args.begin (); i != _args.end (); ++i)
  {
    if (i != _args.begin ())
      std::cout << ' ';
    std::cout << colorArgs.colorize (*i);
  }
  std::cout << "\n";

  std::cout << "#   _rc            " << _rc << "\n";

  std::map <std::string, std::string>::const_iterator m;
  for (m = _overrides.begin (); m != _overrides.end (); ++m)
    std::cout << "#   _overrides     " << m->first << " --> " << m->second << "\n";

  std::cout << "#   _filter        ";
  Color colorFilter ("gray20 on gray8");
  for (i = _filter.begin (); i != _filter.end (); ++i)
  {
    if (i != _filter.begin ())
      std::cout << ' ';
    std::cout << colorFilter.colorize (*i);
  }
  std::cout << "\n";

  std::cout << "#   _command       " << _command << " " << (_readOnly ? "(read)" : "(write)") << "\n";

  for (i = _modifications.begin (); i != _modifications.end (); ++i)
    std::cout << "#   _modifications " << *i << "\n";
}

////////////////////////////////////////////////////////////////////////////////
