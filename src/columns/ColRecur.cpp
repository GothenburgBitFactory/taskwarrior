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
#include <ColRecur.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::ColumnRecur ()
{
  _name  = "recur";
  _type  = "string";
  _style = "duration";
  _label = STRING_COLUMN_LABEL_RECUR;

  _styles.push_back ("duration");
  _styles.push_back ("indicator");

  _examples.push_back ("weekly");
  _examples.push_back (context.config.get ("recurrence.indicator"));
}

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::~ColumnRecur ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnRecur::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnRecur::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "indicator" && _label == STRING_COLUMN_LABEL_RECUR)
    _label = _label.substr (0, context.config.get ("recurrence.indicator").length ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnRecur::measure (Task& task, int& minimum, int& maximum)
{
  if (_style == "default" ||
      _style == "duration")
  {
    minimum = maximum = task.get ("recur").length ();
  }
  else if (_style == "indicator")
  {
    if (task.has (_name))
      minimum = maximum = context.config.get ("recurrence.indicator").length ();
  }
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnRecur::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (_style == "default" ||
      _style == "duration")
  {
    lines.push_back (color.colorize (rightJustify (task.get ("recur"), width)));
  }
  else if (_style == "indicator")
  {
    if (task.has (_name))
      lines.push_back (
        color.colorize (
          rightJustify (context.config.get ("recurrence.indicator"), width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
