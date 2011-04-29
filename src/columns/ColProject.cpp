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
#include <Nibbler.h>
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
  minimum = 0;
  maximum = project.length ();

  Nibbler nibbler (project);
  std::string word;
  while (nibbler.getUntilWS (word))
  {
    nibbler.skipWS ();
    if (word.length () > minimum)
      minimum = word.length ();
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnProject::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  // TODO Can't use Nibbler here.  Need to use a UTF8-safe version of extractLines.
  Nibbler nibbler (task.get ("project"));
  std::string word;
  while (nibbler.getUntilWS (word))
  {
    nibbler.skipWS ();
    lines.push_back (color.colorize (leftJustify (word, width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
