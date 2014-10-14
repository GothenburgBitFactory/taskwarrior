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
#include <Lexer.h>
#include <CLI.h>
#include <util.h>
#include <i18n.h>

extern Context context;

// Overridden by rc.abbreviation.minimum.
static int minimumMatchLength = 3;

// Alias expansion limit. Any more indicates some kind of error.
static int safetyValveDefault = 10;

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
  _program = argv[0];
  for (int i = 1; i < argc; ++i)
    _original_args.push_back (argv[i]);

  _args = _original_args;

  dump ("CLI::initialize");
  extractOverrides ();
}

////////////////////////////////////////////////////////////////////////////////
void CLI::add (const std::string& arg)
{
  _original_args.push_back (arg);
  _args.push_back (arg);

  dump ("CLI::add");
  extractOverrides ();
  aliasExpansion ();
  categorize ();
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
  dump ("CLI::extractOverrides");
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

  dump ("CLI::aliasExpansion");
}

////////////////////////////////////////////////////////////////////////////////
void CLI::categorize ()
{
  bool foundCommand = false;

  _filter.clear ();
  _modifications.clear ();
  _command = "";
  _readOnly = false;

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

  dump ("CLI::categorize");
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
const std::string CLI::getFilter () const
{
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
void CLI::dump (const std::string& label) const
{
  std::cout << "# " << label << "\n"
            << "#   _program '" << _program << "'\n";

  std::vector <std::string>::const_iterator i;
  for (i = _original_args.begin (); i != _original_args.end (); ++i)
    std::cout << "#   _original_args '" << *i << "'\n";

  for (i = _args.begin (); i != _args.end (); ++i)
    std::cout << "#   _args '" << *i << "'\n";

  std::cout << "#   _rc '" << _rc << "'\n";

  std::map <std::string, std::string>::const_iterator m;
  for (m = _overrides.begin (); m != _overrides.end (); ++m)
    std::cout << "#   _overrides '" << m->first << "' --> '" << m->second << "'\n";

  for (i = _filter.begin (); i != _filter.end (); ++i)
    std::cout << "#   _filter '" << *i << "'\n";

  std::cout << "#   _command '" << _command << "' " << (_readOnly ? "(read)" : "(write)") << "\n";

  for (i = _modifications.begin (); i != _modifications.end (); ++i)
    std::cout << "#   _modifications '" << *i << "'\n";

}

////////////////////////////////////////////////////////////////////////////////
