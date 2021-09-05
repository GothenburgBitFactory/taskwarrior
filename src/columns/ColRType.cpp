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
#include <ColRType.h>
#include <Context.h>
#include <shared.h>
#include <format.h>
#include <cctype>

////////////////////////////////////////////////////////////////////////////////
ColumnRType::ColumnRType ()
{
  _name       = "rtype";
  _style      = "default";
  _label      = "Recurrence type";
  _modifiable = false;
  _styles     = {"default", "indicator"};
  _examples   = {"periodic", "chained"};
}

////////////////////////////////////////////////////////////////////////////////
// Overriden so that style <----> label are linked.
// Note that you can not determine which gets called first.
void ColumnRType::setStyle (const std::string& value)
{
  Column::setStyle (value);

  if (_style == "indicator" && _label == "Recurrence type")
    _label = _label.substr (0, Context::getContext ().config.get ("rtype.indicator").length ());
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnRType::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;
  if (task.has (_name))
  {
    if (_style == "default")
      minimum = maximum = task.get (_name).length ();
    else if (_style == "indicator")
      minimum = maximum = 1;
  }
}

////////////////////////////////////////////////////////////////////////////////
void ColumnRType::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.has (_name))
  {
    if (_style == "default")
      renderStringRight (lines, width, color, task.get (_name));

    else if (_style == "indicator")
    {
      std::string value {" "};
      value[0] = toupper (task.get (_name)[0]);
      renderStringRight (lines, width, color, value);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool ColumnRType::validate (const std::string& input) const
{
  return input == "periodic" ||
         input == "chained";
}

////////////////////////////////////////////////////////////////////////////////
