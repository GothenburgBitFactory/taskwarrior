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
#include <ColID.h>
#include <math.h>
#include <text.h>
#include <i18n.h>

////////////////////////////////////////////////////////////////////////////////
ColumnID::ColumnID ()
{
  _name       = "id";
  _style      = "number";
  _label      = STRING_COLUMN_LABEL_ID;
  _modifiable = false;
  _styles     = {"number"};
  _examples   = {"123"};
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnID::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  int length;

       if (task.id < 10)     length = 1;                                  // Fast
  else if (task.id < 100)    length = 2;                                  // Fast
  else if (task.id < 1000)   length = 3;                                  // Fast
  else if (task.id < 10000)  length = 4;                                  // Fast
  else if (task.id < 100000) length = 5;                                  // Fast
  else                       length = 1 + (int) log10 ((double) task.id); // Slow

  minimum = maximum = length;

  if (_style != "default" &&
      _style != "number")
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnID::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (task.id)
    renderInteger (lines, width, color, task.id);
  else
    renderStringRight (lines, width, color, "-");
}

////////////////////////////////////////////////////////////////////////////////
