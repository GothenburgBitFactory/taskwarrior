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

#define L10N                                           // Localization complete.

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <util.h>
#include <Nibbler.h>
#include <Context.h>
#include <text.h>
#include <i18n.h>
#include <Record.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Record::Record ()
{
}

////////////////////////////////////////////////////////////////////////////////
Record::Record (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Record::~Record ()
{
}

////////////////////////////////////////////////////////////////////////////////
//
// start --> [ --> Att --> ] --> end
//              ^       |
//              +-------+
//
void Record::parse (const std::string& input)
{
  clear ();

  Nibbler n (input);
  std::string line;
  if (n.skip     ('[')       &&
      n.getUntil (']', line) &&
      n.skip     (']')       &&
      n.depleted ())
  {
    if (line.length () == 0)
      throw std::string (STRING_RECORD_EMPTY);

    Nibbler nl (line);
    Att a;
    while (!nl.depleted ())
    {
      a.parse (nl);
      (*this)[a.name ()] = a;
      nl.skip (' ');
    }

    std::string remainder;
    nl.getUntilEOS (remainder);
    if (remainder.length ())
      throw std::string (STRING_RECORD_JUNK_AT_EOL);
  }
  else
    throw std::string (STRING_RECORD_NOT_FF4);
}

////////////////////////////////////////////////////////////////////////////////
