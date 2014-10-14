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
#ifndef INCLUDED_CLI
#define INCLUDED_CLI

#include <string>
#include <vector>
#include <map>

class CLI
{
public:
  CLI ();
  ~CLI ();
  void alias (const std::string&, const std::string&);
  void entity (const std::string&, const std::string&);
  void initialize (int, const char**);

private:
  void extractOverrides ();
  void aliasExpansion ();
  void dump (const std::string&) const;

public:
  std::multimap <std::string, std::string> _entities;
  std::map <std::string, std::string>      _aliases;
  std::string                              _program;
  std::vector <std::string>                _args;
  std::string                              _rc;
  std::map <std::string, std::string>      _overrides;
};

#endif

