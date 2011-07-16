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
#include <ColUUID.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnUUID::ColumnUUID ()
{
  _name  = "uuid";
  _type  = "string";
  _style = "long";
  _label = STRING_COLUMN_LABEL_UUID;

  _styles.push_back ("long");
  _styles.push_back ("short");

  _examples.push_back ("f30cb9c3-3fc0-483f-bfb2-3bf134f00694");
  _examples.push_back ("34f00694");
}

////////////////////////////////////////////////////////////////////////////////
ColumnUUID::~ColumnUUID ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUUID::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnUUID::measure (Task&, int& minimum, int& maximum)
{
       if (_style == "default" || _style == "long") minimum = maximum = 36;
  else if (_style == "short")                       minimum = maximum = 8;
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUUID::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  // f30cb9c3-3fc0-483f-bfb2-3bf134f00694  default
  //                             34f00694  short
  if (_style == "default" ||
      _style == "long")
    lines.push_back (color.colorize (leftJustify (task.get (_name), width)));

  else if (_style == "short")
    lines.push_back (color.colorize (leftJustify (task.get (_name).substr (28), width)));
}

////////////////////////////////////////////////////////////////////////////////
