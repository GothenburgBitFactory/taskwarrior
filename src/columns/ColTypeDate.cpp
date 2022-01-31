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
#include <ColTypeDate.h>
#include <Context.h>
#include <Datetime.h>
#include <Duration.h>
#include <Eval.h>
#include <Variant.h>
#include <Filter.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
ColumnTypeDate::ColumnTypeDate ()
{
  _name      = "";
  _type      = "date";
  _style     = "formatted";
  _label     = "";
  _styles    = {"formatted",
                "julian",
                "epoch", 
                "iso",
                "age",
                "relative",
                "remaining",
                "countdown"};

  Datetime now;
  now -= 125; // So that "age" is non-zero.
  _examples = {now.toString (Context::getContext ().config.get ("dateformat")),
               format (now.toJulian (), 13, 12),
               now.toEpochString (),
               now.toISO (),
               Duration (Datetime () - now).formatVague (true),
               '-' + Duration (Datetime () - now).formatVague (true),
               "",
               Duration (Datetime () - now).format ()};
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnTypeDate::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;
  if (task.has (_name))
  {
    Datetime date (task.get_date (_name));

    if (_style == "default" ||
        _style == "formatted")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat.
      std::string format = Context::getContext ().config.get ("report." + _report + ".dateformat");
      if (format == "")
        format = Context::getContext ().config.get ("dateformat.report");
      if (format == "")
        format = Context::getContext ().config.get ("dateformat");

      minimum = maximum = Datetime::length (format);
    }
    else if (_style == "countdown")
    {
      Datetime now;
      minimum = maximum = Duration (now - date).formatVague (true).length ();
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
      Datetime now;
      if (now > date)
        minimum = maximum = Duration (now - date).formatVague (true).length ();
      else
        minimum = maximum = Duration (date - now).formatVague (true).length () + 1;
    }
    else if (_style == "relative")
    {
      Datetime now;
      if (now < date)
        minimum = maximum = Duration (date - now).formatVague (true).length ();
      else
        minimum = maximum = Duration (now - date).formatVague (true).length () + 1;
    }
    else if (_style == "remaining")
    {
      Datetime now;
      if (date > now)
        minimum = maximum = Duration (date - now).formatVague (true).length ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTypeDate::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    Datetime date (task.get_date (_name));

    if (_style == "default" ||
        _style == "formatted")
    {
      // Determine the output date format, which uses a hierarchy of definitions.
      //   rc.report.<report>.dateformat
      //   rc.dateformat.report
      //   rc.dateformat
      std::string format = Context::getContext ().config.get ("report." + _report + ".dateformat");
      if (format == "")
      {
        format = Context::getContext ().config.get ("dateformat.report");
        if (format == "")
          format = Context::getContext ().config.get ("dateformat");
      }

      renderStringLeft (lines, width, color, date.toString (format));
    }
    else if (_style == "countdown")
    {
      Datetime now;
      renderStringRight (lines, width, color, Duration (date - now).formatVague (true));
    }
    else if (_style == "julian")
      renderStringRight (lines, width, color, format (date.toJulian (), 13, 12));

    else if (_style == "epoch")
      renderStringRight (lines, width, color, date.toEpochString ());

    else if (_style == "iso")
      renderStringLeft (lines, width, color, date.toISO ());

    else if (_style == "age")
    {
      Datetime now;
      if (now > date)
        renderStringRight (lines, width, color, Duration (now - date).formatVague (true));
      else
        renderStringRight (lines, width, color, '-' + Duration (date - now).formatVague (true));
    }
    else if (_style == "relative")
    {
      Datetime now;
      if (now < date)
        renderStringRight (lines, width, color, Duration (date - now).formatVague (true));
      else
        renderStringRight (lines, width, color, '-' + Duration (now - date).formatVague (true));
    }

    else if (_style == "remaining")
    {
      Datetime now;
      if (date > now)
        renderStringRight (lines, width, color, Duration (date - now).formatVague (true));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnTypeDate::validate (const std::string& input) const
{
  return input.length () ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
void ColumnTypeDate::modify (Task& task, const std::string& value)
{
  // Try to evaluate 'value'.  It might work.
  Variant evaluatedValue;
  try
  {
    Eval e;
    e.addSource (domSource);
    e.evaluateInfixExpression (value, evaluatedValue);
  }

  catch (...)
  {
    evaluatedValue = Variant (value);
  }

  // If v is duration, we need to convert it to date (and implicitly add now),
  // else store as date.
  std::string label = "  [1;37;43mMODIFICATION[0m ";
  if (evaluatedValue.type () == Variant::type_duration)
  {
    Context::getContext ().debug (label + _name + " <-- '" + format ("{1}", format (evaluatedValue.get_duration ())) + "' <-- '" + (std::string) evaluatedValue + "' <-- '" + value + '\'');
    evaluatedValue.cast (Variant::type_date);
  }
  else
  {
    evaluatedValue.cast (Variant::type_date);
    Context::getContext ().debug (label + _name + " <-- '" + format ("{1}", evaluatedValue.get_date ()) + "' <-- '" + (std::string) evaluatedValue + "' <-- '" + value + '\'');
  }

  // If a date doesn't parse (2/29/2014) then it evaluates to zero.
  if (value != "" &&
      evaluatedValue.get_date () == 0)
    throw format ("'{1}' is not a valid date in the '{2}' format.", value, Variant::dateFormat);

  task.set (_name, evaluatedValue.get_date ());
}

////////////////////////////////////////////////////////////////////////////////
