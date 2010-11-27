////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006 - 2010, Paul Beckingham, Federico Hernandez.
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
#include "Att.h"

class Subst
{
public:
  Subst ();                        // Default constructor
  Subst (const std::string&);      // Default constructor
  Subst (const Subst&);            // Copy constructor
  Subst& operator= (const Subst&); // Assignment operator
  ~Subst ();                       // Destructor

  bool valid (const std::string&) const;
  void parse (const std::string&);
  void apply (std::string&, std::vector <Att>&) const;
  void clear ();

public:
  std::string mFrom;
  std::string mTo;
  bool mGlobal;
};

#endif
////////////////////////////////////////////////////////////////////////////////
