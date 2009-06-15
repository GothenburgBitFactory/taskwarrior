////////////////////////////////////////////////////////////////////////////////
// task - a command line task list manager.
//
// Copyright 2006 - 2009, Paul Beckingham.
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
#ifndef INCLUDED_CONFIG
#define INCLUDED_CONFIG

#include <map>
#include <vector>
#include <string>

class Config : public std::map <std::string, std::string>
{
public:
  Config ();
  Config (const std::string&);

  bool load (const std::string&);
  void createDefault (const std::string&);
  void setDefaults ();

  const std::string get (const char*);
  const std::string get (const char*, const char*);
  const std::string get (const std::string&);
  const std::string get (const std::string&, const std::string&);
  bool get (const std::string&, const bool);
  int get (const std::string&, const int);
  double get (const std::string&, const double);
  void set (const std::string&, const int);
  void set (const std::string&, const double);
  void set (const std::string&, const std::string&);
  void all (std::vector <std::string>&);
};

#endif

////////////////////////////////////////////////////////////////////////////////
