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

#include <fstream>
#include <sstream>
#include <text.h>
#include <util.h>
#include <stdlib.h>
#include "StringTable.h"

////////////////////////////////////////////////////////////////////////////////
StringTable::StringTable ()
{
}

////////////////////////////////////////////////////////////////////////////////
StringTable::~StringTable ()
{
}

////////////////////////////////////////////////////////////////////////////////
// UTF-8 encoding
//
//  123 This is the string
//  124 This is another string
//  ...
void StringTable::load (const std::string& file)
{
  this->clear ();  // Allows dynamic reload.

  std::ifstream in;
  in.open (file.c_str (), std::ifstream::in);
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
    {
      // Remove comments.
      std::string::size_type pound = line.find ("#"); // no i18n
      if (pound != std::string::npos)
        line = line.substr (0, pound);

      line = trim (line, " \t"); // no i18n

      // Skip empty lines.
      if (line.length () > 0)
      {
        std::string::size_type equal = line.find (" "); // no i18n
        if (equal != std::string::npos)
        {
          int key = ::atoi (trim (line.substr (0, equal), " \t").c_str ()); // no i18n
          std::string value = trim (line.substr (equal+1, line.length () - equal), " \t"); // no i18n
          (*this)[key] = value;
        }
      }
    }

    in.close ();
  }
  else
    throw std::string ("Could not read string file '") + file + "'"; // TODO i18n
}

////////////////////////////////////////////////////////////////////////////////
std::string StringTable::get (int id, const std::string& alternate)
{
  // Return the right string.
  if (this->find (id) != this->end ())
    return (*this)[id];

  return alternate;
}

////////////////////////////////////////////////////////////////////////////////
