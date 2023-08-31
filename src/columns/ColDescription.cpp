////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <ColDescription.h>
#include <stdlib.h>
#include <Context.h>
#include <Datetime.h>
#include <shared.h>
#include <format.h>
#include <utf8.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
ColumnDescription::ColumnDescription ()
{
  _name       = "description";
  _style      = "combined";
  _label      = "Description";
  _modifiable = true;

  _styles     = {"combined",
                 "desc",
                 "oneline",
                 "truncated",
                 "count",
                 "truncated_count"};

  _dateformat = Context::getContext ().config.get ("dateformat.annotation");
  if (_dateformat == "")
    _dateformat = Context::getContext ().config.get ("dateformat");

  std::string t  = Datetime ().toString (_dateformat);
  std::string d  = "Move your clothes down on to the lower peg";
  std::string a1 = "Immediately before your lunch";
  std::string a2 = "If you are playing in the match this afternoon";
  std::string a3 = "Before you write your letter home";
  std::string a4 = "If you're not getting your hair cut";

  _examples = {d + "\n  " + t + ' ' + a1
                 + "\n  " + t + ' ' + a2
                 + "\n  " + t + ' ' + a3
                 + "\n  " + t + ' ' + a4,
               d,
               d + ' ' + t + ' ' + a1
                 + ' ' + t + ' ' + a2
                 + ' ' + t + ' ' + a3
                 + ' ' + t + ' ' + a4,
               d.substr (0, 20) + "...",
               d + " [4]",
               d.substr (0, 20) + "... [4]"};

  _hyphenate = Context::getContext ().config.getBoolean ("hyphenate");

  _indent = Context::getContext ().config.getInteger ("indent.annotation");
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
      unsigned int min_anno = _indent + Datetime::length (_dateformat);
      if (min_anno > minimum)
        minimum = min_anno;

      for (auto& i : task.getAnnotations ())
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
      auto min_anno = Datetime::length (_dateformat);
      for (auto& i : task.getAnnotations ())
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
      for (const auto& i : task.getAnnotations ())
      {
        Datetime dt (strtoll (i.first.substr (11).c_str (), nullptr, 10));
        description += '\n' + std::string (_indent, ' ') + dt.toString (_dateformat) + ' ' + i.second;
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
      for (const auto& i : task.getAnnotations ())
      {
        Datetime dt (strtoll (i.first.substr (11).c_str (), nullptr, 10));
        description += ' ' + dt.toString (_dateformat) + ' ' + i.second;
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
      description += " [" + format (task.annotation_count) + ']';

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
      annos_count = " [" + format (task.annotation_count) + ']';
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
