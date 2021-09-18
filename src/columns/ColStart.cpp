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
#include <ColStart.h>
#include <Context.h>
#include <utf8.h>

////////////////////////////////////////////////////////////////////////////////
ColumnStart::ColumnStart ()
{
  _name  = "start";
  _label = "Started";

  _styles.push_back ("active");
  _examples.push_back (Context::getContext ().config.get ("active.indicator"));
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnStart::setStyle (const std::string& value)
{
  Column::setStyle (value);

  if (_style == "active" && _label == "Started")
    _label = "A";
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnStart::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;
  if (task.has (_name))
  {
    if (_style == "active")
      minimum = maximum = utf8_width (Context::getContext ().config.get ("active.indicator"));
    else
      ColumnTypeDate::measure (task, minimum, maximum);

    // TODO Throw on bad format.
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnStart::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "active")
    {
      if (! task.has ("end"))
        renderStringRight (lines, width, color, Context::getContext ().config.get ("active.indicator"));
    }
    else
      ColumnTypeDate::render (lines, task, width, color);
  }
}

////////////////////////////////////////////////////////////////////////////////
