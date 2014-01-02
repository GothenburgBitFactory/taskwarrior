////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2013, Paul Beckingham, Federico Hernandez.
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

#include <Context.h>
#include <OldDuration.h>
#include <ColRecur.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::ColumnRecur ()
{
  _name  = "recur";

  // This is 'string', and not 'duration' to force the value to be stored as a
  // raw duration, so that it can be reevaluated every time.
  _type  = "string";

  _style = "duration";
  _label = STRING_COLUMN_LABEL_RECUR;

  _styles.push_back ("duration");
  _styles.push_back ("indicator");

  _examples.push_back ("weekly");
  _examples.push_back (context.config.get ("recurrence.indicator"));
}

////////////////////////////////////////////////////////////////////////////////
ColumnRecur::~ColumnRecur ()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnRecur::validate (std::string& value)
{
  return true;
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
  if (_style == "default" ||
      _style == "duration")
  {
    minimum = maximum = Duration (task.get ("recur")).formatCompact ().length ();
  }
  else if (_style == "indicator")
  {
    if (task.has (_name))
      minimum = maximum = context.config.get ("recurrence.indicator").length ();
  }
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnRecur::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (_style == "default" ||
      _style == "duration")
  {
    lines.push_back (
      color.colorize (
        rightJustify (
          Duration (task.get ("recur")).formatCompact (),
          width)));
  }
  else if (_style == "indicator")
  {
    if (task.has (_name))
      lines.push_back (
        color.colorize (
          rightJustify (context.config.get ("recurrence.indicator"), width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
