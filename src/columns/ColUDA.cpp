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
#include <Context.h>
#include <ISO8601.h>
#include <ColUDA.h>
#include <text.h>
#include <utf8.h>
#include <i18n.h>
#include <stdlib.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnUDA::ColumnUDA ()
{
  _name      = "<uda>";
  _type      = "string";
  _style     = "default";
  _label     = "";
  _uda       = true;
  _hyphenate = (_type == "string") ? true : false;
  _styles    = {_style, "indicator"};
}

////////////////////////////////////////////////////////////////////////////////
ColumnUDA::~ColumnUDA ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnUDA::validate (std::string& value)
{
  // No restrictions.
  if (_values.size () == 0)
    return true;

  // Look for exact match value.
  for (auto& i : _values)
    if (i == value)
      return true;

  // Fail if not found.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnUDA::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    if (_style == "default")
    {
      std::string value = task.get (_name);
      if (value != "")
      {
        if (_type == "date")
        {
          // Determine the output date format, which uses a hierarchy of definitions.
          //   rc.report.<report>.dateformat
          //   rc.dateformat.report
          //   rc.dateformat
          ISO8601d date ((time_t) strtol (value.c_str (), NULL, 10));
          std::string format = context.config.get ("report." + _report + ".dateformat");
          if (format == "")
            format = context.config.get ("dateformat.report");
          if (format == "")
            format = context.config.get ("dateformat");

          minimum = maximum = ISO8601d::length (format);
        }
        else if (_type == "duration")
        {
          minimum = maximum = utf8_width (ISO8601p (value).format ());
        }
        else if (_type == "string")
        {
          std::string stripped = Color::strip (value);
          maximum = longestLine (stripped);
          minimum = longestWord (stripped);
        }
        else if (_type == "numeric")
        {
          minimum = maximum = utf8_width (value);
        }
      }
    }
    else if (_style == "indicator")
    {
      if (task.has (_name))
        minimum = maximum = utf8_width (context.config.get ("uda." + _name + ".indicator"));
      else
        minimum = maximum = 0;
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUDA::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "default")
    {
      std::string value = task.get (_name);
      if (_type == "date")
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
              ISO8601d ((time_t) strtol (value.c_str (), NULL, 10)).toString (format), width)));
      }
      else if (_type == "duration")
      {
        lines.push_back (
          color.colorize (
            rightJustify (
              ISO8601p (value).format (),
              width)));
      }
      else if (_type == "string")
      {
        std::vector <std::string> raw;
        wrapText (raw, value, width, _hyphenate);

        for (auto& i : raw)
          lines.push_back (color.colorize (leftJustify (i, width)));
      }
      else if (_type == "numeric")
      {
        lines.push_back (color.colorize (rightJustify (value, width)));
      }
    }
    else if (_style == "indicator")
    {
      if (task.has (_name))
        lines.push_back (
          color.colorize (
            rightJustify (context.config.get ("uda." + _name + ".indicator"), width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
