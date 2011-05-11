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
#include <Context.h>
#include <ColString.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnString::ColumnString ()
{
  _type  = "string";
  _style = "default";
  _label = "";
}

////////////////////////////////////////////////////////////////////////////////
ColumnString::~ColumnString ()
{
}

////////////////////////////////////////////////////////////////////////////////
// ColumnString is unique - it copies the report name into the label.  This is
// a kludgy reuse of an otherwise unused member.
void ColumnString::setReport (const std::string& value)
{
  _report = _label = value;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnString::measure (const std::string& value, int& minimum, int& maximum)
{
  std::string stripped = Color::strip (value);
  maximum = stripped.length ();

  if (_style == "left"  ||
      _style == "right" ||
      _style == "default")
    minimum = longestWord (stripped);
  else
    throw std::string ("Unrecognized column format 'string.") + _style + "'";
}

////////////////////////////////////////////////////////////////////////////////
void ColumnString::render (
  std::vector <std::string>& lines,
  const std::string& value,
  int width,
  Color& color)
{
  if (_style == "default" || _style == "left")
  {
    std::vector <std::string> raw;
    wrapText (raw, value, width);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (leftJustify (*i, width)));
  }
  else if (_style == "right")
  {
    std::vector <std::string> raw;
    wrapText (raw, value, width);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (rightJustify (*i, width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
