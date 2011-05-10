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

#include <algorithm>
#include <Context.h>
#include <ColTags.h>
#include <text.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnTags::ColumnTags ()
{
  _type  = "string";
  _style = "default";
  _label = "Tags";
}

////////////////////////////////////////////////////////////////////////////////
ColumnTags::~ColumnTags ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnTags::setStyle (const std::string& value)
{
  _style = value;

       if (_style == "indicator" && _label == "Tags") _label = _label.substr (0, context.config.get ("tag.indicator").length ());
  else if (_style == "count"     && _label == "Tags") _label = "Tag";
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnTags::measure (Task& task, int& minimum, int& maximum)
{

       if (_style == "indicator") minimum = maximum = context.config.get ("tag.indicator").length ();
  else if (_style == "count")     minimum = maximum = 3;
  else if (_style == "default")
  {
    std::string tags = task.get ("tags");
    minimum = 0;
    maximum = tags.length ();

    if (maximum)
    {
      std::vector <std::string> all;
      split (all, tags, ',');
      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        if (i->length () > minimum)
          minimum = i->length () + 1;
    }
  }
  else
    throw std::string ("Unrecognized column format 'tags.") + _style + "'";

}

////////////////////////////////////////////////////////////////////////////////
void ColumnTags::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string tags = task.get ("tags");
  if (tags != "")
  {
    if (_style == "indicator")
    {
      lines.push_back (
        color.colorize (
          rightJustify (context.config.get ("tag.indicator"), width)));
    }
    else if (_style == "count")
    {
      std::vector <std::string> all;
      split (all, tags, ',');
      lines.push_back (
        color.colorize (
          rightJustify ("[" + format ((int)all.size ()) + "]", width)));
    }
    else if (_style == "default")
    {
      std::replace (tags.begin (), tags.end (), ',', ' ');
      std::vector <std::string> all;
      wrapText (all, tags, width);

      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        lines.push_back (color.colorize (leftJustify (*i, width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
