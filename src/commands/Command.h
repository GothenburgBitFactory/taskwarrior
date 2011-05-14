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
#ifndef INCLUDED_COMMAND
#define INCLUDED_COMMAND

#include <string>

class Command
{
public:
  Command ();
  Command (const Command&);
  Command& operator= (const Command&);
  bool operator== (const Command&) const;     // TODO Is this necessary?
  virtual ~Command ();

  static Command* factory (const std::string&);

  virtual bool read_only () const;
  virtual bool implements (const std::string&) const = 0;
  virtual int execute (const std::string&, std::string&) = 0;
};

#endif
////////////////////////////////////////////////////////////////////////////////
