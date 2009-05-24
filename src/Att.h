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
#ifndef INCLUDED_ATT
#define INCLUDED_ATT

#include <string>
#include <vector>
#include "Mod.h"

class Att
{
public:
  Att ();                                       // Default constructor
  Att (const std::string&, const std::string&); // Simple constructor
  Att (const std::string&, int);                // Simple constructor
  Att (const Att&);                             // Copy constructor
  Att& operator= (const Att&);                  // Assignment operator
  ~Att ();                                      // Destructor

  void parse (const std::string&);
  std::string composeF4 () const;

  void addMod (const Mod&);
  // TODO Need method to access mods.

  std::string name () const;
  void name (const std::string&);

  std::string value () const;
  void value (const std::string&);

  int value_int () const;
  void value_int (int);

private:
  void enquote (std::string&) const;
  void dequote (std::string&) const;
  void encode (std::string&) const;
  void decode (std::string&) const;

private:
  std::string mName;
  std::string mValue;
  std::vector <Mod> mMods;
};

#endif
////////////////////////////////////////////////////////////////////////////////
