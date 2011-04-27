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

#include <math.h>
#include <Context.h>
#include <ColID.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnID::ColumnID ()
{
  setLabel ("id");
}

////////////////////////////////////////////////////////////////////////////////
ColumnID::~ColumnID ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnID::measure (Task& task, int& minimum, int& maximum)
{
  int length;

       if (task.id < 10)     length = 1;                              // Fast
  else if (task.id < 100)    length = 2;                              // Fast
  else if (task.id < 1000)   length = 3;                              // Fast
  else if (task.id < 10000)  length = 4;                              // Fast
  else                       length = (int) log10 ((double) task.id); // Slow

  minimum = maximum = length;
}

////////////////////////////////////////////////////////////////////////////////
void ColumnID::renderHeader (std::vector <std::string>& lines, int width)
{
  lines.push_back ("ID");
}

////////////////////////////////////////////////////////////////////////////////
void ColumnID::render (std::vector <std::string>& lines, Task* task, int width)
{
}

////////////////////////////////////////////////////////////////////////////////
std::string ColumnID::type () const
{
  return "number";
}

////////////////////////////////////////////////////////////////////////////////
