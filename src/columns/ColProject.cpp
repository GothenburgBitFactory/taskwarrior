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

#include <iostream> // TODO Remove
#include <math.h>
#include <Context.h>
#include <ColProject.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnProject::ColumnProject ()
{
  _type  = "string";
  _style = "default";
  _label = "Project";
}

////////////////////////////////////////////////////////////////////////////////
ColumnProject::~ColumnProject ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnProject::measure (Task& task, int& minimum, int& maximum)
{
  std::string project = task.get ("project");
  minimum = maximum = project.length ();

  std::string::size_type space = project.find (' ');
  if (space == std::string::npos)
  {
    std::cout << "# ColProject::measure project=" << project << " min=" << minimum << " max=" << maximum << "\n";
    return;
  }

  minimum = 0;
  int longest = 0;
  std::string::size_type last = -1;
  while (space != std::string::npos)
  {
    if (space - last - 1 > longest)
      longest = space - last - 1;

    last = space;
    space = project.find (' ', last + 1);
  }

  if (longest)
    minimum = longest;

  std::cout << "# ColProject::measure project=" << project << " min=" << minimum << " max=" << maximum << "\n";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnProject::render (std::vector <std::string>& lines, Task& task, int width)
{
}

////////////////////////////////////////////////////////////////////////////////
