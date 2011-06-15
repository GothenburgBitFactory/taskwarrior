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
#include <math.h>
#include <Context.h>
#include <ColDate.h>
#include <Date.h>
#include <Duration.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDate::ColumnDate ()
{
  _type      = "date";
  _style     = "default";
  _label     = "";
  _attribute = "";
}

////////////////////////////////////////////////////////////////////////////////
ColumnDate::~ColumnDate ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDate::measure (Task& task, int& minimum, int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_attribute))
  {
    Date date ((time_t) strtol (task.get (_attribute).c_str (), NULL, 10));

    if (_style == "default")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat.
      std::string format = context.config.get ("report." + _report + ".dateformat");
      if (format == "")
        format = context.config.get ("dateformat.report");
      if (format == "")
        format = context.config.get ("dateformat");

      minimum = maximum = date.toString (format).length ();
    }
    else if (_style == "julian")
    {
      // (JD − 2440587.5) × 86400 
      double julian = (date.toEpoch () / 86400.0) + 2440587.5;
      minimum = maximum = format (julian, 13, 12).length ();
    }
    else if (_style == "epoch")
    {
      minimum = maximum = date.toEpochString ().length ();
    }
    else if (_style == "iso")
    {
      minimum = maximum = date.toISO ().length ();
    }
    else if (_style == "age")
    {
      Date now;
      minimum = maximum = Duration (now - date).formatCompact ().length ();
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _attribute, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDate::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_attribute))
  {
    if (_style == "default")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat.
      std::string format = context.config.get ("report." + _report + ".dateformat");
      if (format == "")
        format = context.config.get ("dateformat.report");
      if (format == "")
        format = context.config.get ("dateformat");

      lines.push_back (
        color.colorize (
          leftJustify (
            Date ((time_t) strtol (task.get (_attribute).c_str (), NULL, 10))
              .toString (format), width)));
    }
    else if (_style == "julian")
    {
      double julian = (Date ((time_t) strtol (task.get (_attribute).c_str (), NULL, 10))
        .toEpoch () / 86400.0) + 2440587.5;

      lines.push_back (
        color.colorize (
          rightJustify (
            format (julian, 13, 12), width)));
    }
    else if (_style == "epoch")
    {
      lines.push_back (
        color.colorize (
          rightJustify (
            Date ((time_t) strtol (task.get (_attribute).c_str (), NULL, 10))
              .toEpochString (), width)));
    }
    else if (_style == "iso")
    {
      lines.push_back (
        color.colorize (
          leftJustify (
            Date ((time_t) strtol (task.get (_attribute).c_str (), NULL, 10))
              .toISO (), width)));
    }
    else if (_style == "age")
    {
      Date date ((time_t) strtol (task.get (_attribute).c_str (), NULL, 10));
      Date now;

      lines.push_back (
        color.colorize (
          rightJustify (
            Duration (now - date).formatCompact (), width)));
    }
    else if (_style == "short")
    {
    }
    else if (_style == "active")
    {
    }
    else if (_style == "countdown")
    {
    }
    else if (_style == "remaining")
    {
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
