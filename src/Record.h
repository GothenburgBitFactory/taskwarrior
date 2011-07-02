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
#ifndef INCLUDED_RECORD
#define INCLUDED_RECORD
#define L10N                                           // Localization complete.

#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <Att.h>

class Record : public std::map <std::string, Att>
{
public:
  Record ();                         // Default constructor
  Record (const std::string&);       // Copy constructor
  virtual ~Record ();                // Destructor

  std::string composeF4 () const;
  std::string composeCSV () const;
  void parse (const std::string&);

  bool has (const std::string&) const;
  std::vector <Att> all ();
  const std::string get (const std::string&) const;
  int get_int (const std::string&) const;
  unsigned long get_ulong (const std::string&) const;
  time_t get_date (const std::string&) const;
  time_t get_duration (const std::string&) const;
  void set (const std::string&, const std::string&);
  void set (const std::string&, int);
  void remove (const std::string&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
