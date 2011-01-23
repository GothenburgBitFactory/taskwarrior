////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2010 - 2011, Paul Beckingham, Federico Hernandez.
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
#ifndef INCLUDED_JSON
#define INCLUDED_JSON

#include <string>
#include <Tree.h>
#include <Nibbler.h>

class JSON
{
public:
  JSON ();                       // Default constructor
  JSON (const std::string&);     // Constructor
  JSON (const JSON&);            // Copy constructor
  JSON& operator= (const JSON&); // Assignment operator
  ~JSON ();                      // Destructor

  static std::string encode (const std::string&);
  static std::string decode (const std::string&);

  Tree* tree ();

private:
  bool parseObject (Tree*, Nibbler&);
  bool parsePair (Tree*, Nibbler&);
  bool parseArray (Tree*, Nibbler&);
  bool parseValue (Tree*, Nibbler&);
  bool parseString (Tree*, Nibbler&);
  bool parseNumber (Tree*, Nibbler&);

private:
  Tree root;
};

#endif
////////////////////////////////////////////////////////////////////////////////
