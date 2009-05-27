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
#ifndef INCLUDED_RECORD
#define INCLUDED_RECORD

#include <vector>
#include <map>
#include "Att.h"

class Record : public std::map <std::string, Att>
{
public:
  Record ();                         // Default constructor
  Record (const Record&);            // Copy constructor
  Record& operator= (const Record&); // Assignment operator
  virtual ~Record ();                // Destructor

  virtual std::string composeCSV () = 0;

  std::string composeF4 ();
  void parse (const std::string&);

  std::vector <Att> all ();
  const std::string get (const std::string&);
  int get_int (const std::string&);
  void set (const std::string&, const std::string&);
  void set (const std::string&, int);
  void remove (const std::string&);
};

#endif
////////////////////////////////////////////////////////////////////////////////
