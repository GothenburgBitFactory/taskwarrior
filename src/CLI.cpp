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
#include <Context.h>
#include <CLI.h>
#include <i18n.h>

extern Context context;

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
  _program = argv[0];
  for (int i = 1; i < argc; ++i)
    _args.push_back (argv[i]);

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
      std::string::size_type sep = arg.find ('=', 3);
      if (sep == std::string::npos)
        sep = arg.find (':', 3);
      if (sep != std::string::npos)
        _overrides[i->substr (3, sep - 3)] = i->substr (sep + 1);
    }
    else
      reconstructed.push_back (*i);
  }

  _args = reconstructed;
}

////////////////////////////////////////////////////////////////////////////////
