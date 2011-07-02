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
#include <Date.h>
#include <ColDescription.h>
#include <text.h>
#include <util.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDescription::ColumnDescription ()
{
  _name  = "description";
  _type  = "string";
  _style = "default";
  _label = STRING_COLUMN_LABEL_DESC;
}

////////////////////////////////////////////////////////////////////////////////
ColumnDescription::~ColumnDescription ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnDescription::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDescription::measure (Task& task, int& minimum, int& maximum)
{
  std::string description = task.get (_name);

  // The text
  // <indent> <date> <anno>
  // ...
  if (_style == "default")
  {
    int indent = context.config.getInteger ("indent.annotation");
    std::string format = context.config.get ("dateformat.annotation");
    if (format == "")
      format = context.config.get ("dateformat");

    int min_desc = longestWord (description);
    int min_anno = indent + Date::length (format);
    minimum = max (min_desc, min_anno);
    maximum = description.length ();

    std::vector <Att> annos;
    task.getAnnotations (annos);
    std::vector <Att>::iterator i;
    for (i = annos.begin (); i != annos.end (); i++)
    {
      int len = min_anno + 1 + i->value ().length ();
      if (len > maximum)
        maximum = len;
    }
  }

  // Just the text
  else if (_style == "desc")
  {
    maximum = description.length ();
    minimum = longestWord (description);
  }

  // The text <date> <anno> ...
  else if (_style == "oneline")
  {
    std::string format = context.config.get ("dateformat.annotation");
    if (format == "")
      format = context.config.get ("dateformat");

    int min_desc = longestWord (description);
    int min_anno = Date::length (format);
    minimum = max (min_desc, min_anno);
    maximum = description.length ();

    std::vector <Att> annos;
    task.getAnnotations (annos);
    std::vector <Att>::iterator i;
    for (i = annos.begin (); i != annos.end (); i++)
      maximum += i->value ().length () + minimum + 1;
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
    std::vector <Att> annos;
    task.getAnnotations (annos);

    // <description> + ' ' + '[' + <count> + ']'
    maximum = description.length () + 3 + format ((int)annos.size ()).length ();
    minimum = longestWord (description);
  }

  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDescription::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string description = task.get (_name);

  // This is a description
  // <date> <anno>
  // ...
  if (_style == "default")
  {
    int indent = context.config.getInteger ("indent.annotation");

    std::vector <Att> annos;
    task.getAnnotations (annos);
    if (annos.size ())
    {
      std::string format = context.config.get ("dateformat.annotation");
      if (format == "")
        format = context.config.get ("dateformat");

      std::vector <Att>::iterator i;
      for (i = annos.begin (); i != annos.end (); i++)
      {
        Date dt (atoi (i->name ().substr (11).c_str ()));
        description += "\n" + std::string (indent, ' ') + dt.toString (format) + " " + i->value ();
      }
    }

    std::vector <std::string> raw;
    wrapText (raw, description, width);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (leftJustify (*i, width)));
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
    std::vector <Att> annos;
    task.getAnnotations (annos);
    if (annos.size ())
    {
      std::string format = context.config.get ("dateformat.annotation");
      if (format == "")
        format = context.config.get ("dateformat");

      std::vector <Att>::iterator i;
      for (i = annos.begin (); i != annos.end (); i++)
      {
        Date dt (atoi (i->name ().substr (11).c_str ()));
        description += " " + dt.toString (format) + " " + i->value ();
      }
    }

    std::vector <std::string> raw;
    wrapText (raw, description, width);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (leftJustify (*i, width)));
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
    std::vector <Att> annos;
    task.getAnnotations (annos);

    if (annos.size ())
      description += " [" + format ((int) annos.size ()) + "]";

    std::vector <std::string> raw;
    wrapText (raw, description, width);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (leftJustify (*i, width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
