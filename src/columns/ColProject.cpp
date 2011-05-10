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

#include <Context.h>
#include <ColProject.h>
#include <text.h>

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

  if (_style == "parent")
  {
    std::string::size_type period = project.find ('.');
    if (period != std::string::npos)
      project = project.substr (0, period);
  }
  else if (_style != "default")
    throw std::string ("Unrecognized column format 'project.") + _style + "'";

  minimum = longestWord (project);
  maximum = project.length ();
}

////////////////////////////////////////////////////////////////////////////////
void ColumnProject::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string project = task.get ("project");
  if (_style == "parent")
  {
    std::string::size_type period = project.find ('.');
    if (period != std::string::npos)
      project = project.substr (0, period);
  }

  std::vector <std::string> raw;
  wrapText (raw, project, width);

  std::vector <std::string>::iterator i;
  for (i = raw.begin (); i != raw.end (); ++i)
    lines.push_back (color.colorize (leftJustify (*i, width)));
}

////////////////////////////////////////////////////////////////////////////////
