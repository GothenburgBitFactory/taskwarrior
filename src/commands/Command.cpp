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
#include <CmdHelp.h>
#include <CmdInstall.h>
#include <CmdLogo.h>
#include <CmdTip.h>
#include <Context.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
void Command::factory (std::map <std::string, Command*>& all)
{
  Command* c;

  c = new CmdExec ();    all[c->keyword ()] = c;
  c = new CmdHelp ();    all[c->keyword ()] = c;
  c = new CmdInstall (); all[c->keyword ()] = c;
  c = new CmdLogo ();    all[c->keyword ()] = c;
  c = new CmdTip ();     all[c->keyword ()] = c;
}

////////////////////////////////////////////////////////////////////////////////
Command::Command ()
: _usage ("")
, _description ("")
, _read_only (true)
, _displays_id (true)
{
}

////////////////////////////////////////////////////////////////////////////////
Command::Command (const Command& other)
{
  _usage       = other._usage;
  _description = other._description;
  _read_only   = other._read_only;
  _displays_id = other._displays_id;
}

////////////////////////////////////////////////////////////////////////////////
Command& Command::operator= (const Command& other)
{
  if (this != &other)
  {
    _usage       = other._usage;
    _description = other._description;
    _read_only   = other._read_only;
    _displays_id = other._displays_id;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Command::operator== (const Command& other) const
{
  return _usage       == other._usage       &&
         _description == other._description &&
         _read_only   == other._read_only   &&
         _displays_id == other._displays_id;
}

////////////////////////////////////////////////////////////////////////////////
Command::~Command ()
{
}

////////////////////////////////////////////////////////////////////////////////
std::string Command::keyword () const
{
  return _keyword;
}

////////////////////////////////////////////////////////////////////////////////
std::string Command::usage () const
{
  return _usage;
}

////////////////////////////////////////////////////////////////////////////////
std::string Command::description () const
{
  return _description;
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
