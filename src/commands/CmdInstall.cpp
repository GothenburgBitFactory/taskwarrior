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

#include <iostream>
#include <CmdInstall.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdInstall::CmdInstall ()
/*
: _name ("")
*/
{
}

////////////////////////////////////////////////////////////////////////////////
CmdInstall::CmdInstall (const CmdInstall& other)
{
/*
  _minimum = other._minimum;
*/
}

////////////////////////////////////////////////////////////////////////////////
CmdInstall& CmdInstall::operator= (const CmdInstall& other)
{
  if (this != &other)
  {
/*
    _name    = other._name;
*/
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool CmdInstall::operator== (const CmdInstall& other) const
{
  return false;
/*
  return _name    == other._name    &&
         _minimum == other._minimum &&
         _maximum == other._maximum &&
         _wrap    == other._wrap    &&
         _just    == other._just    &&
         _sizing  == other._sizing;
*/
}

////////////////////////////////////////////////////////////////////////////////
CmdInstall::~CmdInstall ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool CmdInstall::read_only () const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CmdInstall::implements (const std::string& command_line) const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Algorithm:
//   Copy file rc.data.location/extensions
//   Generate UUID
//   Call the "install" function once, store results in rc:
//     extension.<uuid>=<JSON>
std::string CmdInstall::execute (const std::string&)
{
  return "output";
}

////////////////////////////////////////////////////////////////////////////////
