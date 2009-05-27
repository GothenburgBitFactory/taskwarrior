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
#ifndef INCLUDED_SUBST
#define INCLUDED_SUBST

#include <string>
#include <Record.h>

class Subst
{
public:
  Subst ();                        // Default constructor
  Subst (const std::string&);      // Default constructor
  Subst (const Subst&);            // Copy constructor
  Subst& operator= (const Subst&); // Assignment operator
  ~Subst ();                       // Destructor

  bool parse (const std::string&);
  void apply (Record&) const;

public:
  std::string mFrom;
  std::string mTo;
  bool mGlobal;
};

#endif
////////////////////////////////////////////////////////////////////////////////
