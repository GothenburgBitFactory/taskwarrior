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

#include <algorithm>
#include <Context.h>
#include <ColDepends.h>
#include <text.h>
#include <i18n.h>
#include <main.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDepends::ColumnDepends ()
{
  _name  = "depends";
  _type  = "string";
  _style = "list";
  _label = STRING_COLUMN_LABEL_DEP;

  _styles.push_back ("list");
  _styles.push_back ("count");
  _styles.push_back ("indicator");

  _examples.push_back ("1 2 10");
  _examples.push_back ("[3]");
  _examples.push_back (context.config.get ("dependency.indicator"));
}

////////////////////////////////////////////////////////////////////////////////
ColumnDepends::~ColumnDepends ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnDepends::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnDepends::setStyle (const std::string& value)
{
  _style = value;

       if (_style == "indicator" && _label == STRING_COLUMN_LABEL_DEP) _label = _label.substr (0, context.config.get ("dependency.indicator").length ());
  else if (_style == "count"     && _label == STRING_COLUMN_LABEL_DEP) _label = STRING_COLUMN_LABEL_DEP_S;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDepends::measure (Task& task, int& minimum, int& maximum)
{
  std::vector <Task> blocking;
  dependencyGetBlocking (task, blocking);

       if (_style == "indicator") minimum = maximum = context.config.get ("dependency.indicator").length ();
  else if (_style == "count")     minimum = maximum = 2 + format ((int) blocking.size ()).length ();
  else if (_style == "default" ||
           _style == "list")
  {
    minimum = maximum = 0;
    if (task.has ("depends"))
    {
      std::vector <int> blocking_ids;
      std::vector <Task>::iterator i;
      for (i = blocking.begin (); i != blocking.end (); ++i)
        blocking_ids.push_back (i->id);

      std::string all;
      join (all, " ", blocking_ids);
      maximum = all.length ();

      int length;
      for (i = blocking.begin (); i != blocking.end (); ++i)
      {
        length = format (i->id).length ();
        if (length > minimum)
          minimum = length;
      }
    }
  }
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDepends::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "indicator")
    {
      lines.push_back (
        color.colorize (
          rightJustify (context.config.get ("dependency.indicator"), width)));
      return;
    }

    std::vector <Task> blocking;
    dependencyGetBlocking (task, blocking);

    if (_style == "count")
    {
      lines.push_back (
        color.colorize (
          rightJustify ("[" + format ((int)blocking.size ()) + "]", width)));
    }
    else if (_style == "default" ||
             _style == "list")
    {
      std::vector <int> blocking_ids;
      std::vector <Task>::iterator t;
      for (t = blocking.begin (); t != blocking.end (); ++t)
        blocking_ids.push_back (t->id);

      std::string combined;
      join (combined, " ", blocking_ids);

      std::vector <std::string> all;
      wrapText (all, combined, width);

      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        lines.push_back (color.colorize (leftJustify (*i, width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
