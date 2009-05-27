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

#include <Subst.h>

////////////////////////////////////////////////////////////////////////////////
Subst::Subst ()
: mFrom ("")
, mTo ("")
, mGlobal (false)
{
}

////////////////////////////////////////////////////////////////////////////////
Subst::Subst (const std::string& input)
{
  parse (input);
}

////////////////////////////////////////////////////////////////////////////////
Subst::Subst (const Subst& other)
{
  mFrom   = other.mFrom;
  mTo     = other.mTo;
  mGlobal = other.mGlobal;
}

////////////////////////////////////////////////////////////////////////////////
Subst& Subst::operator= (const Subst& other)
{
  if (this != &other)
  {
    mFrom   = other.mFrom;
    mTo     = other.mTo;
    mGlobal = other.mGlobal;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Subst::~Subst ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Subst::parse (const std::string& input)
{
  size_t first = input.find ('/');
  if (first != std::string::npos)
  {
    size_t second = input.find ('/', first + 1);
    if (second != std::string::npos)
    {
      size_t third = input.find ('/', second + 1);
      if (third != std::string::npos)
      {
        if (first == 0 &&
            first < second &&
            second < third &&
            (third == input.length () - 1 ||
             third == input.length () - 2))
        {
          mFrom = input.substr (first  + 1, second - first  - 1);
          mTo   = input.substr (second + 1, third  - second - 1);

          mGlobal = false;
          if (third == input.length () - 2 &&
              input.find ('g', third + 1) != std::string::npos)
            mGlobal = true;

          return true;
        }
      }
    }
  }

 return false;
}

////////////////////////////////////////////////////////////////////////////////
void Subst::apply (Record& record) const
{
  // TODO Apply /mFrom/mTo/mGlobal to record.get ("description")
  // TODO Apply /mFrom/mTo/mGlobal to record.get ("annotation...")
}

////////////////////////////////////////////////////////////////////////////////
