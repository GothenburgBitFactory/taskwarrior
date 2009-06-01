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
#include <string>
#include "Nibbler.h"
#include "T2.h"
#include "util.h"

////////////////////////////////////////////////////////////////////////////////
T2::T2 ()
{
  // Each new task gets a uuid.
  set ("uuid", uuid ());
}

////////////////////////////////////////////////////////////////////////////////
// Attempt an FF4 parse first, using Record::parse, and in the event of an error
// try a legacy parse (F3, FF2).  Note that FF1 is no longer supported.
T2::T2 (const std::string& input)
{
  try
  {
    parse (input);
  }

  catch (std::string& e)
  {
    legacyParse (input);
  }
}

////////////////////////////////////////////////////////////////////////////////
T2& T2::operator= (const T2& other)
{
  throw std::string ("unimplemented T2::operator=");
  if (this != &other)
  {
    mId = other.mId;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
T2::~T2 ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Support FF2, FF3.
void T2::legacyParse (const std::string& input)
{
}

////////////////////////////////////////////////////////////////////////////////
std::string T2::composeCSV ()
{
  throw std::string ("unimplemented T2::composeCSV");
  return "";
}

////////////////////////////////////////////////////////////////////////////////
void T2::getAnnotations (std::vector <Att>& annotations) const
{
  annotations.clear ();

  Record::const_iterator ci;
  for (ci = this->begin (); ci != this->end (); ++ci)
    if (ci->first.substr (0, 11) == "annotation_")
      annotations.push_back (ci->second);
}

////////////////////////////////////////////////////////////////////////////////
void T2::setAnnotations (const std::vector <Att>& annotations)
{
  // Erase old annotations.
  Record::iterator i;
  for (i = this->begin (); i != this->end (); ++i)
    if (i->first.substr (0, 11) == "annotation_")
      this->erase (i);

  std::vector <Att>::const_iterator ci;
  for (ci = annotations.begin (); ci != annotations.end (); ++ci)
    (*this)[ci->name ()] = *ci;
}

////////////////////////////////////////////////////////////////////////////////
// The timestamp is part of the name:
//    annotation_1234567890:"..."
//
void T2::addAnnotation (const std::string& description)
{
  std::stringstream s;
  s << "annotation_" << time (NULL);

  (*this)[s.str ()] = Att (s.str (), description);
}

////////////////////////////////////////////////////////////////////////////////
bool T2::validate () const
{
  // TODO Verify until > due
  // TODO Verify entry < until, due, start, end
  return true;
}

////////////////////////////////////////////////////////////////////////////////
