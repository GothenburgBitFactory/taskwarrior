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
#include <ColDescription.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDescription::ColumnDescription ()
{
  _type  = "string";
  _style = "default";
  _label = "Description";
}

////////////////////////////////////////////////////////////////////////////////
ColumnDescription::~ColumnDescription ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDescription::measure (Task& task, int& minimum, int& maximum)
{
  std::string description = task.get ("description");

/*
  std::vector <Att> annos;
  task.getAnnotations (annos);
*/

  // TODO Render Date () in appropriate format, to calculate length.

  // The text
  // <date> <anno>
  // ...
  if (_style == "default")
  {
  }

  // The text
  else if (_style == "desc")
  {
  }

  // The text <date> <anno> ...
  else if (_style == "oneline")
  {
  }

  // The te...
  else if (_style == "truncated")
  {
    minimum = 4;
    maximum = description.length ();
  }

  // The text [2]
  else if (_style == "count")
  {
  }

  else
    throw std::string ("Unrecognized column format '") + _type + "." + _style + "'";

/*
  std::string project = task.get ("project");

  if (_style == "parent")
  {
    std::string::size_type period = project.find ('.');
    if (period != std::string::npos)
      project = project.substr (0, period);
  }
  else if (_style != "default")
    throw std::string ("Unrecognized column format '") + _type + "." + _style + "'";

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
*/
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDescription::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string description = task.get ("description");

  // This is a description
  // <date> <anno>
  // ...
  if (_style == "default")
  {
  }

  // This is a description
  else if (_style == "desc")
  {
    std::vector <std::string> raw;
    wrapText (raw, description, width);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (leftJustify (*i, width)));
  }

  // This is a description <date> <anno> ...
  else if (_style == "oneline")
  {
  }

  // This is a des...
  else if (_style == "truncated")
  {
    int len = description.length ();
    if (len > width)
      lines.push_back (color.colorize (description.substr (0, width - 3) + "..."));
    else
      lines.push_back (color.colorize (leftJustify (description, width)));
  }

  // This is a description [2]
  else if (_style == "count")
  {
  }
}

////////////////////////////////////////////////////////////////////////////////
