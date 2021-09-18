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
#include <ColUUID.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
ColumnUUID::ColumnUUID ()
{
  _name       = "uuid";
  _style      = "long";
  _label      = "UUID";
  _modifiable = false;
  _styles     = {"long", "short"};
  _examples   = {"f30cb9c3-3fc0-483f-bfb2-3bf134f00694", "f30cb9c3"};
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnUUID::measure (Task&, unsigned int& minimum, unsigned int& maximum)
{
  // Mandatory attribute, no need to check the value.

       if (_style == "default" || _style == "long") minimum = maximum = 36;
  else if (_style == "short")                       minimum = maximum = 8;
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUUID::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  // No need to check the presence of UUID - all tasks have one.

  // f30cb9c3-3fc0-483f-bfb2-3bf134f00694  default
  // f30cb9c3                              short
  if (_style == "default" ||
      _style == "long")
    renderStringLeft (lines, width, color, task.get (_name));

  else if (_style == "short")
    renderStringLeft (lines, width, color, task.get (_name).substr (0, 8));
}

////////////////////////////////////////////////////////////////////////////////
