////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <algorithm>
#include <Context.h>
#include <ColDepends.h>
#include <text.h>
#include <utf8.h>
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

  _hyphenate = context.config.getBoolean ("hyphenate");
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
void ColumnDepends::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  std::vector <Task> blocking;
  dependencyGetBlocking (task, blocking);

  if (_style == "indicator")
  {
    minimum = maximum = utf8_width (context.config.get ("dependency.indicator"));
    _fixed_width = true;
  }
  else if (_style == "count")
  {
    minimum = maximum = 2 + format ((int) blocking.size ()).length ();
  }
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

      unsigned int length;
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
      wrapText (all, combined, width, _hyphenate);

      std::vector <std::string>::iterator i;
      for (i = all.begin (); i != all.end (); ++i)
        lines.push_back (color.colorize (leftJustify (*i, width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
