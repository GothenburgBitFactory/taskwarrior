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

#include <Context.h>
#include <ColBg.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnBg::ColumnBg ()
{
  _name  = "bg";
  _type  = "string";
  _style = "default";
  _label = STRING_COLUMN_LABEL_BG;

  _styles.push_back ("default");

  _examples.push_back ("'on red'");
}

////////////////////////////////////////////////////////////////////////////////
ColumnBg::~ColumnBg ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnBg::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnBg::measure (Task& task, int& minimum, int& maximum)
{
  std::string bg = task.get (_name);

  minimum = longestWord (bg);
  maximum = bg.length ();
}

////////////////////////////////////////////////////////////////////////////////
void ColumnBg::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string bg = task.get (_name);

  std::vector <std::string> raw;
  wrapText (raw, bg, width, _hyphenate);

  std::vector <std::string>::iterator i;
  for (i = raw.begin (); i != raw.end (); ++i)
    lines.push_back (color.colorize (leftJustify (*i, width)));
}

////////////////////////////////////////////////////////////////////////////////
