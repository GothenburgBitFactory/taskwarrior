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
#ifndef INCLUDED_ATT
#define INCLUDED_ATT
#define L10N                                           // Localization complete.

#include <string>
#include <vector>
#include <Nibbler.h>

class Att
{
public:
  Att ();
  Att (const std::string&, const std::string&, const std::string&);
  Att (const std::string&, const std::string&, const std::string&, const std::string&);
  Att (const std::string&, const std::string&, int);
  Att (const std::string&, const std::string&);
  Att (const std::string&, int);
  Att (const Att&);
  Att& operator= (const Att&);
  bool operator== (const Att&) const;
  ~Att ();

  bool logicSense (bool match) const;
  bool valid (const std::string&) const;
  static bool validInternalName (const std::string&);
  static bool validModifiableName (const std::string&);
  static bool validNameValue (const std::string&, const std::string&, const std::string&);
  static bool validNameValue (std::string&, std::string&, std::string&);
  static bool validMod (const std::string&);
  std::string type (const std::string&) const;
  std::string modType (const std::string&) const;
  void parse (const std::string&);
  void parse (Nibbler&);
  bool match (const Att&) const;

  std::string composeF4 () const;

  void mod (const std::string&);
  std::string mod () const;

  std::string name () const;
  void name (const std::string&);

  std::string value () const;
  void value (const std::string&);

  int value_int () const;
  void value_int (int);

  static void allNames (std::vector <std::string>&);

private:
  void enquote (std::string&) const;
  void dequote (std::string&) const;
  void encode (std::string&) const;
  void decode (std::string&) const;

private:
  std::string mName;
  std::string mValue;
  std::string mMod;
  std::string mSense;
};

#endif
////////////////////////////////////////////////////////////////////////////////
