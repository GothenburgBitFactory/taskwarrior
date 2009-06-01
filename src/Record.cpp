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

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "util.h"
#include "Nibbler.h"
#include "Record.h"

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
std::string Record::composeF4 ()
{
  std::string ff4 = "[";

  bool first = true;
  foreach (r, (*this))
    ff4 += (first ? "" : " ") + r->second.composeF4 ();

  ff4 += "]";
  return ff4;
}

////////////////////////////////////////////////////////////////////////////////
//
// [ --> start --> name --> : --> " --> value --> " --> ] --> end
//                   ^                                |
//                   +------------- \s <--------------+
//
void Record::parse (const std::string& input)
{
  Nibbler n (input);
  std::string line;
  if (n.skip     ('[')       &&
      n.getUntil (']', line) &&
      n.skip     (']')       &&
      n.depleted ())
  {
    if (line.length () == 0)
      throw std::string ("Empty FF4 record");

    Nibbler nl (line);
    Att a;
    while (!nl.depleted () && a.parse (nl))
    {
      (*this)[a.name ()] = a;
      nl.skip (' ');
    }

    std::string remainder;
    nl.getUntilEOS (remainder);
    if (remainder.length ())
      throw std::string ("Unrecognized garbage at end of line");
  }
  else
    throw std::string ("Record not recognized as FF4");
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Att> Record::all ()
{
  std::vector <Att> all;
  foreach (a, (*this))
    all.push_back (a->second);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Record::get (const std::string& name)
{
  if (this->find (name) != this->end ())
    return (*this)[name].value ();

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int Record::get_int (const std::string& name)
{
  if (this->find (name) != this->end ())
    return ::atoi ((*this)[name].value ().c_str ());

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
  std::map <std::string, Att> copy = *this;
  this->clear ();
  foreach (i, copy)
   if (i->first != name)
     (*this)[i->first] = i->second;
}

////////////////////////////////////////////////////////////////////////////////
