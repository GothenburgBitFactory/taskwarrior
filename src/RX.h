////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_RX
#define INCLUDED_RX
#define L10N                                           // Localization complete.

#include <string>
#include <vector>
#include <regex.h>

class RX
{
public:
  RX ();
  RX (const std::string&, bool caseSensitive = true);
  RX (const RX&);
  RX& operator= (const RX&);
  bool operator== (const RX&) const;
  ~RX ();

  bool match (const std::string&);
  bool match (std::vector<std::string>&, const std::string&);
  bool match (std::vector <int>&, std::vector <int>&, const std::string&);

private:
  void compile ();

private:
  bool _compiled;
  std::string _pattern;
  bool _case_sensitive;
  regex_t _regex;
};

#endif

