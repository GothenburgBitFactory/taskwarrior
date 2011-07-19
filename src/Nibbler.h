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
#ifndef INCLUDED_NIBBLER
#define INCLUDED_NIBBLER
#define L10N                                           // Localization complete.

#include <string>
#include <vector>

class Nibbler
{
public:
  Nibbler ();                          // Default constructor
  Nibbler (const char*);               // Constructor
  Nibbler (const std::string&);        // Constructor
  Nibbler (const Nibbler&);            // Copy constructor
  Nibbler& operator= (const Nibbler&); // Assignment operator
  ~Nibbler ();                         // Destructor

  bool getUntil (char, std::string&);
  bool getUntil (const std::string&, std::string&);
  bool getUntilRx (const std::string&, std::string&);
  bool getUntilOneOf (const std::string&, std::string&);
  bool getUntilWS (std::string&);
  bool getUntilEOL (std::string&);
  bool getUntilEOS (std::string&);

/*
  bool getAllOneOf (const std::string&, std::string&);
*/

  bool getQuoted (char, std::string&, bool quote = false);
  bool getDigit (int&);
  bool getInt (int&);
  bool getHex (int&);
  bool getUnsignedInt (int&);
  bool getNumber (double&);
  bool getLiteral (const std::string&);
  bool getRx (const std::string&, std::string&);
  bool getUUID (std::string&);
  bool getDateISO (time_t&);
  bool getDate (const std::string&, time_t&);
  bool getOneOf (const std::vector <std::string>&, std::string&);
  bool getDOM (std::string&);
  bool getWord (std::string&);

  bool skipN (const int quantity = 1);
  bool skip (char);
  bool skipAll (char);
  bool skipAllOneOf (const std::string&);
  bool skipWS ();
  bool skipRx (const std::string&);

  char next ();
  std::string next (const int quantity);

  std::string::size_type cursor ();

  void save ();
  void restore ();

  bool depleted ();

  std::string dump ();

private:
  std::string mInput;
  std::string::size_type mLength;
  std::string::size_type mCursor;
  std::string::size_type mSaved;
};

#endif
////////////////////////////////////////////////////////////////////////////////
