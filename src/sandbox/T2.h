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
#ifndef INCLUDED_T2
#define INCLUDED_T2

#include <string>
#include "Record.h"

class T2 : public Record
{
public:
  T2 ();                    // Default constructor
  T2 (const std::string&);  // Parse
  T2& operator= (const T2&); // Assignment operator
  ~T2 ();                   // Destructor

  std::string composeF4 ();
  std::string composeCSV ();

  // TODO Series of helper functions.
/*
  status getStatus () const;
  void setStatus (status s);
*/

  bool validate () const;

private:
};

#endif
////////////////////////////////////////////////////////////////////////////////
