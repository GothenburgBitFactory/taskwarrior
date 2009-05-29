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
#ifndef INCLUDED_NIBBLER
#define INCLUDED_NIBBLER

#include <string>

class Nibbler
{
public:
  Nibbler ();                          // Default constructor
  Nibbler (const char*);               // Constructor
  Nibbler (const std::string&);        // Constructor
  Nibbler (const Nibbler&);            // Copy constructor
  Nibbler& operator= (const Nibbler&); // Assignment operator
  ~Nibbler ();                         // Destructor

  bool getUntilChar (char, std::string&);
  bool getUntilChars (const std::string&, std::string&);
  bool getUntilString (const std::string&, std::string&);
  bool skip (const int quantity = 1);
  bool skip (char);
  bool skipAll (char);
  bool skipAllChars (const std::string&);
  bool getQuoted (char, std::string&);
  bool getInt (int&);
  bool getUnsignedInt (int&i);
  bool getUntilEOL (std::string&);
  bool getUntilEOS (std::string&);
  bool depleted ();

private:
  std::string mInput;
  std::string::size_type mCursor;
};

#endif
////////////////////////////////////////////////////////////////////////////////
