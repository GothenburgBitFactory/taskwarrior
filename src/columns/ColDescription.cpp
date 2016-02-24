////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <ColDescription.h>
#include <stdlib.h>
#include <Context.h>
#include <ISO8601.h>
#include <text.h>
#include <utf8.h>
#include <util.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnDescription::ColumnDescription ()
{
  _name  = "description";
  _style = "combined";
  _label = STRING_COLUMN_LABEL_DESC;

  _styles = {"combined",
             "desc",
             "oneline",
             "truncated",
             "count",
             "truncated_count"};

  _dateformat = context.config.get ("dateformat.annotation");
  if (_dateformat == "")
    _dateformat = context.config.get ("dateformat");

  std::string t  = ISO8601d ().toString (_dateformat);
  std::string d  = STRING_COLUMN_EXAMPLES_DESC;
  std::string a1 = STRING_COLUMN_EXAMPLES_ANNO1;
  std::string a2 = STRING_COLUMN_EXAMPLES_ANNO2;
  std::string a3 = STRING_COLUMN_EXAMPLES_ANNO3;
  std::string a4 = STRING_COLUMN_EXAMPLES_ANNO4;

  _examples = {d + "\n  " + t + " " + a1
                 + "\n  " + t + " " + a2
                 + "\n  " + t + " " + a3
                 + "\n  " + t + " " + a4,
               d,
               d + " " + t + " " + a1
                 + " " + t + " " + a2
                 + " " + t + " " + a3
                 + " " + t + " " + a4,
               d.substr (0, 20) + "...",
               d + " [4]",
               d.substr (0, 20) + "... [4]"};

  _hyphenate = context.config.getBoolean ("hyphenate");

  _indent = context.config.getInteger ("indent.annotation");
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDescription::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  std::string description = task.get (_name);

  // The text
  // <indent> <date> <anno>
  // ...
  if (_style == "default" ||
      _style == "combined")
  {
    minimum = longestWord (description);
    maximum = utf8_width (description);

    if (task.annotation_count)
    {
      unsigned int min_anno = _indent + ISO8601d::length (_dateformat);
      if (min_anno > minimum)
        minimum = min_anno;

      std::map <std::string, std::string> annos;
      task.getAnnotations (annos);
      for (auto& i : annos)
      {
        unsigned int len = min_anno + 1 + utf8_width (i.second);
        if (len > maximum)
          maximum = len;
      }
    }
  }

  // Just the text
  else if (_style == "desc")
  {
    maximum = utf8_width (description);
    minimum = longestWord (description);
  }

  // The text <date> <anno> ...
  else if (_style == "oneline")
  {
    minimum = longestWord (description);
    maximum = utf8_width (description);

    if (task.annotation_count)
    {
      auto min_anno = ISO8601d::length (_dateformat);
      std::map <std::string, std::string> annos;
      task.getAnnotations (annos);
      for (auto& i : annos)
        maximum += min_anno + 1 + utf8_width (i.second);
    }
  }

  // The te...
  else if (_style == "truncated")
  {
    minimum = 4;
    maximum = utf8_width (description);
  }

  // The text [2]
  else if (_style == "count")
  {
    // <description> + ' ' + '[' + <count> + ']'
    maximum = utf8_width (description) + 1 + 1 + format (task.annotation_count).length () + 1;
    minimum = longestWord (description);
  }

  // The te... [2]
  else if (_style == "truncated_count")
  {
    minimum = 4;
    maximum = utf8_width (description) + 1 + 1 + format (task.annotation_count).length () + 1;
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
  if (_style == "default" ||
      _style == "combined")
  {
    if (task.annotation_count)
    {
      std::map <std::string, std::string> annos;
      task.getAnnotations (annos);
      for (const auto& i : annos)
      {
        ISO8601d dt (strtol (i.first.substr (11).c_str (), NULL, 10));
        description += "\n" + std::string (_indent, ' ') + dt.toString (_dateformat) + " " + i.second;
      }
    }

    std::vector <std::string> raw;
    wrapText (raw, description, width, _hyphenate);

    for (const auto& i : raw)
      renderStringLeft (lines, width, color, i);
  }

  // This is a description
  else if (_style == "desc")
  {
    std::vector <std::string> raw;
    wrapText (raw, description, width, _hyphenate);

    for (const auto& i : raw)
      renderStringLeft (lines, width, color, i);
  }

  // This is a description <date> <anno> ...
  else if (_style == "oneline")
  {
    if (task.annotation_count)
    {
      std::map <std::string, std::string> annos;
      task.getAnnotations (annos);
      for (const auto& i : annos)
      {
        ISO8601d dt (strtol (i.first.substr (11).c_str (), NULL, 10));
        description += " " + dt.toString (_dateformat) + " " + i.second;
      }
    }

    std::vector <std::string> raw;
    wrapText (raw, description, width, _hyphenate);

    for (const auto& i : raw)
      renderStringLeft (lines, width, color, i);
  }

  // This is a des...
  else if (_style == "truncated")
  {
    int len = utf8_width (description);
    if (len > width)
      renderStringLeft (lines, width, color, description.substr (0, width - 3) + "...");
    else
      renderStringLeft (lines, width, color, description);
  }

  // This is a description [2]
  else if (_style == "count")
  {
    if (task.annotation_count)
      description += " [" + format (task.annotation_count) + "]";

    std::vector <std::string> raw;
    wrapText (raw, description, width, _hyphenate);

    for (const auto& i : raw)
      renderStringLeft (lines, width, color, i);
  }

  // This is a des... [2]
  else if (_style == "truncated_count")
  {
    int len = utf8_width (description);

    std::string annos_count;
    int len_annos = 0;
    if (task.annotation_count)
    {
      annos_count = " [" + format (task.annotation_count) + "]";
      len_annos = utf8_width (annos_count);
      len += len_annos;
    }

    if (len > width)
      renderStringLeft (lines, width, color, description.substr (0, width - len_annos - 3) + "..." + annos_count);
    else
      renderStringLeft (lines, width, color, description + annos_count);
  }
}

////////////////////////////////////////////////////////////////////////////////
