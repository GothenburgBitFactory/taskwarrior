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
#include <CLI.h>
#include <util.h>
#include <i18n.h>

extern Context context;

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
    _args.push_back (argv[i]);

  dump ("CLI::initialize");
  extractOverrides ();
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
  dump ("CLI::aliasExpansion");
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
void CLI::dump (const std::string& label) const
{
  std::cout << "# " << label << "\n"
            << "#   _program '" << _program << "'\n";

  std::vector <std::string>::const_iterator i;
  for (i = _args.begin (); i != _args.end (); ++i)
    std::cout << "#   _args '" << *i << "'\n";

  std::cout << "#   _rc '" << _rc << "'\n";

  std::map <std::string, std::string>::const_iterator m;
  for (m = _overrides.begin (); m != _overrides.end (); ++m)
    std::cout << "#  _overrides '" << m->first << "' --> '" << m->second << "'\n";
}

////////////////////////////////////////////////////////////////////////////////
