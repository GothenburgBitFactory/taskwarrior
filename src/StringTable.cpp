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
#include "StringTable.h"

////////////////////////////////////////////////////////////////////////////////
StringTable::StringTable ()
{
}

////////////////////////////////////////////////////////////////////////////////
StringTable::StringTable (const StringTable& other)
{
  throw std::string ("unimplemented StringTable::StringTable");
  mMapping = other.mMapping;
}

////////////////////////////////////////////////////////////////////////////////
StringTable& StringTable::operator= (const StringTable& other)
{
  throw std::string ("unimplemented StringTable::operator=");
  if (this != &other)
  {
    mMapping = other.mMapping;
  }

  return *this;
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
  std::ifstream in;
  in.open (file.c_str (), std::ifstream::in);
  if (in.good ())
  {
    std::string line;
    while (getline (in, line))
    {
      // Remove comments.
      std::string::size_type pound = line.find ("#");
      if (pound != std::string::npos)
        line = line.substr (0, pound);

      line = trim (line, " \t");

      // Skip empty lines.
      if (line.length () > 0)
      {
        std::string::size_type equal = line.find (" ");
        if (equal != std::string::npos)
        {
          int key = ::atoi (trim (line.substr (0, equal), " \t").c_str ());
          std::string value = trim (line.substr (equal+1, line.length () - equal), " \t");
          mMapping[key] = value;
        }
      }
    }

    in.close ();
  }
  else
    throw std::string ("Could not read string file '") + file + "'";
}

////////////////////////////////////////////////////////////////////////////////
std::string StringTable::get (int id, const std::string& alternate)
{
  // Return the right string.
  if (mMapping.find (id) != mMapping.end ())
    return mMapping[id];

  return alternate;
}

////////////////////////////////////////////////////////////////////////////////
