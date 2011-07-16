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

#include <stdlib.h>
#include <Context.h>
#include <ColDue.h>
#include <Date.h>
#include <Duration.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDue::ColumnDue ()
{
  _name      = "due";
  _label     = STRING_COLUMN_LABEL_DUE;

  _styles.push_back ("countdown");

  Date now;
  now += 125;
  _examples.push_back (Duration (now - Date ()).formatCompact ());
}

////////////////////////////////////////////////////////////////////////////////
ColumnDue::~ColumnDue ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnDue::validate (std::string& value)
{
  return ColumnDate::validate (value);
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnDue::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "countdown" && _label == STRING_COLUMN_LABEL_DUE)
    _label = STRING_COLUMN_LABEL_COUNT;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDue::measure (Task& task, int& minimum, int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    if (_style == "countdown")
    {
      Date date ((time_t) strtol (task.get (_name).c_str (), NULL, 10));
      Date now;
      minimum = maximum = Duration (now - date).format ().length ();
    }
    else
      ColumnDate::measure (task, minimum, maximum);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDue::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "countdown")
    {
      Date date ((time_t) strtol (task.get (_name).c_str (), NULL, 10));
      Date now;

      lines.push_back (
        color.colorize (
          rightJustify (
            Duration (now - date).format (), width)));
    }
    else
      ColumnDate::render (lines, task, width, color);
  }
}

////////////////////////////////////////////////////////////////////////////////
