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
  _name      = "";
  _type      = "date";
  _style     = "formatted";
  _label     = "";

  _styles.push_back ("formatted");
  _styles.push_back ("julian");
  _styles.push_back ("epoch");
  _styles.push_back ("iso");
  _styles.push_back ("age");
  _styles.push_back ("remaining");
  _styles.push_back ("countdown");

  Date now;
  now -= 125; // So that "age" is non-zero.
  _examples.push_back (now.toString (context.config.get ("dateformat")));
  _examples.push_back (format (now.toJulian (), 13, 12));
  _examples.push_back (now.toEpochString ());
  _examples.push_back (now.toISO ());
  _examples.push_back (Duration (Date () - now).formatCompact ());
  _examples.push_back ("");
  _examples.push_back (Duration (Date () - now).format ());
}

////////////////////////////////////////////////////////////////////////////////
ColumnDate::~ColumnDate ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnDate::validate (std::string& value)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnDate::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    Date date (task.get_date (_name));

    if (_style == "default" ||
        _style == "formatted")
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

      minimum = maximum = Date::length (format);
    }
    else if (_style == "countdown")
    {
      Date now;
      minimum = maximum = Duration (now - date).format ().length ();
    }
    else if (_style == "julian")
    {
      minimum = maximum = format (date.toJulian (), 13, 12).length ();
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
    else if (_style == "remaining")
    {
      Date now;
      if (date > now)
        minimum = maximum = Duration (date - now).format ().length ();
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnDate::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    Date date (task.get_date (_name));

    if (_style == "default" ||
        _style == "formatted")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat
      std::string format = context.config.get ("report." + _report + ".dateformat");
      if (format == "")
        format = context.config.get ("dateformat.report");
      if (format == "")
        format = context.config.get ("dateformat");

      lines.push_back (
        color.colorize (
          leftJustify (
            date.toString (format), width)));
    }
    else if (_style == "countdown")
    {
      Date now;

      lines.push_back (
        color.colorize (
          rightJustify (
            Duration (now - date).format (), width)));
    }
    else if (_style == "julian")
    {
      lines.push_back (
        color.colorize (
          rightJustify (
            format (date.toJulian (), 13, 12), width)));
    }
    else if (_style == "epoch")
    {
      lines.push_back (
        color.colorize (
          rightJustify (
            date.toEpochString (), width)));
    }
    else if (_style == "iso")
    {
      lines.push_back (
        color.colorize (
          leftJustify (
            date.toISO (), width)));
    }
    else if (_style == "age")
    {
      Date now;

      lines.push_back (
        color.colorize (
          leftJustify (
            Duration (now - date).formatCompact (), width)));
    }
    else if (_style == "remaining")
    {
      Date now;
      if (date > now)
        lines.push_back (
          color.colorize (
            rightJustify (
              Duration (date - now).format (), width)));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
