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
#ifndef INCLUDED_PATH
#define INCLUDED_PATH

#include <vector>
#include <string>

class Path
{
public:
  Path ();
  Path (const Path&);
  Path (const std::string&);
  virtual ~Path ();

  Path& operator= (const Path&);
  bool operator== (const Path&);
  operator std::string () const;

  std::string name () const;
  std::string parent () const;
  std::string extension () const;
  bool exists () const;
  bool is_directory () const;
  bool is_absolute () const;
  bool readable () const;
  bool writable () const;
  bool executable () const;

  // Statics
  static std::string expand (const std::string&);
  static std::vector<std::string> glob (const std::string&);

public:
  std::string data;
};

#endif
////////////////////////////////////////////////////////////////////////////////
