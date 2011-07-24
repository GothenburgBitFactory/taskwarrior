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

#include <iostream> // TODO Remove.
#include <Context.h>
#include <E9.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
// Perform all the necessary steps prior to an eval call.
E9::E9 (A3& args)
: _args (args)
{
}

////////////////////////////////////////////////////////////////////////////////
E9::~E9 ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool E9::evalFilter (const Task& task)
{
  if (_args.size () == 0)
    return true;

  // TODO Insert eval here.
  return true;
}

////////////////////////////////////////////////////////////////////////////////
