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
#ifndef INCLUDED_CMD
#define INCLUDED_CMD

#include <vector>
#include <string>
#include "Cmd.h"

class Cmd
{
public:
  Cmd ();                      // Default constructor
  Cmd (const std::string&);    // Default constructor
  ~Cmd ();                     // Destructor

  bool valid            (const std::string&);
  bool validCustom      (const std::string&);
  void parse            (const std::string&);
  void allCustomReports (std::vector <std::string>&) const;

  bool isReadOnlyCommand ();
  bool isWriteCommand ();

public:
  std::string command;

private:
  void load ();

private:
  std::vector <std::string> commands;
  std::vector <std::string> customReports;
};

#endif
////////////////////////////////////////////////////////////////////////////////
