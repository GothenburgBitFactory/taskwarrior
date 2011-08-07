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

#define L10N                                           // Localization complete.

#include <math.h>
#include <Context.h>
#include <ColIMask.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnIMask::ColumnIMask ()
{
  _name       = "imask";
  _type       = "number";
  _style      = "number";
  _label      = STRING_COLUMN_LABEL_MASK_IDX;
  _modifiable = false;

  _styles.push_back ("number");

  _examples.push_back ("12");
}

////////////////////////////////////////////////////////////////////////////////
ColumnIMask::~ColumnIMask ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnIMask::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnIMask::measure (Task& task, int& minimum, int& maximum)
{
  minimum = maximum = task.get ("imask").length ();

  if (_style != "default" &&
      _style != "number")
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnIMask::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  lines.push_back (color.colorize (rightJustify (task.get ("imask"), width)));
}

////////////////////////////////////////////////////////////////////////////////
