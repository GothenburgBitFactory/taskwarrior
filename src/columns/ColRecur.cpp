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
#include <ColRecur.h>
#include <Context.h>
#include <ISO8601.h>
#include <Eval.h>
#include <Variant.h>
#include <Lexer.h>
#include <Filter.h>
#include <Dates.h>
#include <text.h>
#include <utf8.h>
#include <i18n.h>

extern Context context;
extern Task& contextTask;

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::ColumnRecur ()
{
  _name     = "recur";
  _style    = "duration";
  _label    = STRING_COLUMN_LABEL_RECUR;
  _styles   = {"duration", "indicator"};
  _examples = {"weekly", context.config.get ("recurrence.indicator")};
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnRecur::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "indicator" && _label == STRING_COLUMN_LABEL_RECUR)
    _label = _label.substr (0, context.config.get ("recurrence.indicator").length ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnRecur::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (task.has (_name))
  {
    if (_style == "default" ||
        _style == "duration")
    {
      minimum = maximum = ISO8601p (task.get ("recur")).format ().length ();
    }
    else if (_style == "indicator")
    {
      if (task.has ("recur"))
        minimum = maximum = utf8_width (context.config.get ("recurrence.indicator"));
      else
        minimum = maximum = 0;
    }
    else
      throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnRecur::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "default" ||
        _style == "duration")
      renderStringRight (lines, width, color, ISO8601p (task.get ("recur")).format ());

    else if (_style == "indicator")
      renderStringRight (lines, width, color, context.config.get ("recurrence.indicator"));
  }
}

////////////////////////////////////////////////////////////////////////////////
// The duration is stored in raw form, but it must still be valid,
// and therefore is parsed first.
void ColumnRecur::modify (Task& task, const std::string& value)
{
  // Try to evaluate 'value'.  It might work.
  Variant evaluatedValue;
  try
  {
    Eval e;
    e.addSource (domSource);
    e.addSource (namedDates);
    contextTask = task;
    e.evaluateInfixExpression (value, evaluatedValue);
  }

  catch (...)
  {
    evaluatedValue = Variant (value);
  }

  if (evaluatedValue.type () == Variant::type_duration)
  {
    // Store the raw value, for 'recur'.
    std::string label = "  [1;37;43mMODIFICATION[0m ";
    context.debug (label + _name + " <-- '" + value + "'");
    task.set (_name, value);
  }
  else
    throw format (STRING_TASK_INVALID_DUR, value);
}

////////////////////////////////////////////////////////////////////////////////
