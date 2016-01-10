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
#include <ColStatus.h>
#include <text.h>
#include <i18n.h>
#include <utf8.h>

////////////////////////////////////////////////////////////////////////////////
ColumnStatus::ColumnStatus ()
{
  _name     = "status";
  _style    = "long";
  _label    = STRING_COLUMN_LABEL_STATUS;
  _styles   = {"long", "short"};
  _examples = {STRING_COLUMN_LABEL_STAT_PE,
               STRING_COLUMN_LABEL_STAT_P};
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnStatus::setStyle (const std::string& value)
{
  _style = value;

  if (_style == "short" && _label == STRING_COLUMN_LABEL_STATUS)
    _label = STRING_COLUMN_LABEL_STAT;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnStatus::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  Task::status status = task.getStatus ();

  if (_style == "default" ||
      _style == "long")
  {
    if (status == Task::pending)
      minimum = maximum = utf8_width (STRING_COLUMN_LABEL_STAT_PE);
    else if (status == Task::deleted)
      minimum = maximum = utf8_width (STRING_COLUMN_LABEL_STAT_DE);
    else if (status == Task::waiting)
      minimum = maximum = utf8_width (STRING_COLUMN_LABEL_STAT_WA);
    else if (status == Task::completed)
      minimum = maximum = utf8_width (STRING_COLUMN_LABEL_STAT_CO);
    else if (status == Task::recurring)
      minimum = maximum = utf8_width (STRING_COLUMN_LABEL_STAT_RE);
  }
  else if (_style == "short")
    minimum = maximum = 1;
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnStatus::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  Task::status status = task.getStatus ();
  std::string value;

  if (_style == "default" ||
      _style == "long")
  {
         if (status == Task::pending)   value = STRING_COLUMN_LABEL_STAT_PE;
    else if (status == Task::completed) value = STRING_COLUMN_LABEL_STAT_CO;
    else if (status == Task::deleted)   value = STRING_COLUMN_LABEL_STAT_DE;
    else if (status == Task::waiting)   value = STRING_COLUMN_LABEL_STAT_WA;
    else if (status == Task::recurring) value = STRING_COLUMN_LABEL_STAT_RE;
  }

  else if (_style == "short")
  {
         if (status == Task::pending)   value = STRING_COLUMN_LABEL_STAT_P;
    else if (status == Task::completed) value = STRING_COLUMN_LABEL_STAT_C;
    else if (status == Task::deleted)   value = STRING_COLUMN_LABEL_STAT_D;
    else if (status == Task::waiting)   value = STRING_COLUMN_LABEL_STAT_W;
    else if (status == Task::recurring) value = STRING_COLUMN_LABEL_STAT_R;
  }

  renderStringLeft (lines, width, color, value);
}

////////////////////////////////////////////////////////////////////////////////
