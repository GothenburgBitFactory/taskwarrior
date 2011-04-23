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
#include <Install.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Install::Install ()
{
}

////////////////////////////////////////////////////////////////////////////////
Install::Install (const Install& other)
{
/*
  _minimum = other._minimum;
*/
}

////////////////////////////////////////////////////////////////////////////////
Install& Install::operator= (const Install& other)
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
bool Install::operator== (const Install& other) const
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
Install::~Install ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Install::implements (const std::string&) const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Install::execute (const std::string&)
{
  return "output";
}

////////////////////////////////////////////////////////////////////////////////
