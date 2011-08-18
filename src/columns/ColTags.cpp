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
#include <ColTags.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnTags::ColumnTags ()
{
  _name  = "tags";
  _type  = "string";
  _style = "list";
  _label = STRING_COLUMN_LABEL_TAGS;

  _styles.push_back ("list");
  _styles.push_back ("indicator");
  _styles.push_back ("count");

  _examples.push_back (STRING_COLUMN_EXAMPLES_TAGS);
  _examples.push_back (context.config.get ("tag.indicator"));
  _examples.push_back ("[2]");

  _hyphenate = context.config.getBoolean ("hyphenate");
}

////////////////////////////////////////////////////////////////////////////////
ColumnTags::~ColumnTags ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnTags::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnTags::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "indicator" &&
      _label == STRING_COLUMN_LABEL_TAGS)
    _label = _label.substr (0, context.config.get ("tag.indicator").length ());

  else if (_style == "count" &&
            _label == STRING_COLUMN_LABEL_TAGS)
    _label = STRING_COLUMN_LABEL_TAG;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnTags::measure (Task& task, int& minimum, int& maximum)
{

       if (_style == "indicator") minimum = maximum = context.config.get ("tag.indicator").length ();
  else if (_style == "count")     minimum = maximum = 3;
  else if (_style == "default" ||
           _style == "list")
  {
    std::string tags = task.get (_name);
    minimum = 0;
    maximum = tags.length ();

    if (maximum)
    {
      std::vector <std::string> all;
      split (all, tags, ',');
      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        if ((int)i->length () > minimum)
          minimum = i->length () + 1;
    }
  }
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTags::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  std::string tags = task.get (_name);
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
    else if (_style == "default" ||
             _style == "list")
    {
      std::replace (tags.begin (), tags.end (), ',', ' ');
      std::vector <std::string> all;
      wrapText (all, tags, width, _hyphenate);

      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        lines.push_back (color.colorize (leftJustify (*i, width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
