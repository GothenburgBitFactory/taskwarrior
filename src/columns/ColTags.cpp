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
#include <ColTags.h>
#include <text.h>
#include <i18n.h>
#include <utf8.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnTags::ColumnTags ()
{
  _name      = "tags";
  _type      = "string";
  _style     = "list";
  _label     = STRING_COLUMN_LABEL_TAGS;
  _styles    = {"list", "indicator", "count"};
  _examples  = {STRING_COLUMN_EXAMPLES_TAGS,
                context.config.get ("tag.indicator"),
                "[2]"};
  _hyphenate = context.config.getBoolean ("hyphenate");
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

  if (_style == "indicator" &&
      _label == STRING_COLUMN_LABEL_TAGS)
    _label = _label.substr (0, context.config.get ("tag.indicator").length ());

  else if (_style == "count" &&
            _label == STRING_COLUMN_LABEL_TAGS)
    _label = STRING_COLUMN_LABEL_TAG;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnTags::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    if (_style == "indicator")
    {
      if (task.has ("tags"))
        minimum = maximum = utf8_width (context.config.get ("tag.indicator"));
      else
        minimum = maximum = 0;
    }
    else if (_style == "count")
    {
      minimum = maximum = 3;
    }
    else if (_style == "default" ||
             _style == "list")
    {
      std::string tags = task.get (_name);
      maximum = utf8_width (tags);

      if (maximum)
      {
        std::vector <std::string> all;
        split (all, tags, ',');
        for (auto& i : all)
        {
          unsigned int length = utf8_width (i);
          if (length > minimum)
            minimum = length;
        }
      }
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTags::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    std::string tags = task.get (_name);
    if (_style == "default" ||
        _style == "list")
    {
      std::vector <std::string> allTags;
      split (allTags, tags, ',');
      if (allTags.size () > 1)
      {
        std::sort (allTags.begin (), allTags.end ());
        join (tags, " ", allTags);
      }

      std::vector <std::string> all;
      wrapText (all, tags, width, _hyphenate);

      for (auto& i : all)
        lines.push_back (color.colorize (leftJustify (i, width)));
    }
    else if (_style == "indicator")
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
  }
}

////////////////////////////////////////////////////////////////////////////////
