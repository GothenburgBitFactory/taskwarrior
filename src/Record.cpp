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
// The format is:
//
//   [ Att::composeF4 ... ] \n
//
std::string Record::composeF4 () const
{
  std::string ff4 = "[";

  bool first = true;
  std::map <std::string, Att>::const_iterator it;
  for (it = this->begin (); it != this->end (); ++it)
  {
    if (it->second.value () != "")
    {
      ff4 += (first ? "" : " ") + it->second.composeF4 ();
      first = false;
    }
  }

  ff4 += "]\n";
  return ff4;
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
bool Record::has (const std::string& name) const
{
  Record::const_iterator i = this->find (name);
  if (i != this->end ())
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Att> Record::all ()
{
  std::vector <Att> all;
  std::map <std::string, Att>::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    all.push_back (i->second);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Record::get (const std::string& name) const
{
  Record::const_iterator i = this->find (name);
  if (i != this->end ())
    return i->second.value ();

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int Record::get_int (const std::string& name) const
{
  Record::const_iterator i = this->find (name);
  if (i != this->end ())
    return atoi (i->second.value ().c_str ());

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
unsigned long Record::get_ulong (const std::string& name) const
{
  Record::const_iterator i = this->find (name);
  if (i != this->end ())
    return strtoul (i->second.value ().c_str (), NULL, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Record::set (const std::string& name, const std::string& value)
{
  (*this)[name] = Att (name, value);
}

////////////////////////////////////////////////////////////////////////////////
void Record::set (const std::string& name, int value)
{
  std::stringstream svalue;
  svalue << value;

  (*this)[name] = Att (name, svalue.str ());
}

////////////////////////////////////////////////////////////////////////////////
void Record::remove (const std::string& name)
{
  Record::iterator it;
  if ((it = this->find (name)) != this->end ())
    this->erase (it);
}

////////////////////////////////////////////////////////////////////////////////
