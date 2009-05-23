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
#include "Att.h"

class Record
{
public:
  Record ();                         // Default constructor
  Record (const Record&);            // Copy constructor
  Record& operator= (const Record&); // Assignment operator
  virtual ~Record ();                // Destructor

  virtual std::string composeF4 () = 0;
  virtual std::string composeCSV () = 0;
  void parse (const std::string&);

/*
  void getAttributes (std::map<std::string, std::string>&);
  const std::string getAttribute (const std::string&);
  void setAttribute (const std::string&, const std::string&);
  void setAttributes (const std::map <std::string, std::string>&);
  void removeAttribute (const std::string&);
*/

private:
  std::vector <Att> mAtts;
};

#endif
////////////////////////////////////////////////////////////////////////////////
