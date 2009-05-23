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

#include <sstream>
#include "util.h"
#include "Record.h"

////////////////////////////////////////////////////////////////////////////////
Record::Record ()
{
}

////////////////////////////////////////////////////////////////////////////////
Record::Record (const Record& other)
{
  throw std::string ("unimplemented Record::Record");
  mAtts = other.mAtts;
}

////////////////////////////////////////////////////////////////////////////////
Record& Record::operator= (const Record& other)
{
  throw std::string ("unimplemented Record:operator=");
  if (this != &other)
  {
    mAtts = other.mAtts;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Record::~Record ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Record::parse (const std::string& input)
{
  throw std::string ("unimplemented Record::parse");
}

////////////////////////////////////////////////////////////////////////////////
std::vector <Att> Record::all ()
{
  std::vector <Att> all;
  foreach (a, mAtts)
    all.push_back (a->second);

  return all;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Record::get (const std::string& name)
{
  if (mAtts.find (name) != mAtts.end ())
    return mAtts[name].value ();

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int Record::getInt (const std::string& name)
{
  if (mAtts.find (name) != mAtts.end ())
    return ::atoi (mAtts[name].value ().c_str ());

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void Record::set (const std::string& name, const std::string& value)
{
  mAtts[name] = Att (name, value);
}

////////////////////////////////////////////////////////////////////////////////
void Record::set (const std::string& name, int value)
{
  std::stringstream svalue;
  svalue << value;

  mAtts[name] = Att (name, svalue.str ());
}

////////////////////////////////////////////////////////////////////////////////
void Record::remove (const std::string& name)
{
  std::map <std::string, Att> copy = mAtts;
  mAtts.clear ();
  foreach (i, copy)
   if (i->first != name)
     mAtts[i->first] = i->second;
}

////////////////////////////////////////////////////////////////////////////////
