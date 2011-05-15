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
#include <Command.h>
#include <CmdExec.h>
#include <CmdInstall.h>
#include <CmdLogo.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
Command* Command::factory (const std::string& name)
{
  Command* command;
       if (name == "exec")    command = new CmdExec ();
  else if (name == "install") command = new CmdInstall ();
  else if (name == "_logo")   command = new CmdLogo ();
  else
    throw std::string ("Unrecognized command object '") + name + "'";

  // TODO Initialize command object.

  return command;
}

////////////////////////////////////////////////////////////////////////////////
Command::Command ()
: _read_only (true)
, _displays_id (true)
{
}

////////////////////////////////////////////////////////////////////////////////
Command::Command (const Command& other)
{
  _read_only   = other._read_only;
  _displays_id = other._displays_id;
}

////////////////////////////////////////////////////////////////////////////////
Command& Command::operator= (const Command& other)
{
  if (this != &other)
  {
    _read_only   = other._read_only;
    _displays_id = other._displays_id;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::operator== (const Command& other) const
{
  return _read_only   == other._read_only &&
         _displays_id == other._displays_id;
}

////////////////////////////////////////////////////////////////////////////////
Command::~Command ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool Command::read_only () const
{
  return _read_only;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::displays_id () const
{
  return _displays_id;
}

////////////////////////////////////////////////////////////////////////////////
