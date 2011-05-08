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
#ifndef INCLUDED_COLSTRING
#define INCLUDED_COLSTRING

#include <vector>
#include <string>
#include <Column.h>
#include <Color.h>
#include <Task.h>

class ColumnString : public Column
{
public:
  ColumnString ();
  ~ColumnString ();

  void setReport (const std::string&);
  void measure (const std::string&, int&, int&);
  void render (std::vector <std::string>&, const std::string&, int, Color&);

private:
};

#endif
////////////////////////////////////////////////////////////////////////////////
